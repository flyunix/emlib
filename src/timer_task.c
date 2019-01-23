/*
 * File:    timer_task.c
 * Author:  Liu HongLiang<lhl_nciae@sina.cn>
 * Brief:   General Timer Task Framework 
 *
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
 * 2019-01-21 Liu HongLiang<lhl_nciae@sina.cn> created.
 *
 */

#include "em/os.h"
#include "em/errno.h"
#include "em/timer_task.h"

static char* module = "TIMER_TASK";

typedef void* em_tt_obj_t;

typedef emlib_ret_t (*tt_func)(em_tt_obj_t tt_obj);

struct em_timer_task_t{
    em_tt_obj_t tt_obj;

    emlib_ret_t  (*ttask_start)(em_tt_obj_t tt_obj);
    emlib_ret_t  (*ttask_stop)(em_tt_obj_t tt_obj);
    emlib_ret_t  (*ttask_resume)(em_tt_obj_t tt_obj);
    emlib_ret_t  (*ttask_destroy)(em_tt_obj_t tt_obj);

};

static em_timer_task_t template_tt = {
    .tt_obj = NULL,
    .ttask_start   = (tt_func)em_os_ttask_start,
    .ttask_stop    = (tt_func)em_os_ttask_stop,
    .ttask_resume  = (tt_func)em_os_ttask_resume,
    .ttask_destroy = (tt_func)em_os_ttask_destroy
};

static emlib_ret_t create_timer_task(
        em_pool_t *pool, 
        char      *tt_name,
        em_time_val it_value, 
        em_time_val it_interval,
        em_tt_notify_type_e notify_type,
        expired_cb cb,
        void *args,
        em_timer_task_t **em_tt
        )
{
    EMLIB_ASSERT_RETURN(pool && tt_name && cb, EM_EINVAL);    

    emlib_ret_t ret;
    em_os_tt_obj_t *tt_obj;
    EM_CHECK_STACK();

    em_timer_task_t *timer_task = em_pool_calloc(pool, 1, sizeof(em_timer_task_t));

    if(timer_task == NULL) {
        return EM_ENOMEM;
    }

    em_memcpy(timer_task, &template_tt, sizeof(em_timer_task_t));     

    ret = em_create_timer_task(
            pool,
            tt_name, 
            it_value,
            it_interval,
            notify_type,
            cb,
            args,
            &tt_obj
            );

    if(ret != EM_SUCC) {
        return ret;
    }

    timer_task->tt_obj = tt_obj;
    *em_tt = timer_task;

    return EM_SUCC;
}

EM_DEF(emlib_ret_t) em_ttask_create_thnotify(
        em_pool_t *pool, 
        char *name, 
        em_tt_value_t time, 
        expired_cb cb, 
        void *args,
        em_timer_task_t **em_tt
        )
{
    EMLIB_ASSERT_RETURN(pool && name && cb, EM_EINVAL);
    EMLIB_ASSERT_RETURN((time.it_value >= 0) && (time.it_interval >= 0), EM_EINVAL);

    emlib_ret_t ret;
    em_time_val it_value, it_interval;

    EM_CHECK_STACK();
    
    it_value.sec  = time.it_value;
    it_value.msec = 0;
    it_interval.sec  = time.it_interval;
    it_interval.msec = 0;

    ret = create_timer_task(
            pool,
            name,
            it_value,
            it_interval,
            TTN_THREAD_E,
            cb,
            args,
            em_tt
            );

    if(ret != EM_SUCC) {
        return ret;
    }

    return EM_SUCC;
}

EM_DEF(emlib_ret_t) em_ttask_create_signotify(
        em_pool_t *pool, 
        char *name, 
        em_tt_value_t time, 
        expired_cb cb, 
        void *args,
        em_timer_task_t **em_tt
        )
{
    EMLIB_ASSERT_RETURN(pool && name && cb, EM_EINVAL);
    EMLIB_ASSERT_RETURN((time.it_value >= 0) && (time.it_interval >= 0), EM_EINVAL);

    emlib_ret_t ret;
    em_time_val it_value, it_interval;

    EM_CHECK_STACK();
    
    it_value.sec = time.it_value;
    it_value.msec = 0;
    it_interval.sec = time.it_interval;
    it_interval.msec = 0;

    ret = create_timer_task(
            pool,
            name,
            it_value,
            it_interval,
            TTN_SIGNAL_E,
            cb,
            args,
            em_tt
            );

    if(ret != EM_SUCC) {
        return ret;
    }

    return EM_SUCC;
}
EM_DEF(emlib_ret_t) em_ttask_start(em_timer_task_t *tt)
{
    EMLIB_ASSERT_RETURN(tt, EM_EINVAL);

    return tt->ttask_start(tt->tt_obj);    
}

EM_DEF(emlib_ret_t) em_ttask_stop(em_timer_task_t *tt)
{
    EMLIB_ASSERT_RETURN(tt, EM_EINVAL);

    return tt->ttask_stop(tt->tt_obj);
}

EM_DEF(emlib_ret_t) em_ttask_resume(em_timer_task_t *tt)
{
    EMLIB_ASSERT_RETURN(tt, EM_EINVAL);

    return tt->ttask_resume(tt->tt_obj);
}

EM_DEF(emlib_ret_t) em_ttask_destroy(em_timer_task_t *tt)
{
    EMLIB_ASSERT_RETURN(tt, EM_EINVAL);

    return tt->ttask_destroy(tt->tt_obj);
}


