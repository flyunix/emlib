/*
 * File:    sock.h
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
#ifndef __EM_SOCK_H__
#define __EM_SOCK_H__

/**
 * @file sock.h
 * @brief Socket Abstraction.
 */

#include <em/types.h>

DECLS_BEGIN


/**
 * @defgroup EM_SOCK Socket Abstraction
 * @ingroup EM_IO
 * @{
 *
 * The EMLIB socket abstraction layer is a thin and very portable abstraction
 * for socket API. It provides API similar to BSD socket API. The abstraction
 * is needed because BSD socket API is not always available on all platforms,
 * therefore it wouldn't be possible to create a trully portable network
 * programs unless we provide such abstraction.
 *
 * Applications can use this API directly in their application, just
 * as they would when using traditional BSD socket API, provided they
 * call #em_init() first.
 *
 * \section em_sock_examples_sec Examples
 */


/**
 * Supported address families. 
 * APPLICATION MUST USE THESE VALUES INSTEAD OF NORMAL AF_*, BECAUSE
 * THE LIBRARY WILL DO TRANSLATION TO THE NATIVE VALUE.
 */

/** Address family is unspecified. @see em_AF_UNSPEC() */
extern const em_uint16_t EM_AF_UNSPEC;

/** Unix domain socket.	@see em_AF_UNIX()*/
extern const em_uint16_t EM_AF_UNIX;

/** POSIX name for AF_UNIX	*/
#define EM_AF_LOCAL	 EM_AF_UNIX;

/** Internet IP protocol. @see em_AF_INET() */
extern const em_uint16_t EM_AF_INET;

/** IP version 6. @see em_AF_INET6() */
extern const em_uint16_t EM_AF_INET6;

/** Packet family. @see em_AF_PACKET() */
extern const em_uint16_t EM_AF_PACKET;

/** IRDA sockets. @see em_AF_IRDA() */
extern const em_uint16_t EM_AF_IRDA;

/*
 * Accessor functions for various address family constants. These
 * functions are provided because Symbian doesn't allow exporting
 * global variables from a DLL.
 */

/* When emlib is not built as DLL, these accessor functions are
 * simply a macro to get their constants
 */
/** Get #EM_AF_UNSPEC value */
#   define em_AF_UNSPEC()   EM_AF_UNSPEC
/** Get #EM_AF_UNIX value. */
#   define em_AF_UNIX()	    EM_AF_UNIX
/** Get #EM_AF_INET value. */
#   define em_AF_INET()	    EM_AF_INET
/** Get #EM_AF_INET6 value. */
#   define em_AF_INET6()    EM_AF_INET6
/** Get #EM_AF_PACKET value. */
#   define em_AF_PACKET()   EM_AF_PACKET
/** Get #EM_AF_IRDA value. */
#   define em_AF_IRDA()	    EM_AF_IRDA


/**
 * Supported types of sockets.
 * APPLICATION MUST USE THESE VALUES INSTEAD OF NORMAL SOCK_*, BECAUSE
 * THE LIBRARY WILL TRANSLATE THE VALUE TO THE NATIVE VALUE.
 */

/** Sequenced, reliable, connection-based byte streams.
 *  @see em_SOCK_STREAM() */
extern const em_uint16_t EM_SOCK_STREAM;

/** Connectionless, unreliable datagrams of fixed maximum lengths.
 *  @see em_SOCK_DGRAM() */
extern const em_uint16_t EM_SOCK_DGRAM;

/** Raw protocol interface. @see em_SOCK_RAW() */
extern const em_uint16_t EM_SOCK_RAW;

/** Reliably-delivered messages.  @see em_SOCK_RDM() */
extern const em_uint16_t EM_SOCK_RDM;


/*
 * Accessor functions for various constants. These functions are provided
 * because Symbian doesn't allow exporting global variables from a DLL.
 */

/** Get #EM_SOCK_STREAM constant */
#   define em_SOCK_STREAM() EM_SOCK_STREAM
/** Get #EM_SOCK_DGRAM constant */
#   define em_SOCK_DGRAM()  EM_SOCK_DGRAM
/** Get #EM_SOCK_RAW constant */
#   define em_SOCK_RAW()    EM_SOCK_RAW
/** Get #EM_SOCK_RDM constant */
#   define em_SOCK_RDM()    EM_SOCK_RDM


/**
 * Socket level specified in #em_sock_setsockopt() or #em_sock_getsockopt().
 * APPLICATION MUST USE THESE VALUES INSTEAD OF NORMAL SOL_*, BECAUSE
 * THE LIBRARY WILL TRANSLATE THE VALUE TO THE NATIVE VALUE.
 */
/** Socket level. @see em_SOL_SOCKET() */
extern const em_uint16_t EM_SOL_SOCKET;
/** IP level. @see em_SOL_IP() */
extern const em_uint16_t EM_SOL_IP;
/** TCP level. @see em_SOL_TCP() */
extern const em_uint16_t EM_SOL_TCP;
/** UDP level. @see em_SOL_UDP() */
extern const em_uint16_t EM_SOL_UDP;
/** IP version 6. @see em_SOL_IPV6() */
extern const em_uint16_t EM_SOL_IPV6;

/*
 * Accessor functions for various constants. These functions are provided
 * because Symbian doesn't allow exporting global variables from a DLL.
 */

/** Get #EM_SOL_SOCKET constant */
#   define em_SOL_SOCKET()  EM_SOL_SOCKET
/** Get #EM_SOL_IP constant */
#   define em_SOL_IP()	    EM_SOL_IP
/** Get #EM_SOL_TCP constant */
#   define em_SOL_TCP()	    EM_SOL_TCP
/** Get #EM_SOL_UDP constant */
#   define em_SOL_UDP()	    EM_SOL_UDP
/** Get #EM_SOL_IPV6 constant */
#   define em_SOL_IPV6()    EM_SOL_IPV6


/* IP_TOS 
 *
 * Note:
 *  TOS CURRENTLY DOES NOT WORK IN Windows 2000 and above!
 *  See http://support.microsoft.com/kb/248611
 */
/** IP_TOS optname in setsockopt(). @see em_IP_TOS() */
extern const em_uint16_t EM_IP_TOS;

/*
 * IP TOS related constats.
 *
 * Note:
 *  TOS CURRENTLY DOES NOT WORK IN Windows 2000 and above!
 *  See http://support.microsoft.com/kb/248611
 */
/** Minimize delays. @see em_IPTOS_LOWDELAY() */
extern const em_uint16_t EM_IPTOS_LOWDELAY;

/** Optimize throughput. @see em_IPTOS_THROUGHPUT() */
extern const em_uint16_t EM_IPTOS_THROUGHPUT;

/** Optimize for reliability. @see em_IPTOS_RELIABILITY() */
extern const em_uint16_t EM_IPTOS_RELIABILITY;

/** "filler data" where slow transmission does't matter.
 *  @see em_IPTOS_MINCOST() */
extern const em_uint16_t EM_IPTOS_MINCOST;


/** Get #EM_IP_TOS constant */
#   define em_IP_TOS()		EM_IP_TOS

/** Get #EM_IPTOS_LOWDELAY constant */
#   define em_IPTOS_LOWDELAY()	EM_IP_TOS_LOWDELAY

/** Get #EM_IPTOS_THROUGHPUT constant */
#   define em_IPTOS_THROUGHPUT() EM_IP_TOS_THROUGHPUT

/** Get #EM_IPTOS_RELIABILITY constant */
#   define em_IPTOS_RELIABILITY() EM_IP_TOS_RELIABILITY

/** Get #EM_IPTOS_MINCOST constant */
#   define em_IPTOS_MINCOST()	EM_IP_TOS_MINCOST


/** IPV6_TCLASS optname in setsockopt(). @see em_IPV6_TCLASS() */
extern const em_uint16_t EM_IPV6_TCLASS;


/** Get #EM_IPV6_TCLASS constant */
#   define em_IPV6_TCLASS()	EM_IPV6_TCLASS


/**
 * Values to be specified as \c optname when calling #em_sock_setsockopt() 
 * or #em_sock_getsockopt().
 */

/** Socket type. @see em_SO_TYPE() */
extern const em_uint16_t EM_SO_TYPE;

/** Buffer size for receive. @see em_SO_RCVBUF() */
extern const em_uint16_t EM_SO_RCVBUF;

/** Buffer size for send. @see em_SO_SNDBUF() */
extern const em_uint16_t EM_SO_SNDBUF;

/** Disables the Nagle algorithm for send coalescing. @see em_TCP_NODELAY */
extern const em_uint16_t EM_TCP_NODELAY;

/** Allows the socket to be bound to an address that is already in use.
 *  @see em_SO_REUSEADDR */
extern const em_uint16_t EM_SO_REUSEADDR;

/** Do not generate SIGPIPE. @see em_SO_NOSIGPIPE */
extern const em_uint16_t EM_SO_NOSIGPIPE;

/** Set the protocol-defined priority for all packets to be sent on socket.
*/
extern const em_uint16_t EM_SO_PRIORITY;

/** IP multicast interface. @see em_IP_MULTICAST_IF() */
extern const em_uint16_t EM_IP_MULTICAST_IF;

/** IP multicast ttl. @see em_IP_MULTICAST_TTL() */
extern const em_uint16_t EM_IP_MULTICAST_TTL;

/** IP multicast loopback. @see em_IP_MULTICAST_LOOP() */
extern const em_uint16_t EM_IP_MULTICAST_LOOP;

/** Add an IP group membership. @see em_IP_ADD_MEMBERSHIP() */
extern const em_uint16_t EM_IP_ADD_MEMBERSHIP;

/** Drop an IP group membership. @see em_IP_DROP_MEMBERSHIP() */
extern const em_uint16_t EM_IP_DROP_MEMBERSHIP;


/** Get #EM_SO_TYPE constant */
#   define em_SO_TYPE()	    EM_SO_TYPE

/** Get #EM_SO_RCVBUF constant */
#   define em_SO_RCVBUF()   EM_SO_RCVBUF

/** Get #EM_SO_SNDBUF constant */
#   define em_SO_SNDBUF()   EM_SO_SNDBUF

/** Get #EM_TCP_NODELAY constant */
#   define em_TCP_NODELAY() EM_TCP_NODELAY

/** Get #EM_SO_REUSEADDR constant */
#   define em_SO_REUSEADDR() EM_SO_REUSEADDR

/** Get #EM_SO_NOSIGPIPE constant */
#   define em_SO_NOSIGPIPE() EM_SO_NOSIGPIPE

/** Get #EM_SO_PRIORITY constant */
#   define em_SO_PRIORITY() EM_SO_PRIORITY

/** Get #EM_IP_MULTICAST_IF constant */
#   define em_IP_MULTICAST_IF()    EM_IP_MULTICAST_IF

/** Get #EM_IP_MULTICAST_TTL constant */
#   define em_IP_MULTICAST_TTL()   EM_IP_MULTICAST_TTL

/** Get #EM_IP_MULTICAST_LOOP constant */
#   define em_IP_MULTICAST_LOOP()  EM_IP_MULTICAST_LOOP

/** Get #EM_IP_ADD_MEMBERSHIP constant */
#   define em_IP_ADD_MEMBERSHIP()  EM_IP_ADD_MEMBERSHIP

/** Get #EM_IP_DROP_MEMBERSHIP constant */
#   define em_IP_DROP_MEMBERSHIP() EM_IP_DROP_MEMBERSHIP


/*
 * Flags to be specified in #em_sock_recv, #em_sock_send, etc.
 */

/** Out-of-band messages. @see em_MSG_OOB() */
extern const int EM_MSG_OOB;

/** Peek, don't remove from buffer. @see em_MSG_PEEK() */
extern const int EM_MSG_PEEK;

/** Don't route. @see em_MSG_DONTROUTE() */
extern const int EM_MSG_DONTROUTE;


/** Get #EM_MSG_OOB constant */
#   define em_MSG_OOB()		EM_MSG_OOB

/** Get #EM_MSG_PEEK constant */
#   define em_MSG_PEEK()	EM_MSG_PEEK

/** Get #EM_MSG_DONTROUTE constant */
#   define em_MSG_DONTROUTE()	EM_MSG_DONTROUTE


/**
 * Flag to be specified in #em_sock_shutdown().
 */
typedef enum em_socket_sd_type
{
    EM_SD_RECEIVE   = 0,    /**< No more receive.	    */
    EM_SHUT_RD	    = 0,    /**< Alias for SD_RECEIVE.	    */
    EM_SD_SEND	    = 1,    /**< No more sending.	    */
    EM_SHUT_WR	    = 1,    /**< Alias for SD_SEND.	    */
    EM_SD_BOTH	    = 2,    /**< No more send and receive.  */
    EM_SHUT_RDWR    = 2     /**< Alias for SD_BOTH.	    */
} em_socket_sd_type;



/** Address to accept any incoming messages. */
#define EM_INADDR_ANY	    ((em_uint32_t)0)

/** Address indicating an error return */
#define EM_INADDR_NONE	    ((em_uint32_t)0xffffffff)

/** Address to send to all hosts. */
#define EM_INADDR_BROADCAST ((em_uint32_t)0xffffffff)


/** 
 * Maximum length specifiable by #em_sock_listen().
 * If the build system doesn't override this value, then the lowest 
 * denominator (five, in Win32 systems) will be used.
 */
#if !defined(EM_SOMAXCONN)
#  define EM_SOMAXCONN	5
#endif


/**
 * Constant for invalid socket returned by #em_sock_socket() and
 * #em_sock_accept().
 */
#define EM_INVALID_SOCKET   (-1)

/* Must undefine s_addr because of em_in_addr below */
#undef s_addr

/**
 * This structure describes Internet address.
 */
typedef struct em_in_addr
{
    em_uint32_t	s_addr;		/**< The 32bit IP address.	    */
} em_in_addr;


/**
 * Maximum length of text representation of an IPv4 address.
 */
#define EM_INET_ADDRSTRLEN	16

/**
 * Maximum length of text representation of an IPv6 address.
 */
#define EM_INET6_ADDRSTRLEN	46

/**
 * The size of sin_zero field in em_sockaddr_in structure. Most OSes
 * use 8, but others such as the BSD TCP/IP stack in eCos uses 24.
 */
#ifndef EM_SOCKADDR_IN_SIN_ZERO_LEN
#   define EM_SOCKADDR_IN_SIN_ZERO_LEN	8
#endif

/**
 * This structure describes Internet socket address.
 * If EM_SOCKADDR_HAS_LEN is not zero, then sin_zero_len member is added
 * to this struct. As far the application is concerned, the value of
 * this member will always be zero. Internally, EMLIB may modify the value
 * before calling OS socket API, and reset the value back to zero before
 * returning the struct to application.
 */
struct em_sockaddr_in
{
#if defined(EM_SOCKADDR_HAS_LEN) && EM_SOCKADDR_HAS_LEN!=0
    em_uint8_t  sin_zero_len;	/**< Just ignore this.		    */
    em_uint8_t  sin_family;	/**< Address family.		    */
#else
    em_uint16_t	sin_family;	/**< Address family.		    */
#endif
    em_uint16_t	sin_port;	/**< Transport layer port number.   */
    em_in_addr	sin_addr;	/**< IP address.		    */
    char	sin_zero[EM_SOCKADDR_IN_SIN_ZERO_LEN]; /**< Padding.*/
};


#undef s6_addr

/**
 * This structure describes IPv6 address.
 */
typedef union em_in6_addr
{
    /* This is the main entry */
    em_uint8_t  s6_addr[16];   /**< 8-bit array */

    /* While these are used for proper alignment */
    em_uint32_t	u6_addr32[4];

    /* Do not use this with Winsock2, as this will align em_sockaddr_in6
     * to 64-bit boundary and Winsock2 doesn't like it!
     * Update 26/04/2010:
     *  This is now disabled, see http://trac.emsip.org/repos/ticket/1058
     */
#if 0 && defined(EM_HAS_INT64) && EM_HAS_INT64!=0 && \
    (!defined(EM_WIN32) || EM_WIN32==0)
    em_int64_t	u6_addr64[2];
#endif

} em_in6_addr;


/** Initializer value for em_in6_addr. */
#define EM_IN6ADDR_ANY_INIT { { { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 } } }

/** Initializer value for em_in6_addr. */
#define EM_IN6ADDR_LOOPBACK_INIT { { { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 } } }

/**
 * This structure describes IPv6 socket address.
 * If EM_SOCKADDR_HAS_LEN is not zero, then sin_zero_len member is added
 * to this struct. As far the application is concerned, the value of
 * this member will always be zero. Internally, EMLIB may modify the value
 * before calling OS socket API, and reset the value back to zero before
 * returning the struct to application.
 */
typedef struct em_sockaddr_in6
{
#if defined(EM_SOCKADDR_HAS_LEN) && EM_SOCKADDR_HAS_LEN!=0
    em_uint8_t  sin6_zero_len;	    /**< Just ignore this.	   */
    em_uint8_t  sin6_family;	    /**< Address family.	   */
#else
    em_uint16_t	sin6_family;	    /**< Address family		    */
#endif
    em_uint16_t	sin6_port;	    /**< Transport layer port number. */
    em_uint32_t	sin6_flowinfo;	    /**< IPv6 flow information	    */
    em_in6_addr sin6_addr;	    /**< IPv6 address.		    */
    em_uint32_t sin6_scope_id;	    /**< Set of interfaces for a scope	*/
} em_sockaddr_in6;


/**
 * This structure describes common attributes found in transport addresses.
 * If EM_SOCKADDR_HAS_LEN is not zero, then sa_zero_len member is added
 * to this struct. As far the application is concerned, the value of
 * this member will always be zero. Internally, EMLIB may modify the value
 * before calling OS socket API, and reset the value back to zero before
 * returning the struct to application.
 */
typedef struct em_addr_hdr
{
#if defined(EM_SOCKADDR_HAS_LEN) && EM_SOCKADDR_HAS_LEN!=0
    em_uint8_t  sa_zero_len;
    em_uint8_t  sa_family;
#else
    em_uint16_t	sa_family;	/**< Common data: address family.   */
#endif
} em_addr_hdr;


/**
 * This union describes a generic socket address.
 */
typedef union em_sockaddr
{
    em_addr_hdr	    addr;	/**< Generic transport address.	    */
    em_sockaddr_in  ipv4;	/**< IPv4 transport address.	    */
    em_sockaddr_in6 ipv6;	/**< IPv6 transport address.	    */
} em_sockaddr;


/**
 * This structure provides multicast group information for IPv4 addresses.
 */
typedef struct em_ip_mreq {
    em_in_addr imr_multiaddr;	/**< IP multicast address of group. */
    em_in_addr imr_interface;	/**< local IP address of interface. */
} em_ip_mreq;


/**
 * Options to be set for the socket. 
 */
typedef struct em_sockopt_params
{
    /* The number of options to be applied. */
    unsigned cnt;

    /* Array of options to be applied. */
    struct {
        /* The level at which the option is defined. */
        int level;

        /* Option name. */
        int optname;

        /* Pointer to the buffer in which the option is specified. */
        void *optval;

        /* Buffer size of the buffer pointed by optval. */
        int optlen;
    } options[EM_MAX_SOCKOPT_PARAMS];
} em_sockopt_params;

/*****************************************************************************
 *
 * SOCKET ADDRESS MANIPULATION.
 *
 *****************************************************************************
 */

/**
 * Convert 16-bit value from network byte order to host byte order.
 *
 * @param netshort  16-bit network value.
 * @return	    16-bit host value.
 */
EM_DECL(em_uint16_t) em_ntohs(em_uint16_t netshort);

/**
 * Convert 16-bit value from host byte order to network byte order.
 *
 * @param hostshort 16-bit host value.
 * @return	    16-bit network value.
 */
EM_DECL(em_uint16_t) em_htons(em_uint16_t hostshort);

/**
 * Convert 32-bit value from network byte order to host byte order.
 *
 * @param netlong   32-bit network value.
 * @return	    32-bit host value.
 */
EM_DECL(em_uint32_t) em_ntohl(em_uint32_t netlong);

/**
 * Convert 32-bit value from host byte order to network byte order.
 *
 * @param hostlong  32-bit host value.
 * @return	    32-bit network value.
 */
EM_DECL(em_uint32_t) em_htonl(em_uint32_t hostlong);

/**
 * Convert an Internet host address given in network byte order
 * to string in standard numbers and dots notation.
 *
 * @param inaddr    The host address.
 * @return	    The string address.
 */
EM_DECL(char*) em_inet_ntoa(em_in_addr inaddr);

/**
 * This function converts the Internet host address cp from the standard
 * numbers-and-dots notation into binary data and stores it in the structure
 * that inp points to. 
 *
 * @param cp	IP address in standard numbers-and-dots notation.
 * @param inp	Structure that holds the output of the conversion.
 *
 * @return	nonzero if the address is valid, zero if not.
 */
EM_DECL(int) em_inet_aton(const em_str_t *cp, struct em_in_addr *inp);

/**
 * This function converts an address in its standard text presentation form
 * into its numeric binary form. It supports both IPv4 and IPv6 address
 * conversion.
 *
 * @param af	Specify the family of the address.  The EM_AF_INET and 
 *		EM_AF_INET6 address families shall be supported.  
 * @param src	Points to the string being passed in. 
 * @param dst	Points to a buffer into which the function stores the 
 *		numeric address; this shall be large enough to hold the
 *		numeric address (32 bits for EM_AF_INET, 128 bits for
 *		EM_AF_INET6).  
 *
 * @return	EM_SUCCESS if conversion was successful.
 */
EM_DECL(emlib_ret_t) em_inet_pton(int af, const em_str_t *src, void *dst);

/**
 * This function converts a numeric address into a text string suitable
 * for presentation. It supports both IPv4 and IPv6 address
 * conversion. 
 * @see em_sockaddr_print()
 *
 * @param af	Specify the family of the address. This can be EM_AF_INET
 *		or EM_AF_INET6.
 * @param src	Points to a buffer holding an IPv4 address if the af argument
 *		is EM_AF_INET, or an IPv6 address if the af argument is
 *		EM_AF_INET6; the address must be in network byte order.  
 * @param dst	Points to a buffer where the function stores the resulting
 *		text string; it shall not be NULL.  
 * @param size	Specifies the size of this buffer, which shall be large 
 *		enough to hold the text string (EM_INET_ADDRSTRLEN characters
 *		for IPv4, EM_INET6_ADDRSTRLEN characters for IPv6).
 *
 * @return	EM_SUCCESS if conversion was successful.
 */
EM_DECL(emlib_ret_t) em_inet_ntop(int af, const void *src,
        char *dst, int size);

/**
 * Converts numeric address into its text string representation.
 * @see em_sockaddr_print()
 *
 * @param af	Specify the family of the address. This can be EM_AF_INET
 *		or EM_AF_INET6.
 * @param src	Points to a buffer holding an IPv4 address if the af argument
 *		is EM_AF_INET, or an IPv6 address if the af argument is
 *		EM_AF_INET6; the address must be in network byte order.  
 * @param dst	Points to a buffer where the function stores the resulting
 *		text string; it shall not be NULL.  
 * @param size	Specifies the size of this buffer, which shall be large 
 *		enough to hold the text string (EM_INET_ADDRSTRLEN characters
 *		for IPv4, EM_INET6_ADDRSTRLEN characters for IPv6).
 *
 * @return	The address string or NULL if failed.
 */
EM_DECL(char*) em_inet_ntop2(int af, const void *src,
        char *dst, int size);

/**
 * Print socket address.
 *
 * @param addr	The socket address.
 * @param buf	Text buffer.
 * @param size	Size of buffer.
 * @param flags	Bitmask combination of these value:
 *		  - 1: port number is included.
 *		  - 2: square bracket is included for IPv6 address.
 *
 * @return	The address string.
 */
EM_DECL(char*) em_sockaddr_print(const em_sockaddr_t *addr,
        char *buf, int size,
        unsigned flags);

/**
 * Convert address string with numbers and dots to binary IP address.
 * 
 * @param cp	    The IP address in numbers and dots notation.
 * @return	    If success, the IP address is returned in network
 *		    byte order. If failed, EM_INADDR_NONE will be
 *		    returned.
 * @remark
 * This is an obsolete interface to #em_inet_aton(); it is obsolete
 * because -1 is a valid address (255.255.255.255), and #em_inet_aton()
 * provides a cleaner way to indicate error return.
 */
EM_DECL(em_in_addr) em_inet_addr(const em_str_t *cp);

/**
 * Convert address string with numbers and dots to binary IP address.
 * 
 * @param cp	    The IP address in numbers and dots notation.
 * @return	    If success, the IP address is returned in network
 *		    byte order. If failed, EM_INADDR_NONE will be
 *		    returned.
 * @remark
 * This is an obsolete interface to #em_inet_aton(); it is obsolete
 * because -1 is a valid address (255.255.255.255), and #em_inet_aton()
 * provides a cleaner way to indicate error return.
 */
EM_DECL(em_in_addr) em_inet_addr2(const char *cp);

/**
 * Initialize IPv4 socket address based on the address and port info.
 * The string address may be in a standard numbers and dots notation or 
 * may be a hostname. If hostname is specified, then the function will 
 * resolve the host into the IP address.
 *
 * @see em_sockaddr_init()
 *
 * @param addr	    The IP socket address to be set.
 * @param cp	    The address string, which can be in a standard 
 *		    dotted numbers or a hostname to be resolved.
 * @param port	    The port number, in host byte order.
 *
 * @return	    Zero on success.
 */
EM_DECL(emlib_ret_t) em_sockaddr_in_init( em_sockaddr_in *addr,
        const em_str_t *cp,
        em_uint16_t port);

/**
 * Initialize IP socket address based on the address and port info.
 * The string address may be in a standard numbers and dots notation or 
 * may be a hostname. If hostname is specified, then the function will 
 * resolve the host into the IP address.
 *
 * @see em_sockaddr_in_init()
 *
 * @param af	    Internet address family.
 * @param addr	    The IP socket address to be set.
 * @param cp	    The address string, which can be in a standard 
 *		    dotted numbers or a hostname to be resolved.
 * @param port	    The port number, in host byte order.
 *
 * @return	    Zero on success.
 */
EM_DECL(emlib_ret_t) em_sockaddr_init(int af, 
        em_sockaddr *addr,
        const em_str_t *cp,
        em_uint16_t port);

/**
 * Compare two socket addresses.
 *
 * @param addr1	    First address.
 * @param addr2	    Second address.
 *
 * @return	    Zero on equal, -1 if addr1 is less than addr2,
 *		    and +1 if addr1 is more than addr2.
 */
EM_DECL(int) em_sockaddr_cmp(const em_sockaddr_t *addr1,
        const em_sockaddr_t *addr2);

/**
 * Get pointer to the address part of a socket address.
 * 
 * @param addr	    Socket address.
 *
 * @return	    Pointer to address part (sin_addr or sin6_addr,
 *		    depending on address family)
 */
EM_DECL(void*) em_sockaddr_get_addr(const em_sockaddr_t *addr);

/**
 * Check that a socket address contains a non-zero address part.
 *
 * @param addr	    Socket address.
 *
 * @return	    Non-zero if address is set to non-zero.
 */
EM_DECL(em_bool_t) em_sockaddr_has_addr(const em_sockaddr_t *addr);

/**
 * Get the address part length of a socket address, based on its address
 * family. For EM_AF_INET, the length will be sizeof(em_in_addr), and
 * for EM_AF_INET6, the length will be sizeof(em_in6_addr).
 * 
 * @param addr	    Socket address.
 *
 * @return	    Length in bytes.
 */
EM_DECL(unsigned) em_sockaddr_get_addr_len(const em_sockaddr_t *addr);

/**
 * Get the socket address length, based on its address
 * family. For EM_AF_INET, the length will be sizeof(em_sockaddr_in), and
 * for EM_AF_INET6, the length will be sizeof(em_sockaddr_in6).
 * 
 * @param addr	    Socket address.
 *
 * @return	    Length in bytes.
 */
EM_DECL(unsigned) em_sockaddr_get_len(const em_sockaddr_t *addr);

/** 
 * Copy only the address part (sin_addr/sin6_addr) of a socket address.
 *
 * @param dst	    Destination socket address.
 * @param src	    Source socket address.
 *
 * @see @em_sockaddr_cp()
 */
EM_DECL(void) em_sockaddr_copy_addr(em_sockaddr *dst,
        const em_sockaddr *src);
/**
 * Copy socket address. This will copy the whole structure depending
 * on the address family of the source socket address.
 *
 * @param dst	    Destination socket address.
 * @param src	    Source socket address.
 *
 * @see @em_sockaddr_copy_addr()
 */
EM_DECL(void) em_sockaddr_cp(em_sockaddr_t *dst, const em_sockaddr_t *src);

/*
 * If the source's and desired address family matches, copy the address,
 * otherwise synthesize a new address with the desired address family,
 * from the source address. This can be useful to generate an IPv4-mapped
 * IPv6 address.
 *
 * @param dst_af    Desired address family.
 * @param dst	    Destination socket address, invalid if synthesis is
 *		    required and failed.
 * @param src	    Source socket address.
 *
 * @return	    EM_SUCCESS on success, or the error status
 *		    if synthesis is required and failed.
 */
EM_DECL(emlib_ret_t) em_sockaddr_synthesize(int dst_af,
        em_sockaddr_t *dst,
        const em_sockaddr_t *src);

/**
 * Get the IP address of an IPv4 socket address.
 * The address is returned as 32bit value in host byte order.
 *
 * @param addr	    The IP socket address.
 * @return	    32bit address, in host byte order.
 */
EM_DECL(em_in_addr) em_sockaddr_in_get_addr(const em_sockaddr_in *addr);

/**
 * Set the IP address of an IPv4 socket address.
 *
 * @param addr	    The IP socket address.
 * @param hostaddr  The host address, in host byte order.
 */
EM_DECL(void) em_sockaddr_in_set_addr(em_sockaddr_in *addr,
        em_uint32_t hostaddr);

/**
 * Set the IP address of an IP socket address from string address, 
 * with resolving the host if necessary. The string address may be in a
 * standard numbers and dots notation or may be a hostname. If hostname
 * is specified, then the function will resolve the host into the IP
 * address.
 *
 * @see em_sockaddr_set_str_addr()
 *
 * @param addr	    The IP socket address to be set.
 * @param cp	    The address string, which can be in a standard 
 *		    dotted numbers or a hostname to be resolved.
 *
 * @return	    EM_SUCCESS on success.
 */
EM_DECL(emlib_ret_t) em_sockaddr_in_set_str_addr( em_sockaddr_in *addr,
        const em_str_t *cp);

/**
 * Set the IP address of an IPv4 or IPv6 socket address from string address,
 * with resolving the host if necessary. The string address may be in a
 * standard IPv6 or IPv6 address or may be a hostname. If hostname
 * is specified, then the function will resolve the host into the IP
 * address according to the address family.
 *
 * @param af	    Address family.
 * @param addr	    The IP socket address to be set.
 * @param cp	    The address string, which can be in a standard 
 *		    IP numbers (IPv4 or IPv6) or a hostname to be resolved.
 *
 * @return	    EM_SUCCESS on success.
 */
EM_DECL(emlib_ret_t) em_sockaddr_set_str_addr(int af,
        em_sockaddr *addr,
        const em_str_t *cp);

/**
 * Get the port number of a socket address, in host byte order. 
 * This function can be used for both IPv4 and IPv6 socket address.
 * 
 * @param addr	    Socket address.
 *
 * @return	    Port number, in host byte order.
 */
EM_DECL(em_uint16_t) em_sockaddr_get_port(const em_sockaddr_t *addr);

/**
 * Get the transport layer port number of an Internet socket address.
 * The port is returned in host byte order.
 *
 * @param addr	    The IP socket address.
 * @return	    Port number, in host byte order.
 */
EM_DECL(em_uint16_t) em_sockaddr_in_get_port(const em_sockaddr_in *addr);

/**
 * Set the port number of an Internet socket address.
 *
 * @param addr	    The socket address.
 * @param hostport  The port number, in host byte order.
 */
EM_DECL(emlib_ret_t) em_sockaddr_set_port(em_sockaddr *addr, 
        em_uint16_t hostport);

/**
 * Set the port number of an IPv4 socket address.
 *
 * @see em_sockaddr_set_port()
 *
 * @param addr	    The IP socket address.
 * @param hostport  The port number, in host byte order.
 */
EM_DECL(void) em_sockaddr_in_set_port(em_sockaddr_in *addr, 
        em_uint16_t hostport);

/**
 * Parse string containing IP address and optional port into socket address,
 * possibly also with address family detection. This function supports both
 * IPv4 and IPv6 parsing, however IPv6 parsing may only be done if IPv6 is
 * enabled during compilation.
 *
 * This function supports parsing several formats. Sample IPv4 inputs and
 * their default results::
 *  - "10.0.0.1:80": address 10.0.0.1 and port 80.
 *  - "10.0.0.1": address 10.0.0.1 and port zero.
 *  - "10.0.0.1:": address 10.0.0.1 and port zero.
 *  - "10.0.0.1:0": address 10.0.0.1 and port zero.
 *  - ":80": address 0.0.0.0 and port 80.
 *  - ":": address 0.0.0.0 and port 0.
 *  - "localhost": address 127.0.0.1 and port 0.
 *  - "localhost:": address 127.0.0.1 and port 0.
 *  - "localhost:80": address 127.0.0.1 and port 80.
 *
 * Sample IPv6 inputs and their default results:
 *  - "[fec0::01]:80": address fec0::01 and port 80
 *  - "[fec0::01]": address fec0::01 and port 0
 *  - "[fec0::01]:": address fec0::01 and port 0
 *  - "[fec0::01]:0": address fec0::01 and port 0
 *  - "fec0::01": address fec0::01 and port 0
 *  - "fec0::01:80": address fec0::01:80 and port 0
 *  - "::": address zero (::) and port 0
 *  - "[::]": address zero (::) and port 0
 *  - "[::]:": address zero (::) and port 0
 *  - ":::": address zero (::) and port 0
 *  - "[::]:80": address zero (::) and port 0
 *  - ":::80": address zero (::) and port 80
 *
 * Note: when the IPv6 socket address contains port number, the IP 
 * part of the socket address should be enclosed with square brackets, 
 * otherwise the port number will be included as part of the IP address
 * (see "fec0::01:80" example above).
 *
 * @param af	    Optionally specify the address family to be used. If the
 *		    address family is to be deducted from the input, specify
 *		    em_AF_UNSPEC() here. Other supported values are
 *		    #em_AF_INET() and #em_AF_INET6()
 * @param options   Additional options to assist the parsing, must be zero
 *		    for now.
 * @param str	    The input string to be parsed.
 * @param addr	    Pointer to store the result.
 *
 * @return	    EM_SUCCESS if the parsing is successful.
 *
 * @see em_sockaddr_parse2()
 */
EM_DECL(emlib_ret_t) em_sockaddr_parse(int af, unsigned options,
        const em_str_t *str,
        em_sockaddr *addr);

/**
 * This function is similar to #em_sockaddr_parse(), except that it will not
 * convert the hostpart into IP address (thus possibly resolving the hostname
 * into a #em_sockaddr. 
 *
 * Unlike #em_sockaddr_parse(), this function has a limitation that if port 
 * number is specified in an IPv6 input string, the IP part of the IPv6 socket
 * address MUST be enclosed in square brackets, otherwise the port number will
 * be considered as part of the IPv6 IP address.
 *
 * @param af	    Optionally specify the address family to be used. If the
 *		    address family is to be deducted from the input, specify
 *		    #em_AF_UNSPEC() here. Other supported values are
 *		    #em_AF_INET() and #em_AF_INET6()
 * @param options   Additional options to assist the parsing, must be zero
 *		    for now.
 * @param str	    The input string to be parsed.
 * @param hostpart  Optional pointer to store the host part of the socket 
 *		    address, with any brackets removed.
 * @param port	    Optional pointer to store the port number. If port number
 *		    is not found, this will be set to zero upon return.
 * @param raf	    Optional pointer to store the detected address family of
 *		    the input address.
 *
 * @return	    EM_SUCCESS if the parsing is successful.
 *
 * @see em_sockaddr_parse()
 */
EM_DECL(emlib_ret_t) em_sockaddr_parse2(int af, unsigned options,
        const em_str_t *str,
        em_str_t *hostpart,
        em_uint16_t *port,
        int *raf);

/*****************************************************************************
 *
 * HOST NAME AND ADDRESS.
 *
 *****************************************************************************
 */

/**
 * Get system's host name.
 *
 * @return	    The hostname, or empty string if the hostname can not
 *		    be identified.
 */
EM_DECL(const em_str_t*) em_gethostname(void);

/**
 * Get host's IP address, which the the first IP address that is resolved
 * from the hostname.
 *
 * @return	    The host's IP address, EM_INADDR_NONE if the host
 *		    IP address can not be identified.
 */
EM_DECL(em_in_addr) em_gethostaddr(void);


/*****************************************************************************
 *
 * SOCKET API.
 *
 *****************************************************************************
 */

/**
 * Create new socket/endpoint for communication.
 *
 * @param family    Specifies a communication domain; this selects the
 *		    protocol family which will be used for communication.
 * @param type	    The socket has the indicated type, which specifies the 
 *		    communication semantics.
 * @param protocol  Specifies  a  particular  protocol  to  be used with the
 *		    socket.  Normally only a single protocol exists to support 
 *		    a particular socket  type  within  a given protocol family, 
 *		    in which a case protocol can be specified as 0.
 * @param sock	    New socket descriptor, or EM_INVALID_SOCKET on error.
 *
 * @return	    Zero on success.
 */
EM_DECL(emlib_ret_t) em_sock_socket(int family, 
        int type, 
        int protocol,
        em_sock_t *sock);

/**
 * Close the socket descriptor.
 *
 * @param sockfd    The socket descriptor.
 *
 * @return	    Zero on success.
 */
EM_DECL(emlib_ret_t) em_sock_close(em_sock_t sockfd);


/**
 * This function gives the socket sockfd the local address my_addr. my_addr is
 * addrlen bytes long.  Traditionally, this is called assigning a name to
 * a socket. When a socket is created with #em_sock_socket(), it exists in a
 * name space (address family) but has no name assigned.
 *
 * @param sockfd    The socket desriptor.
 * @param my_addr   The local address to bind the socket to.
 * @param addrlen   The length of the address.
 *
 * @return	    Zero on success.
 */
EM_DECL(emlib_ret_t) em_sock_bind( em_sock_t sockfd, 
        const em_sockaddr_t *my_addr,
        int addrlen);

/**
 * Bind the IP socket sockfd to the given address and port.
 *
 * @param sockfd    The socket descriptor.
 * @param addr	    Local address to bind the socket to, in host byte order.
 * @param port	    The local port to bind the socket to, in host byte order.
 *
 * @return	    Zero on success.
 */
EM_DECL(emlib_ret_t) em_sock_bind_in( em_sock_t sockfd, 
        em_uint32_t addr,
        em_uint16_t port);

/**
 * Bind the IP socket sockfd to the given address and a random port in the
 * specified range.
 *
 * @param sockfd    	The socket desriptor.
 * @param addr      	The local address and port to bind the socket to.
 * @param port_range	The port range, relative the to start port number
 * 			specified in port field in #addr. Note that if the
 * 			port is zero, this param will be ignored.
 * @param max_try   	Maximum retries.
 *
 * @return	    	Zero on success.
 */
EM_DECL(emlib_ret_t) em_sock_bind_random( em_sock_t sockfd,
        const em_sockaddr_t *addr,
        em_uint16_t port_range,
        em_uint16_t max_try);

#if EM_HAS_TCP
/**
 * Listen for incoming connection. This function only applies to connection
 * oriented sockets (such as EM_SOCK_STREAM or EM_SOCK_SEQPACKET), and it
 * indicates the willingness to accept incoming connections.
 *
 * @param sockfd	The socket descriptor.
 * @param backlog	Defines the maximum length the queue of pending
 *			connections may grow to.
 *
 * @return		Zero on success.
 */
EM_DECL(emlib_ret_t) em_sock_listen( em_sock_t sockfd, 
        int backlog );

/**
 * Accept new connection on the specified connection oriented server socket.
 *
 * @param serverfd  The server socket.
 * @param newsock   New socket on success, of EM_INVALID_SOCKET if failed.
 * @param addr	    A pointer to sockaddr type. If the argument is not NULL,
 *		    it will be filled by the address of connecting entity.
 * @param addrlen   Initially specifies the length of the address, and upon
 *		    return will be filled with the exact address length.
 *
 * @return	    Zero on success, or the error number.
 */
EM_DECL(emlib_ret_t) em_sock_accept( em_sock_t serverfd,
        em_sock_t *newsock,
        em_sockaddr_t *addr,
        int *addrlen);
#endif

/**
 * The file descriptor sockfd must refer to a socket.  If the socket is of
 * type EM_SOCK_DGRAM  then the serv_addr address is the address to which
 * datagrams are sent by default, and the only address from which datagrams
 * are received. If the socket is of type EM_SOCK_STREAM or EM_SOCK_SEQPACKET,
 * this call attempts to make a connection to another socket.  The
 * other socket is specified by serv_addr, which is an address (of length
 * addrlen) in the communications space of the  socket.  Each  communications
 * space interprets the serv_addr parameter in its own way.
 *
 * @param sockfd	The socket descriptor.
 * @param serv_addr	Server address to connect to.
 * @param addrlen	The length of server address.
 *
 * @return		Zero on success.
 */
EM_DECL(emlib_ret_t) em_sock_connect( em_sock_t sockfd,
        const em_sockaddr_t *serv_addr,
        int addrlen);

/**
 * Return the address of peer which is connected to socket sockfd.
 *
 * @param sockfd	The socket descriptor.
 * @param addr		Pointer to sockaddr structure to which the address
 *			will be returned.
 * @param namelen	Initially the length of the addr. Upon return the value
 *			will be set to the actual length of the address.
 *
 * @return		Zero on success.
 */
EM_DECL(emlib_ret_t) em_sock_getpeername(em_sock_t sockfd,
        em_sockaddr_t *addr,
        int *namelen);

/**
 * Return the current name of the specified socket.
 *
 * @param sockfd	The socket descriptor.
 * @param addr		Pointer to sockaddr structure to which the address
 *			will be returned.
 * @param namelen	Initially the length of the addr. Upon return the value
 *			will be set to the actual length of the address.
 *
 * @return		Zero on success.
 */
EM_DECL(emlib_ret_t) em_sock_getsockname( em_sock_t sockfd,
        em_sockaddr_t *addr,
        int *namelen);

/**
 * Get socket option associated with a socket. Options may exist at multiple
 * protocol levels; they are always present at the uppermost socket level.
 *
 * @param sockfd	The socket descriptor.
 * @param level		The level which to get the option from.
 * @param optname	The option name.
 * @param optval	Identifies the buffer which the value will be
 *			returned.
 * @param optlen	Initially contains the length of the buffer, upon
 *			return will be set to the actual size of the value.
 *
 * @return		Zero on success.
 */
EM_DECL(emlib_ret_t) em_sock_getsockopt( em_sock_t sockfd,
        em_uint16_t level,
        em_uint16_t optname,
        void *optval,
        int *optlen);
/**
 * Manipulate the options associated with a socket. Options may exist at 
 * multiple protocol levels; they are always present at the uppermost socket 
 * level.
 *
 * @param sockfd	The socket descriptor.
 * @param level		The level which to get the option from.
 * @param optname	The option name.
 * @param optval	Identifies the buffer which contain the value.
 * @param optlen	The length of the value.
 *
 * @return		EM_SUCCESS or the status code.
 */
EM_DECL(emlib_ret_t) em_sock_setsockopt( em_sock_t sockfd,
        em_uint16_t level,
        em_uint16_t optname,
        const void *optval,
        int optlen);

/**
 * Set socket options associated with a socket. This method will apply all the 
 * options specified, and ignore any errors that might be raised.
 *
 * @param sockfd	The socket descriptor.
 * @param params	The socket options.
 *
 * @return		EM_SUCCESS or the last error code. 
 */
EM_DECL(emlib_ret_t) em_sock_setsockopt_params( em_sock_t sockfd,
        const em_sockopt_params *params);					       

/**
 * Helper function to set socket buffer size using #em_sock_setsockopt()
 * with capability to auto retry with lower buffer setting value until
 * the highest possible value is successfully set.
 *
 * @param sockfd	The socket descriptor.
 * @param optname	The option name, valid values are em_SO_RCVBUF()
 *			and em_SO_SNDBUF().
 * @param auto_retry	Option whether auto retry with lower value is
 *			enabled.
 * @param buf_size	On input, specify the prefered buffer size setting,
 *			on output, the buffer size setting applied.
 *
 * @return		EM_SUCCESS or the status code.
 */
EM_DECL(emlib_ret_t) em_sock_setsockopt_sobuf( em_sock_t sockfd,
        em_uint16_t optname,
        em_bool_t auto_retry,
        unsigned *buf_size);


/**
 * Receives data stream or message coming to the specified socket.
 *
 * @param sockfd	The socket descriptor.
 * @param buf		The buffer to receive the data or message.
 * @param len		On input, the length of the buffer. On return,
 *			contains the length of data received.
 * @param flags		Flags (such as em_MSG_PEEK()).
 *
 * @return		EM_SUCCESS or the error code.
 */
EM_DECL(emlib_ret_t) em_sock_recv(em_sock_t sockfd,
        void *buf,
        em_ssize_t *len,
        unsigned flags);

/**
 * Receives data stream or message coming to the specified socket.
 *
 * @param sockfd	The socket descriptor.
 * @param buf		The buffer to receive the data or message.
 * @param len		On input, the length of the buffer. On return,
 *			contains the length of data received.
 * @param flags		Flags (such as em_MSG_PEEK()).
 * @param from		If not NULL, it will be filled with the source
 *			address of the connection.
 * @param fromlen	Initially contains the length of from address,
 *			and upon return will be filled with the actual
 *			length of the address.
 *
 * @return		EM_SUCCESS or the error code.
 */
EM_DECL(emlib_ret_t) em_sock_recvfrom( em_sock_t sockfd,
        void *buf,
        em_ssize_t *len,
        unsigned flags,
        em_sockaddr_t *from,
        int *fromlen);

/**
 * Transmit data to the socket.
 *
 * @param sockfd	Socket descriptor.
 * @param buf		Buffer containing data to be sent.
 * @param len		On input, the length of the data in the buffer.
 *			Upon return, it will be filled with the length
 *			of data sent.
 * @param flags		Flags (such as em_MSG_DONTROUTE()).
 *
 * @return		EM_SUCCESS or the status code.
 */
EM_DECL(emlib_ret_t) em_sock_send(em_sock_t sockfd,
        const void *buf,
        em_ssize_t *len,
        unsigned flags);

/**
 * Transmit data to the socket to the specified address.
 *
 * @param sockfd	Socket descriptor.
 * @param buf		Buffer containing data to be sent.
 * @param len		On input, the length of the data in the buffer.
 *			Upon return, it will be filled with the length
 *			of data sent.
 * @param flags		Flags (such as em_MSG_DONTROUTE()).
 * @param to		The address to send.
 * @param tolen		The length of the address in bytes.
 *
 * @return		EM_SUCCESS or the status code.
 */
EM_DECL(emlib_ret_t) em_sock_sendto(em_sock_t sockfd,
        const void *buf,
        em_ssize_t *len,
        unsigned flags,
        const em_sockaddr_t *to,
        int tolen);

#if EM_HAS_TCP
/**
 * The shutdown call causes all or part of a full-duplex connection on the
 * socket associated with sockfd to be shut down.
 *
 * @param sockfd	The socket descriptor.
 * @param how		If how is EM_SHUT_RD, further receptions will be 
 *			disallowed. If how is EM_SHUT_WR, further transmissions
 *			will be disallowed. If how is EM_SHUT_RDWR, further 
 *			receptions andtransmissions will be disallowed.
 *
 * @return		Zero on success.
 */
EM_DECL(emlib_ret_t) em_sock_shutdown( em_sock_t sockfd,
        int how);
#endif

/*****************************************************************************
 *
 * Utilities.
 *
 *****************************************************************************
 */

/**
 * Print socket address string. This method will enclose the address string 
 * with square bracket if it's IPv6 address.
 *
 * @param host_str  The host address string.
 * @param port	    The port address.
 * @param buf	    Text buffer.
 * @param size	    Size of buffer.
 * @param flags	    Bitmask combination of these value:
 *		    - 1: port number is included. 
 *
 * @return	The address string.
 */
EM_DECL(char *) em_addr_str_print( const em_str_t *host_str, int port, 
        char *buf, int size, unsigned flag);


/**
 * @}
 */


DECLS_END

#endif	/* __EM_SOCK_H__ */

