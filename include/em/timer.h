/*
 * File:    timer.h
 * Author:  Liu HongLiang<lhl_nciae@sina.cn>
 * Brief:   
 * The timer scheduling implementation here is based on ACE library's 
 * ACE_Timer_Heap, with only little modification to suit our library's style
 * (I even left most of the comments in the original source).
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
 * 2019-01-12 Liu HongLiang<lhl_nciae@sina.cn> created.
 *
 */
#ifndef __EM_TIMER_H__
#define __EM_TIMER_H__

/**
 * @file timer.h
 * @brief Timer Heap
 */

#include <em/types.h>
#include <em/lock.h>

DECLS_BEGIN

/**
 * @defgroup EM_TIMER Timer Heap Management.
 * @ingroup EM_MISC
 *
 * To quote the original quote in ACE_Timer_Heap_T class:
 *
 *      This implementation uses a heap-based callout queue of
 *      absolute times.  Therefore, in the average and worst case,
 *      scheduling, canceling, and expiring timers is O(log N) (where
 *      N is the total number of timers).  In addition, we can also
 *      preallocate as many \a ACE_Timer_Nodes as there are slots in
 *      the heap.  This allows us to completely remove the need for
 *      dynamic memory allocation, which is important for real-time
 *      systems.
 *
 * You can find the fine ACE library at:
 *  http://www.cs.wustl.edu/~schmidt/ACE.html
 *
 * ACE is Copyright (C)1993-2006 Douglas C. Schmidt <d.schmidt@vanderbilt.edu>
 *
 * @{
 *
 * \section em_timer_examples_sec Examples
 *
 * For some examples on how to use the timer heap, please see the link below.
 *
 *  - \ref page_emlib_timer_test
 */


/**
 * The type for internal timer ID.
 */
typedef int em_timer_id_t;

/** 
 * Forward declaration for em_timer_entry. 
 */
struct em_timer_entry;

/**
 * The type of callback function to be called by timer scheduler when a timer
 * has expired.
 *
 * @param timer_heap    The timer heap.
 * @param entry         Timer entry which timer's has expired.
 */
typedef void em_timer_heap_callback(em_timer_heap_t *timer_heap,
        struct em_timer_entry *entry);


/**
 * This structure represents an entry to the timer.
 */
typedef struct em_timer_entry
{
    /** 
     * User data to be associated with this entry. 
     * Applications normally will put the instance of object that
     * owns the timer entry in this field.
     */
    void *user_data;

    /** 
     * Arbitrary ID assigned by the user/owner of this entry. 
     * Applications can use this ID to distinguish multiple
     * timer entries that share the same callback and user_data.
     */
    int id;

    /** 
     * Callback to be called when the timer expires. 
     */
    em_timer_heap_callback *cb;

    /** 
     * Internal unique timer ID, which is assigned by the timer heap. 
     * Application should not touch this ID.
     */
    em_timer_id_t _timer_id;

    /** 
     * The future time when the timer expires, which the value is updated
     * by timer heap when the timer is scheduled.
     */
    em_time_val _timer_value;

    /**
     * Internal: the group lock used by this entry, set when
     * em_timer_heap_schedule_w_lock() is used.
     */
    em_grp_lock_t *_grp_lock;

#if EM_TIMER_DEBUG
    const char	*src_file;
    int		 src_line;
#endif
} em_timer_entry;


/**
 * Calculate memory size required to create a timer heap.
 *
 * @param count     Number of timer entries to be supported.
 * @return          Memory size requirement in bytes.
 */
EM_DECL(em_size_t) em_timer_heap_mem_size(em_size_t count);

/**
 * Create a timer heap.
 *
 * @param pool      The pool where allocations in the timer heap will be 
 *                  allocated. The timer heap will dynamicly allocate 
 *                  more storate from the pool if the number of timer 
 *                  entries registered is more than the size originally 
 *                  requested when calling this function.
 * @param count     The maximum number of timer entries to be supported 
 *                  initially. If the application registers more entries 
 *                  during runtime, then the timer heap will resize.
 * @param ht        Pointer to receive the created timer heap.
 *
 * @return          EM_SUCC, or the appropriate error code.
 */
EM_DECL(emlib_ret_t) em_timer_heap_create( em_pool_t *pool,
        em_size_t count,
        em_timer_heap_t **ht);

/**
 * Destroy the timer heap.
 *
 * @param ht        The timer heap.
 */
EM_DECL(void) em_timer_heap_destroy( em_timer_heap_t *ht );


/**
 * Set lock object to be used by the timer heap. By default, the timer heap
 * uses dummy synchronization.
 *
 * @param ht        The timer heap.
 * @param lock      The lock object to be used for synchronization.
 * @param auto_del  If nonzero, the lock object will be destroyed when
 *                  the timer heap is destroyed.
 */
EM_DECL(void) em_timer_heap_set_lock( em_timer_heap_t *ht,
        em_lock_t *lock,
        em_bool_t auto_del );

/**
 * Set maximum number of timed out entries to process in a single poll.
 *
 * @param ht        The timer heap.
 * @param count     Number of entries.
 *
 * @return          The old number.
 */
EM_DECL(unsigned) em_timer_heap_set_max_timed_out_per_poll(em_timer_heap_t *ht,
        unsigned count );

/**
 * Initialize a timer entry. Application should call this function at least
 * once before scheduling the entry to the timer heap, to properly initialize
 * the timer entry.
 *
 * @param entry     The timer entry to be initialized.
 * @param id        Arbitrary ID assigned by the user/owner of this entry.
 *                  Applications can use this ID to distinguish multiple
 *                  timer entries that share the same callback and user_data.
 * @param user_data User data to be associated with this entry. 
 *                  Applications normally will put the instance of object that
 *                  owns the timer entry in this field.
 * @param cb        Callback function to be called when the timer elapses.
 *
 * @return          The timer entry itself.
 */
EM_DECL(em_timer_entry*) em_timer_entry_init( em_timer_entry *entry,
        int id,
        void *user_data,
        em_timer_heap_callback *cb );

/**
 * Queries whether a timer entry is currently running.
 *
 * @param entry     The timer entry to query.
 *
 * @return          EM_TRUE if the timer is running.  EM_FALSE if not.
 */
EM_DECL(em_bool_t) em_timer_entry_running( em_timer_entry *entry );

/**
 * Schedule a timer entry which will expire AFTER the specified delay.
 *
 * @param ht        The timer heap.
 * @param entry     The entry to be registered. 
 * @param delay     The interval to expire.
 * @return          EM_SUCC, or the appropriate error code.
 */
#if EM_TIMER_DEBUG
#  define em_timer_heap_schedule(ht,e,d) \
    em_timer_heap_schedule_dbg(ht,e,d,__FILE__,__LINE__)

EM_DECL(emlib_ret_t) em_timer_heap_schedule_dbg( em_timer_heap_t *ht,
        em_timer_entry *entry,
        const em_time_val *delay,
        const char *src_file,
        int src_line);
#else
EM_DECL(emlib_ret_t) em_timer_heap_schedule( em_timer_heap_t *ht,
        em_timer_entry *entry, 
        const em_time_val *delay);
#endif	/* EM_TIMER_DEBUG */

/**
 * Schedule a timer entry which will expire AFTER the specified delay, and
 * increment the reference counter of the group lock while the timer entry
 * is active. The group lock reference counter will automatically be released
 * after the timer callback is called or when the timer is cancelled.
 *
 * @param ht        The timer heap.
 * @param entry     The entry to be registered.
 * @param id_val    The value to be set to the "id" field of the timer entry
 * 		    once the timer is scheduled.
 * @param delay     The interval to expire.
 * @param grp_lock  The group lock.
 *
 * @return          EM_SUCC, or the appropriate error code.
 */
#if EM_TIMER_DEBUG
#  define em_timer_heap_schedule_w_grp_lock(ht,e,d,id,g) \
    em_timer_heap_schedule_w_grp_lock_dbg(ht,e,d,id,g,__FILE__,__LINE__)

EM_DECL(emlib_ret_t) em_timer_heap_schedule_w_grp_lock_dbg(
        em_timer_heap_t *ht,
        em_timer_entry *entry,
        const em_time_val *delay,
        int id_val,
        em_grp_lock_t *grp_lock,
        const char *src_file,
        int src_line);
#else
EM_DECL(emlib_ret_t) em_timer_heap_schedule_w_grp_lock(
        em_timer_heap_t *ht,
        em_timer_entry *entry,
        const em_time_val *delay,
        int id_val,
        em_grp_lock_t *grp_lock);
#endif	/* EM_TIMER_DEBUG */

/**
 * Cancel a previously registered timer. This will also decrement the
 * reference counter of the group lock associated with the timer entry,
 * if the entry was scheduled with one.
 *
 * @param ht        The timer heap.
 * @param entry     The entry to be cancelled.
 * @return          The number of timer cancelled, which should be one if the
 *                  entry has really been registered, or zero if no timer was
 *                  cancelled.
 */
EM_DECL(int) em_timer_heap_cancel( em_timer_heap_t *ht,
        em_timer_entry *entry);

/**
 * Cancel only if the previously registered timer is active. This will
 * also decrement the reference counter of the group lock associated
 * with the timer entry, if the entry was scheduled with one. In any
 * case, set the "id" to the specified value.
 *
 * @param ht        The timer heap.
 * @param entry     The entry to be cancelled.
 * @param id_val    Value to be set to "id"
 *
 * @return          The number of timer cancelled, which should be one if the
 *                  entry has really been registered, or zero if no timer was
 *                  cancelled.
 */
EM_DECL(int) em_timer_heap_cancel_if_active(em_timer_heap_t *ht,
        em_timer_entry *entry,
        int id_val);

/**
 * Get the number of timer entries.
 *
 * @param ht        The timer heap.
 * @return          The number of timer entries.
 */
EM_DECL(em_size_t) em_timer_heap_count( em_timer_heap_t *ht );

/**
 * Get the earliest time registered in the timer heap. The timer heap
 * MUST have at least one timer being scheduled (application should use
 * #em_timer_heap_count() before calling this function).
 *
 * @param ht        The timer heap.
 * @param timeval   The time deadline of the earliest timer entry.
 *
 * @return          EM_SUCC, or EM_ENOTFOUND if no entry is scheduled.
 */
EM_DECL(emlib_ret_t) em_timer_heap_earliest_time( em_timer_heap_t *ht, 
        em_time_val *timeval);

/**
 * Poll the timer heap, check for expired timers and call the callback for
 * each of the expired timers.
 *
 * @param ht         The timer heap.
 * @param next_delay If this parameter is not NULL, it will be filled up with
 *		     the time delay until the next timer elapsed, or 
 *		     EM_MAXINT32 in the sec part if no entry exist.
 *
 * @return           The number of timers expired.
 */
EM_DECL(unsigned) em_timer_heap_poll( em_timer_heap_t *ht, 
        em_time_val *next_delay);

#if EM_TIMER_DEBUG
/**
 * Dump timer heap entries.
 *
 * @param ht	    The timer heap.
 */
EM_DECL(void) em_timer_heap_dump(em_timer_heap_t *ht);
#endif

/**
 * @}
 */

DECLS_END

#endif	/* __EM_TIMER_H__ */

