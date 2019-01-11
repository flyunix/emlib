/*
 * File:    os.h
 * Author:  Liu HongLiang<lhl_nciae@sina.cn>
 * Brief:   Os dependent functions
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
 * 2019-01-02 Liu HongLiang<lhl_nciae@sina.cn> created.
 *
 */

#ifndef __EM_OS_H__
#define __EM_OS_H__

/**
 * @file os.h
 * @brief OS dependent functions
 */
#include "em/types.h"

DECLS_BEGIN

/**
 * @defgroup EM_OS Operating System Dependent Functionality.
 */


/* **************************************************************************/
/**
 * @defgroup EM_SYS_INFO System Information
 * @ingroup EM_OS
 * @{
 */

/**
 * These enumeration contains constants to indicate support of miscellaneous
 * system features. These will go in "flags" field of #em_sys_info structure.
 */
typedef enum em_sys_info_flag
{
    /**
     * Support for Apple iOS background feature.
     */
    EM_SYS_HAS_IOS_BG = 1

} em_sys_info_flag;


/**
 * This structure contains information about the system. Use #em_get_sys_info()
 * to obtain the system information.
 */
typedef struct em_sys_info
{
    /**
     * Null terminated string containing processor information (e.g. "i386",
     * "x86_64"). It may contain empty string if the value cannot be obtained.
     */
    em_str_t	machine;

    /**
     * Null terminated string identifying the system operation (e.g. "Linux",
     * "win32", "wince"). It may contain empty string if the value cannot be
     * obtained.
     */
    em_str_t	os_name;

    /**
     * A number containing the operating system version number. By convention,
     * this field is divided into four bytes, where the highest order byte
     * contains the most major version of the OS, the next less significant
     * byte contains the less major version, and so on. How the OS version
     * number is mapped into these four bytes would be specific for each OS.
     * For example, Linux-2.6.32-28 would yield "os_ver" value of 0x0206201c,
     * while for Windows 7 it will be 0x06010000 (because dwMajorVersion is
     * 6 and dwMinorVersion is 1 for Windows 7).
     *
     * This field may contain zero if the OS version cannot be obtained.
     */
    uint32	os_ver;

    /**
     * Null terminated string identifying the SDK name that is used to build
     * the library (e.g. "glibc", "uclibc", "msvc", "wince"). It may contain
     * empty string if the value cannot eb obtained.
     */
    em_str_t	sdk_name;

    /**
     * A number containing the SDK version, using the numbering convention as
     * the "os_ver" field. The value will be zero if the version cannot be
     * obtained.
     */
    uint32	sdk_ver;

    /**
     * A longer null terminated string identifying the underlying system with
     * as much information as possible.
     */
    em_str_t	info;

    /**
     * Other flags containing system specific information. The value is
     * bitmask of #em_sys_info_flag constants.
     */
    uint32	flags;

} em_sys_info;


/**
 * Obtain the system information.
 *
 * @return	System information structure.
 */
EM_DECL(const em_sys_info*) em_get_sys_info(void);

/*
 * @}
 */

/* **************************************************************************/
/**
 * @defgroup EM_THREAD Threads
 * @ingroup EM_OS
 * @{
 * This module provides multithreading API.
 *
 * \section em_thread_examples_sec Examples
 *
 * For examples, please see:
 *  - \ref page_emlib_thread_test
 *  - \ref page_emlib_sleep_test
 *
 */

/**
 * Thread creation flags:
 * - em_THREAD_SUSPENDED: specify that the thread should be created suspended.
 */
typedef enum em_thread_create_flags
{
    EM_THREAD_SUSPENDED = 1
} em_thread_create_flags;


/**
 * Type of thread entry function.
 */
typedef int (EM_THREAD_FUNC em_thread_proc)(void*);

/**
 * Size of thread struct.
 */
#if !defined(EM_THREAD_DESC_SIZE)
#   define EM_THREAD_DESC_SIZE	    (64)
#endif

/**
 * Thread structure, to thread's state when the thread is created by external
 * or native API. 
 */
typedef long em_thread_desc[EM_THREAD_DESC_SIZE];

/**
 * Get process ID.
 * @return process ID.
 */
EM_DECL(uint32) em_getpid(void);

/**
 * Create a new thread.
 *
 * @param pool          The memory pool from which the thread record 
 *                      will be allocated from.
 * @param thread_name   The optional name to be assigned to the thread.
 * @param proc          Thread entry function.
 * @param arg           Argument to be passed to the thread entry function.
 * @param stack_size    The size of the stack for the new thread, or ZERO or
 *                      em_THREAD_DEFAULT_STACK_SIZE to let the 
 *		        library choose the reasonable size for the stack. 
 *                      For some systems, the stack will be allocated from 
 *                      the pool, so the pool must have suitable capacity.
 * @param flags         Flags for thread creation, which is bitmask combination 
 *                      from enum em_thread_create_flags.
 * @param thread        Pointer to hold the newly created thread.
 *
 * @return	        EM_SUCC on success, or the error code.
 */
EM_DECL(emlib_ret_t) em_thread_create(  em_pool_t *pool, 
        const char *thread_name,
        em_thread_proc *proc, 
        void *arg,
        em_size_t stack_size, 
        unsigned flags,
        em_thread_t **thread );

/**
 * Register a thread that was created by EXTERNAL or NATIVE API to EMLIB.
 * This function must be called in the context of the thread being registered.
 * When the thread is created by EXTERNAL function or API call,
 * it must be 'registered' to EMLIB using em_thread_register(), so that it can
 * cooperate with EMLIB's framework. During registration, some data needs to
 * be maintained, and this data must remain available during the thread's 
 * lifetime.
 *
 * @param thread_name   The optional name to be assigned to the thread.
 * @param desc          Thread descriptor, which must be available throughout 
 *                      the lifetime of the thread.
 * @param thread        Pointer to hold the created thread handle.
 *
 * @return              em_SUCC on success, or the error code.
 */
EM_DECL(emlib_ret_t) em_thread_register ( const char *thread_name,
        em_thread_desc desc,
        em_thread_t **thread);

/**
 * Check if this thread has been registered to EMLIB.
 *
 * @return		Non-zero if it is registered.
 */
EM_DECL(em_bool_t) em_thread_is_registered(void);


/**
 * Get thread priority value for the thread.
 *
 * @param thread	Thread handle.
 *
 * @return		Thread priority value, or -1 on error.
 */
EM_DECL(int) em_thread_get_prio(em_thread_t *thread);


/**
 * Set the thread priority. The priority value must be in the priority
 * value range, which can be retrieved with #em_thread_get_prio_min() and
 * #em_thread_get_prio_max() functions.
 *
 * @param thread	Thread handle.
 * @param prio		New priority to be set to the thread.
 *
 * @return		EM_SUCC on success or the error code.
 */
EM_DECL(emlib_ret_t) em_thread_set_prio(em_thread_t *thread,  int prio);

/**
 * Get the lowest priority value available for this thread.
 *
 * @param thread	Thread handle.
 * @return		Minimum thread priority value, or -1 on error.
 */
EM_DECL(int) em_thread_get_prio_min(em_thread_t *thread);


/**
 * Get the highest priority value available for this thread.
 *
 * @param thread	Thread handle.
 * @return		Minimum thread priority value, or -1 on error.
 */
EM_DECL(int) em_thread_get_prio_max(em_thread_t *thread);


/**
 * Return native handle from em_thread_t for manipulation using native
 * OS APIs.
 *
 * @param thread	EMLIB thread descriptor.
 *
 * @return		Native thread handle. For example, when the
 *			backend thread uses pthread, this function will
 *			return pointer to pthread_t, and on Windows,
 *			this function will return HANDLE.
 */
EM_DECL(void*) em_thread_get_os_handle(em_thread_t *thread);

/**
 * Get thread name.
 *
 * @param thread    The thread handle.
 *
 * @return Thread name as null terminated string.
 */
EM_DECL(const char*) em_thread_get_name(em_thread_t *thread);

/**
 * Resume a suspended thread.
 *
 * @param thread    The thread handle.
 *
 * @return zero on success.
 */
EM_DECL(emlib_ret_t) em_thread_resume(em_thread_t *thread);

/**
 * Get the current thread.
 *
 * @return Thread handle of current thread.
 */
EM_DECL(em_thread_t*) em_thread_this(void);

/**
 * Join thread, and block the caller thread until the specified thread exits.
 * If it is called from within the thread itself, it will return immediately
 * with failure status.
 * If the specified thread has already been dead, or it does not exist,
 * the function will return immediately with successful status.
 *
 * @param thread    The thread handle.
 *
 * @return EM_SUCC on success.
 */
EM_DECL(emlib_ret_t) em_thread_join(em_thread_t *thread);


/**
 * Destroy thread and release resources allocated for the thread.
 * However, the memory allocated for the em_thread_t itself will only be released
 * when the pool used to create the thread is destroyed.
 *
 * @param thread    The thread handle.
 *
 * @return zero on success.
 */
EM_DECL(emlib_ret_t) em_thread_destroy(em_thread_t *thread);


/**
 * Put the current thread to sleep for the specified miliseconds.
 *
 * @param msec Miliseconds delay.
 *
 * @return zero if successfull.
 */
EM_DECL(emlib_ret_t) em_thread_sleep(unsigned msec);

/**
 * @def em_CHECK_STACK()
 * EM_CHECK_STACK() macro is used to check the sanity of the stack.
 * The OS implementation may check that no stack overflow occurs, and
 * it also may collect statistic about stack usage.
 */
#if defined(EM_OS_HAS_CHECK_STACK) && EM_OS_HAS_CHECK_STACK!=0

#  define EM_CHECK_STACK() em_thread_check_stack(__FILE__, __LINE__)

/** @internal
 * The implementation of stack checking. 
 */
EM_DECL(void) em_thread_check_stack(const char *file, int line);

/** @internal
 * Get maximum stack usage statistic. 
 */
EM_DECL(uint32) em_thread_get_stack_max_usage(em_thread_t *thread);

/** @internal
 * Dump thread stack status. 
 */
EM_DECL(emlib_ret_t) em_thread_get_stack_info(em_thread_t *thread,
        const char **file,
        int *line);
#else

#  define EM_CHECK_STACK()
/** em_thread_get_stack_max_usage() for the thread */
#  define em_thread_get_stack_max_usage(thread)	    0
/** em_thread_get_stack_info() for the thread */
#  define em_thread_get_stack_info(thread,f,l)	    (*(f)="",*(l)=0)
#endif	/* em_OS_HAS_CHECK_STACK */

/**
 * @}
 */

/* **************************************************************************/
/**
 * @defgroup em_TLS Thread Local Storage.
 * @ingroup em_OS
 * @{
 */

/** 
 * Allocate thread local storage index. The initial value of the variable at
 * the index is zero.
 *
 * @param index	    Pointer to hold the return value.
 * @return	    EM_SUCC on success, or the error code.
 */
EM_DECL(emlib_ret_t) em_thread_local_alloc(long *index);

/**
 * Deallocate thread local variable.
 *
 * @param index	    The variable index.
 */
EM_DECL(void) em_thread_local_free(long index);

/**
 * Set the value of thread local variable.
 *
 * @param index	    The index of the variable.
 * @param value	    The value.
 */
EM_DECL(emlib_ret_t) em_thread_local_set(long index, void *value);

/**
 * Get the value of thread local variable.
 *
 * @param index	    The index of the variable.
 * @return	    The value.
 */
EM_DECL(void*) em_thread_local_get(long index);


/**
 * @}
 */


/* **************************************************************************/
/**
 * @defgroup EM_ATOMIC Atomic Variables
 * @ingroup EM_OS
 * @{
 *
 * This module provides API to manipulate atomic variables.
 *
 * \section em_atomic_examples_sec Examples
 *
 * For some example codes, please see:
 *  - @ref page_emlib_atomic_test
 */


/**
 * Create atomic variable.
 *
 * @param pool	    The pool.
 * @param initial   The initial value of the atomic variable.
 * @param atomic    Pointer to hold the atomic variable upon return.
 *
 * @return	    EM_SUCC on success, or the error code.
 */
EM_DECL(emlib_ret_t) em_atomic_create( em_pool_t *pool, 
        em_atomic_value_t initial,
        em_atomic_t **atomic );

/**
 * Destroy atomic variable.
 *
 * @param atomic_var	the atomic variable.
 *
 * @return EM_SUCC if success.
 */
EM_DECL(emlib_ret_t) em_atomic_destroy( em_atomic_t *atomic_var );

/**
 * Set the value of an atomic type, and return the previous value.
 *
 * @param atomic_var	the atomic variable.
 * @param value		value to be set to the variable.
 */
EM_DECL(void) em_atomic_set( em_atomic_t *atomic_var, 
        em_atomic_value_t value);

/**
 * Get the value of an atomic type.
 *
 * @param atomic_var	the atomic variable.
 *
 * @return the value of the atomic variable.
 */
EM_DECL(em_atomic_value_t) em_atomic_get(em_atomic_t *atomic_var);

/**
 * Increment the value of an atomic type.
 *
 * @param atomic_var	the atomic variable.
 */
EM_DECL(void) em_atomic_inc(em_atomic_t *atomic_var);

/**
 * Increment the value of an atomic type and get the result.
 *
 * @param atomic_var	the atomic variable.
 *
 * @return              The incremented value.
 */
EM_DECL(em_atomic_value_t) em_atomic_inc_and_get(em_atomic_t *atomic_var);

/**
 * Decrement the value of an atomic type.
 *
 * @param atomic_var	the atomic variable.
 */
EM_DECL(void) em_atomic_dec(em_atomic_t *atomic_var);

/**
 * Decrement the value of an atomic type and get the result.
 *
 * @param atomic_var	the atomic variable.
 *
 * @return              The decremented value.
 */
EM_DECL(em_atomic_value_t) em_atomic_dec_and_get(em_atomic_t *atomic_var);

/**
 * Add a value to an atomic type.
 *
 * @param atomic_var	The atomic variable.
 * @param value		Value to be added.
 */
EM_DECL(void) em_atomic_add( em_atomic_t *atomic_var,
        em_atomic_value_t value);

/**
 * Add a value to an atomic type and get the result.
 *
 * @param atomic_var	The atomic variable.
 * @param value		Value to be added.
 *
 * @return              The result after the addition.
 */
EM_DECL(em_atomic_value_t) em_atomic_add_and_get( em_atomic_t *atomic_var,
        em_atomic_value_t value);

/**
 * @}
 */

/* **************************************************************************/
/**
 * @defgroup EM_MUTEX Mutexes.
 * @ingroup EM_OS
 * @{
 *
 * Mutex manipulation. Alternatively, application can use higher abstraction
 * for lock objects, which provides uniform API for all kinds of lock 
 * mechanisms, including mutex. See @ref EM_LOCK for more information.
 */

/**
 * Mutex types:
 *  - EM_MUTEX_DEFAULT: default mutex type, which is system dependent.
 *  - EM_MUTEX_SIMPLE: non-recursive mutex.
 *  - EM_MUTEX_RECURSE: recursive mutex.
 */
typedef enum em_mutex_type_e
{
    EM_MUTEX_DEFAULT,
    EM_MUTEX_SIMPLE,
    EM_MUTEX_RECURSE
} em_mutex_type_e;


/**
 * Create mutex of the specified type.
 *
 * @param pool	    The pool.
 * @param name	    Name to be associated with the mutex (for debugging).
 * @param type	    The type of the mutex, of type #em_mutex_type_e.
 * @param mutex	    Pointer to hold the returned mutex instance.
 *
 * @return	    EM_SUCC on success, or the error code.
 */
EM_DECL(emlib_ret_t) em_mutex_create(em_pool_t *pool, 
        const char *name,
        int type, 
        em_mutex_t **mutex);

/**
 * Create simple, non-recursive mutex.
 * This function is a simple wrapper for #em_mutex_create to create 
 * non-recursive mutex.
 *
 * @param pool	    The pool.
 * @param name	    Mutex name.
 * @param mutex	    Pointer to hold the returned mutex instance.
 *
 * @return	    EM_SUCC on success, or the error code.
 */
EM_DECL(emlib_ret_t) em_mutex_create_simple( em_pool_t *pool, const char *name,
        em_mutex_t **mutex );

/**
 * Create recursive mutex.
 * This function is a simple wrapper for #em_mutex_create to create 
 * recursive mutex.
 *
 * @param pool	    The pool.
 * @param name	    Mutex name.
 * @param mutex	    Pointer to hold the returned mutex instance.
 *
 * @return	    EM_SUCC on success, or the error code.
 */
EM_DECL(emlib_ret_t) em_mutex_create_recursive( em_pool_t *pool,
        const char *name,
        em_mutex_t **mutex );

/**
 * Acquire mutex lock.
 *
 * @param mutex	    The mutex.
 * @return	    EM_SUCC on success, or the error code.
 */
EM_DECL(emlib_ret_t) em_mutex_lock(em_mutex_t *mutex);

/**
 * Release mutex lock.
 *
 * @param mutex	    The mutex.
 * @return	    EM_SUCC on success, or the error code.
 */
EM_DECL(emlib_ret_t) em_mutex_unlock(em_mutex_t *mutex);

/**
 * Try to acquire mutex lock.
 *
 * @param mutex	    The mutex.
 * @return	    EM_SUCC on success, or the error code if the
 *		    lock couldn't be acquired.
 */
EM_DECL(emlib_ret_t) em_mutex_trylock(em_mutex_t *mutex);

/**
 * Destroy mutex.
 *
 * @param mutex	    Te mutex.
 * @return	    EM_SUCC on success, or the error code.
 */
EM_DECL(emlib_ret_t) em_mutex_destroy(em_mutex_t *mutex);

/**
 * Determine whether calling thread is owning the mutex (only available when
 * em_DEBUG is set).
 * @param mutex	    The mutex.
 * @return	    Non-zero if yes.
 */
EM_DECL(em_bool_t) em_mutex_is_locked(em_mutex_t *mutex);

/**
 * @}
 */

/* **************************************************************************/
/**
 * @defgroup EM_RW_MUTEX Reader/Writer Mutex
 * @ingroup EM_OS
 * @{
 * Reader/writer mutex is a classic synchronization object where multiple
 * readers can acquire the mutex, but only a single writer can acquire the 
 * mutex.
 */

/**
 * Opaque declaration for reader/writer mutex.
 * Reader/writer mutex is a classic synchronization object where multiple
 * readers can acquire the mutex, but only a single writer can acquire the 
 * mutex.
 */
typedef struct em_rwmutex_t em_rwmutex_t;

/**
 * Create reader/writer mutex.
 *
 * @param pool	    Pool to allocate memory for the mutex.
 * @param name	    Name to be assigned to the mutex.
 * @param mutex	    Pointer to receive the newly created mutex.
 *
 * @return	    EM_SUCC on success, or the error code.
 */
EM_DECL(emlib_ret_t) em_rwmutex_create(em_pool_t *pool, const char *name,
        em_rwmutex_t **mutex);

/**
 * Lock the mutex for reading.
 *
 * @param mutex	    The mutex.
 * @return	    EM_SUCC on success, or the error code.
 */
EM_DECL(emlib_ret_t) em_rwmutex_lock_read(em_rwmutex_t *mutex);

/**
 * Lock the mutex for writing.
 *
 * @param mutex	    The mutex.
 * @return	    EM_SUCC on success, or the error code.
 */
EM_DECL(emlib_ret_t) em_rwmutex_lock_write(em_rwmutex_t *mutex);

/**
 * Release read lock.
 *
 * @param mutex	    The mutex.
 * @return	    EM_SUCC on success, or the error code.
 */
EM_DECL(emlib_ret_t) em_rwmutex_unlock_read(em_rwmutex_t *mutex);

/**
 * Release write lock.
 *
 * @param mutex	    The mutex.
 * @return	    EM_SUCC on success, or the error code.
 */
EM_DECL(emlib_ret_t) em_rwmutex_unlock_write(em_rwmutex_t *mutex);

/**
 * Destroy reader/writer mutex.
 *
 * @param mutex	    The mutex.
 * @return	    EM_SUCC on success, or the error code.
 */
EM_DECL(emlib_ret_t) em_rwmutex_destroy(em_rwmutex_t *mutex);


/**
 * @}
 */


/* **************************************************************************/
/**
 * @defgroup EM_CRIT_SEC Critical sections.
 * @ingroup EM_OS
 * @{
 * Critical section protection can be used to protect regions where:
 *  - mutual exclusion protection is needed.
 *  - it's rather too expensive to create a mutex.
 *  - the time spent in the region is very very brief.
 *
 * Critical section is a global object, and it prevents any threads from
 * entering any regions that are protected by critical section once a thread
 * is already in the section.
 *
 * Critial section is \a not recursive!
 *
 * Application <b>MUST NOT</b> call any functions that may cause current
 * thread to block (such as allocating memory, performing I/O, locking mutex,
 * etc.) while holding the critical section.
 */
/**
 * Enter critical section.
 */
EM_DECL(void) em_enter_critical_section(void);

/**
 * Leave critical section.
 */
EM_DECL(void) em_leave_critical_section(void);

/**
 * @}
 */

/* **************************************************************************/
#if defined(EM_HAS_SEMAPHORE) && EM_HAS_SEMAPHORE != 0
/**
 * @defgroup EM_SEM Semaphores.
 * @ingroup EM_OS
 * @{
 *
 * This module provides abstraction for semaphores(POSIX Anonymous Semaphores), where available.
 */

/**
 * Create semaphore.
 *
 * @param pool	    The pool.
 * @param name	    Name to be assigned to the semaphore (for logging purpose)
 * @param initial   The initial count of the semaphore.
 * @param max	    The maximum count of the semaphore.
 * @param sem	    Pointer to hold the semaphore created.
 *
 * @return	    EM_SUCC on success, or the error code.
 */
EM_DECL(emlib_ret_t) em_sem_create( em_pool_t *pool, 
        const char *name,
        unsigned initial, 
        unsigned max,
        em_sem_t **sem);

/**
 * Wait for semaphore.
 *
 * @param sem	The semaphore.
 *
 * @return	EM_SUCC on success, or the error code.
 */
EM_DECL(emlib_ret_t) em_sem_wait(em_sem_t *sem);

/**
 * Try wait for semaphore.
 *
 * @param sem	The semaphore.
 *
 * @return	EM_SUCC on success, or the error code.
 */
EM_DECL(emlib_ret_t) em_sem_trywait(em_sem_t *sem);

/**
 * Release semaphore.
 *
 * @param sem	The semaphore.
 *
 * @return	EM_SUCC on success, or the error code.
 */
EM_DECL(emlib_ret_t) em_sem_post(em_sem_t *sem);

/**
 * Destroy semaphore.
 *
 * @param sem	The semaphore.
 *
 * @return	EM_SUCC on success, or the error code.
 */
EM_DECL(emlib_ret_t) em_sem_destroy(em_sem_t *sem);

/**
 * @}
 */
#endif	/* em_HAS_SEMAPHORE */


/* **************************************************************************/
#if defined(EM_HAS_EVENT_OBJ) && EM_HAS_EVENT_OBJ != 0
/**
 * @defgroup EM_EVENT Event Object.
 * @ingroup EM_OS
 * @{
 *
 * This module provides abstraction to event object (e.g. Win32 Event) where
 * available. Event objects can be used for synchronization among threads.
 *
 */

/**
 * Create event object.
 *
 * @param pool		The pool.
 * @param name		The name of the event object (for logging purpose).
 * @param manual_reset	Specify whether the event is manual-reset
 * @param initial	Specify the initial state of the event object.
 * @param event		Pointer to hold the returned event object.
 *
 * @return event handle, or NULL if failed.
 */
EM_DECL(emlib_ret_t) em_event_create(em_pool_t *pool, const char *name,
        em_bool_t manual_reset, em_bool_t initial,
        em_event_t **event);

/**
 * Wait for event to be signaled.
 *
 * @param event	    The event object.
 *
 * @return zero if successfull.
 */
EM_DECL(emlib_ret_t) em_event_wait(em_event_t *event);

/**
 * Try wait for event object to be signalled.
 *
 * @param event The event object.
 *
 * @return zero if successfull.
 */
EM_DECL(emlib_ret_t) em_event_trywait(em_event_t *event);

/**
 * Set the event object state to signaled. For AUTO-RESET EVENT, this 
 * will only release the first thread that are waiting on the event. For
 * MANUAL-RESET EVENT, the state remains signaled until the event is reset.
 * If there is no thread waiting on the event, the event object state 
 * remains signaled.
 *
 * @param event	    The event object.
 *
 * @return zero if successfull.
 */
EM_DECL(emlib_ret_t) em_event_set(em_event_t *event);

/**
 * Set the event object to signaled state to release appropriate number of
 * waiting threads and then reset the event object to non-signaled. For
 * MANUAL-RESET event, this function will release all waiting threads. For
 * AUTO-RESET event, this function will only release one waiting thread.
 *
 * @param event	    The event object.
 *
 * @return zero if successfull.
 */
EM_DECL(emlib_ret_t) em_event_pulse(em_event_t *event);

/**
 * Set the event object state to non-signaled.
 *
 * @param event	    The event object.
 *
 * @return zero if successfull.
 */
EM_DECL(emlib_ret_t) em_event_reset(em_event_t *event);

/**
 * Destroy the event object.
 *
 * @param event	    The event object.
 *
 * @return zero if successfull.
 */
EM_DECL(emlib_ret_t) em_event_destroy(em_event_t *event);

/**
 * @}
 */
#endif	/* EM_HAS_EVENT_OBJ */

/* **************************************************************************/
/**
 * @addtogroup EM_TIME Time Data Type and Manipulation.
 * @ingroup Em_OS
 * @{
 * This module provides API for manipulating time.
 *
 * \section em_time_examples_sec Examples
 *
 * For examples, please see:
 *  - \ref page_emlib_sleep_test
 */

/**
 * Get current time of day in local representation.
 *
 * @param tv	Variable to store the result.
 *
 * @return zero if successfull.
 */
EM_DECL(emlib_ret_t) em_gettimeofday(em_time_val *tv);


/**
 * Parse time value into date/time representation.
 *
 * @param tv	The time.
 * @param pt	Variable to store the date time result.
 *
 * @return zero if successfull.
 */
EM_DECL(emlib_ret_t) em_time_decode(const em_time_val *tv, em_parsed_time *pt);

/**
 * Encode date/time to time value.
 *
 * @param pt	The date/time.
 * @param tv	Variable to store time value result.
 *
 * @return zero if successfull.
 */
EM_DECL(emlib_ret_t) em_time_encode(const em_parsed_time *pt, em_time_val *tv);

/**
 * Convert local time to GMT.
 *
 * @param tv	Time to convert.  *
 * @return zero if successfull.
 */
EM_DECL(emlib_ret_t) em_time_local_to_gmt(em_time_val *tv);

/**
 * Convert GMT to local time.
 *
 * @param tv	Time to convert.
 *
 * @return zero if successfull.
 */
EM_DECL(emlib_ret_t) em_time_gmt_to_local(em_time_val *tv);

#if 0
/**
 * @}
 */

/* **************************************************************************/
#if defined(EM_TERM_HAS_COLOR) && EM_TERM_HAS_COLOR != 0

/**
 * @defgroup EM_TERM Terminal
 * @ingroup EM_OS
 * @{
 */

/**
 * Set current terminal color.
 *
 * @param color	    The RGB color.
 *
 * @return zero on success.
 */
EM_DECL(emlib_ret_t) em_term_set_color(em_color_t color);

/**
 * Get current terminal foreground color.
 *
 * @return RGB color.
 */
EM_DECL(em_color_t) em_term_get_color(void);

/**
 * @}
 */

#endif	/* EM_TERM_HAS_COLOR */
#endif

/* **************************************************************************/
/**
 * @defgroup EM_TIMESTAMP High Resolution Timestamp
 * @ingroup EM_OS
 * @{
 *
 * EMLIB provides <b>High Resolution Timestamp</b> API to access highest 
 * resolution timestamp value provided by the platform. The API is usefull
 * to measure precise elapsed time, and can be used in applications such
 * as profiling.
 *
 * The timestamp value is represented in cycles, and can be related to
 * normal time (in seconds or sub-seconds) using various functions provided.
 *
 * \section em_timestamp_examples_sec Examples
 *
 * For examples, please see:
 *  - \ref page_emlib_sleep_test
 *  - \ref page_emlib_timestamp_test
 */

/*
 * High resolution timer.
 */
#if defined(EM_HAS_HIGH_RES_TIMER) && EM_HAS_HIGH_RES_TIMER != 0

/**
 * Get monotonic time since some unspecified starting point.
 *
 * @param tv	Variable to store the result.
 *
 * @return EM_SUCC if successful.
 */
EM_DECL(emlib_ret_t) em_gettickcount(em_time_val *tv);

/**
 * Acquire high resolution timer value. The time value are stored
 * in cycles.
 *
 * @param ts	    High resolution timer value.
 * @return	    EM_SUCC or the appropriate error code.
 *
 * @see em_get_timestamp_freq().
 */
EM_DECL(emlib_ret_t) em_get_timestamp(em_timestamp *ts);

/**
 * Get high resolution timer frequency, in cycles per second.
 *
 * @param freq	    Timer frequency, in cycles per second.
 * @return	    EM_SUCC or the appropriate error code.
 */
EM_DECL(emlib_ret_t) em_get_timestamp_freq(em_timestamp *freq);

/**
 * Set timestamp from 32bit values.
 * @param t	    The timestamp to be set.
 * @param hi	    The high 32bit part.
 * @param lo	    The low 32bit part.
 */
EM_INLINE(void) em_set_timestamp32(em_timestamp *t, uint32 hi,
        uint32 lo)
{
    t->u32.hi = hi;
    t->u32.lo = lo;
}


/**
 * Compare timestamp t1 and t2.
 * @param t1	    t1.
 * @param t2	    t2.
 * @return	    -1 if (t1 < t2), 1 if (t1 > t2), or 0 if (t1 == t2)
 */
EM_INLINE(int) em_cmp_timestamp(const em_timestamp *t1, const em_timestamp *t2)
{
#if EM_HAS_INT64
    if (t1->u64 < t2->u64)
        return -1;
    else if (t1->u64 > t2->u64)
        return 1;
    else
        return 0;
#else
    if (t1->u32.hi < t2->u32.hi ||
            (t1->u32.hi == t2->u32.hi && t1->u32.lo < t2->u32.lo))
        return -1;
    else if (t1->u32.hi > t2->u32.hi ||
            (t1->u32.hi == t2->u32.hi && t1->u32.lo > t2->u32.lo))
        return 1;
    else
        return 0;
#endif
}


/**
 * Add timestamp t2 to t1.
 * @param t1	    t1.
 * @param t2	    t2.
 */
EM_INLINE(void) em_add_timestamp(em_timestamp *t1, const em_timestamp *t2)
{
#if EM_HAS_INT64
    t1->u64 += t2->u64;
#else
    uint32 old = t1->u32.lo;
    t1->u32.hi += t2->u32.hi;
    t1->u32.lo += t2->u32.lo;
    if (t1->u32.lo < old)
        ++t1->u32.hi;
#endif
}

/**
 * Add timestamp t2 to t1.
 * @param t1	    t1.
 * @param t2	    t2.
 */
EM_INLINE(void) em_add_timestamp32(em_timestamp *t1, uint32 t2)
{
#if EM_HAS_INT64
    t1->u64 += t2;
#else
    uint32 old = t1->u32.lo;
    t1->u32.lo += t2;
    if (t1->u32.lo < old)
        ++t1->u32.hi;
#endif
}

/**
 * Substract timestamp t2 from t1.
 * @param t1	    t1.
 * @param t2	    t2.
 */
EM_INLINE(void) em_sub_timestamp(em_timestamp *t1, const em_timestamp *t2)
{
#if EM_HAS_INT64
    t1->u64 -= t2->u64;
#else
    t1->u32.hi -= t2->u32.hi;
    if (t1->u32.lo >= t2->u32.lo)
        t1->u32.lo -= t2->u32.lo;
    else {
        t1->u32.lo -= t2->u32.lo;
        --t1->u32.hi;
    }
#endif
}

/**
 * Substract timestamp t2 from t1.
 * @param t1	    t1.
 * @param t2	    t2.
 */
EM_INLINE(void) em_sub_timestamp32(em_timestamp *t1, uint32 t2)
{
#if EM_HAS_INT64
    t1->u64 -= t2;
#else
    if (t1->u32.lo >= t2)
        t1->u32.lo -= t2;
    else {
        t1->u32.lo -= t2;
        --t1->u32.hi;
    }
#endif
}

/**
 * Get the timestamp difference between t2 and t1 (that is t2 minus t1),
 * and return a 32bit signed integer difference.
 */
EM_INLINE(int32) em_timestamp_diff32(const em_timestamp *t1,
        const em_timestamp *t2)
{
    /* Be careful with the signess (I think!) */
#if EM_HAS_INT64
    int64 diff = t2->u64 - t1->u64;
    return (int32) diff;
#else
    int32 diff = t2->u32.lo - t1->u32.lo;
    return diff;
#endif
}


/**
 * Calculate the elapsed time, and store it in em_time_val.
 * This function calculates the elapsed time using highest precision
 * calculation that is available for current platform, considering
 * whether floating point or 64-bit precision arithmetic is available. 
 * For maximum portability, application should prefer to use this function
 * rather than calculating the elapsed time by itself.
 *
 * @param start     The starting timestamp.
 * @param stop      The end timestamp.
 *
 * @return	    Elapsed time as #em_time_val.
 *
 * @see em_elapsed_usec(), em_elapsed_cycle(), em_elapsed_nanosec()
 */
EM_DECL(em_time_val) em_elapsed_time( const em_timestamp *start,
        const em_timestamp *stop );

/**
 * Calculate the elapsed time as 32-bit miliseconds.
 * This function calculates the elapsed time using highest precision
 * calculation that is available for current platform, considering
 * whether floating point or 64-bit precision arithmetic is available. 
 * For maximum portability, application should prefer to use this function
 * rather than calculating the elapsed time by itself.
 *
 * @param start     The starting timestamp.
 * @param stop      The end timestamp.
 *
 * @return	    Elapsed time in milisecond.
 *
 * @see em_elapsed_time(), em_elapsed_cycle(), em_elapsed_nanosec()
 */
EM_DECL(uint32) em_elapsed_msec( const em_timestamp *start,
        const em_timestamp *stop );

/**
 * Variant of #em_elapsed_msec() which returns 64bit value.
 */
EM_DECL(uint64) em_elapsed_msec64(const em_timestamp *start,
        const em_timestamp *stop );

/**
 * Calculate the elapsed time in 32-bit microseconds.
 * This function calculates the elapsed time using highest precision
 * calculation that is available for current platform, considering
 * whether floating point or 64-bit precision arithmetic is available. 
 * For maximum portability, application should prefer to use this function
 * rather than calculating the elapsed time by itself.
 *
 * @param start     The starting timestamp.
 * @param stop      The end timestamp.
 *
 * @return	    Elapsed time in microsecond.
 *
 * @see em_elapsed_time(), em_elapsed_cycle(), em_elapsed_nanosec()
 */
EM_DECL(uint32) em_elapsed_usec( const em_timestamp *start,
        const em_timestamp *stop );

/**
 * Calculate the elapsed time in 32-bit nanoseconds.
 * This function calculates the elapsed time using highest precision
 * calculation that is available for current platform, considering
 * whether floating point or 64-bit precision arithmetic is available. 
 * For maximum portability, application should prefer to use this function
 * rather than calculating the elapsed time by itself.
 *
 * @param start     The starting timestamp.
 * @param stop      The end timestamp.
 *
 * @return	    Elapsed time in nanoseconds.
 *
 * @see em_elapsed_time(), em_elapsed_cycle(), em_elapsed_usec()
 */
EM_DECL(uint32) em_elapsed_nanosec( const em_timestamp *start,
        const em_timestamp *stop );

/**
 * Calculate the elapsed time in 32-bit cycles.
 * This function calculates the elapsed time using highest precision
 * calculation that is available for current platform, considering
 * whether floating point or 64-bit precision arithmetic is available. 
 * For maximum portability, application should prefer to use this function
 * rather than calculating the elapsed time by itself.
 *
 * @param start     The starting timestamp.
 * @param stop      The end timestamp.
 *
 * @return	    Elapsed time in cycles.
 *
 * @see em_elapsed_usec(), em_elapsed_time(), em_elapsed_nanosec()
 */
EM_DECL(uint32) em_elapsed_cycle( const em_timestamp *start,
        const em_timestamp *stop );


#endif	/* EM_HAS_HIGH_RES_TIMER */

/**
 * Allows the calling thread to sleep for an interval specified with with second precision.
 *
 * @param  sv  The sleep interval.
 *
 * @return	  EM_SUCC or EM_Errno
 *
 */

EM_DECL(emlib_ret_t) em_sleep(uint32 sec);

/*
 * Sleep usec.
 */
EM_DECL(emlib_ret_t) em_usleep(uint32 usec);

/*
 * Sleep msec.
 */
EM_DECL(emlib_ret_t)em_msleep(uint32 msec);


/* **************************************************************************/
/**
 * @defgroup EM_APP_OS Application execution
 * @ingroup EM_OS
 * @{
 */

/**
 * Type for application main function.
 */
typedef int (*em_main_func_ptr)(int argc, char *argv[]);

/**
 * Run the application. This function has to be called in the main thread
 * and after doing the necessary initialization according to the flags
 * provided, it will call main_func() function.
 *
 * @param main_func Application's main function.
 * @param argc	    Number of arguments from the main() function, which
 * 		    will be passed to main_func() function.
 * @param argv	    The arguments from the main() function, which will
 * 		    be passed to main_func() function.
 * @param flags     Flags for application execution, currently must be 0.
 *
 * @return          main_func()'s return value.
 */
EM_DECL(int) em_run_app(em_main_func_ptr main_func, int argc, char *argv[],
        unsigned flags);

/** @} */


/* **************************************************************************/
/**
 * Internal EMLIB function to initialize the threading subsystem.
 * @return          EM_SUCC or the appropriate error code.
 */
emlib_ret_t em_thread_init(void);


DECLS_END

#endif  /* __EM_OS_H__ */

