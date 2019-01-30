/*
 * File:    ip_helper.h
 * Author:  Liu HongLiang<lhl_nciae@sina.cn>
 * Brief:   IP Interface and Routing Helper
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
#ifndef __EM_IP_ROUTE_H__
#define __EM_IP_ROUTE_H__

/**
 * @file ip_helper.h
 * @brief IP helper API
 */

#include <em/sock.h>

DECLS_BEGIN

/**
 * @defgroup em_ip_helper IP Interface and Routing Helper
 * @ingroup EM_IO
 * @{
 *
 * This module provides functions to query local host's IP interface and 
 * routing table.
 */

/**
 * This structure describes IP routing entry.
 */
typedef union em_ip_route_entry
{
    /** IP routing entry for IP version 4 routing */
    struct
    {
        em_in_addr	if_addr;    /**< Local interface IP address.	*/
        em_in_addr	dst_addr;   /**< Destination IP address.	*/
        em_in_addr	mask;	    /**< Destination mask.		*/
    } ipv4;
} em_ip_route_entry;


/**
 * Enumerate the local IP interfaces currently active in the host.
 *
 * @param af	    Family of the address to be retrieved. Application
 *		    may specify em_AF_UNSPEC() to retrieve all addresses,
 *		    or em_AF_INET() or em_AF_INET6() to retrieve interfaces
 *		    with specific address family.
 * @param count	    On input, specify the number of entries. On output,
 *		    it will be filled with the actual number of entries.
 * @param ifs	    Array of socket addresses, which address part will
 *		    be filled with the interface address. The address
 *		    family part will be initialized with the address
 *		    family of the IP address.
 *
 * @return	    EM_SUCCESS on success, or the appropriate error code.
 */
EM_DECL(emlib_ret_t) em_enum_ip_interface(int af,
        unsigned *count,
        em_sockaddr ifs[]);


/**
 * Enumerate the IP routing table for this host.
 *
 * @param count	    On input, specify the number of routes entries. On output,
 *		    it will be filled with the actual number of route entries.
 * @param routes    Array of IP routing entries.
 *
 * @return	    EM_SUCCESS on success, or the appropriate error code.
 */
EM_DECL(emlib_ret_t) em_enum_ip_route(unsigned *count,
        em_ip_route_entry routes[]);



/** @} */

DECLS_END

#endif	/* __EM_IP_ROUTE_H__ */

