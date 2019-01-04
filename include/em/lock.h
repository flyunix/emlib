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

/**
 * Create simple, non recursive mutex lock object.
 *
 * @param pool	    Memory pool.
 * @param name	    Lock object's name.
 * @param lock	    Pointer to store the returned handle.
 *
 * @return	    EM_SUCC or the appropriate error code.
 */
EM_DECL(emlib_ret_t) em_lock_create_simple_mutex( em_pool_t *pool,
						  const char *name,
						  em_lock_t **lock );

/**
 * Create recursive mutex lock object.
 *
 * @param pool	    Memory pool.
 * @param name	    Lock object's name.
 * @param lock	    Pointer to store the returned handle.
 *
 * @return	    EM_SUCC or the appropriate error code.
 */
EM_DECL(emlib_ret_t) em_lock_create_recursive_mutex( em_pool_t *pool,
						     const char *name,
						     em_lock_t **lock );

DECLS_END

#endif
