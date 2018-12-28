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
 * Include error message string in the library (em_strerror()).
 * This is very much desirable!
 *
 * Default: 1
 */
#ifndef EM_HAS_ERROR_STRING
#   define EM_HAS_ERROR_STRING	    1
#endif


/********************************************************************
 * General macros.
 */

#define EM_INLINE_SPECIFIER static inline


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

#endif/*__EM_CONFIG_H_*/
