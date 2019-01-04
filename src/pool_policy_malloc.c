/*
 * File:    pool_policy_malloc.c
 * Author:  Liu HongLiang<lhl_nciae@sina.cn>
 * Brief:   Memory Pool
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
 * 2019-01-02 Liu HongLiang<lhl_nciae@sina.cn> created.
 *
 */
#include "em/pool.h"
#include <em/except.h>
#include "em/os.h"
#include <stdlib.h>

#if !EM_HAS_POOL_ALT_API

/*
 * This file contains pool default policy definition and implementation.
 */
#include "em/pool_signature.h"

static void *default_block_alloc(em_pool_factory *factory, em_size_t size)
{
    void *p;

    EM_CHECK_STACK();

    if (factory->on_block_alloc) {
	int rc;
	rc = factory->on_block_alloc(factory, size);
	if (!rc)
	    return NULL;
    }

    p = malloc(size+(SIG_SIZE << 1));

    if (p == NULL) {
	if (factory->on_block_free) 
	    factory->on_block_free(factory, size);
    } else {
	/* Apply signature when EM_SAFE_POOL is set. It will move
	 * "p" pointer forward.
	 */
	APPLY_SIG(p, size);
    }

    return p;
}

static void default_block_free(em_pool_factory *factory, void *mem, 
			       em_size_t size)
{
    EM_CHECK_STACK();

    if (factory->on_block_free) 
        factory->on_block_free(factory, size);

    /* Check and remove signature when EM_SAFE_POOL is set. It will
     * move "mem" pointer backward.
     */
    REMOVE_SIG(mem, size);

    /* Note that when EM_SAFE_POOL is set, the actual size of the block
     * is size + SIG_SIZE*2.
     */

    free(mem);
}

static void default_pool_callback(em_pool_t *pool, em_size_t size)
{
    EM_CHECK_STACK();
    EM_UNUSED_ARG(pool);
    EM_UNUSED_ARG(size);

    EM_THROW(EM_NO_MEMORY_EXCEPTION);
}

EM_DEF_DATA(em_pool_factory_policy) em_pool_factory_default_policy =
{
    &default_block_alloc,
    &default_block_free,
    &default_pool_callback,
    0
};

EM_DEF(const em_pool_factory_policy*) em_pool_factory_get_default_policy(void)
{
    return &em_pool_factory_default_policy;
}


#endif	/* EM_HAS_POOL_ALT_API */
