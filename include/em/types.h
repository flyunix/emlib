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

typedef uint8      em_uint8_t;
typedef uint16     em_uint16_t;
typedef uint32     em_uint32_t;
typedef uint64     em_uint64_t;

typedef int8      em_int8_t;
typedef int16     em_int16_t;
typedef int32     em_int32_t;
typedef int64     em_int64_t;

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
 *
 * This structure represents high resolution (64bit) time value. The time
 * values represent time in cycles, which is retrieved by calling
 * #em_get_timestamp().
 */
typedef union em_timestamp
{
    struct
    {
#if defined(EM_IS_LITTLE_ENDIAN) && EM_IS_LITTLE_ENDIAN!=0
        uint32 lo;     /**< Low 32-bit value of the 64-bit value. */
        uint32 hi;     /**< high 32-bit value of the 64-bit value. */
#else
        uint32 hi;     /**< high 32-bit value of the 64-bit value. */
        uint32 lo;     /**< Low 32-bit value of the 64-bit value. */
#endif
    } u32;                  /**< The 64-bit value as two 32-bit values. */
    uint64 u64;        /**< The whole 64-bit value, where available. */
} em_timestamp;

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

/** OS timer task handle.*/
typedef struct em_os_tt_obj_t em_os_tt_obj_t;

typedef struct em_timer_task_t em_timer_task_t;

/** Semaphore handle. */
typedef struct em_sem_t em_sem_t;

/** Event object. */
typedef struct em_event_t em_event_t;

/** Unidirectional stream pipe object. */
typedef struct em_pipe_t em_pipe_t;

/** Operating system handle. */
typedef void *em_oshandle_t;

/** Exception id. */
typedef int em_exception_id_t;

/** Socket handle. */
typedef long em_sock_t;

/** Generic socket address. */
typedef void em_sockaddr_t;

/** Forward declaration. */
typedef struct em_sockaddr_in em_sockaddr_in;

/** Utility macro to compute the number of elements in static array. */
#define EM_ARRAY_SIZE(a)    (sizeof(a)/sizeof(a[0]))


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

/**
 * @}
 */
/**
 * @addtogroup EM_TIME Time Data Type and Manipulation.
 * @ingroup EM_MISC
 * @{
 */

/**
 * Representation of time value in this library.
 * This type can be used to represent either an interval or a specific time
 * or date. 
 */
typedef struct em_time_val
{
    /** The seconds part of the time. */
    long    sec;

    /** The miliseconds fraction of the time. */
    long    msec;

} em_time_val;


/*
 * For Posix system sleep.
 */
typedef struct em_sleep_val
{
    long sv_sec;/*seconds*/
    long sv_nsec;/** nanoseconds [0 .. 999999999] * */
}em_sleep_val;

/**
 * Normalize the value in time value.
 * @param t     Time value to be normalized.
 */
EM_DECL(void) em_time_val_normalize(em_time_val *t);

/**
 * Get the total time value in miliseconds. This is the same as
 * multiplying the second part with 1000 and then add the miliseconds
 * part to the result.
 *
 * @param t     The time value.
 * @return      Total time in miliseconds.
 * @hideinitializer
 */
#define EM_TIME_VAL_MSEC(t)	((t).sec * 1000 + (t).msec)

/**
 * This macro will check if \a t1 is equal to \a t2.
 *
 * @param t1    The first time value to compare.
 * @param t2    The second time value to compare.
 * @return      Non-zero if both time values are equal.
 * @hideinitializer
 */
#define EM_TIME_VAL_EQ(t1, t2)	((t1).sec==(t2).sec && (t1).msec==(t2).msec)

/**
 * This macro will check if \a t1 is greater than \a t2
 *
 * @param t1    The first time value to compare.
 * @param t2    The second time value to compare.
 * @return      Non-zero if t1 is greater than t2.
 * @hideinitializer
 */
#define EM_TIME_VAL_GT(t1, t2)	((t1).sec>(t2).sec || \
                                ((t1).sec==(t2).sec && (t1).msec>(t2).msec))

/**
 * This macro will check if \a t1 is greater than or equal to \a t2
 *
 * @param t1    The first time value to compare.
 * @param t2    The second time value to compare.
 * @return      Non-zero if t1 is greater than or equal to t2.
 * @hideinitializer
 */
#define EM_TIME_VAL_GTE(t1, t2)	(EM_TIME_VAL_GT(t1,t2) || \
                                 EM_TIME_VAL_EQ(t1,t2))

/**
 * This macro will check if \a t1 is less than \a t2
 *
 * @param t1    The first time value to compare.
 * @param t2    The second time value to compare.
 * @return      Non-zero if t1 is less than t2.
 * @hideinitializer
 */
#define EM_TIME_VAL_LT(t1, t2)	(!(EM_TIME_VAL_GTE(t1,t2)))

/**
 * This macro will check if \a t1 is less than or equal to \a t2.
 *
 * @param t1    The first time value to compare.
 * @param t2    The second time value to compare.
 * @return      Non-zero if t1 is less than or equal to t2.
 * @hideinitializer
 */
#define EM_TIME_VAL_LTE(t1, t2)	(!EM_TIME_VAL_GT(t1, t2))

/**
 * Add \a t2 to \a t1 and store the result in \a t1. Effectively
 *
 * this macro will expand as: (\a t1 += \a t2).
 * @param t1    The time value to add.
 * @param t2    The time value to be added to \a t1.
 * @hideinitializer
 */
#define EM_TIME_VAL_ADD(t1, t2)	    do {			    \
					(t1).sec += (t2).sec;	    \
					(t1).msec += (t2).msec;	    \
					em_time_val_normalize(&(t1)); \
				    } while (0)


#define EM_TIME_VAL_SET(t1, t2)	    do {			    \
					(t1).sec = (t2).sec;	    \
					(t1).msec = (t2).msec;	    \
					em_time_val_normalize(&(t1)); \
				    } while (0)
/**
 * Substract \a t2 from \a t1 and store the result in \a t1. Effectively
 * this macro will expand as (\a t1 -= \a t2).
 *
 * @param t1    The time value to subsctract.
 * @param t2    The time value to be substracted from \a t1.
 * @hideinitializer
 */
#define EM_TIME_VAL_SUB(t1, t2)	    do {			    \
					(t1).sec -= (t2).sec;	    \
					(t1).msec -= (t2).msec;	    \
					em_time_val_normalize(&(t1)); \
				    } while (0)


/**
 * This structure represent the parsed representation of time.
 * It is acquired by calling #em_time_decode().
 */
typedef struct em_parsed_time
{
    /** This represents day of week where value zero means Sunday */
    int wday;

    /* This represents day of the year, 0-365, where zero means
     *  1st of January.
     */
    /*int yday; */

    /** This represents day of month: 1-31 */
    int day;

    /** This represents month, with the value is 0 - 11 (zero is January) */
    int mon;

    /** This represent the actual year (unlike in ANSI libc where
     *  the value must be added by 1900).
     */
    int year;

    /** This represents the second part, with the value is 0-59 */
    int sec;

    /** This represents the minute part, with the value is: 0-59 */
    int min;

    /** This represents the hour part, with the value is 0-23 */
    int hour;

    /** This represents the milisecond part, with the value is 0-999 */
    int msec;

} em_parsed_time;


#endif
