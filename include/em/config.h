/*
 * File:    config.h
 * Author:  Liu HongLiang<lhl_nciae@sina.cn>
 * Brief:   Emlib config.
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
 */

/*
 * History:
 * ================================================================
 * 2018-12-20 Liu HongLiang<lhl_nciae@sina.cn> created.
 */

#ifndef __EM_CONFIG_H_
#define __EM_CONFIG_H_

/********************************************************************
 * Include compiler specific configuration.
 */
#if defined(__GNUC__)
#  include "compat/cc_gcc.h"
#else
#  error "Unknown compiler."
#endif

/********************************************************************
 * Include target OS specific configuration.
 */

#if defined(EM_LINUX) || defined(linux) || defined(__linux)
    /*
     * Linux
     */
#   undef EM_LINUX
#   define EM_LINUX	    1
#   include "em/compat/os_linux.h"
#endif

/********************************************************************
 * Target machine specific configuration.
 */

#if defined (EM_M_I386) || defined(_i386_) || defined(i_386_) || \
	defined(_X86_) || defined(x86) || defined(__i386__) || \
	defined(__i386) || defined(_M_IX86) || defined(__I86__)
    /*
     * Generic i386 processor family, little-endian
     */
#   undef EM_M_I386
#   define EM_M_I386		1
#   define EM_M_NAME		"i386"
#   define EM_HAS_PENTIUM	1
#   define EM_IS_LITTLE_ENDIAN	1
#   define EM_IS_BIG_ENDIAN	0


#elif defined (EM_M_X86_64) || defined(__amd64__) || defined(__amd64) || \
	defined(__x86_64__) || defined(__x86_64) || \
	defined(_M_X64) || defined(_M_AMD64)
    /*
     * AMD 64bit processor, little endian
     */
#   undef EM_X86_64
#   define EM_M_X86_64		1
#   define EM_M_NAME		"x86_64"
#   define EM_HAS_PENTIUM	1
#   define EM_IS_LITTLE_ENDIAN	1
#   define EM_IS_BIG_ENDIAN	0

#elif defined(ARM) || defined(_ARM_) ||  defined(__arm__) || defined(_M_ARM)
#   define EM_HAS_PENTIUM	0

#   define EM_IS_LITTLE_ENDIAN	1
#   define EM_IS_BIG_ENDIAN	0
    /*
     * ARM, bi-endian, so raise error if endianness is not configured
     */
#   if !EM_IS_LITTLE_ENDIAN && !EM_IS_BIG_ENDIAN
#   	error Endianness must be declared for this processor
#   endif


#   if defined (EM_M_ARMV7) || defined(ARMV7)
#	undef EM_M_ARMV7
#	define EM_M_ARM7		1
#	define EM_M_NAME		"armv7"
#   elif defined (EM_M_ARMV4) || defined(ARMV4)
#	undef EM_M_ARMV4
#	define EM_M_ARMV4		1
#	define EM_M_NAME		"armv4"
#   endif 

#else
#   error "Please specify target machine."
#endif

/**
 * @defgroup em_config Build Configuration
 * @{
 *
 * This section contains macros that can set during EMLIB build process
 * to controll various aspects of the library.
 *
 * <b>Note</b>: the values in this page does NOT necessarily reflect to the
 * macro values during the build process.
 */

/**
 * If this macro is set to 1, it will enable some debugging checking
 * in the library.
 *
 * Default: equal to (NOT NDEBUG).
 */
#ifndef EM_DEBUG
#  ifndef NDEBUG
#    define EM_DEBUG		    1
#  else
#    define EM_DEBUG		    0
#  endif
#endif

/**
 * Expand functions in *_i.h header files as inline.
 *
 * Default: 0.
 */
#ifndef EM_FUNCTIONS_ARE_INLINED
#  define EM_FUNCTIONS_ARE_INLINED  0
#endif

/**
 * Is native platform error positive number?
 * Default: 1 (yes)
 */
#ifndef EM_NATIVE_ERR_POSITIVE
#   define EM_NATIVE_ERR_POSITIVE   1
#endif

/**
 * Specify this as \a stack_size argument in #em_thread_create() to specify
 * that thread should use default stack size for the current platform.
 *
 * Default: 8192
 */
#ifndef EM_THREAD_DEFAULT_STACK_SIZE 
#  define EM_THREAD_DEFAULT_STACK_SIZE    8192
#endif


/**
 * Specify if EM_CHECK_STACK() macro is enabled to check the sanity of 
 * the stack. The OS implementation may check that no stack overflow 
 * occurs, and it also may collect statistic about stack usage. Note
 * that this will increase the footprint of the libraries since it
 * tracks the filename and line number of each functions.
 */
#ifndef EM_OS_HAS_CHECK_STACK
#	define EM_OS_HAS_CHECK_STACK		0
#endif

/**
 * If pool debugging is used, then each memory allocation from the pool
 * will call malloc(), and pool will release all memory chunks when it
 * is destroyed. This works better when memory verification programs
 * such as Rational Purify is used.
 *
 * Default: 0
 */
#ifndef EM_POOL_DEBUG
#  define EM_POOL_DEBUG		    0
#endif

/**
 * Enable timer heap debugging facility. When this is enabled, application
 * can call em_timer_heap_dump() to show the contents of the timer heap
 * along with the source location where the timer entries were scheduled.
 * See https://trac.emsip.org/repos/ticket/1527 for more info.
 *
 * Default: 0
 */
#ifndef EM_TIMER_DEBUG
#  define EM_TIMER_DEBUG	    0
#endif

/**
 * Include error message string in the library (em_strerror()).
 * This is very much desirable!
 *
 * Default: 1
 */
#ifndef EM_HAS_ERROR_STRING
#   define EM_HAS_ERROR_STRING	    1
#endif


/**
 * Do we have alternate pool implementation?
 *
 * Default: 0
 */
#ifndef EM_HAS_POOL_ALT_API
#   define EM_HAS_POOL_ALT_API	    EM_POOL_DEBUG
#endif

/**
 * Specify if EM_CHECK_STACK() macro is enabled to check the sanity of 
 * the stack. The OS implementation may check that no stack overflow 
 * occurs, and it also may collect statistic about stack usage. Note
 * that this will increase the footprint of the libraries since it
 * tracks the filename and line number of each functions.
 */
#ifndef EM_OS_HAS_CHECK_STACK
#	define EM_OS_HAS_CHECK_STACK		1
#endif

/**
 * Enable name registration for exceptions with #em_exception_id_alloc().
 * If this feature is enabled, then the library will keep track of
 * names associated with each exception ID requested by application via
 * #em_exception_id_alloc().
 *
 * Disabling this macro will reduce the code and .bss size by a tad bit.
 * See also #EM_MAX_EXCEPTION_ID.
 *
 * Default: 1
 */
#ifndef EM_HAS_EXCEPTION_NAMES
#   define EM_HAS_EXCEPTION_NAMES   1
#endif

/**
 * Maximum number of unique exception IDs that can be requested
 * with #em_exception_id_alloc(). For each entry, a small record will
 * be allocated in the .bss segment.
 *
 * Default: 16
 */
#ifndef EM_MAX_EXCEPTION_ID
#   define EM_MAX_EXCEPTION_ID      16
#endif

/**
 * Has semaphore functionality?
 *
 * Default: 1
 */
#ifndef EM_HAS_SEMAPHORE
#  define EM_HAS_SEMAPHORE	    1
#endif


/**
 * Event object (for synchronization, e.g. in Win32)
 *
 * Default: 1
 */
#ifndef EM_HAS_EVENT_OBJ
#  define EM_HAS_EVENT_OBJ	    1
#endif


/**
 * Maximum file name length.
 */
#ifndef EM_MAXPATH
#   define EM_MAXPATH		    260
#endif

/**
 * Declare maximum logging level/verbosity. Lower number indicates higher
 * importance, with the highest importance has level zero. The least
 * important level is five in this implementation, but this can be extended
 * by supplying the appropriate implementation.
 *
 * The level conventions:
 *  - 0: fatal error
 *  - 1: error
 *  - 2: warning
 *  - 3: info
 *  - 4: debug
 *  - 5: trace
 *  - 6: more detailed trace
 *
 * Default: 4
 */
#ifndef EM_LOG_MAX_LEVEL
#  define EM_LOG_MAX_LEVEL   4
#endif

/**
 * Maximum number of socket options in em_sockopt_params.
 *
 * Default: 4
 */
#ifndef EM_MAX_SOCKOPT_PARAMS
#   define EM_MAX_SOCKOPT_PARAMS	    4
#endif

/**
 * Support TCP in the library.
 * Disabling TCP will reduce the footprint slightly (about 6KB).
 *
 * Default: 1
 */
#ifndef EM_HAS_TCP
#  define EM_HAS_TCP		    1
#endif

/**
 * Support IPv6 in the library. If this support is disabled, some IPv6
 * related functions will return EM_EIPV6NOTSUP.
 *
 * Default: 0 (disabled, for now)
 */
#ifndef EM_HAS_IPV6
#  define EM_HAS_IPV6		    0
#endif

 /**
 * Maximum hostname length.
 * Libraries sometimes needs to make copy of an address to stack buffer;
 * the value here affects the stack usage.
 *
 * Default: 128
 */
#ifndef EM_MAX_HOSTNAME
#  define EM_MAX_HOSTNAME	    (128)
#endif
/********************************************************************
 * General macros.
 */

/**
 *  * @def EM_DECL_NO_RETURN(type)
 *   * @param type The return type of the function.
 *    * Declare a function that will not return.
 *     */
/**
 *  * @def EM_IDECL_NO_RETURN(type)
 *   * @param type The return type of the function.
 *    * Declare an inline function that will not return.
 *     */
/**
 *  * @def EM_BEGIN_DECL
 *   * Mark beginning of declaration section in a header file.
 *    */
/**
 *  * @def EM_END_DECL
 *   * Mark end of declaration section in a header file.
 *    */
#ifdef __cplusplus
#  define EM_DECL_NO_RETURN(type)   EM_DECL(type) EM_NORETURN
#  define EM_IDECL_NO_RETURN(type)  EM_INLINE(type) EM_NORETURN
#  define EM_BEGIN_DECL    extern "C" {
#  define EM_END_DECL    }
#else
#  define EM_DECL_NO_RETURN(type)   EM_NORETURN EM_DECL(type)
#  define EM_IDECL_NO_RETURN(type)  EM_NORETURN EM_INLINE(type)
#  define EM_BEGIN_DECL
#  define EM_END_DECL
#endif

/**
 * @def EM_DECL(type)
 * @param type The return type of the function.
 * Declare a function.
 */
#if !defined(EM_DECL)
#   if defined(__cplusplus)
#	define EM_DECL(type)	    type
#   else
#	define EM_DECL(type)	    extern type
#   endif
#endif

#if !defined(EM_DECL_DATA)
#   define EM_DECL_DATA(type)	extern type
#endif

#if !defined(EM_DEF_DATA)
#   define EM_DEF_DATA(type)	type
#endif
/**
 * @def EM_DEF(type)
 * @param type The return type of the function.
 * Define a function.
 */
#if !defined(EM_DEF)
#   define EM_DEF(type)		    type
#endif

/**
 * @def EM_INLINE(type)
 * @param type The return type of the function.
 * Expand the function as inline.
 */
#define EM_INLINE(type)	  EM_INLINE_SPECIFIER type

/**
 * @def EM_IDECL(type)
 * @param type  The function's return type.
 * Declare a function that may be expanded as inline.
 */
/**
 * @def EM_IDEF(type)
 * @param type  The function's return type.
 * Define a function that may be expanded as inline.
 */

#if EM_FUNCTIONS_ARE_INLINED
#  define EM_IDECL(type)  EM_INLINE(type)
#  define EM_IDEF(type)   EM_INLINE(type)
#else
#  define EM_IDECL(type)  EM_DECL(type)
#  define EM_IDEF(type)   EM_DEF(type)
#endif

/**
 * @def EM_UNUSED_ARG(arg)
 * @param arg   The argument name.
 * EM_UNUSED_ARG prevents warning about unused argument in a function.
 */
#define EM_UNUSED_ARG(arg)  (void)arg

/**
 * @def EM_TODO(id)
 * @param id    Any identifier that will be printed as TODO message.
 * EM_TODO macro will display TODO message as warning during compilation.
 * Example: EM_TODO(CLEAN_UP_ERROR);
 */
#ifndef EM_TODO
#  define EM_TODO(id)	    TODO___##id:
#endif

/**
 * Simulate race condition by sleeping the thread in strategic locations.
 * Default: no!
 */
#ifndef EM_RACE_ME
#  define EM_RACE_ME(x)
#endif

/** EMLIB version major number. */
#define EM_VERSION_NUM_MAJOR    0	

/** EMLIB version minor number. */
#define EM_VERSION_NUM_MINOR    6	

/** EMLIB version revision number. */
#define EM_VERSION_NUM_REV      1 

/**
 * Extra suffix for the version (e.g. "-trunk"), or empty for
 * web release version.
 */
#define EM_VERSION_NUM_EXTRA	""

/**
 * EMLIB version number consists of three bytes with the following format:
 * 0xMMIIRR00, where MM: major number, II: minor number, RR: revision
 * number, 00: always zero for now.
 */
#define EM_VERSION_NUM	((EM_VERSION_NUM_MAJOR << 24) |	\
        (EM_VERSION_NUM_MINOR << 16) | \
        (EM_VERSION_NUM_REV << 8))

/**
 * EMLIB version string constant. @see EM_get_version()
 */
EM_DECL_DATA(const char*) EM_VERSION;

/**
 * Get EMLIB version string.
 *
 * @return #EM_VERSION constant.
 */
EM_DECL(const char*) em_get_version(void);

#endif/*__EM_CONFIG_H_*/
