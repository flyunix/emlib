/*
 * File:    sleep.c
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
 * 2019-01-11 Liu HongLiang<lhl_nciae@sina.cn> created.
 *
 */
#include "test.h"

/**
 * \page page_emlib_sleep_test Test: Sleep, Time, and Timestamp
 *
 * This file provides implementation of \b sleep_test().
 *
 * \section sleep_test_sec Scope of the Test
 *
 * This tests:
 *  - whether em_thread_sleep() works.
 *  - whether em_gettimeofday() works.
 *  - whether em_get_timestamp() and friends works.
 *
 * API tested:
 *  - em_thread_sleep()
 *  - em_gettimeofday()
 *  - EM_TIME_VAL_SUB()
 *  - EM_TIME_VAL_LTE()
 *  - em_get_timestamp()
 *  - em_get_timestamp_freq() (implicitly)
 *  - em_elapsed_time()
 *  - em_elapsed_usec()
 *
 *
 * This file is <b>emlib-test/sleep.c</b>
 *
 * \include emlib-test/sleep.c
 */

#if INCLUDE_SLEEP_TEST

#include <emlib.h>

#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

static const char *module = "test_sleep";

typedef emlib_ret_t (*sleep_func)(unsigned msec);

#define SLEEP_DURATION_TEST(sleep_func) sleep_duration_test(sleep_func, #sleep_func)

static int simple_sleep_test(void)
{
    enum { COUNT = 2 };
    int i;
    emlib_ret_t rc;

    EM_LOG(EM_LOG_DEBUG, "..will write messages every 1 second:");

    for (i=0; i<COUNT; ++i) {
        em_time_val tv;
        em_parsed_time pt;

        rc = em_thread_sleep(1000);
        if (rc != EM_SUCC) {
            app_perror("...error: em_thread_sleep()", rc);
            return -10;
        }

        rc = em_gettimeofday(&tv);
        if (rc != EM_SUCC) {
            app_perror("...error: em_gettimeofday()", rc);
            return -11;
        }

        em_time_decode(&tv, &pt);

        EM_LOG(EM_LOG_DEBUG, 
                    "...%04d-%02d-%02d %02d:%02d:%02d.%03d",
                    pt.year, pt.mon, pt.day,
                    pt.hour, pt.min, pt.sec, pt.msec);
    }

    return 0;
}

static int sleep_duration_test(sleep_func sleep, char *sleep_name)
{
    enum { MIS = 10};
    unsigned duration[] = { 500, 2000, 1000, 200, 100 };
    unsigned i;
    emlib_ret_t rc;

    EM_LOG(EM_LOG_DEBUG, "..running sleep duration test with %s", sleep_name);

    /* Test sleep() and em_gettimeofday() */
    for (i=0; i<EM_ARRAY_SIZE(duration); ++i) {
        em_time_val start, stop;
        uint32 msec;

        /* Mark start of test. */
        rc = em_gettimeofday(&start);
        if (rc != EM_SUCC) {
            app_perror("...error: em_gettimeofday()", rc);
            return -10;
        }

        /* Sleep */
        rc = sleep(duration[i]);

        if (rc != EM_SUCC) {
            EM_LOG(EM_LOG_ERROR, "%s for %d msec", sleep_name, duration[i]);
            app_perror("...error: sleep()", rc);
            return -20;
        }

        /* Mark end of test. */
        rc = em_gettimeofday(&stop);

        /* Calculate duration (store in stop). */
        EM_TIME_VAL_SUB(stop, start);

        /* Convert to msec. */
        msec = EM_TIME_VAL_MSEC(stop);

        /* Check if it's within range. */
        if (msec < duration[i] * (1000-MIS)/1000 ||
                msec > duration[i] * (1000+MIS)/1000)
        {
            EM_LOG(EM_LOG_DEBUG, 
                        "...error: slept for %d ms instead of %d ms "
                        "(outside %d%%%% err window)",
                        msec, duration[i], MIS);
            return -30;
        }
    }


    /* Test em_thread_sleep() and em_get_timestamp() and friends */
    for (i=0; i<EM_ARRAY_SIZE(duration); ++i) {
        em_time_val t1, t2;
        em_timestamp start, stop;
        uint32 msec;

        sleep(0);

        /* Mark start of test. */
        rc = em_get_timestamp(&start);
        if (rc != EM_SUCC) {
            app_perror("...error: em_get_timestamp()", rc);
            return -60;
        }

        /* ..also with gettimeofday() */
        em_gettimeofday(&t1);

        /* Sleep */
        rc = sleep(duration[i]);
        if (rc != EM_SUCC) {
            app_perror("...error: sleep()", rc);
            return -70;
        }

        /* Mark end of test. */
        em_get_timestamp(&stop);

        /* ..also with gettimeofday() */
        em_gettimeofday(&t2);

        /* Compare t1 and t2. */
        if (EM_TIME_VAL_LT(t2, t1)) {
            EM_LOG(EM_LOG_DEBUG, "...error: t2 is less than t1!!");
            return -75;
        }

        /* Get elapsed time in msec */
        msec = em_elapsed_msec(&start, &stop);

        /* Check if it's within range. */
        if (msec < duration[i] * (1000-MIS)/1000 ||
                msec > duration[i] * (1000+MIS)/1000)
        {
            EM_LOG(EM_LOG_DEBUG,  
                        "...error: slept for %d ms instead of %d ms "
                        "(outside %d%%%% err window)",
                        msec, duration[i], MIS);
            EM_TIME_VAL_SUB(t2, t1);
            EM_LOG(EM_LOG_DEBUG, 
                        "...info: gettimeofday() reported duration is "
                        "%d msec",
                        EM_TIME_VAL_MSEC(t2));

            return -76;
        }
    }

    /* All done. */
    return 0;
}

emlib_ret_t sleep_test()
{
    int rc;

    rc = simple_sleep_test();
    if (rc != EM_SUCC)
        return rc;

    rc = SLEEP_DURATION_TEST(em_msleep);
    if (rc != EM_SUCC)
        return rc;

    rc = SLEEP_DURATION_TEST(em_thread_sleep);
    if (rc != EM_SUCC)
        return rc;

    return 0;
}

#else
/* To prevent warning about "translation unit is empty"
 * when this test is disabled. 
 */
int dummy_sleep_test;
#endif  /* INCLUDE_SLEEP_TEST */
