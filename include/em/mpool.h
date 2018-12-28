/*
 * File:    mpool.h
 * Author:  Liu HongLiang<lhl_nciae@sina.cn>
 * Brief:   Memory Pool Framework.
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
 * 2018-12-20 Liu HongLiang<lhl_nciae@sina.cn> created.
 *
 */

#ifndef __EM_MPOOL_H__
#define __EM_MPOOL_H__

#include "types.h"

DECLS_BEGIN

typedef struct em_pool_t {
    int32 flags;
}em_pool_t;

DECLS_END

#endif /*__EM_MPOOL_H__*/
