/*
 * File:    addr_resolv_sock.c
 * Author:  Liu HongLiang<lhl_nciae@sina.cn>
 * Brief:   Abstraction of Network Address Resolution.
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
#include <em/addr_resolv.h>
#include <em/assert.h>
#include <em/string.h>
#include <em/errno.h>
#include <em/ip_helper.h>
#include <em/compat/socket.h>

#if defined(EM_GETADDRINFO_USE_CFHOST) && EM_GETADDRINFO_USE_CFHOST!=0
#   include <CoreFoundation/CFString.h>
#   include <CFNetwork/CFHost.h>
#endif

const static char * module = "ADDR_RESOLV_SOCK";

EM_DEF(emlib_ret_t) em_gethostbyname(const em_str_t *hostname, em_hostent *phe)
{
    struct hostent *he;
    char copy[EM_MAX_HOSTNAME];

    EM_ASSERT(hostname && hostname ->slen < EM_MAX_HOSTNAME);

    if (hostname->slen >= EM_MAX_HOSTNAME)
        return EM_ENAMETOOLONG;

    em_memcpy(copy, hostname->ptr, hostname->slen);
    copy[ hostname->slen ] = '\0';

    he = gethostbyname(copy);
    if (!he) {
        return EM_ERESOLVE;
        /* DO NOT use em_get_netos_error() since host resolution error
         * is reported in h_errno instead of errno!
         return em_get_netos_error();
         */
    }

    phe->h_name = he->h_name;
    phe->h_aliases = he->h_aliases;
    phe->h_addrtype = he->h_addrtype;
    phe->h_length = he->h_length;
    phe->h_addr_list = he->h_addr_list;

    return EM_SUCC;
}

/* Resolve IPv4/IPv6 address */
EM_DEF(emlib_ret_t) em_getaddrinfo(int af, const em_str_t *nodename,
        unsigned *count, em_addrinfo ai[])
{
#if defined(EM_SOCK_HAS_GETADDRINFO) && EM_SOCK_HAS_GETADDRINFO!=0
    char nodecopy[EM_MAX_HOSTNAME];
    em_bool_t has_addr = EM_FALSE;
    unsigned i;
#if defined(EM_GETADDRINFO_USE_CFHOST) && EM_GETADDRINFO_USE_CFHOST!=0
    CFStringRef hostname;
    CFHostRef hostRef;
    emlib_ret_t status = EM_SUCC;
#else
    int rc;
    struct addrinfo hint, *res, *orig_res;
#endif

    EM_ASSERT_RETURN(nodename && count && *count && ai, EM_EINVAL);
    EM_ASSERT_RETURN(nodename->ptr && nodename->slen, EM_EINVAL);
    EM_ASSERT_RETURN(af==EM_AF_INET || af==EM_AF_INET6 ||
            af==EM_AF_UNSPEC, EM_EINVAL);

    EM_UNUSED_ARG(has_addr);

    /* Copy node name to null terminated string. */
    if (nodename->slen >= EM_MAX_HOSTNAME)
        return EM_ENAMETOOLONG;
    em_memcpy(nodecopy, nodename->ptr, nodename->slen);
    nodecopy[nodename->slen] = '\0';

#if defined(EM_GETADDRINFO_USE_CFHOST) && EM_GETADDRINFO_USE_CFHOST!=0
    hostname =  CFStringCreateWithCStringNoCopy(kCFAllocatorDefault, nodecopy,
            kCFStringEncodingASCII,
            kCFAllocatorNull);
    hostRef = CFHostCreateWithName(kCFAllocatorDefault, hostname);
    if (CFHostStartInfoResolution(hostRef, kCFHostAddresses, nil)) {
        CFArrayRef addrRef = CFHostGetAddressing(hostRef, nil);
        i = 0;
        if (addrRef != nil) {
            CFIndex idx, naddr;

            naddr = CFArrayGetCount(addrRef);
            for (idx = 0; idx < naddr && i < *count; idx++) {
                struct sockaddr *addr;
                size_t addr_size;

                addr = (struct sockaddr *)
                    CFDataGetBytePtr(CFArrayGetValueAtIndex(addrRef, idx));
                /* This should not happen. */
                EM_ASSERT(addr);

                /* Ignore unwanted address families */
                if (af!=EM_AF_UNSPEC && addr->sa_family != af)
                    continue;

                /* Store canonical name */
                em_ansi_strcpy(ai[i].ai_canonname, nodecopy);

                /* Store address */
                addr_size = sizeof(*addr);
                if (addr->sa_family == EM_AF_INET6) {
                    addr_size = addr->sa_len;
                }
                EM_ASSERT_ON_FAIL(addr_size <= sizeof(em_sockaddr), continue);
                em_memcpy(&ai[i].ai_addr, addr, addr_size);
                EM_SOCKADDR_RESET_LEN(&ai[i].ai_addr);

                i++;
            }
        }

        *count = i;
        if (*count == 0)
            status = EM_ERESOLVE;

    } else {
        status = EM_ERESOLVE;
    }

    CFRelease(hostRef);
    CFRelease(hostname);

    return status;
#else
    /* Call getaddrinfo() */
    em_bzero(&hint, sizeof(hint));
    hint.ai_family = af;
    hint.ai_socktype = em_SOCK_DGRAM() | em_SOCK_STREAM();

    rc = getaddrinfo(nodecopy, NULL, &hint, &res);
    if (rc != 0)
        return EM_ERESOLVE;

    orig_res = res;

    /* Enumerate each item in the result */
    for (i=0; i<*count && res; res=res->ai_next) {
        /* Ignore unwanted address families */
        if (af!=EM_AF_UNSPEC && res->ai_family != af)
            continue;

        /* Store canonical name (possibly truncating the name) */
        if (res->ai_canonname) {
            em_ansi_strncpy(ai[i].ai_canonname, res->ai_canonname,
                    sizeof(ai[i].ai_canonname));
            ai[i].ai_canonname[sizeof(ai[i].ai_canonname)-1] = '\0';
        } else {
            em_ansi_strcpy(ai[i].ai_canonname, nodecopy);
        }

        /* Store address */
        EM_ASSERT_ON_FAIL(res->ai_addrlen <= sizeof(em_sockaddr), continue);
        em_memcpy(&ai[i].ai_addr, res->ai_addr, res->ai_addrlen);
        EM_SOCKADDR_RESET_LEN(&ai[i].ai_addr);

        /* Next slot */
        ++i;
    }

    *count = i;

    freeaddrinfo(orig_res);

    /* Done */
    return (*count > 0? EM_SUCC : EM_ERESOLVE);
#endif

#else	/* EM_SOCK_HAS_GETADDRINFO */
    em_bool_t has_addr = EM_FALSE;

    EMLIB_ASSERT_RETURN(count && *count, EM_EINVAL);

    EM_UNUSED_ARG(has_addr);

    if (af == EM_AF_INET || af == EM_AF_UNSPEC) {
        em_hostent he;
        unsigned i, max_count;
        emlib_ret_t status;

        status = em_gethostbyname(nodename, &he);
        if (status != EM_SUCC)
            return status;

        max_count = *count;
        *count = 0;

        em_bzero(ai, max_count * sizeof(em_addrinfo));

        for (i=0; he.h_addr_list[i] && *count<max_count; ++i) {
            em_ansi_strncpy(ai[*count].ai_canonname, he.h_name,
                    sizeof(ai[*count].ai_canonname));
            ai[*count].ai_canonname[sizeof(ai[*count].ai_canonname)-1] = '\0';

            ai[*count].ai_addr.ipv4.sin_family = EM_AF_INET;
            em_memcpy(&ai[*count].ai_addr.ipv4.sin_addr,
                    he.h_addr_list[i], he.h_length);
            EM_SOCKADDR_RESET_LEN(&ai[*count].ai_addr);

            (*count)++;
        }

        return (*count > 0? EM_SUCC : EM_ERESOLVE);

    } else {
        /* IPv6 is not supported */
        *count = 0;

        return EM_EIPV6NOTSUP;
    }
#endif	/* EM_SOCK_HAS_GETADDRINFO */
}

