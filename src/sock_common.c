/*
 * File:    sock_common.c
 * Author:  Liu HongLiang<lhl_nciae@sina.cn>
 * Brief:   Abstraction of BSD socket.
 *
 * Copyright (c) Liu HongLiang
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

/*
 * History:
 * ================================================================
 * 2019-01-29 Liu HongLiang<lhl_nciae@sina.cn> created.
 *
 */
#include <em/sock.h>
#include <em/assert.h>
#include <em/ctype.h>
#include <em/errno.h>
#include <em/ip_helper.h>
#include <em/os.h>
#include <em/addr_resolv.h>
#include <em/rand.h>
#include <em/string.h>
#include <em/compat/socket.h>

#if 0
/* Enable some tracing */
#include <em/log.h>
#define THIS_FILE   "sock_common.c"
#define TRACE_(arg)	EM_LOG(4,arg)
#else
#define TRACE_(arg)
#endif

const static char * module = "SCOK_COMMON";
/*
 * Convert address string with numbers and dots to binary IP address.
 */ 
EM_DEF(em_in_addr) em_inet_addr(const em_str_t *cp)
{
    em_in_addr addr;

    em_inet_aton(cp, &addr);
    return addr;
}

/*
 * Convert address string with numbers and dots to binary IP address.
 */ 
EM_DEF(em_in_addr) em_inet_addr2(const char *cp)
{
    em_str_t str = em_str((char*)cp);
    return em_inet_addr(&str);
}

/*
 * Get text representation.
 */
EM_DEF(char*) em_inet_ntop2( int af, const void *src,
        char *dst, int size)
{
    emlib_ret_t status;

    status = em_inet_ntop(af, src, dst, size);
    return (status==EM_SUCC)? dst : NULL;
}

/*
 * Print socket address.
 */
EM_DEF(char*) em_sockaddr_print( const em_sockaddr_t *addr,
        char *buf, int size,
        unsigned flags)
{
    enum {
        WITH_PORT = 1,
        WITH_BRACKETS = 2
    };

    char txt[EM_INET6_ADDRSTRLEN];
    char port[32];
    const em_addr_hdr *h = (const em_addr_hdr*)addr;
    char *bquote, *equote;
    emlib_ret_t status;

    status = em_inet_ntop(h->sa_family, em_sockaddr_get_addr(addr),
            txt, sizeof(txt));
    if (status != EM_SUCC)
        return "";

    if (h->sa_family != EM_AF_INET6 || (flags & WITH_BRACKETS)==0) {
        bquote = ""; equote = "";
    } else {
        bquote = "["; equote = "]";
    }

    if (flags & WITH_PORT) {
        em_ansi_snprintf(port, sizeof(port), ":%d",
                em_sockaddr_get_port(addr));
    } else {
        port[0] = '\0';
    }

    em_ansi_snprintf(buf, size, "%s%s%s%s",
            bquote, txt, equote, port);

    return buf;
}

/*
 * Set the IP address of an IP socket address from string address, 
 * with resolving the host if necessary. The string address may be in a
 * standard numbers and dots notation or may be a hostname. If hostname
 * is specified, then the function will resolve the host into the IP
 * address.
 */
EM_DEF(emlib_ret_t) em_sockaddr_in_set_str_addr( em_sockaddr_in *addr,
        const em_str_t *str_addr)
{
    EM_CHECK_STACK();

    EMLIB_ASSERT_RETURN(!str_addr || str_addr->slen < EM_MAX_HOSTNAME, 
            (addr->sin_addr.s_addr=EM_INADDR_NONE, EM_EINVAL));

    EM_SOCKADDR_RESET_LEN(addr);
    addr->sin_family = EM_AF_INET;
    em_bzero(addr->sin_zero, sizeof(addr->sin_zero));

    if (str_addr && str_addr->slen) {
        addr->sin_addr = em_inet_addr(str_addr);
        if (addr->sin_addr.s_addr == EM_INADDR_NONE) {
            em_addrinfo ai;
            unsigned count = 1;
            emlib_ret_t status;

            status = em_getaddrinfo(em_AF_INET(), str_addr, &count, &ai);
            if (status==EM_SUCC) {
                em_memcpy(&addr->sin_addr, &ai.ai_addr.ipv4.sin_addr,
                        sizeof(addr->sin_addr));
            } else {
                return status;
            }
        }

    } else {
        addr->sin_addr.s_addr = 0;
    }

    return EM_SUCC;
}

/* Set address from a name */
EM_DEF(emlib_ret_t) em_sockaddr_set_str_addr(int af,
        em_sockaddr *addr,
        const em_str_t *str_addr)
{
    emlib_ret_t status;

    if (af == EM_AF_INET) {
        return em_sockaddr_in_set_str_addr(&addr->ipv4, str_addr);
    }

    EMLIB_ASSERT_RETURN(af==EM_AF_INET6, EM_EAFNOTSUP);

    /* IPv6 specific */

    addr->ipv6.sin6_family = EM_AF_INET6;
    EM_SOCKADDR_RESET_LEN(addr);

    if (str_addr && str_addr->slen) {
        status = em_inet_pton(EM_AF_INET6, str_addr, &addr->ipv6.sin6_addr);
        if (status != EM_SUCC) {
            em_addrinfo ai;
            unsigned count = 1;

            status = em_getaddrinfo(EM_AF_INET6, str_addr, &count, &ai);
            if (status==EM_SUCC) {
                em_memcpy(&addr->ipv6.sin6_addr, &ai.ai_addr.ipv6.sin6_addr,
                        sizeof(addr->ipv6.sin6_addr));
                addr->ipv6.sin6_scope_id = ai.ai_addr.ipv6.sin6_scope_id;
            }
        }
    } else {
        status = EM_SUCC;
    }

    return status;
}

/*
 * Set the IP address and port of an IP socket address.
 * The string address may be in a standard numbers and dots notation or 
 * may be a hostname. If hostname is specified, then the function will 
 * resolve the host into the IP address.
 */
EM_DEF(emlib_ret_t) em_sockaddr_in_init( em_sockaddr_in *addr,
        const em_str_t *str_addr,
        em_uint16_t port)
{
    EMLIB_ASSERT_RETURN(addr, (addr->sin_addr.s_addr=EM_INADDR_NONE, EM_EINVAL));

    EM_SOCKADDR_RESET_LEN(addr);
    addr->sin_family = EM_AF_INET;
    em_bzero(addr->sin_zero, sizeof(addr->sin_zero));
    em_sockaddr_in_set_port(addr, port);
    return em_sockaddr_in_set_str_addr(addr, str_addr);
}

/*
 * Initialize IP socket address based on the address and port info.
 */
EM_DEF(emlib_ret_t) em_sockaddr_init(int af, 
        em_sockaddr *addr,
        const em_str_t *cp,
        em_uint16_t port)
{
    emlib_ret_t status;

    if (af == EM_AF_INET) {
        return em_sockaddr_in_init(&addr->ipv4, cp, port);
    }

    /* IPv6 specific */
    EMLIB_ASSERT_RETURN(af==EM_AF_INET6, EM_EAFNOTSUP);

    em_bzero(addr, sizeof(em_sockaddr_in6));
    addr->addr.sa_family = EM_AF_INET6;

    status = em_sockaddr_set_str_addr(af, addr, cp);
    if (status != EM_SUCC)
        return status;

    addr->ipv6.sin6_port = em_htons(port);
    return EM_SUCC;
}

/*
 * Compare two socket addresses.
 */
EM_DEF(int) em_sockaddr_cmp( const em_sockaddr_t *addr1,
        const em_sockaddr_t *addr2)
{
    const em_sockaddr *a1 = (const em_sockaddr*) addr1;
    const em_sockaddr *a2 = (const em_sockaddr*) addr2;
    int port1, port2;
    int result;

    /* Compare address family */
    if (a1->addr.sa_family < a2->addr.sa_family)
        return -1;
    else if (a1->addr.sa_family > a2->addr.sa_family)
        return 1;

    /* Compare addresses */
    result = em_memcmp(em_sockaddr_get_addr(a1),
            em_sockaddr_get_addr(a2),
            em_sockaddr_get_addr_len(a1));
    if (result != 0)
        return result;

    /* Compare port number */
    port1 = em_sockaddr_get_port(a1);
    port2 = em_sockaddr_get_port(a2);

    if (port1 < port2)
        return -1;
    else if (port1 > port2)
        return 1;

    /* TODO:
     *	Do we need to compare flow label and scope id in IPv6? 
     */

    /* Looks equal */
    return 0;
}

/*
 * Get first IP address associated with the hostname.
 */
EM_DEF(em_in_addr) em_gethostaddr(void)
{
    em_sockaddr_in addr;
    const em_str_t *hostname = em_gethostname();

    em_sockaddr_in_set_str_addr(&addr, hostname);
    return addr.sin_addr;
}

/*
 * Get port number of a em_sockaddr_in
 */
EM_DEF(em_uint16_t) em_sockaddr_in_get_port(const em_sockaddr_in *addr)
{
    return em_ntohs(addr->sin_port);
}

/*
 * Get the address part
 */
EM_DEF(void*) em_sockaddr_get_addr(const em_sockaddr_t *addr)
{
    const em_sockaddr *a = (const em_sockaddr*)addr;

    EMLIB_ASSERT_RETURN(a->addr.sa_family == EM_AF_INET ||
            a->addr.sa_family == EM_AF_INET6, NULL);

    if (a->addr.sa_family == EM_AF_INET6)
        return (void*) &a->ipv6.sin6_addr;
    else
        return (void*) &a->ipv4.sin_addr;
}

/*
 * Check if sockaddr contains a non-zero address
 */
EM_DEF(em_bool_t) em_sockaddr_has_addr(const em_sockaddr_t *addr)
{
    const em_sockaddr *a = (const em_sockaddr*)addr;

    /* It's probably not wise to raise assertion here if
     * the address doesn't contain a valid address family, and
     * just return EM_FALSE instead.
     * 
     * The reason is because application may need to distinguish 
     * these three conditions with sockaddr:
     *	a) sockaddr is not initialized. This is by convention
     *	   indicated by sa_family==0.
     *	b) sockaddr is initialized with zero address. This is
     *	   indicated with the address field having zero address.
     *	c) sockaddr is initialized with valid address/port.
     *
     * If we enable this assertion, then application will loose
     * the capability to specify condition a), since it will be
     * forced to always initialize sockaddr (even with zero address).
     * This may break some parts of upper layer libraries.
     */
    //EMLIB_ASSERT_RETURN(a->addr.sa_family == EM_AF_INET ||
    //		     a->addr.sa_family == EM_AF_INET6, EM_FALSE);

    if (a->addr.sa_family!=EM_AF_INET && a->addr.sa_family!=EM_AF_INET6) {
        return EM_FALSE;
    } else if (a->addr.sa_family == EM_AF_INET6) {
        em_uint8_t zero[24];
        em_bzero(zero, sizeof(zero));
        return em_memcmp(a->ipv6.sin6_addr.s6_addr, zero, 
                sizeof(em_in6_addr)) != 0;
    } else
        return a->ipv4.sin_addr.s_addr != EM_INADDR_ANY;
}

/*
 * Get port number
 */
EM_DEF(em_uint16_t) em_sockaddr_get_port(const em_sockaddr_t *addr)
{
    const em_sockaddr *a = (const em_sockaddr*) addr;

    EMLIB_ASSERT_RETURN(a->addr.sa_family == EM_AF_INET ||
            a->addr.sa_family == EM_AF_INET6, (em_uint16_t)0xFFFF);

    return em_ntohs((em_uint16_t)(a->addr.sa_family == EM_AF_INET6 ?
                a->ipv6.sin6_port : a->ipv4.sin_port));
}

/*
 * Get the length of the address part.
 */
EM_DEF(unsigned) em_sockaddr_get_addr_len(const em_sockaddr_t *addr)
{
    const em_sockaddr *a = (const em_sockaddr*) addr;
    EMLIB_ASSERT_RETURN(a->addr.sa_family == EM_AF_INET ||
            a->addr.sa_family == EM_AF_INET6, 0);
    return a->addr.sa_family == EM_AF_INET6 ?
        sizeof(em_in6_addr) : sizeof(em_in_addr);
}

/*
 * Get socket address length.
 */
EM_DEF(unsigned) em_sockaddr_get_len(const em_sockaddr_t *addr)
{
    const em_sockaddr *a = (const em_sockaddr*) addr;
    EMLIB_ASSERT_RETURN(a->addr.sa_family == EM_AF_INET ||
            a->addr.sa_family == EM_AF_INET6, 0);
    return a->addr.sa_family == EM_AF_INET6 ?
        sizeof(em_sockaddr_in6) : sizeof(em_sockaddr_in);
}

/*
 * Copy only the address part (sin_addr/sin6_addr) of a socket address.
 */
EM_DEF(void) em_sockaddr_copy_addr( em_sockaddr *dst,
        const em_sockaddr *src)
{
    /* Destination sockaddr might not be initialized */
    const char *srcbuf = (char*)em_sockaddr_get_addr(src);
    char *dstbuf = ((char*)dst) + (srcbuf - (char*)src);
    em_memcpy(dstbuf, srcbuf, em_sockaddr_get_addr_len(src));
}

/*
 * Copy socket address.
 */
EM_DEF(void) em_sockaddr_cp(em_sockaddr_t *dst, const em_sockaddr_t *src)
{
    em_memcpy(dst, src, em_sockaddr_get_len(src));
}

/*
 * Synthesize address.
 */
EM_DEF(emlib_ret_t) em_sockaddr_synthesize(int dst_af,
        em_sockaddr_t *dst,
        const em_sockaddr_t *src)
{
    char ip_addr_buf[EM_INET6_ADDRSTRLEN];
    unsigned int count = 1;
    em_addrinfo ai[1];
    em_str_t ip_addr;
    emlib_ret_t status;

    /* Validate arguments */
    EMLIB_ASSERT_RETURN(src && dst, EM_EINVAL);

    if (dst_af == ((const em_sockaddr *)src)->addr.sa_family) {
        em_sockaddr_cp(dst, src);
        return EM_SUCC;
    }

    em_sockaddr_print(src, ip_addr_buf, sizeof(ip_addr_buf), 0);
    ip_addr = em_str(ip_addr_buf);

    /* Try to synthesize address using em_getaddrinfo(). */
    status = em_getaddrinfo(dst_af, &ip_addr, &count, ai); 
    if (status == EM_SUCC && count > 0) {
        em_sockaddr_cp(dst, &ai[0].ai_addr);
        em_sockaddr_set_port(dst, em_sockaddr_get_port(src));
    }

    return status;
}

/*
 * Set port number of em_sockaddr_in
 */
EM_DEF(void) em_sockaddr_in_set_port(em_sockaddr_in *addr, 
        em_uint16_t hostport)
{
    addr->sin_port = em_htons(hostport);
}

/*
 * Set port number of em_sockaddr
 */
EM_DEF(emlib_ret_t) em_sockaddr_set_port(em_sockaddr *addr, 
        em_uint16_t hostport)
{
    int af = addr->addr.sa_family;

    EMLIB_ASSERT_RETURN(af==EM_AF_INET || af==EM_AF_INET6, EM_EINVAL);

    if (af == EM_AF_INET6)
        addr->ipv6.sin6_port = em_htons(hostport);
    else
        addr->ipv4.sin_port = em_htons(hostport);

    return EM_SUCC;
}

/*
 * Get IPv4 address
 */
EM_DEF(em_in_addr) em_sockaddr_in_get_addr(const em_sockaddr_in *addr)
{
    em_in_addr in_addr;
    in_addr.s_addr = em_ntohl(addr->sin_addr.s_addr);
    return in_addr;
}

/*
 * Set IPv4 address
 */
EM_DEF(void) em_sockaddr_in_set_addr(em_sockaddr_in *addr,
        em_uint32_t hostaddr)
{
    addr->sin_addr.s_addr = em_htonl(hostaddr);
}

/*
 * Parse address
 */
EM_DEF(emlib_ret_t) em_sockaddr_parse2(int af, unsigned options,
        const em_str_t *str,
        em_str_t *p_hostpart,
        em_uint16_t *p_port,
        int *raf)
{
    const char *end = str->ptr + str->slen;
    const char *last_colon_pos = NULL;
    unsigned colon_cnt = 0;
    const char *p;

    EMLIB_ASSERT_RETURN((af==EM_AF_INET || af==EM_AF_INET6 || af==EM_AF_UNSPEC) &&
            options==0 &&
            str!=NULL, EM_EINVAL);

    /* Special handling for empty input */
    if (str->slen==0 || str->ptr==NULL) {
        if (p_hostpart)
            p_hostpart->slen = 0;
        if (p_port)
            *p_port = 0;
        if (raf)
            *raf = EM_AF_INET;
        return EM_SUCC;
    }

    /* Count the colon and get the last colon */
    for (p=str->ptr; p!=end; ++p) {
        if (*p == ':') {
            ++colon_cnt;
            last_colon_pos = p;
        }
    }

    /* Deduce address family if it's not given */
    if (af == EM_AF_UNSPEC) {
        if (colon_cnt > 1)
            af = EM_AF_INET6;
        else
            af = EM_AF_INET;
    } else if (af == EM_AF_INET && colon_cnt > 1)
        return EM_EINVAL;

    if (raf)
        *raf = af;

    if (af == EM_AF_INET) {
        /* Parse as IPv4. Supported formats:
         *  - "10.0.0.1:80"
         *  - "10.0.0.1"
         *  - "10.0.0.1:"
         *  - ":80"
         *  - ":"
         */
        em_str_t hostpart;
        unsigned long port;

        hostpart.ptr = (char*)str->ptr;

        if (last_colon_pos) {
            em_str_t port_part;
            int i;

            hostpart.slen = last_colon_pos - str->ptr;

            port_part.ptr = (char*)last_colon_pos + 1;
            port_part.slen = end - port_part.ptr;

            /* Make sure port number is valid */
            for (i=0; i<port_part.slen; ++i) {
                if (!em_isdigit(port_part.ptr[i]))
                    return EM_EINVAL;
            }
            port = em_strtoul(&port_part);
            if (port > 65535)
                return EM_EINVAL;
        } else {
            hostpart.slen = str->slen;
            port = 0;
        }

        if (p_hostpart)
            *p_hostpart = hostpart;
        if (p_port)
            *p_port = (em_uint16_t)port;

        return EM_SUCC;

    } else if (af == EM_AF_INET6) {

        /* Parse as IPv6. Supported formats:
         *  - "fe::01:80"  ==> note: port number is zero in this case, not 80!
         *  - "[fe::01]:80"
         *  - "fe::01"
         *  - "fe::01:"
         *  - "[fe::01]"
         *  - "[fe::01]:"
         *  - "[::]:80"
         *  - ":::80"
         *  - "[::]"
         *  - "[::]:"
         *  - ":::"
         *  - "::"
         */
        em_str_t hostpart, port_part;

        if (*str->ptr == '[') {
            char *end_bracket;
            int i;
            unsigned long port;

            if (last_colon_pos == NULL)
                return EM_EINVAL;

            end_bracket = em_strchr(str, ']');
            if (end_bracket == NULL)
                return EM_EINVAL;

            hostpart.ptr = (char*)str->ptr + 1;
            hostpart.slen = end_bracket - hostpart.ptr;

            if (last_colon_pos < end_bracket) {
                port_part.ptr = NULL;
                port_part.slen = 0;
            } else {
                port_part.ptr = (char*)last_colon_pos + 1;
                port_part.slen = end - port_part.ptr;
            }

            /* Make sure port number is valid */
            for (i=0; i<port_part.slen; ++i) {
                if (!em_isdigit(port_part.ptr[i]))
                    return EM_EINVAL;
            }
            port = em_strtoul(&port_part);
            if (port > 65535)
                return EM_EINVAL;

            if (p_hostpart)
                *p_hostpart = hostpart;
            if (p_port)
                *p_port = (em_uint16_t)port;

            return EM_SUCC;

        } else {
            /* Treat everything as part of the IPv6 IP address */
            if (p_hostpart)
                *p_hostpart = *str;
            if (p_port)
                *p_port = 0;

            return EM_SUCC;
        }

    } else {
        return EM_EAFNOTSUP;
    }

}

/*
 * Parse address
 */
EM_DEF(emlib_ret_t) em_sockaddr_parse( int af, unsigned options,
        const em_str_t *str,
        em_sockaddr *addr)
{
    em_str_t hostpart;
    em_uint16_t port;
    emlib_ret_t status;

    EMLIB_ASSERT_RETURN(addr, EM_EINVAL);
    EMLIB_ASSERT_RETURN(af==EM_AF_UNSPEC ||
            af==EM_AF_INET ||
            af==EM_AF_INET6, EM_EINVAL);
    EMLIB_ASSERT_RETURN(options == 0, EM_EINVAL);

    status = em_sockaddr_parse2(af, options, str, &hostpart, &port, &af);
    if (status != EM_SUCC)
        return status;

#if !defined(EM_HAS_IPV6) || !EM_HAS_IPV6
    if (af==EM_AF_INET6)
        return EM_EIPV6NOTSUP;
#endif

    status = em_sockaddr_init(af, addr, &hostpart, port);
#if defined(EM_HAS_IPV6) && EM_HAS_IPV6
    if (status != EM_SUCC && af == EM_AF_INET6) {
        /* Parsing does not yield valid address. Try to treat the last 
         * portion after the colon as port number.
         */
        const char *last_colon_pos=NULL, *p;
        const char *end = str->ptr + str->slen;
        unsigned long long_port;
        em_str_t port_part;
        int i;

        /* Parse as IPv6:port */
        for (p=str->ptr; p!=end; ++p) {
            if (*p == ':')
                last_colon_pos = p;
        }

        if (last_colon_pos == NULL)
            return status;

        hostpart.ptr = (char*)str->ptr;
        hostpart.slen = last_colon_pos - str->ptr;

        port_part.ptr = (char*)last_colon_pos + 1;
        port_part.slen = end - port_part.ptr;

        /* Make sure port number is valid */
        for (i=0; i<port_part.slen; ++i) {
            if (!em_isdigit(port_part.ptr[i]))
                return status;
        }
        long_port = em_strtoul(&port_part);
        if (long_port > 65535)
            return status;

        port = (em_uint16_t)long_port;

        status = em_sockaddr_init(EM_AF_INET6, addr, &hostpart, port);
    }
#endif

    return status;
}

/* Resolve the IP address of local machine */
EM_DEF(emlib_ret_t) em_gethostip(int af, em_sockaddr *addr)
{
    unsigned i, count, cand_cnt;
    enum {
        CAND_CNT = 8,

        /* Weighting to be applied to found addresses */
        WEIGHT_HOSTNAME	= 1,	/* hostname IP is not always valid! */
        WEIGHT_DEF_ROUTE = 2,
        WEIGHT_INTERFACE = 1,
        WEIGHT_LOOPBACK = -5,
        WEIGHT_LINK_LOCAL = -4,
        WEIGHT_DISABLED = -50,

        MIN_WEIGHT = WEIGHT_DISABLED+1	/* minimum weight to use */
    };
    /* candidates: */
    em_sockaddr cand_addr[CAND_CNT];
    int		cand_weight[CAND_CNT];
    int	        selected_cand;
    char	strip[EM_INET6_ADDRSTRLEN+10];
    /* Special IPv4 addresses. */
    struct spec_ipv4_t
    {
        em_uint32_t addr;
        em_uint32_t mask;
        int	    weight;
    } spec_ipv4[] =
    {
        /* 127.0.0.0/8, loopback addr will be used if there is no other
         * addresses.
         */
        { 0x7f000000, 0xFF000000, WEIGHT_LOOPBACK },

        /* 0.0.0.0/8, special IP that doesn't seem to be practically useful */
        { 0x00000000, 0xFF000000, WEIGHT_DISABLED },

        /* 169.254.0.0/16, a zeroconf/link-local address, which has higher
         * priority than loopback and will be used if there is no other
         * valid addresses.
         */
        { 0xa9fe0000, 0xFFFF0000, WEIGHT_LINK_LOCAL }
    };
    /* Special IPv6 addresses */
    struct spec_ipv6_t
    {
        em_uint8_t addr[16];
        em_uint8_t mask[16];
        int	   weight;
    } spec_ipv6[] =
    {
        /* Loopback address, ::1/128 */
        { {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
            {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
                0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
            WEIGHT_LOOPBACK
        },

        /* Link local, fe80::/10 */
        { {0xfe,0x80,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {0xff,0xc0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            WEIGHT_LINK_LOCAL
        },

        /* Disabled, ::/128 */
        { {0x0,0x0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            { 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
                0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
            WEIGHT_DISABLED
        }
    };
    em_addrinfo ai;
    emlib_ret_t status;

    /* May not be used if TRACE_ is disabled */
    EM_UNUSED_ARG(strip);

#ifdef _MSC_VER
    /* Get rid of "uninitialized he variable" with MS compilers */
    em_bzero(&ai, sizeof(ai));
#endif

    cand_cnt = 0;
    em_bzero(cand_addr, sizeof(cand_addr));
    em_bzero(cand_weight, sizeof(cand_weight));
    for (i=0; i<EM_ARRAY_SIZE(cand_addr); ++i) {
        cand_addr[i].addr.sa_family = (em_uint16_t)af;
        EM_SOCKADDR_RESET_LEN(&cand_addr[i]);
    }

    addr->addr.sa_family = (em_uint16_t)af;
    EM_SOCKADDR_RESET_LEN(addr);

#if !defined(EM_GETHOSTIP_DISABLE_LOCAL_RESOLUTION) || \
    EM_GETHOSTIP_DISABLE_LOCAL_RESOLUTION == 0
    /* Get hostname's IP address */
    {
        const em_str_t *hostname = em_gethostname();
        count = 1;

        if (hostname->slen > 0)
            status = em_getaddrinfo(af, hostname, &count, &ai);
        else
            status = EM_ERESOLVE;

        if (status == EM_SUCC) {
            EMLIB_ASSERT(ai.ai_addr.addr.sa_family == (em_uint16_t)af);
            em_sockaddr_copy_addr(&cand_addr[cand_cnt], &ai.ai_addr);
            em_sockaddr_set_port(&cand_addr[cand_cnt], 0);
            cand_weight[cand_cnt] += WEIGHT_HOSTNAME;
            ++cand_cnt;

            TRACE_((THIS_FILE, "hostname IP is %s",
                        em_sockaddr_print(&ai.ai_addr, strip, sizeof(strip), 3)));
        }
    }
#else
    EM_UNUSED_ARG(ai);
#endif

    /* Get default interface (interface for default route) */
    if (cand_cnt < EM_ARRAY_SIZE(cand_addr)) {
        status = em_getdefaultipinterface(af, addr);
        if (status == EM_SUCC) {
            TRACE_((THIS_FILE, "default IP is %s",
                        em_sockaddr_print(addr, strip, sizeof(strip), 3)));

            em_sockaddr_set_port(addr, 0);
            for (i=0; i<cand_cnt; ++i) {
                if (em_sockaddr_cmp(&cand_addr[i], addr)==0)
                    break;
            }

            cand_weight[i] += WEIGHT_DEF_ROUTE;
            if (i >= cand_cnt) {
                em_sockaddr_copy_addr(&cand_addr[i], addr);
                ++cand_cnt;
            }
        }
    }


    /* Enumerate IP interfaces */
    if (cand_cnt < EM_ARRAY_SIZE(cand_addr)) {
        unsigned start_if = cand_cnt;
        count = EM_ARRAY_SIZE(cand_addr) - start_if;

        status = em_enum_ip_interface(af, &count, &cand_addr[start_if]);
        if (status == EM_SUCC && count) {
            /* Clear the port number */
            for (i=0; i<count; ++i)
                em_sockaddr_set_port(&cand_addr[start_if+i], 0);

            /* For each candidate that we found so far (that is the hostname
             * address and default interface address, check if they're found
             * in the interface list. If found, add the weight, and if not,
             * decrease the weight.
             */
            for (i=0; i<cand_cnt; ++i) {
                unsigned j;
                for (j=0; j<count; ++j) {
                    if (em_sockaddr_cmp(&cand_addr[i], 
                                &cand_addr[start_if+j])==0)
                        break;
                }

                if (j == count) {
                    /* Not found */
                    cand_weight[i] -= WEIGHT_INTERFACE;
                } else {
                    cand_weight[i] += WEIGHT_INTERFACE;
                }
            }

            /* Add remaining interface to candidate list. */
            for (i=0; i<count; ++i) {
                unsigned j;
                for (j=0; j<cand_cnt; ++j) {
                    if (em_sockaddr_cmp(&cand_addr[start_if+i], 
                                &cand_addr[j])==0)
                        break;
                }

                if (j == cand_cnt) {
                    em_sockaddr_copy_addr(&cand_addr[cand_cnt], 
                            &cand_addr[start_if+i]);
                    cand_weight[cand_cnt] += WEIGHT_INTERFACE;
                    ++cand_cnt;
                }
            }
        }
    }

    /* Apply weight adjustment for special IPv4/IPv6 addresses
     * See http://trac.emsip.org/repos/ticket/1046
     */
    if (af == EM_AF_INET) {
        for (i=0; i<cand_cnt; ++i) {
            unsigned j;
            for (j=0; j<EM_ARRAY_SIZE(spec_ipv4); ++j) {
                em_uint32_t a = em_ntohl(cand_addr[i].ipv4.sin_addr.s_addr);
                em_uint32_t pa = spec_ipv4[j].addr;
                em_uint32_t pm = spec_ipv4[j].mask;

                if ((a & pm) == pa) {
                    cand_weight[i] += spec_ipv4[j].weight;
                    break;
                }
            }
        }
    } else if (af == EM_AF_INET6) {
        for (i=0; i<EM_ARRAY_SIZE(spec_ipv6); ++i) {
            unsigned j;
            for (j=0; j<cand_cnt; ++j) {
                em_uint8_t *a = cand_addr[j].ipv6.sin6_addr.s6_addr;
                em_uint8_t am[16];
                em_uint8_t *pa = spec_ipv6[i].addr;
                em_uint8_t *pm = spec_ipv6[i].mask;
                unsigned k;

                for (k=0; k<16; ++k) {
                    am[k] = (em_uint8_t)((a[k] & pm[k]) & 0xFF);
                }

                if (em_memcmp(am, pa, 16)==0) {
                    cand_weight[j] += spec_ipv6[i].weight;
                }
            }
        }
    } else {
        return EM_EAFNOTSUP;
    }

    /* Enumerate candidates to get the best IP address to choose */
    selected_cand = -1;
    for (i=0; i<cand_cnt; ++i) {
        TRACE_((THIS_FILE, "Checking candidate IP %s, weight=%d",
                    em_sockaddr_print(&cand_addr[i], strip, sizeof(strip), 3),
                    cand_weight[i]));

        if (cand_weight[i] < MIN_WEIGHT) {
            continue;
        }

        if (selected_cand == -1)
            selected_cand = i;
        else if (cand_weight[i] > cand_weight[selected_cand])
            selected_cand = i;
    }

    /* If else fails, returns loopback interface as the last resort */
    if (selected_cand == -1) {
        if (af==EM_AF_INET) {
            addr->ipv4.sin_addr.s_addr = em_htonl (0x7f000001);
        } else {
            em_in6_addr *s6_addr;

            s6_addr = (em_in6_addr*) em_sockaddr_get_addr(addr);
            em_bzero(s6_addr, sizeof(em_in6_addr));
            s6_addr->s6_addr[15] = 1;
        }
        TRACE_((THIS_FILE, "Loopback IP %s returned",
                    em_sockaddr_print(addr, strip, sizeof(strip), 3)));
    } else {
        em_sockaddr_copy_addr(addr, &cand_addr[selected_cand]);
        TRACE_((THIS_FILE, "Candidate %s selected",
                    em_sockaddr_print(addr, strip, sizeof(strip), 3)));
    }

    return EM_SUCC;
}

/* Get IP interface for sending to the specified destination */
EM_DEF(emlib_ret_t) em_getipinterface(int af,
        const em_str_t *dst,
        em_sockaddr *itf_addr,
        em_bool_t allow_resolve,
        em_sockaddr *p_dst_addr)
{
    em_sockaddr dst_addr;
    em_sock_t fd;
    int len;
    em_uint8_t zero[64];
    emlib_ret_t status;

    em_sockaddr_init(af, &dst_addr, NULL, 53);
    status = em_inet_pton(af, dst, em_sockaddr_get_addr(&dst_addr));
    if (status != EM_SUCC) {
        /* "dst" is not an IP address. */
        if (allow_resolve) {
            status = em_sockaddr_init(af, &dst_addr, dst, 53);
        } else {
            em_str_t cp;

            if (af == EM_AF_INET) {
                cp = em_str("1.1.1.1");
            } else {
                cp = em_str("1::1");
            }
            status = em_sockaddr_init(af, &dst_addr, &cp, 53);
        }

        if (status != EM_SUCC)
            return status;
    }

    /* Create UDP socket and connect() to the destination IP */
    status = em_sock_socket(af, em_SOCK_DGRAM(), 0, &fd);
    if (status != EM_SUCC) {
        return status;
    }

    status = em_sock_connect(fd, &dst_addr, em_sockaddr_get_len(&dst_addr));
    if (status != EM_SUCC) {
        em_sock_close(fd);
        return status;
    }

    len = sizeof(*itf_addr);
    status = em_sock_getsockname(fd, itf_addr, &len);
    if (status != EM_SUCC) {
        em_sock_close(fd);
        return status;
    }

    em_sock_close(fd);

    /* Check that the address returned is not zero */
    em_bzero(zero, sizeof(zero));
    if (em_memcmp(em_sockaddr_get_addr(itf_addr), zero,
                em_sockaddr_get_addr_len(itf_addr))==0)
    {
        return EM_ENOTFOUND;
    }

    if (p_dst_addr)
        *p_dst_addr = dst_addr;

    return EM_SUCC;
}

/* Get the default IP interface */
EM_DEF(emlib_ret_t) em_getdefaultipinterface(int af, em_sockaddr *addr)
{
    em_str_t cp;

    if (af == EM_AF_INET) {
        cp = em_str("1.1.1.1");
    } else {
        cp = em_str("1::1");
    }

    return em_getipinterface(af, &cp, addr, EM_FALSE, NULL);
}


/*
 * Bind socket at random port.
 */
EM_DEF(emlib_ret_t) em_sock_bind_random(  em_sock_t sockfd,
        const em_sockaddr_t *addr,
        em_uint16_t port_range,
        em_uint16_t max_try)
{
    em_sockaddr bind_addr;
    int addr_len;
    em_uint16_t base_port;
    emlib_ret_t status = EM_SUCC;

    EM_CHECK_STACK();

    EMLIB_ASSERT_RETURN(addr, EM_EINVAL);

    em_sockaddr_cp(&bind_addr, addr);
    addr_len = em_sockaddr_get_len(addr);
    base_port = em_sockaddr_get_port(addr);

    if (base_port == 0 || port_range == 0) {
        return em_sock_bind(sockfd, &bind_addr, addr_len);
    }

    for (; max_try; --max_try) {
        em_uint16_t port;
        port = (em_uint16_t)(base_port + em_rand() % (port_range + 1));
        em_sockaddr_set_port(&bind_addr, port);
        status = em_sock_bind(sockfd, &bind_addr, addr_len);
        if (status == EM_SUCC)
            break;
    }

    return status;
}


/*
 * Adjust socket send/receive buffer size.
 */
EM_DEF(emlib_ret_t) em_sock_setsockopt_sobuf( em_sock_t sockfd,
        em_uint16_t optname,
        em_bool_t auto_retry,
        unsigned *buf_size)
{
    emlib_ret_t status;
    int try_size, cur_size, i, step, size_len;
    enum { MAX_TRY = 20 };

    EM_CHECK_STACK();

    EMLIB_ASSERT_RETURN(sockfd != EM_INVALID_SOCKET &&
            buf_size &&
            *buf_size > 0 &&
            (optname == em_SO_RCVBUF() ||
             optname == em_SO_SNDBUF()),
            EM_EINVAL);

    size_len = sizeof(cur_size);
    status = em_sock_getsockopt(sockfd, em_SOL_SOCKET(), optname,
            &cur_size, &size_len);
    if (status != EM_SUCC)
        return status;

    try_size = *buf_size;
    step = (try_size - cur_size) / MAX_TRY;
    if (step < 4096)
        step = 4096;

    for (i = 0; i < (MAX_TRY-1); ++i) {
        if (try_size <= cur_size) {
            /* Done, return current size */
            *buf_size = cur_size;
            break;
        }

        status = em_sock_setsockopt(sockfd, em_SOL_SOCKET(), optname,
                &try_size, sizeof(try_size));
        if (status == EM_SUCC) {
            status = em_sock_getsockopt(sockfd, em_SOL_SOCKET(), optname,
                    &cur_size, &size_len);
            if (status != EM_SUCC) {
                /* Ops! No info about current size, just return last try size
                 * and quit.
                 */
                *buf_size = try_size;
                break;
            }
        }

        if (!auto_retry)
            break;

        try_size -= step;
    }

    return status;
}


EM_DEF(char *) em_addr_str_print( const em_str_t *host_str, int port, 
        char *buf, int size, unsigned flag)
{
    enum {
        WITH_PORT = 1
    };
    char *bquote, *equote;
    int af = em_AF_UNSPEC();    
    em_in6_addr dummy6;

    /* Check if this is an IPv6 address */
    if (em_inet_pton(em_AF_INET6(), host_str, &dummy6) == EM_SUCC)
        af = em_AF_INET6();

    if (af == em_AF_INET6()) {
        bquote = "[";
        equote = "]";    
    } else {
        bquote = "";
        equote = "";    
    } 

    if (flag & WITH_PORT) {
        em_ansi_snprintf(buf, size, "%s%.*s%s:%d",
                bquote, (int)host_str->slen, host_str->ptr, equote, 
                port);
    } else {
        em_ansi_snprintf(buf, size, "%s%.*s%s",
                bquote, (int)host_str->slen, host_str->ptr, equote);
    }
    return buf;
}
