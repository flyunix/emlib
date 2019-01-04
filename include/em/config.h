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
    /*
     * ARM, bi-endian, so raise error if endianness is not configured
     */
#   if !EM_IS_LITTLE_ENDIAN && !EM_IS_BIG_ENDIAN
#   	error Endianness must be declared for this processor
#   endif

#   define EM_IS_LITTLE_ENDIAN	1
#   define EM_IS_BIG_ENDIAN	0

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
#   define EM_NATIVE_ERR_POSITIVE   0
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
#	define EM_OS_HAS_CHECK_STACK		0
#endif


/********************************************************************
 * General macros.
 */


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

/** EMLIB version major number. */
#define EM_VERSION_NUM_MAJOR	2

/** EMLIB version minor number. */
#define EM_VERSION_NUM_MINOR	8

/** EMLIB version revision number. */
#define EM_VERSION_NUM_REV      0

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
