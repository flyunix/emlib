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

#include "types.h"
#include "pthread_lock.h"
#include "pthread_nest_lock.h"

embed_ret_t pthread_mutex_test(void)
{
    /*TEST: pthread_mutex_lock*/
    em_locker * locker = pthread_lock_create();

    if(locker == NULL) {
        EM_LOG(EM_LOG_ERROR, "pthread_lock_create failed.");
        return 0;
    }

    em_lock(locker);
    em_unlock(locker);
    em_lock_destroy(locker);

    return EMBED_SUCC;
}

pthread_nest_mutex_test(void)
{
    /*TEST: nest mutex lock*/
    em_locker *nest_locker = pthread_nest_lock_create();

    if(nest_locker == NULL) {
        EM_LOG(EM_LOG_ERROR, "pthread_nest_lock_create failed.");
        return 0;
    }

    em_lock(nest_locker);
    em_unlock(nest_locker);
    em_lock_destroy(nest_locker);
}
