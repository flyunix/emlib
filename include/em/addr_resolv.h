/*
 * File:    addr_resolv.h
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
#ifndef __EM_ADDR_RESOLV_H__
#define __EM_ADDR_RESOLV_H__

/**
 * @file addr_resolv.h
 * @brief IP address resolution.
 */

#include <em/sock.h>

DECLS_BEGIN
/**
 * @defgroup em_addr_resolve Network Address Resolution
 * @ingroup EM_IO
 * @{
 *
 * This module provides function to resolve Internet address of the
 * specified host name. To resolve a particular host name, application
 * can just call #em_gethostbyname().
 *
 * Example:
 * <pre>
 *   ...
 *   em_hostent he;
 *   emlib_ret_t rc;
 *   em_str_t host = em_str("host.example.com");
 *   
 *   rc = em_gethostbyname( &host, &he);
 *   if (rc != EM_SUCCESS) {
 *      char errbuf[80];
 *      em_strerror( rc, errbuf, sizeof(errbuf));
 *      EM_LOG(2,("sample", "Unable to resolve host, error=%s", errbuf));
 *      return rc;
 *   }
 *
 *   // process address...
 *   addr.sin_addr.s_addr = *(em_uint32_t*)he.h_addr;
 *   ...
 * </pre>
 *
 * It's pretty simple really...
 */

/** This structure describes an Internet host address. */
typedef struct em_hostent
{
    char    *h_name;		/**< The official name of the host. */
    char   **h_aliases;		/**< Aliases list. */
    int	     h_addrtype;	/**< Host address type. */
    int	     h_length;		/**< Length of address. */
    char   **h_addr_list;	/**< List of addresses. */
} em_hostent;

/** Shortcut to h_addr_list[0] */
#define h_addr h_addr_list[0]

/** 
 * This structure describes address information em_getaddrinfo().
 */
typedef struct em_addrinfo
{
    char	 ai_canonname[EM_MAX_HOSTNAME]; /**< Canonical name for host*/
    em_sockaddr  ai_addr;			/**< Binary address.	    */
} em_addrinfo;


/**
 * This function fills the structure of type em_hostent for a given host name.
 * For host resolution function that also works with IPv6, please see
 * #em_getaddrinfo().
 *
 * @param name	    Host name to resolve. Specifying IPv4 address here
 *		    may fail on some platforms (e.g. Windows)
 * @param he	    The em_hostent structure to be filled. Note that
 *		    the pointers in this structure points to temporary
 *		    variables which value will be reset upon subsequent
 *		    invocation.
 *
 * @return	    EM_SUCCESS, or the appropriate error codes.
 */ 
EM_DECL(emlib_ret_t) em_gethostbyname(const em_str_t *name, em_hostent *he);


/**
 * Resolve the primary IP address of local host. 
 *
 * @param af	    The desired address family to query. Valid values
 *		    are em_AF_INET() or em_AF_INET6().
 * @param addr      On successful resolution, the address family and address
 *		    part of this socket address will be filled up with the host
 *		    IP address, in network byte order. Other parts of the socket
 *		    address are untouched.
 *
 * @return	    EM_SUCCESS on success, or the appropriate error code.
 */
EM_DECL(emlib_ret_t) em_gethostip(int af, em_sockaddr *addr);


/**
 * Get the interface IP address to send data to the specified destination.
 *
 * @param af	    The desired address family to query. Valid values
 *		    are em_AF_INET() or em_AF_INET6().
 * @param dst	    The destination host.
 * @param itf_addr  On successful resolution, the address family and address
 *		    part of this socket address will be filled up with the host
 *		    IP address, in network byte order. Other parts of the socket
 *		    address should be ignored.
 * @param allow_resolve   If \a dst may contain hostname (instead of IP
 * 		    address), specify whether hostname resolution should
 * 	            be performed. If not, default interface address will
 *  		    be returned.
 * @param p_dst_addr If not NULL, it will be filled with the IP address of
 * 		    the destination host.
 *
 * @return	    EM_SUCCESS on success, or the appropriate error code.
 */
EM_DECL(emlib_ret_t) em_getipinterface(int af,
        const em_str_t *dst,
        em_sockaddr *itf_addr,
        em_bool_t allow_resolve,
        em_sockaddr *p_dst_addr);

/**
 * Get the IP address of the default interface. Default interface is the
 * interface of the default route.
 *
 * @param af	    The desired address family to query. Valid values
 *		    are em_AF_INET() or em_AF_INET6().
 * @param addr      On successful resolution, the address family and address
 *		    part of this socket address will be filled up with the host
 *		    IP address, in network byte order. Other parts of the socket
 *		    address are untouched.
 *
 * @return	    EM_SUCCESS on success, or the appropriate error code.
 */
EM_DECL(emlib_ret_t) em_getdefaultipinterface(int af,
        em_sockaddr *addr);


/**
 * This function translates the name of a service location (for example, 
 * a host name) and returns a set of addresses and associated information
 * to be used in creating a socket with which to address the specified 
 * service.
 *
 * @param af	    The desired address family to query. Valid values
 *		    are em_AF_INET(), em_AF_INET6(), or em_AF_UNSPEC().
 * @param name	    Descriptive name or an address string, such as host
 *		    name.
 * @param count	    On input, it specifies the number of elements in
 *		    \a ai array. On output, this will be set with the
 *		    number of address informations found for the
 *		    specified name.
 * @param ai	    Array of address info to be filled with the information
 *		    about the host.
 *
 * @return	    EM_SUCCESS on success, or the appropriate error code.
 */
EM_DECL(emlib_ret_t) em_getaddrinfo(int af, const em_str_t *name,
        unsigned *count, em_addrinfo ai[]);



/** @} */

DECLS_END

#endif	/* __EM_ADDR_RESOLV_H__ */

