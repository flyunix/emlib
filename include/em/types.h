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

#include "config.h"

#include <stdio.h>
#include <stdbool.h>

/* ************************************************************************* */
/*
 * Data structure types.
 */

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

/** Large unsigned integer. */
typedef size_t		em_size_t;
typedef long        em_ssize_t;

typedef int em_bool_t;

typedef enum _embed_constants_
{
    EM_SUCC = 0,
    EM_FAIL = 1 ,
    EM_TRUE = 1,
    EM_FALSE = 0
}emlib_ret_t;

/**
 * This type is used as replacement to legacy C string, and used throughout
 * the library. By convention, the string is NOT null terminated.
 */
struct em_str_t
{
    /** Buffer pointer, which is by convention NOT null terminated. */
    char       *ptr;

    /** The length of the string. */
    em_ssize_t  slen;

    /** The length of Buffer. */
    em_ssize_t blen;
};

/**
 * The opaque data type for linked list, which is used as arguments throughout
 * the linked list operations.
 */
typedef void em_list_type;

/** 
 * List.
 */
typedef struct em_list em_list;

/**
 * Forward declaration for memory pool factory.
 */
typedef struct em_pool_factory em_pool_factory;

/**
 * Opaque data type for memory pool.
 */
typedef struct em_pool_t em_pool_t;

/**
 * Forward declaration for caching pool, a pool factory implementation.
 */
typedef struct em_caching_pool em_caching_pool;

/**
 * This type is used as replacement to legacy C string, and used throughout
 * the library.
 */
typedef struct em_str_t em_str_t;

/**
 * Opaque data type for I/O Queue structure.
 */
typedef struct em_ioqueue_t em_ioqueue_t;

/**
 * Opaque data type for key that identifies a handle registered to the
 * I/O queue framework.
 */
typedef struct em_ioqueue_key_t em_ioqueue_key_t;

/**
 * Opaque data to identify timer heap.
 */
typedef struct em_timer_heap_t em_timer_heap_t;

/** 
 * Opaque data type for atomic operations.
 */
typedef struct em_atomic_t em_atomic_t;

/**
 * Value type of an atomic variable.
 */
typedef EM_ATOMIC_VALUE_TYPE em_atomic_value_t;
 
/* ************************************************************************* */

/** Thread handle. */
typedef struct em_thread_t em_thread_t;

/** Lock object. */
typedef struct em_lock_t em_lock_t;

/** Group lock */
typedef struct em_grp_lock_t em_grp_lock_t;

/** Mutex handle. */
typedef struct em_mutex_t em_mutex_t;

/** Semaphore handle. */
typedef struct em_sem_t em_sem_t;

/** Event object. */
typedef struct em_event_t em_event_t;

/** Unidirectional stream pipe object. */
typedef struct em_pipe_t em_pipe_t;

/** Operating system handle. */
typedef void *em_oshandle_t;


/** Utility macro to compute the number of elements in static array. */
#define EMLIB_ARRAY_SIZE(a)    (sizeof(a)/sizeof(a[0]))


#define EMLIB_PTR_ALIGNMENT    4

/*Max object name size*/
#define EM_MAX_OBJ_NAME 32 
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

/* ************************************************************************* */
/*
 * General.
 */
/**
 * Initialize the EM Library.
 * This function must be called before using the library. The purpose of this
 * function is to initialize static library data, such as character table used
 * in random string generation, and to initialize operating system dependent
 * functionality (such as WSAStartup() in Windows).
 *
 * Apart from calling em_init(), application typically should also initialize
 * the random seed by calling em_srand().
 *
 * @return EM_SUCC on success.
 */
EM_DECL(emlib_ret_t) em_init(void);


/**
 * Shutdown EMLIB.
 */
EM_DECL(void) em_shutdown(void);

/**
 * Type of callback to register to em_atexit().
 */
typedef void (*em_exit_callback)(void);

/**
 * Register cleanup function to be called by EMLIB when em_shutdown() is 
 * called.
 *
 * @param func	    The function to be registered.
 *
 * @return EM_SUCC on success.
 */
EM_DECL(emlib_ret_t) em_atexit(em_exit_callback func);

#endif
