/*
 * File:    types.h
 * Author:  Liu HongLiang<lhl_nciae@sina.cn>
 * Brief:   Wrapper for system base data type.
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

#ifndef _COMMON_TYPE_H_
#define _COMMON_TYPE_H_

#include <stdio.h>
#include <stdbool.h>

/*large unsigned int*/
typedef size_t embed_size_t;
typedef bool embed_bool_t;

/*基本数据类型*/
typedef unsigned char      uint8;
typedef unsigned short     uint16;
typedef unsigned int       uint32;  
typedef unsigned long long uint64;  

typedef signed  char      sint8;    
typedef signed  short     sint16;   
typedef signed  int       sint32; 
typedef signed  long long sint64;

typedef char      int8;
typedef short     int16;   
typedef int       int32;   
typedef long long int64;

typedef enum _embed_constants_
{
    EMBED_SUCC = 0,
    EMBED_INVALID_PARAMS,
    EMBED_FAIL,
    EMBED_NOMEM,
    EMBED_INVALID_LOGIC
}embed_ret_t;

/** Utility macro to compute the number of elements in static array. */
#define EMBED_ARRAY_SIZE(a)    (sizeof(a)/sizeof(a[0]))


#define EMBED_PTR_ALIGNMENT    4

/*Test pointer p is alignment for EMBED_PTR_ALIGNMENT*/
#define IS_ALIGNED(p)   ((((unsigned long)p) & (EMBED_PTR_ALIGNMENT-1)) == 0) 

/*
 ** Macros to compute minimum and maximum of two numbers.
 */

#define MIN(A,B) ((A)<(B)?(A):(B))
#define MAX(A,B) ((A)>(B)?(A):(B))

/*MACRO for String*/
#define STRCMP(_a_,_C_,_b_) ( strcmp(_a_,_b_) _C_ 0 )
#define STRNCMP(_a_,_C_,_b_,_n_) ( strncmp(_a_,_b_,_n_) _C_ 0 )

/*MACRO for C++ project*/
#ifdef __cplusplus
#define DECLS_BEGIN extern "C" {
#define DECLS_END   }
#else
#define DECLS_BEGIN
#define DECLS_END
#endif/*__cplusplus*/

/*MACRO for Memory free*/
#define SAFE_FREE(p) \
do {\
   free(p);\
   p = NULL;\
}while(0)

#endif
