/*
 * File:    timer_task.h
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

#ifndef __TIMER_TASK_H__
#define __TIMER_TASK_H__

#include "em/types.h"

DECLS_BEGIN

typedef void (*expired_cb)(void *args);

typedef struct {
    uint32 it_value;
    uint32 it_interval;
}em_tt_value_t;


/**
 * Create timer task, which expired notify type is Thread Task.
 *
 * @return	    EM_SUCC or the appropriate error code.
 */
EM_DECL(emlib_ret_t) em_ttask_create_thnotify(
        em_pool_t *pool, 
        char *name, 
        em_tt_value_t time, 
        expired_cb cb, 
        void *args,
        em_timer_task_t **em_tt
        );

/**
 * Create timer task, which expired notify type is Signal Handle.
 * @param 
 *
 * @return	    EM_SUCC or the appropriate error code.
 */
EM_DECL(emlib_ret_t) em_ttask_create_signotify(
        em_pool_t *pool, 
        char *name, 
        em_tt_value_t time, 
        expired_cb cb, 
        void *args,
        em_timer_task_t **em_tt
        );
/**
 * Start os timer task.
 * @param tt    timer task object.
 *
 * @return	    EM_SUCC or the appropriate error code.
 */
EM_DECL(emlib_ret_t) em_ttask_start(em_timer_task_t *tt);

#define TIMER_MOD_MS_ONCE(tt, ms)   \
    do{ \
        em_time_val it_value;  \
        em_time_val it_interval; \
        \
        it_value.sec = ms / 1000; \
        it_value.msec = (ms % 1000); \
        \
        it_interval.sec = 0; \
        it_interval.msec = 0; \
        \
        em_ttask_mod(tt, it_value, it_interval); \
    }while(0)

#define TIMER_MOD_MS(tt, ms)   \
    do{ \
        em_time_val it_value;  \
        em_time_val it_interval; \
        \
        it_value.sec = ms / 1000; \
        it_value.msec = (ms % 1000); \
        \
        it_interval.sec = ms / 1000; \
        it_interval.msec = (ms % 1000); \
        \
        em_ttask_mod(tt, it_value, it_interval); \
    }while(0)

/**
 * Modify os timer task.
 * @param tt    timer task   object.
 *              it_value     unit:s
 *              it_interval  unis:s
 *
 * @return	    EM_SUCC or the appropriate error code.
 */
EM_DECL(emlib_ret_t) em_ttask_mod(
        em_timer_task_t *tt, 
        em_time_val it_value, 
        em_time_val it_interval);

/**
 * Stop os timer task.
 * @param tt    timer task object.
 *
 * @return	    EM_SUCC or the appropriate error code.
 */
EM_DECL(emlib_ret_t) em_ttask_stop(em_timer_task_t *tt);

/**
 * resume os timer task.
 * @param tt    timer task object.
 *
 * @return	    EM_SUCC or the appropriate error code.
 */
EM_DECL(emlib_ret_t) em_ttask_resume(em_timer_task_t *tt);

/**
 * destroy os timer task.
 * @param tt    timer task object.
 *
 * @return	    EM_SUCC or the appropriate error code.
 */
EM_DECL(emlib_ret_t) em_ttask_destroy(em_timer_task_t *tt);

DECLS_END

#endif /*End of __TIMER_TASK_H__*/
