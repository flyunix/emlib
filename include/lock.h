/*
 * File:    lock.h
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
 * 2018-12-21 Liu HongLiang<lhl_nciae@sina.cn> created.
 *
 */

#ifndef __LOCK_H__
#define __LOCK_H__

#include "types.h"
#include "assert.h"

DECLS_BEGIN

/*Base Data Type*/
typedef struct _em_lock em_locker;

typedef embed_ret_t (*lock_func)(em_locker *thiz);

struct _em_lock {
    lock_func lock;
    lock_func trylock;
    lock_func unlock;
    lock_func destroy;

    int8  priv[0];
};

embed_ret_t em_lock(em_locker *thiz);

embed_ret_t em_trylock(em_locker *thiz);

embed_ret_t em_unlock(em_locker *thiz);

void em_lock_destroy(em_locker *thiz);

DECLS_END

#endif
