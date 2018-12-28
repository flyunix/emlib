/*
 * File:    cc_gcc.h
 * Author:  Liu HongLiang<lhl_nciae@sina.cn>
 * Brief:   Describes GCC compiler specifics.
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
#ifndef __CM_COMPAT_CC_GCC_H__
#define __CM_COMPAT_CC_GCC_H__

#ifndef __GNUC__
#  error "This file is only for gcc!"
#endif

#define EM_CC_NAME		"gcc"
#define EM_CC_VER_1		__GNUC__
#define EM_CC_VER_2		__GNUC_MINOR__

/* __GNUC_PATCHLEVEL__ doesn't exist in gcc-2.9x.x */
#ifdef __GNUC_PATCHLEVEL__
#   define EM_CC_VER_3		__GNUC_PATCHLEVEL__
#else
#   define EM_CC_VER_3		0
#endif

#define EM_THREAD_FUNC	
#define EM_NORETURN		

#define EM_HAS_INT64		1

#ifdef __STRICT_ANSI__
  #include <inttypes.h> 
  typedef int64_t		em_int64_t;
  typedef uint64_t		em_uint64_t;
  #define EM_INLINE_SPECIFIER	static __inline
  #define EM_ATTR_NORETURN	
  #define EM_ATTR_MAY_ALIAS	
#else
  typedef long long		em_int64_t;
  typedef unsigned long long	em_uint64_t;
  #define EM_INLINE_SPECIFIER	static inline
  #define EM_ATTR_NORETURN	__attribute__ ((noreturn))
  #define EM_ATTR_MAY_ALIAS	__attribute__((__may_alias__))
#endif

#define EM_INT64(val)		val##LL
#define EM_UINT64(val)		val##ULL
#define EM_INT64_FMT		"L"


#ifdef __GLIBC__
#   define EM_HAS_BZERO		1
#endif

#define EM_UNREACHED(x)	    	

#define EM_ALIGN_DATA(declaration, alignment) declaration __attribute__((aligned (alignment)))

#endif	/* __EM_COMPAT_CC_GCC_H__ */

