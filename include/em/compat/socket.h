/*
 * File:    socket.h
 * Author:  Liu HongLiang<lhl_nciae@sina.cn>
 * Brief:   Provides all socket related functions,data types, error codes, etc.
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
#ifndef __EM_COMPAT_SOCKET_H__
#define __EM_COMPAT_SOCKET_H__

/**
 * @file socket.h
 */

#if defined(EM_HAS_SYS_TYPES_H) && EM_HAS_SYS_TYPES_H != 0
#  include <sys/types.h>
#endif

#if defined(EM_HAS_SYS_SOCKET_H) && EM_HAS_SYS_SOCKET_H != 0
#  include <sys/socket.h>
#endif

#if defined(EM_HAS_LINUX_SOCKET_H) && EM_HAS_LINUX_SOCKET_H != 0
#  include <linux/socket.h>
#endif

#if defined(EM_HAS_SYS_SELECT_H) && EM_HAS_SYS_SELECT_H != 0
#  include <sys/select.h>
#endif

#if defined(EM_HAS_NETINET_IN_H) && EM_HAS_NETINET_IN_H != 0
#  include <netinet/in.h>
#endif

#if defined(EM_HAS_NETINET_IN_SYSTM_H) && EM_HAS_NETINET_IN_SYSTM_H != 0
/* Required to include netinet/ip.h in FreeBSD 7.0 */
#  include <netinet/in_systm.h>
#endif

#if defined(EM_HAS_NETINET_IP_H) && EM_HAS_NETINET_IP_H != 0
/* To pull in IPTOS_* constants */
#  include <netinet/ip.h>
#endif

#if defined(EM_HAS_NETINET_TCP_H) && EM_HAS_NETINET_TCP_H != 0
/* To pull in TCP_NODELAY constants */
#  include <netinet/tcp.h>
#endif

#if defined(EM_HAS_NET_IF_H) && EM_HAS_NET_IF_H != 0
/* For interface enumeration in ip_helper */
#   include <net/if.h>
#endif

#if defined(EM_HAS_IFADDRS_H) && EM_HAS_IFADDRS_H != 0
/* Interface enum with getifaddrs() which works with IPv6 */
#   include <ifaddrs.h>
#endif

#if defined(EM_HAS_ARPA_INET_H) && EM_HAS_ARPA_INET_H != 0
#  include <arpa/inet.h>
#endif

#if defined(EM_HAS_SYS_IOCTL_H) && EM_HAS_SYS_IOCTL_H != 0
#  include <sys/ioctl.h>	/* FBIONBIO */
#endif

#if defined(EM_HAS_ERRNO_H) && EM_HAS_ERRNO_H != 0
#  include <errno.h>
#endif

#if defined(EM_HAS_NETDB_H) && EM_HAS_NETDB_H != 0
#  include <netdb.h>
#endif

#if defined(EM_HAS_UNISTD_H) && EM_HAS_UNISTD_H != 0
#  include <unistd.h>
#endif

#if defined(EM_HAS_SYS_FILIO_H) && EM_HAS_SYS_FILIO_H != 0
#   include <sys/filio.h>
#endif

#if defined(EM_HAS_SYS_SOCKIO_H) && EM_HAS_SYS_SOCKIO_H != 0
#   include <sys/sockio.h>
#endif


/*
 * Define common errors.
 */
#  define OSERR_EWOULDBLOCK    EWOULDBLOCK
#  define OSERR_EINPROGRESS    EINPROGRESS
#  define OSERR_ECONNRESET     ECONNRESET
#  define OSERR_ENOTCONN       ENOTCONN
#  define OSERR_EAFNOSUPPORT   EAFNOSUPPORT
#  define OSERR_ENOPROTOOPT    ENOPROTOOPT

/*
 * And undefine these..
 */
#undef s_addr
#undef s6_addr
#undef sin_zero

/*
 * This will finally be obsoleted, since it should be declared in
 * os_auto.h
 */
#if !defined(EM_HAS_SOCKLEN_T) || EM_HAS_SOCKLEN_T==0
typedef int socklen_t;
#endif

/* Regarding sin_len member of sockaddr_in:
 *  BSD systems (including MacOS X requires that the sin_len member of 
 *  sockaddr_in be set to sizeof(sockaddr_in), while other systems (Windows
 *  and Linux included) do not.
 *
 *  To maintain compatibility between systems, EMLIB will automatically
 *  set this field before invoking native OS socket API, and it will
 *  always reset the field to zero before returning em_sockaddr_in to
 *  application (such as in em_getsockname() and em_recvfrom()).
 *
 *  Application MUST always set this field to zero.
 *
 *  This way we can avoid hard to find problem such as when the socket 
 *  address is used as hash table key.
 */
#if defined(EM_SOCKADDR_HAS_LEN) && EM_SOCKADDR_HAS_LEN!=0
#   define EM_SOCKADDR_SET_LEN(addr,len) (((em_addr_hdr*)(addr))->sa_zero_len=(len))
#   define EM_SOCKADDR_RESET_LEN(addr)   (((em_addr_hdr*)(addr))->sa_zero_len=0)
#else
#   define EM_SOCKADDR_SET_LEN(addr,len) 
#   define EM_SOCKADDR_RESET_LEN(addr)
#endif

#endif	/* __EM_COMPAT_SOCKET_H__ */

