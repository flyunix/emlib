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

#include "em/errno.h"
#include "em/pthread_lock.h"

#include <stdlib.h>
#include <pthread.h>

static const char *module = "PTHREAD_LOCK";

typedef struct _mutex_t
{
    pthread_mutex_t lock;
}mutex_t;

#define GET_LOCK(thiz) &(thiz->lock)
static emlib_ret_t mutex_lock(mutex_t *thiz)
{
    return_val_if_fail(thiz != NULL, EM_EINVAL);   

    return pthread_mutex_lock(GET_LOCK(thiz)) == 0 ? EM_SUCC : EM_FAIL;
}

static emlib_ret_t mutex_trylock(mutex_t *thiz)
{
    return_val_if_fail(thiz != NULL, EM_EINVAL);   

    return pthread_mutex_trylock(GET_LOCK(thiz)) == 0 ? EM_SUCC : EM_FAIL;
}

static emlib_ret_t mutex_unlock(mutex_t *thiz)
{
    return_val_if_fail(thiz != NULL, EM_EINVAL);   

    return pthread_mutex_unlock(GET_LOCK(thiz)) == 0 ? EM_SUCC : EM_FAIL;

}

static emlib_ret_t mutex_destory(mutex_t *thiz)
{
    return_val_if_fail(thiz != NULL, EM_EINVAL);   
    
    /*mutex_t memory will be free by em_pool_release.*/
    //SAFE_FREE(thiz);
}

/*
 * Create pthread mutex lock.
 *
 * @param:void
 * 
 * @return: mutex_t
 *
 */

em_lock_t * pthread_lock_create(em_pool_t *pool)
{
#if 0
    EMLIB_ASSERT_RETURN(pool, NULL);

    mutex_t *locker = em_pool_calloc(pool, 1, sizeof(mutex_t) + sizeof(mutex_t));

    if(locker == NULL) {
        return NULL;
    }

    pthread_mutex_init(GET_LOCK(locker), NULL);

    locker->lock    = mutex_lock;
    locker->trylock = mutex_trylock;
    locker->unlock  = mutex_unlock;
    locker->destroy = mutex_destory;

    return locker;
#endif 
    return NULL;
}
