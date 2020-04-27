/*
 * File:    nest_lock.c
 * Author:  Liu HongLiang<lhl_nciae@sina.cn>
 * Brief:   nest mutex object of lock abstract interface.
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

#include "em/errno.h"
#include "em/types.h"
#include "em/nest_lock.h"

#include <stdlib.h>

typedef struct _nest_mutex_t
{
    int32 who; 
    int32 refcnt;
    test_self  self;
    em_lock_t* locker;
}nest_mutex_t;

static emlib_ret_t nest_mutex_lock(nest_mutex_t *lock_obj)
{
    return_val_if_fail(lock_obj != NULL, EM_EINVAL);
    nest_mutex_t *nest_mutex = lock_obj;
    return_val_if_fail(nest_mutex->self != NULL, EM_EINVAL);

    emlib_ret_t ret = EM_SUCC;

    if(nest_mutex->who == nest_mutex->self()) {
        nest_mutex->refcnt++;
    } else if((ret = em_lock(nest_mutex->locker)) == EM_SUCC) {
        nest_mutex->who    = nest_mutex->self(); 
        nest_mutex->refcnt = 1;
    }

    return ret;
}

static emlib_ret_t nest_mutex_unlock(nest_mutex_t *lock_obj)
{
    return_val_if_fail(lock_obj != NULL, EM_EINVAL);

    nest_mutex_t *nest_mutex = lock_obj;

    return_val_if_fail(nest_mutex->self != NULL, EM_EINVAL);
    return_val_if_fail(nest_mutex->who == nest_mutex->self(), EM_EINVAL);

    emlib_ret_t ret = EM_SUCC;

    nest_mutex->refcnt--;

    if(nest_mutex->refcnt == 0) {
        nest_mutex->who = LOCK_NO_BINDING;
        ret = em_unlock(nest_mutex->locker);
    }

    return ret;
}

static emlib_ret_t nest_mutex_destroy(nest_mutex_t *lock_obj)
{
#if 0
    return_val_if_fail(lock_obj != NULL, EM_EINVAL);
    nest_mutex_t *nest_mutex = lock_obj;

    /*em_lock_t memory will be free by em_pool_release.*/
    /*Free resource for mutex lock.*/
    em_lock_t *locker = nest_mutex->locker;
    em_lock_destroy(locker);

    /*Free resource for nest_mutex.*/
    SAFE_FREE(thiz);
#endif
    return 0;
}

/*
 * Create nest mutex lock.
 *
 * @parama:An initialized mutex lock.
 * 
 * @return: em_lock_t wrappered with nest mutex locker.
 *
 */
em_lock_t* nest_lock_create(em_pool_t *pool, em_lock_t* mutex_locker, test_self self)
{
#if 0
    return_val_if_fail((mutex_locker != NULL) && (self != NULL) && (pool), NULL);

    em_lock_t *locker = em_pool_calloc(pool, 1, sizeof(em_lock_t) + sizeof(nest_mutex_t));

    if(locker == NULL) {
        return NULL;
    }

    locker->lock      = nest_mutex_lock;
    locker->unlock    = nest_mutex_unlock;
    locker->destroy   = nest_mutex_destroy;

    nest_mutex_t *nest_mutex = (nest_mutex_t*)(locker->lock_obj);

    nest_mutex->locker = mutex_locker;
    nest_mutex->who    = LOCK_NO_BINDING;
    nest_mutex->refcnt = 0;
    nest_mutex->self   = self;

    return locker;
#endif

    return NULL;
}
