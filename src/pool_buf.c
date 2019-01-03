/*
 * File:    pool_buf.h
 * Author:  Liu HongLiang<lhl_nciae@sina.cn>
 * Brief:   Stack/buffer based pool. 
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
#include <em/pool_buf.h>
#include <em/assert.h>
#include <em/os.h>

static const char *module = "POOL_BUF";

static struct em_pool_factory stack_based_factory;

struct creation_param
{
    void	*stack_buf;
    em_size_t	 size;
};

static int is_initialized;
static long tls = -1;
static void* stack_alloc(em_pool_factory *factory, em_size_t size);

static void pool_buf_cleanup(void)
{
    if (tls != -1) {
        em_thread_local_free(tls);
        tls = -1;
    }
    if (is_initialized)
        is_initialized = 0;
}

static emlib_ret_t pool_buf_initialize(void)
{
    //em_atexit(&pool_buf_cleanup);

    stack_based_factory.policy.block_alloc = &stack_alloc;
    return em_thread_local_alloc(&tls);
}

static void* stack_alloc(em_pool_factory *factory, em_size_t size)
{
    struct creation_param *param;
    void *buf;

    EM_UNUSED_ARG(factory);

    param = (struct creation_param*) em_thread_local_get(tls);
    if (param == NULL) {
        /* Don't assert(), this is normal no-memory situation */
        return NULL;
    }

    em_thread_local_set(tls, NULL);

    EMLIB_ASSERT_RETURN(size <= param->size, NULL);

    buf = param->stack_buf;

    /* Prevent the buffer from being reused */
    param->stack_buf = NULL;

    return buf;
}


EM_DEF(em_pool_t*) em_pool_create_on_buf(const char *name,
        void *buf,
        em_size_t size)
{
#if EM_HAS_POOL_ALT_API == 0
    struct creation_param param;
    em_size_t align_diff;

    EMLIB_ASSERT_RETURN(buf && size, NULL);

    if (!is_initialized) {
        if (pool_buf_initialize() != EM_SUCC)
            return NULL;
        is_initialized = 1;
    }

    /* Check and align buffer */
    align_diff = (em_size_t)buf;
    if (align_diff & (EM_POOL_ALIGNMENT-1)) {
        align_diff &= (EM_POOL_ALIGNMENT-1);
        buf = (void*) (((char*)buf) + align_diff);
        size -= align_diff;
    }

    param.stack_buf = buf;
    param.size = size;
    em_thread_local_set(tls, &param);

    return em_pool_create_int(&stack_based_factory, name, size, 0, 
            em_pool_factory_default_policy.callback);
#else
    EM_UNUSED_ARG(buf);
    return em_pool_create(NULL, name, size, size, NULL);
#endif
}

