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

#ifndef __EM_LOCK_H__
#define __EM_LOCK_H__

#include "em/types.h"
#include "em/assert.h"

DECLS_BEGIN

emlib_ret_t em_lock(em_lock_t *thiz);

emlib_ret_t em_trylock(em_lock_t *thiz);

emlib_ret_t em_unlock(em_lock_t *thiz);

void em_lock_destroy(em_lock_t *thiz);

DECLS_END

#endif
