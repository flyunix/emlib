/*
 * File:    timer_task.c
 * Author:  Liu HongLiang<lhl_nciae@sina.cn>
 * Brief:   TEST Module for Timer task.
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
 * 2019-01-22 Liu HongLiang<lhl_nciae@sina.cn> created.
 *
 */

#include "test.h"
#include "em/os.h"
#include "em/log.h"
#include "em/pool.h"
#include "em/errno.h"
#include "em/timer_task.h"

static char *module = "TIMER_TASK_TEST";

static em_pool_t *pool;
static em_time_val start, stop;

void tt_cb_simple(void *args)
{
    EM_ERROR_CHECK_NORET(em_gettimeofday(&stop));
    EM_TIME_VAL_SUB(stop, start);
    EM_LOG(EM_LOG_INFO, "simple timer task callback, timer task elapsed %ds.\n", stop.sec);
}

emlib_ret_t tt_simple_test()
{
    em_timer_task_t *em_tt;
    em_tt_value_t tt_v;

    tt_v.it_value = 1;  
    tt_v.it_interval = 0;

    em_time_val it_value; 
    em_time_val it_interval;

    it_value.sec = 1;
    it_value.msec = 0;
    it_interval.sec = 5;
    it_interval.msec = 0;

    EM_ERROR_CHECK_TEST(em_ttask_create_thnotify(pool, "timer_task_test", tt_v, tt_cb_simple, NULL, &em_tt));
    EM_ERROR_CHECK_TEST(em_gettimeofday(&start));
    EM_ERROR_CHECK_TEST(em_ttask_start(em_tt));
    EM_LOG(EM_LOG_INFO, "sleep for 11s, wait for timer beated.");
    em_sleep(2);
    TIMER_MOD_MS(em_tt, 2000);
    //EM_ERROR_CHECK_TEST(em_ttask_mod(em_tt, it_value, it_interval));
    em_sleep(10);
    EM_ERROR_CHECK(em_ttask_destroy(em_tt));
}

void tt_cb_resume(void *args)
{
    EM_ERROR_CHECK_NORET(em_gettimeofday(&stop));
    EM_TIME_VAL_SUB(stop, start);
    EM_LOG(EM_LOG_INFO, "resume timer task callback, args:%s, timer task elapsed %ds.\n", args, stop.sec);
}

emlib_ret_t tt_resume_test()
{
    em_timer_task_t *em_tt;
    em_tt_value_t tt_v;

    tt_v.it_value = 10;  
    tt_v.it_interval = 10;
    char tt_cb_args[100];
    em_ansi_strcpy(tt_cb_args, "tt_resume_test");

    EM_ERROR_CHECK_TEST(em_ttask_create_thnotify(pool, "timer_task_test", tt_v, tt_cb_resume, tt_cb_args, &em_tt));
    EM_ERROR_CHECK_TEST(em_gettimeofday(&start));
    EM_ERROR_CHECK_TEST(em_ttask_start(em_tt));
    EM_LOG(EM_LOG_INFO, "sleep for 5s, stop it.");
    em_sleep(5);
    EM_ERROR_CHECK_TEST(em_ttask_stop(em_tt));
    EM_LOG(EM_LOG_INFO, "sleep for 3s, resume it.");
    em_sleep(3);
    EM_ERROR_CHECK_TEST(em_ttask_resume(em_tt));
    em_sleep(10);
    EM_ERROR_CHECK(em_ttask_destroy(em_tt));
}

emlib_ret_t timer_task_test(void)
{
    pool = em_pool_create(mem, "timer_task_test", 4096, 0, NULL);

    if (!pool) return TERRNO(); 

    EM_ERROR_CHECK_TEST(tt_simple_test());
    EM_ERROR_CHECK_TEST(tt_resume_test());

    return EM_SUCC;
}
