/*
 * File:    os_linux.h
 * Author:  Liu HongLiang<lhl_nciae@sina.cn>
 * Brief:   Describes Linux operating system specifics.
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
 * 2018-12-27 Liu HongLiang<lhl_nciae@sina.cn> created.
 *
 */

#ifndef __EM_COMPAT_OS_LINUX_H__
#define __EM_COMPAT_OS_LINUX_H__

#define EM_OS_NAME		    "linux"

#define EM_HAS_ARPA_INET_H	    1
#define EM_HAS_ASSERT_H		    1
#define EM_HAS_CTYPE_H		    1
#define EM_HAS_ERRNO_H		    1
#define EM_HAS_LINUX_SOCKET_H	    0
#define EM_HAS_MALLOC_H		    1
#define EM_HAS_NETDB_H		    1
#define EM_HAS_NETINET_IN_H	    1
#define EM_HAS_SETJMP_H		    1
#define EM_HAS_STDARG_H		    1
#define EM_HAS_STDDEF_H		    1
#define EM_HAS_STDIO_H		    1
#define EM_HAS_STDLIB_H		    1
#define EM_HAS_STRING_H		    1
#define EM_HAS_SYS_IOCTL_H	    1
#define EM_HAS_SYS_SELECT_H	    1
#define EM_HAS_SYS_SOCKET_H	    1
#define EM_HAS_SYS_TIME_H	    1
#define EM_HAS_SYS_TIMEB_H	    1
#define EM_HAS_SYS_TYPES_H	    1
#define EM_HAS_TIME_H		    1
#define EM_HAS_UNISTD_H		    1
#define EM_HAS_SEMAPHORE_H	    1

#define EM_HAS_MSWSOCK_H	    0
#define EM_HAS_WINSOCK_H	    0
#define EM_HAS_WINSOCK2_H	    0

#define EM_HAS_LOCALTIME_R	    1

#define EM_SOCK_HAS_INET_ATON   1

/* Set 1 if native sockaddr_in has sin_len member. 
 * Default: 0
 */
#define EM_SOCKADDR_HAS_LEN	    0

/**
 * If this macro is set, it tells select I/O Queue that select() needs to
 * be given correct value of nfds (i.e. largest fd + 1). This requires
 * select ioqueue to re-scan the descriptors on each registration and
 * unregistration.
 * If this macro is not set, then ioqueue will always give FD_SETSIZE for
 * nfds argument when calling select().
 *
 * Default: 0
 */
#define EM_SELECT_NEEDS_NFDS	    0

/* Is errno a good way to retrieve OS errors?
 */
#define EM_HAS_ERRNO_VAR	    1

/* When this macro is set, getsockopt(SOL_SOCKET, SO_ERROR) will return
 * the status of non-blocking connect() operation.
 */
#define EM_HAS_SO_ERROR             1

/* This value specifies the value set in errno by the OS when a non-blocking
 * socket recv() can not return immediate daata.
 */
#define EM_BLOCKING_ERROR_VAL       EAGAIN

/* This value specifies the value set in errno by the OS when a non-blocking
 * socket connect() can not get connected immediately.
 */
#define EM_BLOCKING_CONNECT_ERROR_VAL   EINPROGRESS

/* Default threading is enabled, unless it's overridden. */
#ifndef EM_HAS_THREADS
#  define EM_HAS_THREADS	    (1)
#endif

#define EM_HAS_HIGH_RES_TIMER	    1
#define EM_HAS_MALLOC               1
#ifndef EM_OS_HAS_CHECK_STACK
#   define EM_OS_HAS_CHECK_STACK    0
#endif
#define EM_NATIVE_STRING_IS_UNICODE 0

#define EM_ATOMIC_VALUE_TYPE	    long

/* If 1, use Read/Write mutex emulation for platforms that don't support it */
#define EM_EMULATE_RWMUTEX	    0

/* If 1, pj_thread_create() should enforce the stack size when creating 
 * threads.
 * Default: 0 (let OS decide the thread's stack size).
 */
#define EM_THREAD_SET_STACK_SIZE    	0

/* If 1, pj_thread_create() should allocate stack from the pool supplied.
 * Default: 0 (let OS allocate memory for thread's stack).
 */
#define EM_THREAD_ALLOCATE_STACK    	0

/* Linux has socklen_t */
#define EM_HAS_SOCKLEN_T		1


#endif	/* __EM_COMPAT_OS_LINUX_H__ */

