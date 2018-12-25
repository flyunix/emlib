/*
 * File:    pthread_lock.c
 * Author:  Liu HongLiang<lhl_nciae@sina.cn>
 * Brief:   pthread mutex object of lock abstract interface.
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
#include <pthread.h>

#include "pthread_lock.h"

typedef struct _mutex_t
{
    pthread_mutex_t lock;
}mutex_t;

#define GET_LOCK(thiz) \
    (&((mutex_t*)thiz->priv)->lock)

static embed_ret_t mutex_lock(em_locker *thiz)
{
    return_val_if_fail(thiz != NULL, EMBED_INVALID_PARAMS);   

    return pthread_mutex_lock(GET_LOCK(thiz)) == 0 ? EMBED_SUCC : EMBED_FAIL;
}

static embed_ret_t mutex_trylock(em_locker *thiz)
{
    return_val_if_fail(thiz != NULL, EMBED_INVALID_PARAMS);   

    return pthread_mutex_trylock(GET_LOCK(thiz)) == 0 ? EMBED_SUCC : EMBED_FAIL;
}

static embed_ret_t mutex_unlock(em_locker *thiz)
{
    return_val_if_fail(thiz != NULL, EMBED_INVALID_PARAMS);   

    return pthread_mutex_unlock(GET_LOCK(thiz)) == 0 ? EMBED_SUCC : EMBED_FAIL;

}

static embed_ret_t mutex_destory(em_locker *thiz)
{
    return_val_if_fail(thiz != NULL, EMBED_INVALID_PARAMS);   
    
    SAFE_FREE(thiz);
}

/*
 * Create pthread mutex lock.
 *
 * @param:void
 * 
 * @return: em_locker
 *
 */

em_locker* pthread_lock_create(void)
{
    em_locker *locker = malloc(sizeof(em_locker) + sizeof(mutex_t));     

    if(locker == NULL) {
        return NULL;
    }

    pthread_mutex_init(GET_LOCK(locker), NULL);

    locker->lock    = mutex_lock;
    locker->trylock = mutex_trylock;
    locker->unlock  = mutex_unlock;
    locker->destroy = mutex_destory;

    return locker;
}
