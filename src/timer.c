/*
 * File:    timer.c
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
#include <em/timer.h>
#include <em/pool.h>
#include <em/os.h>
#include <em/string.h>
#include <em/assert.h>
#include <em/errno.h>
#include <em/lock.h>
#include <em/log.h>
#include <em/rand.h>
#include <em/limits.h>

static const char* module = "heap_timer";

#define HEAP_PARENT(X)	(X == 0 ? 0 : (((X) - 1) / 2))
#define HEAP_LEFT(X)	(((X)+(X))+1)


#define DEFAULT_MAX_TIMED_OUT_PER_POLL  (64)

enum
{
    F_DONT_CALL = 1,
    F_DONT_ASSERT = 2,
    F_SET_ID = 4
};


/**
 * The implementation of timer heap.
 */
struct em_timer_heap_t
{
    /** Pool from which the timer heap resize will get the storage from */
    em_pool_t *pool;

    /** Maximum size of the heap. */
    em_size_t max_size;

    /** Current size of the heap. */
    em_size_t cur_size;

    /** Max timed out entries to process per poll. */
    unsigned max_entries_per_poll;

    /** Lock object. */
    em_lock_t *lock;

    /** Autodelete lock. */
    em_bool_t auto_delete_lock;

    /**
     * Current contents of the Heap, which is organized as a "heap" of
     * em_timer_entry *'s.  In this context, a heap is a "partially
     * ordered, almost complete" binary tree, which is stored in an
     * array.
     */
    em_timer_entry **heap;

    /**
     * An array of "pointers" that allows each em_timer_entry in the
     * <heap_> to be located in O(1) time.  Basically, <timer_id_[i]>
     * contains the slot in the <heap_> array where an em_timer_entry
     * with timer id <i> resides.  Thus, the timer id passed back from
     * <schedule_entry> is really an slot into the <timer_ids> array.  The
     * <timer_ids_> array serves two purposes: negative values are
     * treated as "pointers" for the <freelist_>, whereas positive
     * values are treated as "pointers" into the <heap_> array.
     */
    em_timer_id_t *timer_ids;

    /**
     * "Pointer" to the first element in the freelist contained within
     * the <timer_ids_> array, which is organized as a stack.
     */
    em_timer_id_t timer_ids_freelist;

    /** Callback to be called when a timer expires. */
    em_timer_heap_callback *callback;

};



EM_INLINE(void) lock_timer_heap( em_timer_heap_t *ht )
{
    if (ht->lock) {
        em_lock(ht->lock);
    }
}

EM_INLINE(void) unlock_timer_heap( em_timer_heap_t *ht )
{
    if (ht->lock) {
        em_unlock(ht->lock);
    }
}


static void copy_node( em_timer_heap_t *ht, em_size_t slot, 
        em_timer_entry *moved_node )
{
    EM_CHECK_STACK();

    // Insert <moved_node> into its new location in the heap.
    ht->heap[slot] = moved_node;

    // Update the corresponding slot in the parallel <timer_ids_> array.
    ht->timer_ids[moved_node->_timer_id] = (int)slot;
}

static em_timer_id_t pop_freelist( em_timer_heap_t *ht )
{
    // We need to truncate this to <int> for backwards compatibility.
    em_timer_id_t new_id = ht->timer_ids_freelist;

    EM_CHECK_STACK();

    // The freelist values in the <timer_ids_> are negative, so we need
    // to negate them to get the next freelist "pointer."
    ht->timer_ids_freelist =
        -ht->timer_ids[ht->timer_ids_freelist];

    return new_id;

}

static void push_freelist (em_timer_heap_t *ht, em_timer_id_t old_id)
{
    EM_CHECK_STACK();

    // The freelist values in the <timer_ids_> are negative, so we need
    // to negate them to get the next freelist "pointer."
    ht->timer_ids[old_id] = -ht->timer_ids_freelist;
    ht->timer_ids_freelist = old_id;
}


static void reheap_down(em_timer_heap_t *ht, em_timer_entry *moved_node,
        size_t slot, size_t child)
{
    EM_CHECK_STACK();

    // Restore the heap property after a deletion.

    while (child < ht->cur_size)
    {
        // Choose the smaller of the two children.
        if (child + 1 < ht->cur_size
                && EM_TIME_VAL_LT(ht->heap[child + 1]->_timer_value, ht->heap[child]->_timer_value))
            child++;

        // Perform a <copy> if the child has a larger timeout value than
        // the <moved_node>.
        if (EM_TIME_VAL_LT(ht->heap[child]->_timer_value, moved_node->_timer_value))
        {
            copy_node( ht, slot, ht->heap[child]);
            slot = child;
            child = HEAP_LEFT(child);
        }
        else
            // We've found our location in the heap.
            break;
    }

    copy_node( ht, slot, moved_node);
}

static void reheap_up( em_timer_heap_t *ht, em_timer_entry *moved_node,
        size_t slot, size_t parent)
{
    // Restore the heap property after an insertion.

    while (slot > 0)
    {
        // If the parent node is greater than the <moved_node> we need
        // to copy it down.
        if (EM_TIME_VAL_LT(moved_node->_timer_value, ht->heap[parent]->_timer_value))
        {
            copy_node(ht, slot, ht->heap[parent]);
            slot = parent;
            parent = HEAP_PARENT(slot);
        }
        else
            break;
    }

    // Insert the new node into its proper resting place in the heap and
    // update the corresponding slot in the parallel <timer_ids> array.
    copy_node(ht, slot, moved_node);
}


static em_timer_entry * remove_node( em_timer_heap_t *ht, size_t slot)
{
    em_timer_entry *removed_node = ht->heap[slot];

    // Return this timer id to the freelist.
    push_freelist( ht, removed_node->_timer_id );

    // Decrement the size of the heap by one since we're removing the
    // "slot"th node.
    ht->cur_size--;

    // Set the ID
    removed_node->_timer_id = -1;

    // Only try to reheapify if we're not deleting the last entry.

    if (slot < ht->cur_size)
    {
        em_size_t parent;
        em_timer_entry *moved_node = ht->heap[ht->cur_size];

        // Move the end node to the location being removed and update
        // the corresponding slot in the parallel <timer_ids> array.
        copy_node( ht, slot, moved_node);

        // If the <moved_node->time_value_> is great than or equal its
        // parent it needs be moved down the heap.
        parent = HEAP_PARENT (slot);

        if (EM_TIME_VAL_GTE(moved_node->_timer_value, ht->heap[parent]->_timer_value))
            reheap_down( ht, moved_node, slot, HEAP_LEFT(slot));
        else
            reheap_up( ht, moved_node, slot, parent);
    }

    return removed_node;
}

static void grow_heap(em_timer_heap_t *ht)
{
    // All the containers will double in size from max_size_
    size_t new_size = ht->max_size * 2;
    em_timer_id_t *new_timer_ids;
    em_size_t i;

    // First grow the heap itself.

    em_timer_entry **new_heap = 0;

    new_heap = (em_timer_entry**) 
        em_pool_alloc(ht->pool, sizeof(em_timer_entry*) * new_size);
    memcpy(new_heap, ht->heap, ht->max_size * sizeof(em_timer_entry*));
    //delete [] this->heap_;
    ht->heap = new_heap;

    // Grow the array of timer ids.

    new_timer_ids = 0;
    new_timer_ids = (em_timer_id_t*)
        em_pool_alloc(ht->pool, new_size * sizeof(em_timer_id_t));

    memcpy( new_timer_ids, ht->timer_ids, ht->max_size * sizeof(em_timer_id_t));

    //delete [] timer_ids_;
    ht->timer_ids = new_timer_ids;

    // And add the new elements to the end of the "freelist".
    for (i = ht->max_size; i < new_size; i++)
        ht->timer_ids[i] = -((em_timer_id_t) (i + 1));

    ht->max_size = new_size;
}

static void insert_node(em_timer_heap_t *ht, em_timer_entry *new_node)
{
    if (ht->cur_size + 2 >= ht->max_size)
        grow_heap(ht);

    reheap_up( ht, new_node, ht->cur_size, HEAP_PARENT(ht->cur_size));
    ht->cur_size++;
}


static emlib_ret_t schedule_entry( em_timer_heap_t *ht,
        em_timer_entry *entry, 
        const em_time_val *future_time )
{
    if (ht->cur_size < ht->max_size)
    {
        // Obtain the next unique sequence number.
        // Set the entry
        entry->_timer_id = pop_freelist(ht);
        entry->_timer_value = *future_time;
        insert_node( ht, entry);
        return 0;
    }
    else
        return -1;
}


static int cancel( em_timer_heap_t *ht, 
        em_timer_entry *entry, 
        unsigned flags)
{
    long timer_node_slot;

    EM_CHECK_STACK();

    // Check to see if the timer_id is out of range
    if (entry->_timer_id < 0 || (em_size_t)entry->_timer_id > ht->max_size) {
        entry->_timer_id = -1;
        return 0;
    }

    timer_node_slot = ht->timer_ids[entry->_timer_id];

    if (timer_node_slot < 0) { // Check to see if timer_id is still valid.
        entry->_timer_id = -1;
        return 0;
    }

    if (entry != ht->heap[timer_node_slot])
    {
        if ((flags & F_DONT_ASSERT) == 0)
            EMLIB_ASSERT(entry == ht->heap[timer_node_slot]);
        entry->_timer_id = -1;
        return 0;
    }
    else
    {
        remove_node( ht, timer_node_slot);

        if ((flags & F_DONT_CALL) == 0)
            // Call the close hook.
            (*ht->callback)(ht, entry);
        return 1;
    }
}


/*
 * Calculate memory size required to create a timer heap.
 */
EM_DEF(em_size_t) em_timer_heap_mem_size(em_size_t count)
{
    return /* size of the timer heap itself: */
        sizeof(em_timer_heap_t) + 
        /* size of each entry: */
        (count+2) * (sizeof(em_timer_entry*)+sizeof(em_timer_id_t)) +
        /* lock, pool etc: */
        132;
}

/*
 * Create a new timer heap.
 */
EM_DEF(emlib_ret_t) em_timer_heap_create( em_pool_t *pool,
        em_size_t size,
        em_timer_heap_t **p_heap)
{
    em_timer_heap_t *ht;
    em_size_t i;

    EMLIB_ASSERT_RETURN(pool && p_heap, EM_EINVAL);

    *p_heap = NULL;

    /* Magic? */
    size += 2;

    /* Allocate timer heap data structure from the pool */
    ht = EM_POOL_ALLOC_T(pool, em_timer_heap_t);
    if (!ht)
        return EM_ENOMEM;

    /* Initialize timer heap sizes */
    ht->max_size = size;
    ht->cur_size = 0;
    ht->max_entries_per_poll = DEFAULT_MAX_TIMED_OUT_PER_POLL;
    ht->timer_ids_freelist = 1;
    ht->pool = pool;

    /* Lock. */
    ht->lock = NULL;
    ht->auto_delete_lock = 0;

    // Create the heap array.
    ht->heap = (em_timer_entry**)
        em_pool_alloc(pool, sizeof(em_timer_entry*) * size);
    if (!ht->heap)
        return EM_ENOMEM;

    // Create the parallel
    ht->timer_ids = (em_timer_id_t *)
        em_pool_alloc( pool, sizeof(em_timer_id_t) * size);
    if (!ht->timer_ids)
        return EM_ENOMEM;

    // Initialize the "freelist," which uses negative values to
    // distinguish freelist elements from "pointers" into the <heap_>
    // array.
    for (i=0; i<size; ++i)
        ht->timer_ids[i] = -((em_timer_id_t) (i + 1));

    *p_heap = ht;
    return EM_SUCC;
}

EM_DEF(void) em_timer_heap_destroy( em_timer_heap_t *ht )
{
    if (ht->lock && ht->auto_delete_lock) {
        em_lock_destroy(ht->lock);
        ht->lock = NULL;
    }
}

EM_DEF(void) em_timer_heap_set_lock(  em_timer_heap_t *ht,
        em_lock_t *lock,
        em_bool_t auto_del )
{
    if (ht->lock && ht->auto_delete_lock)
        em_lock_destroy(ht->lock);

    ht->lock = lock;
    ht->auto_delete_lock = auto_del;
}


EM_DEF(unsigned) em_timer_heap_set_max_timed_out_per_poll(em_timer_heap_t *ht,
        unsigned count )
{
    unsigned old_count = ht->max_entries_per_poll;
    ht->max_entries_per_poll = count;
    return old_count;
}

EM_DEF(em_timer_entry*) em_timer_entry_init( em_timer_entry *entry,
        int id,
        void *user_data,
        em_timer_heap_callback *cb )
{
    EM_ASSERT(entry && cb);

    entry->_timer_id = -1;
    entry->id = id;
    entry->user_data = user_data;
    entry->cb = cb;
    entry->_grp_lock = NULL;

    return entry;
}

EM_DEF(em_bool_t) em_timer_entry_running( em_timer_entry *entry )
{
    return (entry->_timer_id >= 1);
}

#if EM_TIMER_DEBUG
static emlib_ret_t schedule_w_grp_lock_dbg(em_timer_heap_t *ht,
        em_timer_entry *entry,
        const em_time_val *delay,
        em_bool_t set_id,
        int id_val,
        em_grp_lock_t *grp_lock,
        const char *src_file,
        int src_line)
#else
static emlib_ret_t schedule_w_grp_lock(em_timer_heap_t *ht,
        em_timer_entry *entry,
        const em_time_val *delay,
        em_bool_t set_id,
        int id_val,
        em_grp_lock_t *grp_lock)
#endif
{
    emlib_ret_t status;
    em_time_val expires;

    EMLIB_ASSERT_RETURN(ht && entry && delay, EM_EINVAL);
    EMLIB_ASSERT_RETURN(entry->cb != NULL, EM_EINVAL);

    /* Prevent same entry from being scheduled more than once */
    EMLIB_ASSERT_RETURN(entry->_timer_id < 1, EM_EINVALIDOP);

#if EM_TIMER_DEBUG
    entry->src_file = src_file;
    entry->src_line = src_line;
#endif
    em_gettickcount(&expires);
    EM_TIME_VAL_ADD(expires, *delay);

    lock_timer_heap(ht);
    status = schedule_entry(ht, entry, &expires);
    if (status == EM_SUCC) {
        if (set_id)
            entry->id = id_val;
        entry->_grp_lock = grp_lock;
        if (entry->_grp_lock) {
            em_grp_lock_add_ref(entry->_grp_lock);
        }
    }
    unlock_timer_heap(ht);

    return status;
}


#if EM_TIMER_DEBUG
EM_DEF(emlib_ret_t) em_timer_heap_schedule_dbg( em_timer_heap_t *ht,
        em_timer_entry *entry,
        const em_time_val *delay,
        const char *src_file,
        int src_line)
{
    return schedule_w_grp_lock_dbg(ht, entry, delay, EM_FALSE, 1, NULL,
            src_file, src_line);
}

EM_DEF(emlib_ret_t) em_timer_heap_schedule_w_grp_lock_dbg(
        em_timer_heap_t *ht,
        em_timer_entry *entry,
        const em_time_val *delay,
        int id_val,
        em_grp_lock_t *grp_lock,
        const char *src_file,
        int src_line)
{
    return schedule_w_grp_lock_dbg(ht, entry, delay, EM_TRUE, id_val,
            grp_lock, src_file, src_line);
}

#else
EM_DEF(emlib_ret_t) em_timer_heap_schedule( em_timer_heap_t *ht,
        em_timer_entry *entry,
        const em_time_val *delay)
{
    return schedule_w_grp_lock(ht, entry, delay, EM_FALSE, 1, NULL);
}

EM_DEF(emlib_ret_t) em_timer_heap_schedule_w_grp_lock(em_timer_heap_t *ht,
        em_timer_entry *entry,
        const em_time_val *delay,
        int id_val,
        em_grp_lock_t *grp_lock)
{
    return schedule_w_grp_lock(ht, entry, delay, EM_TRUE, id_val, grp_lock);
}
#endif

static int cancel_timer(em_timer_heap_t *ht,
        em_timer_entry *entry,
        unsigned flags,
        int id_val)
{
    int count;

    EMLIB_ASSERT_RETURN(ht && entry, EM_EINVAL);

    lock_timer_heap(ht);
    count = cancel(ht, entry, flags | F_DONT_CALL);
    if (flags & F_SET_ID) {
        entry->id = id_val;
    }
    if (entry->_grp_lock) {
        em_grp_lock_t *grp_lock = entry->_grp_lock;
        entry->_grp_lock = NULL;
        em_grp_lock_dec_ref(grp_lock);
    }
    unlock_timer_heap(ht);

    return count;
}

EM_DEF(int) em_timer_heap_cancel( em_timer_heap_t *ht,
        em_timer_entry *entry)
{
    return cancel_timer(ht, entry, 0, 0);
}

EM_DEF(int) em_timer_heap_cancel_if_active(em_timer_heap_t *ht,
        em_timer_entry *entry,
        int id_val)
{
    return cancel_timer(ht, entry, F_SET_ID | F_DONT_ASSERT, id_val);
}

EM_DEF(unsigned) em_timer_heap_poll( em_timer_heap_t *ht, 
        em_time_val *next_delay )
{
    em_time_val now;
    unsigned count;

    EMLIB_ASSERT_RETURN(ht, 0);

    lock_timer_heap(ht);
    if (!ht->cur_size && next_delay) {
        next_delay->sec = next_delay->msec = EM_MAXINT32;
        unlock_timer_heap(ht);
        return 0;
    }

    count = 0;
    em_gettickcount(&now);

    while ( ht->cur_size && 
            EM_TIME_VAL_LTE(ht->heap[0]->_timer_value, now) &&
            count < ht->max_entries_per_poll ) 
    {
        em_timer_entry *node = remove_node(ht, 0);
        /* Avoid re-use of this timer until the callback is done. */
        em_timer_id_t node_timer_id = pop_freelist(ht);
        em_grp_lock_t *grp_lock;

        ++count;

        grp_lock = node->_grp_lock;
        node->_grp_lock = NULL;

        unlock_timer_heap(ht);

        EM_RACE_ME(5);

        if (node->cb)
            (*node->cb)(ht, node);

        if (grp_lock)
            em_grp_lock_dec_ref(grp_lock);

        lock_timer_heap(ht);
        /* Now, the timer is really free for re-use. */
        push_freelist(ht, node_timer_id);
    }
    if (ht->cur_size && next_delay) {
        *next_delay = ht->heap[0]->_timer_value;
        EM_TIME_VAL_SUB(*next_delay, now);
        if (next_delay->sec < 0 || next_delay->msec < 0)
            next_delay->sec = next_delay->msec = 0;
    } else if (next_delay) {
        next_delay->sec = next_delay->msec = EM_MAXINT32;
    }
    unlock_timer_heap(ht);

    return count;
}

EM_DEF(em_size_t) em_timer_heap_count( em_timer_heap_t *ht )
{
    EMLIB_ASSERT_RETURN(ht, 0);

    return ht->cur_size;
}

EM_DEF(emlib_ret_t) em_timer_heap_earliest_time( em_timer_heap_t * ht,
        em_time_val *timeval)
{
    EMLIB_ASSERT_RETURN(ht->cur_size != 0, EM_EINVAL);

    if (ht->cur_size == 0)
        return EM_ENOTFOUND;

    lock_timer_heap(ht);
    *timeval = ht->heap[0]->_timer_value;
    unlock_timer_heap(ht);

    return EM_SUCC;
}

#if EM_TIMER_DEBUG
EM_DEF(void) em_timer_heap_dump(em_timer_heap_t *ht)
{
    lock_timer_heap(ht);

    EM_LOG(EM_LOG_DEBUG, "Dumping timer heap:");
    EM_LOG(EM_LOG_DEBUG, "  Cur size: %d entries, max: %d",
            (int)ht->cur_size, (int)ht->max_size);

    if (ht->cur_size) {
        unsigned i;
        em_time_val now;

        EM_LOG(EM_LOG_DEBUG, "  Entries: ");
        EM_LOG(EM_LOG_DEBUG, "    _id\tId\tElapsed\tSource");
        EM_LOG(EM_LOG_DEBUG, "    ----------------------------------");

        em_gettickcount(&now);

        for (i=0; i<(unsigned)ht->cur_size; ++i) {
            em_timer_entry *e = ht->heap[i];
            em_time_val delta;

            if (EM_TIME_VAL_LTE(e->_timer_value, now))
                delta.sec = delta.msec = 0;
            else {
                delta = e->_timer_value;
                EM_TIME_VAL_SUB(delta, now);
            }

            EM_LOG(EM_LOG_DEBUG,, "    %d\t%d\t%d.%03d\t%s:%d",
                    e->_timer_id, e->id,
                    (int)delta.sec, (int)delta.msec,
                    e->src_file, e->src_line);
        }
    }

    unlock_timer_heap(ht);
}
#endif

