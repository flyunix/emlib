/*
 * File:    sock_bsd.c
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
#include <em/os.h>
#include <em/assert.h>
#include <em/string.h>
#include <em/compat/socket.h>
#include <em/addr_resolv.h>
#include <em/errno.h>
//#include <em/unicode.h>

const static char * module = "SOCK_BSD";

/*
 * Address families conversion.
 * The values here are indexed based on em_addr_family.
 */
const em_uint16_t EM_AF_UNSPEC	= AF_UNSPEC;
const em_uint16_t EM_AF_UNIX	= AF_UNIX;
const em_uint16_t EM_AF_INET	= AF_INET;
const em_uint16_t EM_AF_INET6	= AF_INET6;
#ifdef AF_PACKET
const em_uint16_t EM_AF_PACKET	= AF_PACKET;
#else
const em_uint16_t EM_AF_PACKET	= 0xFFFF;
#endif
#ifdef AF_IRDA
const em_uint16_t EM_AF_IRDA	= AF_IRDA;
#else
const em_uint16_t EM_AF_IRDA	= 0xFFFF;
#endif

/*
 * Socket types conversion.
 * The values here are indexed based on em_sock_type
 */
const em_uint16_t EM_SOCK_STREAM= SOCK_STREAM;
const em_uint16_t EM_SOCK_DGRAM	= SOCK_DGRAM;
const em_uint16_t EM_SOCK_RAW	= SOCK_RAW;
const em_uint16_t EM_SOCK_RDM	= SOCK_RDM;

/*
 * Socket level values.
 */
const em_uint16_t EM_SOL_SOCKET	= SOL_SOCKET;
#ifdef SOL_IP
const em_uint16_t EM_SOL_IP	= SOL_IP;
#elif (defined(EM_WIN32) && EM_WIN32) || (defined(EM_WIN64) && EM_WIN64) || \
    (defined (IPPROTO_IP))
const em_uint16_t EM_SOL_IP	= IPPROTO_IP;
#else
const em_uint16_t EM_SOL_IP	= 0;
#endif /* SOL_IP */

#if defined(SOL_TCP)
const em_uint16_t EM_SOL_TCP	= SOL_TCP;
#elif defined(IPPROTO_TCP)
const em_uint16_t EM_SOL_TCP	= IPPROTO_TCP;
#elif (defined(EM_WIN32) && EM_WIN32) || (defined(EM_WIN64) && EM_WIN64)
const em_uint16_t EM_SOL_TCP	= IPPROTO_TCP;
#else
const em_uint16_t EM_SOL_TCP	= 6;
#endif /* SOL_TCP */

#ifdef SOL_UDP
const em_uint16_t EM_SOL_UDP	= SOL_UDP;
#elif defined(IPPROTO_UDP)
const em_uint16_t EM_SOL_UDP	= IPPROTO_UDP;
#elif (defined(EM_WIN32) && EM_WIN32) || (defined(EM_WIN64) && EM_WIN64)
const em_uint16_t EM_SOL_UDP	= IPPROTO_UDP;
#else
const em_uint16_t EM_SOL_UDP	= 17;
#endif /* SOL_UDP */

#ifdef SOL_IPV6
const em_uint16_t EM_SOL_IPV6	= SOL_IPV6;
#elif (defined(EM_WIN32) && EM_WIN32) || (defined(EM_WIN64) && EM_WIN64)
#   if defined(IPPROTO_IPV6) || (_WIN32_WINNT >= 0x0501)
const em_uint16_t EM_SOL_IPV6	= IPPROTO_IPV6;
#   else
const em_uint16_t EM_SOL_IPV6	= 41;
#   endif
#else
const em_uint16_t EM_SOL_IPV6	= 41;
#endif /* SOL_IPV6 */

/* IP_TOS */
#ifdef IP_TOS
const em_uint16_t EM_IP_TOS	= IP_TOS;
#else
const em_uint16_t EM_IP_TOS	= 1;
#endif


/* TOS settings (declared in netinet/ip.h) */
#ifdef IPTOS_LOWDELAY
const em_uint16_t EM_IPTOS_LOWDELAY	= IPTOS_LOWDELAY;
#else
const em_uint16_t EM_IPTOS_LOWDELAY	= 0x10;
#endif
#ifdef IPTOS_THROUGHPUT
const em_uint16_t EM_IPTOS_THROUGHPUT	= IPTOS_THROUGHPUT;
#else
const em_uint16_t EM_IPTOS_THROUGHPUT	= 0x08;
#endif
#ifdef IPTOS_RELIABILITY
const em_uint16_t EM_IPTOS_RELIABILITY	= IPTOS_RELIABILITY;
#else
const em_uint16_t EM_IPTOS_RELIABILITY	= 0x04;
#endif
#ifdef IPTOS_MINCOST
const em_uint16_t EM_IPTOS_MINCOST	= IPTOS_MINCOST;
#else
const em_uint16_t EM_IPTOS_MINCOST	= 0x02;
#endif


/* IPV6_TCLASS */
#ifdef IPV6_TCLASS
const em_uint16_t EM_IPV6_TCLASS = IPV6_TCLASS;
#else
const em_uint16_t EM_IPV6_TCLASS = 0xFFFF;
#endif


/* optname values. */
const em_uint16_t EM_SO_TYPE    = SO_TYPE;
const em_uint16_t EM_SO_RCVBUF  = SO_RCVBUF;
const em_uint16_t EM_SO_SNDBUF  = SO_SNDBUF;
const em_uint16_t EM_TCP_NODELAY= TCP_NODELAY;
const em_uint16_t EM_SO_REUSEADDR= SO_REUSEADDR;
#ifdef SO_NOSIGPIPE
const em_uint16_t EM_SO_NOSIGPIPE = SO_NOSIGPIPE;
#else
const em_uint16_t EM_SO_NOSIGPIPE = 0xFFFF;
#endif
#if defined(SO_PRIORITY)
const em_uint16_t EM_SO_PRIORITY = SO_PRIORITY;
#else
/* This is from Linux, YMMV */
const em_uint16_t EM_SO_PRIORITY = 12;
#endif

/* Multicasting is not supported e.g. in PocketPC 2003 SDK */
#ifdef IP_MULTICAST_IF
const em_uint16_t EM_IP_MULTICAST_IF    = IP_MULTICAST_IF;
const em_uint16_t EM_IP_MULTICAST_TTL   = IP_MULTICAST_TTL;
const em_uint16_t EM_IP_MULTICAST_LOOP  = IP_MULTICAST_LOOP;
const em_uint16_t EM_IP_ADD_MEMBERSHIP  = IP_ADD_MEMBERSHIP;
const em_uint16_t EM_IP_DROP_MEMBERSHIP = IP_DROP_MEMBERSHIP;
#else
const em_uint16_t EM_IP_MULTICAST_IF    = 0xFFFF;
const em_uint16_t EM_IP_MULTICAST_TTL   = 0xFFFF;
const em_uint16_t EM_IP_MULTICAST_LOOP  = 0xFFFF;
const em_uint16_t EM_IP_ADD_MEMBERSHIP  = 0xFFFF;
const em_uint16_t EM_IP_DROP_MEMBERSHIP = 0xFFFF;
#endif

/* recv() and send() flags */
const int EM_MSG_OOB		= MSG_OOB;
const int EM_MSG_PEEK		= MSG_PEEK;
const int EM_MSG_DONTROUTE	= MSG_DONTROUTE;


#if 0
static void CHECK_ADDR_LEN(const em_sockaddr *addr, int len)
{
    em_sockaddr *a = (em_sockaddr*)addr;
    em_assert((a->addr.sa_family==EM_AF_INET && len==sizeof(em_sockaddr_in)) ||
            (a->addr.sa_family==EM_AF_INET6 && len==sizeof(em_sockaddr_in6)));

}
#else
#define CHECK_ADDR_LEN(addr,len)
#endif

/*
 * Convert 16-bit value from network byte order to host byte order.
 */
EM_DEF(em_uint16_t) em_ntohs(em_uint16_t netshort)
{
    return ntohs(netshort);
}

/*
 * Convert 16-bit value from host byte order to network byte order.
 */
EM_DEF(em_uint16_t) em_htons(em_uint16_t hostshort)
{
    return htons(hostshort);
}

/*
 * Convert 32-bit value from network byte order to host byte order.
 */
EM_DEF(em_uint32_t) em_ntohl(em_uint32_t netlong)
{
    return ntohl(netlong);
}

/*
 * Convert 32-bit value from host byte order to network byte order.
 */
EM_DEF(em_uint32_t) em_htonl(em_uint32_t hostlong)
{
    return htonl(hostlong);
}

/*
 * Convert an Internet host address given in network byte order
 * to string in standard numbers and dots notation.
 */
EM_DEF(char*) em_inet_ntoa(em_in_addr inaddr)
{
#if 0
    return inet_ntoa(*(struct in_addr*)&inaddr);
#else
    struct in_addr addr;
    //addr.s_addr = inaddr.s_addr;
    em_memcpy(&addr, &inaddr, sizeof(addr));
    return inet_ntoa(addr);
#endif
}

/*
 * This function converts the Internet host address cp from the standard
 * numbers-and-dots notation into binary data and stores it in the structure
 * that inp points to. 
 */
EM_DEF(int) em_inet_aton(const em_str_t *cp, struct em_in_addr *inp)
{
    char tempaddr[EM_INET_ADDRSTRLEN];

    /* Initialize output with EM_INADDR_NONE.
     * Some apps relies on this instead of the return value
     * (and anyway the return value is quite confusing!)
     */
    inp->s_addr = EM_INADDR_NONE;

    /* Caution:
     *	this function might be called with cp->slen >= 16
     *  (i.e. when called with hostname to check if it's an IP addr).
     */
    EMLIB_ASSERT_RETURN(cp && cp->slen && inp, 0);
    if (cp->slen >= EM_INET_ADDRSTRLEN) {
        return 0;
    }

    em_memcpy(tempaddr, cp->ptr, cp->slen);
    tempaddr[cp->slen] = '\0';

#if defined(EM_SOCK_HAS_INET_ATON) && EM_SOCK_HAS_INET_ATON != 0
    return inet_aton(tempaddr, (struct in_addr*)inp);
#else
    inp->s_addr = inet_addr(tempaddr);
    return inp->s_addr == EM_INADDR_NONE ? 0 : 1;
#endif
}

/*
 * Convert text to IPv4/IPv6 address.
 */
EM_DEF(emlib_ret_t) em_inet_pton(int af, const em_str_t *src, void *dst)
{
    char tempaddr[EM_INET6_ADDRSTRLEN];

    EMLIB_ASSERT_RETURN(af==EM_AF_INET || af==EM_AF_INET6, EM_EAFNOTSUP);
    EMLIB_ASSERT_RETURN(src && src->slen && dst, EM_EINVAL);

    /* Initialize output with EM_IN_ADDR_NONE for IPv4 (to be 
     * compatible with em_inet_aton()
     */
    if (af==EM_AF_INET) {
        ((em_in_addr*)dst)->s_addr = EM_INADDR_NONE;
    }

    /* Caution:
     *	this function might be called with cp->slen >= 46
     *  (i.e. when called with hostname to check if it's an IP addr).
     */
    if (src->slen >= EM_INET6_ADDRSTRLEN) {
        return EM_ENAMETOOLONG;
    }

    em_memcpy(tempaddr, src->ptr, src->slen);
    tempaddr[src->slen] = '\0';

#if defined(EM_SOCK_HAS_INET_PTON) && EM_SOCK_HAS_INET_PTON != 0
    /*
     * Implementation using inet_pton()
     */
    if (inet_pton(af, tempaddr, dst) != 1) {
        emlib_ret_t status = em_get_netos_error();
        if (status == EM_SUCC)
            status = EM_EUNKNOWN;

        return status;
    }

    return EM_SUCC;

#elif defined(EM_WIN32) || defined(EM_WIN64) || defined(EM_WIN32_WINCE)
    /*
     * Implementation on Windows, using WSAStringToAddress().
     * Should also work on Unicode systems.
     */
    {
        EM_DECL_UNICODE_TEMP_BUF(wtempaddr,EM_INET6_ADDRSTRLEN)
            em_sockaddr sock_addr;
        int addr_len = sizeof(sock_addr);
        int rc;

        sock_addr.addr.sa_family = (em_uint16_t)af;
        rc = WSAStringToAddress(
                EM_STRING_TO_NATIVE(tempaddr,wtempaddr,sizeof(wtempaddr)), 
                af, NULL, (LPSOCKADDR)&sock_addr, &addr_len);
        if (rc != 0) {
            /* If you get rc 130022 Invalid argument (WSAEINVAL) with IPv6,
             * check that you have IPv6 enabled (install it in the network
             * adapter).
             */
            emlib_ret_t status = em_get_netos_error();
            if (status == EM_SUCC)
                status = EM_EUNKNOWN;

            return status;
        }

        if (sock_addr.addr.sa_family == EM_AF_INET) {
            em_memcpy(dst, &sock_addr.ipv4.sin_addr, 4);
            return EM_SUCC;
        } else if (sock_addr.addr.sa_family == EM_AF_INET6) {
            em_memcpy(dst, &sock_addr.ipv6.sin6_addr, 16);
            return EM_SUCC;
        } else {
            em_assert(!"Shouldn't happen");
            return EM_EBUG;
        }
    }
#elif !defined(EM_HAS_IPV6) || EM_HAS_IPV6==0
    /* IPv6 support is disabled, just return error without raising assertion */
    return EM_EIPV6NOTSUP;
#else
    em_assert(!"Not supported");
    return EM_EIPV6NOTSUP;
#endif
}

/*
 * Convert IPv4/IPv6 address to text.
 */
EM_DEF(emlib_ret_t) em_inet_ntop(int af, const void *src,
        char *dst, int size)

{
    EMLIB_ASSERT_RETURN(src && dst && size, EM_EINVAL);

    *dst = '\0';

    EMLIB_ASSERT_RETURN(af==EM_AF_INET || af==EM_AF_INET6, EM_EAFNOTSUP);

#if defined(EM_SOCK_HAS_INET_NTOP) && EM_SOCK_HAS_INET_NTOP != 0
    /*
     * Implementation using inet_ntop()
     */
    if (inet_ntop(af, src, dst, size) == NULL) {
        emlib_ret_t status = em_get_netos_error();
        if (status == EM_SUCC)
            status = EM_EUNKNOWN;

        return status;
    }

    return EM_SUCC;

#elif defined(EM_WIN32) || defined(EM_WIN64) || defined(EM_WIN32_WINCE)
    /*
     * Implementation on Windows, using WSAAddressToString().
     * Should also work on Unicode systems.
     */
    {
        EM_DECL_UNICODE_TEMP_BUF(wtempaddr,EM_INET6_ADDRSTRLEN)
            em_sockaddr sock_addr;
        DWORD addr_len, addr_str_len;
        int rc;

        em_bzero(&sock_addr, sizeof(sock_addr));
        sock_addr.addr.sa_family = (em_uint16_t)af;
        if (af == EM_AF_INET) {
            if (size < EM_INET_ADDRSTRLEN)
                return EM_ETOOSMALL;
            em_memcpy(&sock_addr.ipv4.sin_addr, src, 4);
            addr_len = sizeof(em_sockaddr_in);
            addr_str_len = EM_INET_ADDRSTRLEN;
        } else if (af == EM_AF_INET6) {
            if (size < EM_INET6_ADDRSTRLEN)
                return EM_ETOOSMALL;
            em_memcpy(&sock_addr.ipv6.sin6_addr, src, 16);
            addr_len = sizeof(em_sockaddr_in6);
            addr_str_len = EM_INET6_ADDRSTRLEN;
        } else {
            em_assert(!"Unsupported address family");
            return EM_EAFNOTSUP;
        }

#if EM_NATIVE_STRING_IS_UNICODE
        rc = WSAAddressToString((LPSOCKADDR)&sock_addr, addr_len,
                NULL, wtempaddr, &addr_str_len);
        if (rc == 0) {
            em_unicode_to_ansi(wtempaddr, wcslen(wtempaddr), dst, size);
        }
#else
        rc = WSAAddressToString((LPSOCKADDR)&sock_addr, addr_len,
                NULL, dst, &addr_str_len);
#endif

        if (rc != 0) {
            emlib_ret_t status = em_get_netos_error();
            if (status == EM_SUCC)
                status = EM_EUNKNOWN;

            return status;
        }

        return EM_SUCC;
    }

#elif !defined(EM_HAS_IPV6) || EM_HAS_IPV6==0
    /* IPv6 support is disabled, just return error without raising assertion */
    return EM_EIPV6NOTSUP;
#else
    em_assert(!"Not supported");
    return EM_EIPV6NOTSUP;
#endif
}

/*
 * Get hostname.
 */
EM_DEF(const em_str_t*) em_gethostname(void)
{
    static char buf[EM_MAX_HOSTNAME];
    static em_str_t hostname;

    EM_CHECK_STACK();

    if (hostname.ptr == NULL) {
        hostname.ptr = buf;
        if (gethostname(buf, sizeof(buf)) != 0) {
            hostname.ptr[0] = '\0';
            hostname.slen = 0;
        } else {
            hostname.slen = strlen(buf);
        }
    }
    return &hostname;
}

#if defined(EM_WIN32) || defined(EM_WIN64)
/*
 * Create new socket/endpoint for communication and returns a descriptor.
 */
EM_DEF(emlib_ret_t) em_sock_socket(int af, 
        int type, 
        int proto,
        em_sock_t *sock)
{
    EM_CHECK_STACK();

    /* Sanity checks. */
    EMLIB_ASSERT_RETURN(sock!=NULL, EM_EINVAL);
    EMLIB_ASSERT_RETURN((SOCKET)EM_INVALID_SOCKET==INVALID_SOCKET, 
            (*sock=EM_INVALID_SOCKET, EM_EINVAL));

    *sock = WSASocket(af, type, proto, NULL, 0, WSA_FLAG_OVERLAPPED);

    if (*sock == EM_INVALID_SOCKET) 
        return EM_RETURN_OS_ERROR(em_get_native_netos_error());

#if EM_SOCK_DISABLE_WSAECONNRESET && \
    (!defined(EM_WIN32_WINCE) || EM_WIN32_WINCE==0)

#ifndef SIO_UDP_CONNRESET
#define SIO_UDP_CONNRESET _WSAIOW(IOC_VENDOR,12)
#endif

    /* Disable WSAECONNRESET for UDP.
     * See https://trac.emsip.org/repos/ticket/1197
     */
    if (type==EM_SOCK_DGRAM) {
        DWORD dwBytesReturned = 0;
        BOOL bNewBehavior = FALSE;
        DWORD rc;

        rc = WSAIoctl(*sock, SIO_UDP_CONNRESET,
                &bNewBehavior, sizeof(bNewBehavior),
                NULL, 0, &dwBytesReturned,
                NULL, NULL);

        if (rc==SOCKET_ERROR) {
            // Ignored..
        }
    }
#endif

    return EM_SUCC;
}

#else
/*
 * Create new socket/endpoint for communication and returns a descriptor.
 */
EM_DEF(emlib_ret_t) em_sock_socket(int af, 
        int type, 
        int proto, 
        em_sock_t *sock)
{

    EM_CHECK_STACK();

    /* Sanity checks. */
    EMLIB_ASSERT_RETURN(sock!=NULL, EM_EINVAL);
    EMLIB_ASSERT_RETURN(EM_INVALID_SOCKET==-1, 
            (*sock=EM_INVALID_SOCKET, EM_EINVAL));

    *sock = socket(af, type, proto);
    if (*sock == EM_INVALID_SOCKET)
        return EM_RETURN_OS_ERROR(em_get_native_netos_error());
    else {
        em_int32_t val = 1;
        if (type == em_SOCK_STREAM()) {
            em_sock_setsockopt(*sock, em_SOL_SOCKET(), em_SO_NOSIGPIPE(),
                    &val, sizeof(val));
        }
#if defined(EM_SOCK_HAS_IPV6_V6ONLY) && EM_SOCK_HAS_IPV6_V6ONLY != 0
        if (af == EM_AF_INET6) {
            em_sock_setsockopt(*sock, EM_SOL_IPV6, IPV6_V6ONLY,
                    &val, sizeof(val));
        }
#endif
#if defined(EM_IPHONE_OS_HAS_MULTITASKING_SUPPORT) && \
        EM_IPHONE_OS_HAS_MULTITASKING_SUPPORT!=0
        if (type == em_SOCK_DGRAM()) {
            em_sock_setsockopt(*sock, em_SOL_SOCKET(), SO_NOSIGPIPE, 
                    &val, sizeof(val));
        }
#endif
        return EM_SUCC;
    }
}
#endif

/*
 * Bind socket.
 */
EM_DEF(emlib_ret_t) em_sock_bind( em_sock_t sock, 
        const em_sockaddr_t *addr,
        int len)
{
    EM_CHECK_STACK();

    EMLIB_ASSERT_RETURN(addr && len >= (int)sizeof(struct sockaddr_in), EM_EINVAL);

    CHECK_ADDR_LEN(addr, len);

    if (bind(sock, (struct sockaddr*)addr, len) != 0)
        return EM_RETURN_OS_ERROR(em_get_native_netos_error());
    else
        return EM_SUCC;
}


/*
 * Bind socket.
 */
EM_DEF(emlib_ret_t) em_sock_bind_in( em_sock_t sock, 
        em_uint32_t addr32,
        em_uint16_t port)
{
    em_sockaddr_in addr;

    EM_CHECK_STACK();

    EM_SOCKADDR_SET_LEN(&addr, sizeof(em_sockaddr_in));
    addr.sin_family = EM_AF_INET;
    em_bzero(addr.sin_zero, sizeof(addr.sin_zero));
    addr.sin_addr.s_addr = em_htonl(addr32);
    addr.sin_port = em_htons(port);

    return em_sock_bind(sock, &addr, sizeof(em_sockaddr_in));
}


/*
 * Close socket.
 */
EM_DEF(emlib_ret_t) em_sock_close(em_sock_t sock)
{
    int rc;

    EM_CHECK_STACK();
#if defined(EM_WIN32) && EM_WIN32!=0 || \
    defined(EM_WIN64) && EM_WIN64 != 0 || \
    defined(EM_WIN32_WINCE) && EM_WIN32_WINCE!=0
    rc = closesocket(sock);
#else
    rc = close(sock);
#endif

    if (rc != 0)
        return EM_RETURN_OS_ERROR(em_get_native_netos_error());
    else
        return EM_SUCC;
}

/*
 * Get remote's name.
 */
EM_DEF(emlib_ret_t) em_sock_getpeername( em_sock_t sock,
        em_sockaddr_t *addr,
        int *namelen)
{
    EM_CHECK_STACK();
    if (getpeername(sock, (struct sockaddr*)addr, (socklen_t*)namelen) != 0)
        return EM_RETURN_OS_ERROR(em_get_native_netos_error());
    else {
        EM_SOCKADDR_RESET_LEN(addr);
        return EM_SUCC;
    }
}

/*
 * Get socket name.
 */
EM_DEF(emlib_ret_t) em_sock_getsockname( em_sock_t sock,
        em_sockaddr_t *addr,
        int *namelen)
{
    EM_CHECK_STACK();
    if (getsockname(sock, (struct sockaddr*)addr, (socklen_t*)namelen) != 0)
        return EM_RETURN_OS_ERROR(em_get_native_netos_error());
    else {
        EM_SOCKADDR_RESET_LEN(addr);
        return EM_SUCC;
    }
}

/*
 * Send data
 */
EM_DEF(emlib_ret_t) em_sock_send(em_sock_t sock,
        const void *buf,
        em_ssize_t *len,
        unsigned flags)
{
    EM_CHECK_STACK();
    EMLIB_ASSERT_RETURN(len, EM_EINVAL);

#ifdef MSG_NOSIGNAL
    /* Suppress SIGPIPE. See https://trac.emsip.org/repos/ticket/1538 */
    flags |= MSG_NOSIGNAL;
#endif

    *len = send(sock, (const char*)buf, (int)(*len), flags);

    if (*len < 0)
        return EM_RETURN_OS_ERROR(em_get_native_netos_error());
    else
        return EM_SUCC;
}


/*
 * Send data.
 */
EM_DEF(emlib_ret_t) em_sock_sendto(em_sock_t sock,
        const void *buf,
        em_ssize_t *len,
        unsigned flags,
        const em_sockaddr_t *to,
        int tolen)
{
    EM_CHECK_STACK();
    EMLIB_ASSERT_RETURN(len, EM_EINVAL);

    CHECK_ADDR_LEN(to, tolen);

    *len = sendto(sock, (const char*)buf, (int)(*len), flags, 
            (const struct sockaddr*)to, tolen);

    if (*len < 0) 
        return EM_RETURN_OS_ERROR(em_get_native_netos_error());
    else 
        return EM_SUCC;
}

/*
 * Receive data.
 */
EM_DEF(emlib_ret_t) em_sock_recv(em_sock_t sock,
        void *buf,
        em_ssize_t *len,
        unsigned flags)
{
    EM_CHECK_STACK();
    EMLIB_ASSERT_RETURN(buf && len, EM_EINVAL);

    *len = recv(sock, (char*)buf, (int)(*len), flags);

    if (*len < 0) 
        return EM_RETURN_OS_ERROR(em_get_native_netos_error());
    else
        return EM_SUCC;
}

/*
 * Receive data.
 */
EM_DEF(emlib_ret_t) em_sock_recvfrom(em_sock_t sock,
        void *buf,
        em_ssize_t *len,
        unsigned flags,
        em_sockaddr_t *from,
        int *fromlen)
{
    EM_CHECK_STACK();
    EMLIB_ASSERT_RETURN(buf && len, EM_EINVAL);

    *len = recvfrom(sock, (char*)buf, (int)(*len), flags, 
            (struct sockaddr*)from, (socklen_t*)fromlen);

    if (*len < 0) 
        return EM_RETURN_OS_ERROR(em_get_native_netos_error());
    else {
        if (from) {
            EM_SOCKADDR_RESET_LEN(from);
        }
        return EM_SUCC;
    }
}

/*
 * Get socket option.
 */
EM_DEF(emlib_ret_t) em_sock_getsockopt( em_sock_t sock,
        em_uint16_t level,
        em_uint16_t optname,
        void *optval,
        int *optlen)
{
    EM_CHECK_STACK();
    EMLIB_ASSERT_RETURN(optval && optlen, EM_EINVAL);

    if (getsockopt(sock, level, optname, (char*)optval, (socklen_t*)optlen)!=0)
        return EM_RETURN_OS_ERROR(em_get_native_netos_error());
    else
        return EM_SUCC;
}

/*
 * Set socket option.
 */
EM_DEF(emlib_ret_t) em_sock_setsockopt( em_sock_t sock,
        em_uint16_t level,
        em_uint16_t optname,
        const void *optval,
        int optlen)
{
    int status;
    EM_CHECK_STACK();

#if (defined(EM_WIN32) && EM_WIN32) || (defined(EM_SUNOS) && EM_SUNOS)
    /* Some opt may still need int value (e.g:SO_EXCLUSIVEADDRUSE in win32). */
    status = setsockopt(sock, 
            level, 
            ((optname&0xff00)==0xff00)?(int)optname|0xffff0000:optname, 		     
            (const char*)optval, optlen);
#else
    status = setsockopt(sock, level, optname, (const char*)optval, optlen);
#endif

    if (status != 0)
        return EM_RETURN_OS_ERROR(em_get_native_netos_error());
    else
        return EM_SUCC;
}

/*
 * Set socket option.
 */
EM_DEF(emlib_ret_t) em_sock_setsockopt_params( em_sock_t sockfd,
        const em_sockopt_params *params)
{
    unsigned int i = 0;
    emlib_ret_t retval = EM_SUCC;
    EM_CHECK_STACK();
    EMLIB_ASSERT_RETURN(params, EM_EINVAL);

    for (;i<params->cnt && i<EM_MAX_SOCKOPT_PARAMS;++i) {
        emlib_ret_t status = em_sock_setsockopt(sockfd, 
                (em_uint16_t)params->options[i].level,
                (em_uint16_t)params->options[i].optname,
                params->options[i].optval, 
                params->options[i].optlen);
        if (status != EM_SUCC) {
            retval = status;
            EM_PERROR(4,(module, status,
                        "Warning: error applying sock opt %d",
                        params->options[i].optname));
        }
    }

    return retval;
}

/*
 * Connect socket.
 */
EM_DEF(emlib_ret_t) em_sock_connect( em_sock_t sock,
        const em_sockaddr_t *addr,
        int namelen)
{
    EM_CHECK_STACK();
    if (connect(sock, (struct sockaddr*)addr, namelen) != 0)
        return EM_RETURN_OS_ERROR(em_get_native_netos_error());
    else
        return EM_SUCC;
}


/*
 * Shutdown socket.
 */
#if EM_HAS_TCP
EM_DEF(emlib_ret_t) em_sock_shutdown( em_sock_t sock,
        int how)
{
    EM_CHECK_STACK();
    if (shutdown(sock, how) != 0)
        return EM_RETURN_OS_ERROR(em_get_native_netos_error());
    else
        return EM_SUCC;
}

/*
 * Start listening to incoming connections.
 */
EM_DEF(emlib_ret_t) em_sock_listen( em_sock_t sock,
        int backlog)
{
    EM_CHECK_STACK();
    if (listen(sock, backlog) != 0)
        return EM_RETURN_OS_ERROR(em_get_native_netos_error());
    else
        return EM_SUCC;
}

/*
 * Accept incoming connections
 */
EM_DEF(emlib_ret_t) em_sock_accept( em_sock_t serverfd,
        em_sock_t *newsock,
        em_sockaddr_t *addr,
        int *addrlen)
{
    EM_CHECK_STACK();
    EMLIB_ASSERT_RETURN(newsock != NULL, EM_EINVAL);

#if defined(EM_SOCKADDR_HAS_LEN) && EM_SOCKADDR_HAS_LEN!=0
    if (addr) {
        EM_SOCKADDR_SET_LEN(addr, *addrlen);
    }
#endif

    *newsock = accept(serverfd, (struct sockaddr*)addr, (socklen_t*)addrlen);
    if (*newsock==EM_INVALID_SOCKET)
        return EM_RETURN_OS_ERROR(em_get_native_netos_error());
    else {

#if defined(EM_SOCKADDR_HAS_LEN) && EM_SOCKADDR_HAS_LEN!=0
        if (addr) {
            EM_SOCKADDR_RESET_LEN(addr);
        }
#endif

        return EM_SUCC;
    }
}
#endif	/* EM_HAS_TCP */


