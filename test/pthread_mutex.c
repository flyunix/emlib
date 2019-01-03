/*
 * File:    pthread_mutex_test.c
 * Author:  Liu HongLiang<lhl_nciae@sina.cn>
 * Brief:   TEST Module for emlib
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
 * 2018-12-24 Liu HongLiang<lhl_nciae@sina.cn> created.
 *
 */

#include "test.h"
#include "em/pool_buf.h"
#include "em/types.h"
#include "em/pthread_lock.h"
#include "em/pthread_nest_lock.h"

static const char* module = "PTHREAD_MUTEX";

emlib_ret_t pthread_mutex_test(void)
{
    /*TEST: pthread_mutex_lock*/
    int8 buf[256] = {0};

    em_pool_t *pool = em_pool_create_on_buf("pthread_mutex_lock_test", buf, sizeof(buf));
    em_locker * locker = pthread_lock_create(pool);

    if(locker == NULL) {
        EM_LOG(EM_LOG_ERROR, "pthread_lock_create failed.");
        return 0;
    }

    em_lock(locker);
    em_unlock(locker);
    em_lock_destroy(locker);

    return EM_SUCC;
}

emlib_ret_t pthread_nest_mutex_test(void)
{
    /*TEST: nest mutex lock*/
    int8 buf[512] = {0};
    em_pool_t *pool = em_pool_create_on_buf("pthread_nest_mutex_test", buf, sizeof(buf));
    em_locker *nest_locker = pthread_nest_lock_create(pool);

    if(nest_locker == NULL) {
        EM_LOG(EM_LOG_ERROR, "pthread_nest_lock_create failed.");
        return TERRNO();
    }

    em_lock(nest_locker);
    em_unlock(nest_locker);
    em_lock_destroy(nest_locker);

    return EM_SUCC;
}
