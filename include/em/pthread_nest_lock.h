/*
 * File:    pthread_nest_lock.h
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


#ifndef __PTHREAD_NEST_LOCK_H__
#define __PTHREAD_NEST_LOCK_H__

#include "em/pool.h"
#include "em/nest_lock.h"

DECLS_BEGIN

em_lock_t* pthread_nest_lock_create(em_pool_t *pool);

DECLS_END

#endif
