/*
 * File:    pool_signature.h
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
#include <em/assert.h>
#include <em/string.h>

#if EM_SAFE_POOL
#   define SIG_SIZE		sizeof(uint32)

static void apply_signature(void *p, em_size_t size);
static void check_pool_signature(void *p, em_size_t size);

#   define APPLY_SIG(p,sz)	apply_signature(p,sz), \
				p=(void*)(((char*)p)+SIG_SIZE)
#   define REMOVE_SIG(p,sz)	check_pool_signature(p,sz), \
				p=(void*)(((char*)p)-SIG_SIZE)

#   define SIG_BEGIN	    0x600DC0DE
#   define SIG_END	    0x0BADC0DE

static void apply_signature(void *p, em_size_t size)
{
    uint32 sig;

    sig = SIG_BEGIN;
    em_memcpy(p, &sig, SIG_SIZE);

    sig = SIG_END;
    em_memcpy(((char*)p)+SIG_SIZE+size, &sig, SIG_SIZE);
}

static void check_pool_signature(void *p, em_size_t size)
{
    uint32 sig;
    uint8 *mem = (uint8*)p;

    /* Check that signature at the start of the block is still intact */
    sig = SIG_BEGIN;
    em_assert(!em_memcmp(mem-SIG_SIZE, &sig, SIG_SIZE));

    /* Check that signature at the end of the block is still intact.
     * Note that "mem" has been incremented by SIG_SIZE 
     */
    sig = SIG_END;
    em_assert(!em_memcmp(mem+size, &sig, SIG_SIZE));
}

#else
#   define SIG_SIZE	    0
#   define APPLY_SIG(p,sz)
#   define REMOVE_SIG(p,sz)
#endif
