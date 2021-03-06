/*
 * File:    thread.c
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
 * 2019-01-04 Liu HongLiang<lhl_nciae@sina.cn> created.
 *
 */
#include "emlib.h"
#include "test.h"

/**
 * \page page_emlib_thread_test Test: Thread Test
 *
 * This file contains \a thread_test() definition.
 *
 * \section thread_test_scope_sec Scope of Test
 * This tests:
 *  - whether EM_THREAD_SUSPENDED flag works.
 *  - whether multithreading works.
 *  - whether thread timeslicing works, and threads have equal
 *    time-slice proportion.
 *
 * APIs tested:
 *  - em_thread_create()
 *  - em_thread_register()
 *  - em_thread_this()
 *  - em_thread_get_name()
 *  - em_thread_destroy()
 *  - em_thread_resume()
 *  - em_thread_sleep()
 *  - em_thread_join()
 *  - em_thread_destroy()
 *
 *
 * This file is <b>emlib-test/thread.c</b>
 *
 * \include emlib-test/thread.c
 */
#if INCLUDE_THREAD_TEST

#include <emlib.h>

static const char *module = "thread_test";

static volatile int quit_flag=0;

static em_thread_desc desc;

/*
 * Stack overflow check.
 **/
static void* stack_overflow(void *args)
{
    char overbuff[8196];
    EM_CHECK_STACK();
}

/*
 * The thread's entry point.
 *
 * Each of the thread mainly will just execute the loop which
 * increments a variable.
 */
static void* thread_proc(uint32 *pcounter)
{
    /* Test that em_thread_register() works. */
    em_thread_t *this_thread;
    unsigned id;
    emlib_ret_t rc;

    id = *pcounter;
    EM_UNUSED_ARG(id); /* Warning about unused var if TRACE__ is disabled */
    EM_LOG(EM_LOG_INFO, "     thread %d running..", id);

    em_bzero(desc, sizeof(desc));

    EMLIB_ASSERT_RETURN(sizeof(em_thread_desc) >= em_get_threadid_size(), NULL);
    EM_LOG(EM_LOG_DEBUG, "sizeof(em_thread_desc):%d, sizeof(em_thread_t):%d", sizeof(em_thread_desc), em_get_threadid_size());

#if 0
    /*BUG#2: The thread created by use em_thread_create SHOULD NOT call em_thread_register.*/
    rc = em_thread_register("thread", desc, &this_thread);
    if (rc != EM_SUCC) {
        app_perror("...error in em_thread_register", rc);
        return NULL;
    }
#endif

    /* Test that em_thread_this() works */
    this_thread = em_thread_this();
    if (this_thread == NULL) {
        EM_LOG(3, "...error: em_thread_this() returns NULL!");
        return NULL;
    }

    /* Test that em_thread_get_name() works */
    if (em_thread_get_name(this_thread) == NULL) {
        EM_LOG(3, "...error: em_thread_get_name() returns NULL!");
        return NULL;
    }

    /* Main loop */
    for (;!quit_flag;) {
        (*pcounter)++;
        //Must sleep if platform doesn't do time-slicing.
        //em_thread_sleep(0);
    }

    EM_LOG(EM_LOG_INFO, "     thread %d quitting..", id);
    return NULL;
}

/*
 * stack overflow check.
 * */
static int overflow_check_thread(const char *title, unsigned flags)
{
    em_pool_t *pool;
    em_thread_t *thread;
    emlib_ret_t rc;
    uint32 counter = 0;

    EM_LOG(3, "..%s", title);

    pool = em_pool_create(mem, "simple_thread", 4000, 4000, NULL);
    if (!pool)
        return -1000;

    quit_flag = 0;

    EM_LOG(EM_LOG_TRACE, "    Creating thread 0..");
    rc = em_thread_create(pool, "thread", (em_thread_proc*)&stack_overflow,
            &counter,
            EM_THREAD_DEFAULT_STACK_SIZE,
            flags,
            &thread);

    if (rc != EM_SUCC) {
        app_perror("...error: unable to create thread", rc);
        return -1010;
    }

    EM_LOG(EM_LOG_TRACE, "    Main thread waiting..");
    em_thread_sleep(1500);
    EM_LOG(EM_LOG_TRACE, "    Main thread resuming..");

    EM_LOG(3,  "..waiting for thread to quit..");
    em_thread_sleep(1500);

    quit_flag = 1;
    em_thread_join(thread);
    em_pool_release(pool);

    EM_LOG(3, "...%s success", title);

    return EM_SUCC;
}

/*
 * simple_thread()
 */
static int simple_thread(const char *title, unsigned flags)
{
    em_pool_t *pool;
    em_thread_t *thread;
    emlib_ret_t rc;
    uint32 counter = 0;

    EM_LOG(3, "..%s", title);

    pool = em_pool_create(mem, "simple_thread", 4000, 4000, NULL);
    if (!pool)
        return -1000;

    quit_flag = 0;

    EM_LOG(EM_LOG_TRACE, "    Creating thread 0..");
    rc = em_thread_create(pool, "thread", (em_thread_proc*)&thread_proc,
            &counter,
            EM_THREAD_DEFAULT_STACK_SIZE,
            flags,
            &thread);

    if (rc != EM_SUCC) {
        app_perror("...error: unable to create thread", rc);
        return -1010;
    }

    EM_LOG(EM_LOG_TRACE, "    Main thread waiting..");
    em_thread_sleep(1500);
    EM_LOG(EM_LOG_TRACE, "    Main thread resuming..");

    if (flags & EM_THREAD_SUSPENDED) {

        /* Check that counter is still zero */
        if (counter != 0) {
            EM_LOG(3, "...error: thread is not suspended");
            return -1015;
        }

        rc = em_thread_resume(thread);
        if (rc != EM_SUCC) {
            app_perror("...error: resume thread error", rc);
            return -1020;
        }
    }

    EM_LOG(3,  "..waiting for thread to quit..");

    em_thread_sleep(1500);

    quit_flag = 1;
    em_thread_join(thread);

    em_pool_release(pool);

    if (counter == 0) {
        EM_LOG(3, "...error: thread is not running");
        return -1025;
    }

    EM_LOG(3, "...%s success", title);
    return EM_SUCC;
}


/*
 * timeslice_test()
 */
static int timeslice_test(void)
{
    enum { NUM_THREADS = 4 };
    em_pool_t *pool;
    uint32 counter[NUM_THREADS], lowest, highest, diff;
    em_thread_t *thread[NUM_THREADS];
    char thread_name[EM_MAX_OBJ_NAME];
    
    unsigned i;
    emlib_ret_t rc;

    quit_flag = 0;

    pool = em_pool_create(mem, NULL, 4000, 4000, NULL);
    if (!pool)
        return -10;

    EM_LOG(3, "..timeslice testing with %d threads", NUM_THREADS);

    /* Create all threads in suspended mode. */
    for (i=0; i<NUM_THREADS; ++i) {
        em_bzero(thread_name, EM_MAX_OBJ_NAME);
        em_ansi_snprintf(thread_name, EM_MAX_OBJ_NAME, "thread-%d", i);        
        counter[i] = i;
        rc = em_thread_create(pool, thread_name, (em_thread_proc*)&thread_proc,
                &counter[i],
                EM_THREAD_DEFAULT_STACK_SIZE,
                EM_THREAD_SUSPENDED,
                &thread[i]);
        if (rc!=EM_SUCC) {
            app_perror("...ERROR in em_thread_create()", rc);
            return -20;
        }
    }

    /* Sleep for 1 second.
     * The purpose of this is to test whether all threads are suspended.
     */
    EM_LOG(EM_LOG_TRACE, "    Main thread waiting..");
    em_thread_sleep(1000);
    EM_LOG(EM_LOG_TRACE, "    Main thread resuming..");

    /* Check that all counters are still zero. */
    for (i=0; i<NUM_THREADS; ++i) {
        if (counter[i] > i) {
            EM_LOG(3, "....ERROR! Thread %d-th is not suspended!", i);
            return -30;
        }
    }

    /* Now resume all threads. */
    for (i=0; i<NUM_THREADS; ++i) {
        EM_LOG(EM_LOG_TRACE, "    Resuming thread %d [%p]..", i, thread[i]);
        rc = em_thread_resume(thread[i]);
        if (rc != EM_SUCC) {
            app_perror("...ERROR in em_thread_resume()", rc);
            return -40;
        }
    }

    /* Main thread sleeps for some time to allow threads to run.
     * The longer we sleep, the more accurate the calculation will be,
     * but it'll make user waits for longer for the test to finish.
     */
    EM_LOG(EM_LOG_TRACE, "    Main thread waiting (5s)..");
    em_thread_sleep(5000);
    EM_LOG(EM_LOG_TRACE, "    Main thread resuming..");

    /* Signal all threads to quit. */
    quit_flag = 1;

    /* Wait until all threads quit, then destroy. */
    for (i=0; i<NUM_THREADS; ++i) {
        EM_LOG(EM_LOG_TRACE, "    Main thread joining thread %d [%p]..",
                    i, thread[i]);
        rc = em_thread_join(thread[i]);
        if (rc != EM_SUCC) {
            app_perror("...ERROR in em_thread_join()", rc);
            return -50;
        }
        EM_LOG(EM_LOG_TRACE, "    Destroying thread %d [%p]..", i, thread[i]);
        rc = em_thread_destroy(thread[i]);
        if (rc != EM_SUCC) {
            app_perror("...ERROR in em_thread_destroy()", rc);
            return -60;
        }
    }

    EM_LOG(EM_LOG_TRACE, "    Main thread calculating time slices..");

    /* Now examine the value of the counters.
     * Check that all threads had equal proportion of processing.
     */
    lowest = 0xFFFFFFFF;
    highest = 0;
    for (i=0; i<NUM_THREADS; ++i) {
        if (counter[i] < lowest)
            lowest = counter[i];
        if (counter[i] > highest)
            highest = counter[i];
    }

    /* Check that all threads are running. */
    if (lowest < 2) {
        EM_LOG(3, "...ERROR: not all threads were running!");
        return -70;
    }

    /* The difference between lowest and higest should be lower than 50%.
    */
    diff = (highest-lowest)*100 / ((highest+lowest)/2);
    if ( diff >= 50) {
        EM_LOG(3, "...ERROR: thread didn't have equal timeslice!");
        EM_LOG(3, ".....lowest counter=%u, highest counter=%u, diff=%u%%",
                    lowest, highest, diff);
        return -80;
    } else {
        EM_LOG(3, "...info: timeslice diff between lowest & highest=%u%%",
                    diff);
    }

    em_pool_release(pool);
    return 0;
}

emlib_ret_t thread_test(void)
{
    int rc;

    //overflow_check_thread("stack overflow test", 0);
    
    EM_ERROR_CHECK_TEST(simple_thread("simple thread test", 0));

    EM_ERROR_CHECK_TEST(simple_thread("suspended thread test", EM_THREAD_SUSPENDED));

    EM_ERROR_CHECK_TEST(timeslice_test());
}

#else
/* To prevent warning about "translation unit is empty"
 * when this test is disabled.
 */
int dummy_thread_test;
#endif	/* INCLUDE_THREAD_TEST */


