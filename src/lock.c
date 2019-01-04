/*
 * File:    lock.c
 * Author:  Liu HongLiang<lhl_nciae@sina.cn>
 * Brief:   Abstract lock module interface.
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

#include "em/os.h"
#include "em/lock.h"
#include "em/errno.h"

static  const char *module = "LOCK";

typedef void LOCK_OBJ;

typedef emlib_ret_t (*lock_func)(LOCK_OBJ *lock_obj);

struct em_lock_t {
    LOCK_OBJ  *lock_obj;

    lock_func lock;
    lock_func trylock;
    lock_func unlock;
    lock_func destroy;
};

/******************************************************************************
 * Implementation of lock object with mutex.
 */
static em_lock_t mutex_lock_template = 
{
    NULL,
    (lock_func) &em_mutex_lock,
    (lock_func) &em_mutex_trylock,
    (lock_func) &em_mutex_unlock,
    (lock_func) &em_mutex_destroy
};

static emlib_ret_t create_mutex_lock( em_pool_t *pool,
        const char *name,
        int type,
        em_lock_t **lock )
{
    em_lock_t *p_lock;
    em_mutex_t *mutex;
    emlib_ret_t rc;

    EMLIB_ASSERT_RETURN(pool && lock, EM_EINVAL);

    p_lock = EM_POOL_ALLOC_T(pool, em_lock_t);
    if (!p_lock)
        return EM_ENOMEM;

    em_memcpy(p_lock, &mutex_lock_template, sizeof(em_lock_t));
    rc = em_mutex_create(pool, name, type, &mutex);
    if (rc != EM_SUCC)
        return rc;

    p_lock->lock_obj = mutex;
    *lock = p_lock;
    return EM_SUCC;
}

EM_DEF(emlib_ret_t) em_lock_create_simple_mutex( em_pool_t *pool,
						 const char *name,
						 em_lock_t **lock )
{
    return create_mutex_lock(pool, name, EM_MUTEX_SIMPLE, lock);
}

EM_DEF(emlib_ret_t) em_lock_create_recursive_mutex( em_pool_t *pool,
						    const char *name,
						    em_lock_t **lock )
{
    return create_mutex_lock(pool, name, EM_MUTEX_RECURSE, lock);
}

inline emlib_ret_t em_lock(em_lock_t *thiz)
{
    return_val_if_fail((thiz != NULL) && (thiz->lock), EM_FAIL);

    return thiz->lock(thiz->lock_obj);
}

inline emlib_ret_t em_trylock(em_lock_t *thiz)
{
    return_val_if_fail((thiz != NULL) && (thiz->trylock), EM_FAIL);

    return thiz->trylock(thiz->lock_obj);
}

inline emlib_ret_t em_unlock(em_lock_t *thiz) 
{
    return_val_if_fail((thiz != NULL) && (thiz->unlock), EM_FAIL);

    return thiz->unlock(thiz->lock_obj);
}


inline void em_lock_destroy(em_lock_t *thiz)
{
    return_if_fail(thiz != NULL);

    thiz->destroy(thiz->lock_obj);
}

