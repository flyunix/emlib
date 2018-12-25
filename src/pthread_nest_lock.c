/*
 * File:    pthread_nest_lock.c
 * Author:  Liu HongLiang<lhl_nciae@sina.cn>
 * Brief:   pthread nest mutex object of lock abstract interface.
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


#include <pthread.h>

#include "log.h"
#include "nest_lock.h"
#include "pthread_lock.h"

static int32 pthread_test_self()
{
    return pthread_self();
}

em_locker* pthread_nest_lock_create()
{
    em_locker *nest_locker = pthread_lock_create();

    if(nest_locker == NULL) {
        EM_LOG(EM_LOG_ERROR, "pthread_lock_create failed.");
        return NULL;
    }

    nest_locker = nest_lock_create(nest_locker, pthread_test_self);

    if(nest_locker == NULL) {
        EM_LOG(EM_LOG_ERROR, "nest_lock_create failed.");
        return NULL;
    }

    return nest_locker;
}
