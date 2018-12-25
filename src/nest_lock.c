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

#include <stdlib.h>

#include "types.h"
#include "nest_lock.h"

typedef struct _nest_mutex_t
{
    int32 who; 
    int32 refcnt;
    test_self  self;
    em_locker* locker;
}nest_mutex_t;

#define GET_NEST_MUTEX(thiz) \
    (nest_mutex_t*)thiz->priv

static embed_ret_t nest_mutex_lock(em_locker *thiz)
{
    return_val_if_fail(thiz != NULL, EMBED_INVALID_PARAMS);
    nest_mutex_t *nest_mutex = GET_NEST_MUTEX(thiz);
    return_val_if_fail(nest_mutex->self != NULL, EMBED_INVALID_PARAMS);

    embed_ret_t ret = EMBED_SUCC;

    if(nest_mutex->who == nest_mutex->self()) {
        nest_mutex->refcnt++;
    } else if((ret = em_lock(nest_mutex->locker)) == EMBED_SUCC) {
        nest_mutex->who    = nest_mutex->self(); 
        nest_mutex->refcnt = 1;
    }

    return ret;
}

static embed_ret_t nest_mutex_unlock(em_locker *thiz)
{
    return_val_if_fail(thiz != NULL, EMBED_INVALID_PARAMS);

    nest_mutex_t *nest_mutex = GET_NEST_MUTEX(thiz);

    return_val_if_fail(nest_mutex->self != NULL, EMBED_INVALID_PARAMS);
    return_val_if_fail(nest_mutex->who == nest_mutex->self(), EMBED_INVALID_LOGIC);

    embed_ret_t ret = EMBED_SUCC;

    nest_mutex->refcnt--;

    if(nest_mutex->refcnt == 0) {
        nest_mutex->who = LOCK_NO_BINDING;
        ret = em_unlock(nest_mutex->locker);
    }

    return ret;
}

static embed_ret_t nest_mutex_destroy(em_locker *thiz)
{
    return_val_if_fail(thiz != NULL, EMBED_INVALID_PARAMS);
    nest_mutex_t *nest_mutex = GET_NEST_MUTEX(thiz);

    /*Free resource for mutex lock.*/
    em_locker *locker = nest_mutex->locker;
    em_lock_destroy(locker);

    /*Free resource for nest_mutex.*/
    SAFE_FREE(thiz);
}

/*
 * Create nest mutex lock.
 *
 * @parama:An initialized mutex lock.
 * 
 * @return: em_locker wrappered with nest mutex locker.
 *
 */
em_locker* nest_lock_create(em_locker* mutex_locker, test_self self)
{
    return_val_if_fail((mutex_locker != NULL) && (self != NULL), NULL);

    em_locker *locker = malloc(sizeof(em_locker) + sizeof(nest_mutex_t));

    if(locker == NULL) {
        return NULL;
    }

    locker->lock      = nest_mutex_lock;
    locker->unlock    = nest_mutex_unlock;
    locker->destroy   = nest_mutex_destroy;

    nest_mutex_t *nest_mutex = GET_NEST_MUTEX(locker);

    nest_mutex->locker = mutex_locker;
    nest_mutex->who    = LOCK_NO_BINDING;
    nest_mutex->refcnt = 0;
    nest_mutex->self   = self;

    return locker;
}
