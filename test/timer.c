/*
 * File:    timer.c
 * Author:  Liu HongLiang<lhl_nciae@sina.cn>
 * Brief:   TEST Module for Posix Thread.
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
#include "test.h"

static const char* module = "TIMER_TEST";

/**
 * \page page_emlib_timer_test Test: Timer
 *
 * This file provides implementation of \b timer_test(). It tests the
 * functionality of the timer heap.
 *
 */


#if INCLUDE_TIMER_TEST

#include <emlib.h>

#define LOOP		16
#define MIN_COUNT	250
#define MAX_COUNT	(LOOP * MIN_COUNT)
#define MIN_DELAY	4
#define	D		(MAX_COUNT / 32000)
#define DELAY		(D < MIN_DELAY ? MIN_DELAY : D)
#define THIS_FILE	"timer_test"


static void timer_callback(em_timer_heap_t *ht, em_timer_entry *e)
{
    EM_UNUSED_ARG(ht);
    EM_UNUSED_ARG(e);
}

static int test_timer_heap(void)
{
    int i, j;
    em_timer_entry *entry;
    em_pool_t *pool;
    em_timer_heap_t *timer;
    em_time_val delay;
    emlib_ret_t status;
    int err=0;
    em_size_t size;
    unsigned count;

    size = em_timer_heap_mem_size(MAX_COUNT)+MAX_COUNT*sizeof(em_timer_entry);
    pool = em_pool_create( mem, NULL, size, 4000, NULL);
    if (!pool) {
        EM_LOG(EM_LOG_INFO, "...error: unable to create pool of %u bytes", size);
        return -10;
    }

    entry = (em_timer_entry*)em_pool_calloc(pool, MAX_COUNT, sizeof(*entry));
    if (!entry)
        return -20;

    for (i=0; i<MAX_COUNT; ++i) {
        entry[i].cb = &timer_callback;
    }
    status = em_timer_heap_create(pool, MAX_COUNT, &timer);
    if (status != EM_SUCC) {
        app_perror("...error: unable to create timer heap", status);
        return -30;
    }

    count = MIN_COUNT;
    for (i=0; i<LOOP; ++i) {
        int early = 0;
        int done=0;
        int cancelled=0;
        int rc;
        em_timestamp t1, t2, t_sched, t_cancel, t_poll;
        em_time_val now, expire;

        em_gettimeofday(&now);
        em_srand(now.sec);
        t_sched.u32.lo = t_cancel.u32.lo = t_poll.u32.lo = 0;

        // Register timers
        for (j=0; j<(int)count; ++j) {
            delay.sec = em_rand() % DELAY;
            delay.msec = em_rand() % 1000;

            // Schedule timer
            em_get_timestamp(&t1);
            rc = em_timer_heap_schedule(timer, &entry[j], &delay);
            if (rc != 0)
                return -40;
            em_get_timestamp(&t2);

            t_sched.u32.lo += (t2.u32.lo - t1.u32.lo);

            // Poll timers.
            em_get_timestamp(&t1);
            rc = em_timer_heap_poll(timer, NULL);
            em_get_timestamp(&t2);
            if (rc > 0) {
                t_poll.u32.lo += (t2.u32.lo - t1.u32.lo);
                early += rc;
            }
        }

        // Set the time where all timers should finish
        em_gettimeofday(&expire);
        delay.sec = DELAY; 
        delay.msec = 0;
        EM_TIME_VAL_ADD(expire, delay);

        // Wait unfil all timers finish, cancel some of them.
        do {
            int index = em_rand() % count;
            em_get_timestamp(&t1);
            rc = em_timer_heap_cancel(timer, &entry[index]);
            em_get_timestamp(&t2);
            if (rc > 0) {
                cancelled += rc;
                t_cancel.u32.lo += (t2.u32.lo - t1.u32.lo);
            }

            em_gettimeofday(&now);

            em_get_timestamp(&t1);
            rc = em_timer_heap_poll(timer, NULL);
            em_get_timestamp(&t2);
            if (rc > 0) {
                done += rc;
                t_poll.u32.lo += (t2.u32.lo - t1.u32.lo);
            }

        } while (EM_TIME_VAL_LTE(now, expire)&&em_timer_heap_count(timer) > 0);

        if (em_timer_heap_count(timer)) {
            EM_LOG(EM_LOG_INFO, "ERROR: %d timers left", em_timer_heap_count(timer));
            ++err;
        }
        t_sched.u32.lo /= count; 
        t_cancel.u32.lo /= count;
        t_poll.u32.lo /= count;
        EM_LOG(EM_LOG_INFO, "...ok (count:%d, early:%d, done:%d, cancelled:%d, "
                "sched:%d, cancel:%d poll:%d)", 
                count, early, done, cancelled, t_sched.u32.lo, t_cancel.u32.lo,
                t_poll.u32.lo);

        count = count * 2;
        if (count > MAX_COUNT)
            break;
    }

    em_pool_release(pool);
    return err;
}


emlib_ret_t timer_test()
{
    return test_timer_heap();
}

#else
/* To prevent warning about "translation unit is empty"
 * when this test is disabled. 
 */
int dummy_timer_test;
#endif	/* INCLUDE_TIMER_TEST */


