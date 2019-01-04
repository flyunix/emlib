/*
 * File:    pool.c
 * Author:  Liu HongLiang<lhl_nciae@sina.cn>
 * Brief:   TEST Module for memory pool 
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
 * 2019-01-02 Liu HongLiang<lhl_nciae@sina.cn> created.
 *
 */
#include <em/pool.h>
#include <em/pool_buf.h>
//#include <em/rand.h>
#include <em/log.h>
//#include <em/except.h>
#include "test.h"

#include <stdlib.h>

/**
 * \page page_emlib_pool_test Test: Pool
 *
 * This file provides implementation of \b pool_test(). It tests the
 * functionality of the memory pool.
 *
 *
 * This file is <b>emlib-test/pool.c</b>
 *
 * \include test/pool.c
 */


#if INCLUDE_POOL_TEST

#define SIZE	4096

/* Normally we should throw exception when memory alloc fails.
 * Here we do nothing so that the flow will go back to original caller,
 * which will test the result using NULL comparison. Normally caller will
 * catch the exception instead of checking for NULLs.
 */
static void null_callback(em_pool_t *pool, em_size_t size)
{
    EM_UNUSED_ARG(pool);
    EM_UNUSED_ARG(size);
}

#define GET_FREE(p)	(em_pool_get_capacity(p)-em_pool_get_used_size(p))

/* Test that the capacity and used size reported by the pool is correct. 
*/
static int capacity_test(void)
{
    EM_LOG_MOD(EM_LOG_INFO, "test", "...capacity_test()");

    em_pool_t *pool = em_pool_create(mem, "capacity_test", SIZE, 0, &null_callback);
    em_size_t freesize;

    if (!pool)
        return -200;

    freesize = GET_FREE(pool);

    if (em_pool_alloc(pool, freesize) == NULL) {
        EM_LOG_MOD(EM_LOG_INFO, "test", "...error: wrong freesize %u reported",
                freesize);
        em_pool_release(pool);
        return -210;
    }

    em_pool_release(pool);
    return 0;
}

/* Test that the alignment works. */
static int pool_alignment_test(void)
{
    em_pool_t *pool;
    void *ptr;
    enum { MEMSIZE = 64, LOOP = 1 };
    unsigned i;

    EM_LOG_MOD(EM_LOG_INFO, "test", "...alignment test");

    pool = em_pool_create(mem, "pool_alignment_test", EM_POOL_SIZE + MEMSIZE, MEMSIZE, NULL);
    if (!pool)
        return -300;

#define IS_ALIGNED(p)	((((unsigned long)(em_ssize_t)p) & \
            (EM_POOL_ALIGNMENT-1)) == 0)

    for (i=0; i<LOOP; ++i) {
        /* Test first allocation */
        ptr = em_pool_alloc(pool, 1);
        if (!IS_ALIGNED(ptr)) {
            em_pool_release(pool);
            return -310;
        }

        /* Test subsequent allocation */
        ptr = em_pool_alloc(pool, 1);
        if (!IS_ALIGNED(ptr)) {
            em_pool_release(pool);
            return -320;
        }

        /* Test allocation after new block is created */
        ptr = em_pool_alloc(pool, MEMSIZE*2+1);
        if (!IS_ALIGNED(ptr)) {
            em_pool_release(pool);
            return -330;
        }

        /* Reset the pool */
        em_pool_reset(pool);
    }

    /* Done */
    em_pool_release(pool);

    return 0;
}

/* Test that the alignment works for pool on buf. */
static int pool_buf_alignment_test(void)
{
    em_pool_t *pool;
    char buf[512];
    void *ptr;
    enum { LOOP = 1 };
    unsigned i;

    EM_LOG_MOD(EM_LOG_INFO, "test", "...pool_buf alignment test");

    pool = em_pool_create_on_buf("pool_buf_alignment_test", buf, sizeof(buf));
    if (!pool)
        return -400;

    for (i=0; i<LOOP; ++i) {
        /* Test first allocation */
        ptr = em_pool_alloc(pool, 1);
        if (!IS_ALIGNED(ptr)) {
            em_pool_release(pool);
            return -410;
        }

        /* Test subsequent allocation */
        ptr = em_pool_alloc(pool, 1);
        if (!IS_ALIGNED(ptr)) {
            em_pool_release(pool);
            return -420;
        }

        /* Reset the pool */
        em_pool_reset(pool);
    }

    /* Done */
    return 0;
}
/* Test function to drain the pool's space. 
*/
static int drain_test(em_size_t size, em_size_t increment)
{
    em_pool_t *pool = em_pool_create(mem, "drain_test", size * 2, increment, 
            &null_callback);
    em_size_t freesize;
    void *p;
    int status = 0;

    EM_LOG_MOD(EM_LOG_INFO, "test", "...drain_test(%d,%d)", size, increment);

    if (!pool)
        return -10;

    /* Get free size */
    freesize = GET_FREE(pool);
    if (freesize < 1) {
        status=-15; 
        goto on_error;
    }

    /* Drain the pool until there's nothing left. */
    while (freesize > 0) {
        int size2;

        if (freesize > 255)
            size2 = ((rand() & 0x000000FF) + EM_POOL_ALIGNMENT) & 
                ~(EM_POOL_ALIGNMENT - 1);
        else
            size2 = (int)freesize;

        p = em_pool_alloc(pool, size2);
        if (!p) {
            status=-20; goto on_error;
        }

        freesize -= size2;
    }

    /* Check that capacity is zero. */
    if (GET_FREE(pool) != 0) {
        EM_LOG_MOD(EM_LOG_INFO, "test", "....error: returned free=%u (expecting 0)",
                GET_FREE(pool));
        status=-30; goto on_error;
    }

    /* Try to allocate once more */
    p = em_pool_alloc(pool, 257);
    if (!p) {
        status=-40; goto on_error;
    }

    /* Check that capacity is NOT zero. */
    if (GET_FREE(pool) == 0) {
        status=-50; goto on_error;
    }


on_error:
    em_pool_release(pool);
    return status;
}

#if 0
/* Test the buffer based pool */
static int pool_buf_test(void)
{
    enum { STATIC_BUF_SIZE = 40 };
    /* 16 is the internal struct in pool_buf */
    static char buf[ STATIC_BUF_SIZE + sizeof(em_pool_t) + 
        sizeof(em_pool_block) + 2 * EM_POOL_ALIGNMENT];
    em_pool_t *pool;
    void *p;
    //EM_USE_EXCEPTION;

    EM_LOG_MOD(EM_LOG_INFO, "test", "...pool_buf test");

    pool = em_pool_create_on_buf("no name", buf, sizeof(buf));
    if (!pool)
        return -70;

    /* Drain the pool */
    EM_TRY {
        if ((p=em_pool_alloc(pool, STATIC_BUF_SIZE/2)) == NULL)
            return -75;

        if ((p=em_pool_alloc(pool, STATIC_BUF_SIZE/2)) == NULL)
            return -76;
    }
    EM_CATCH_ANY {
        return -77;
    }
    EM_END;

    /* On the next alloc, exception should be thrown */
    EM_TRY {
        p = em_pool_alloc(pool, STATIC_BUF_SIZE);
        if (p != NULL) {
            /* This is unexpected, the alloc should fail */
            return -78;
        }
    }
    EM_CATCH_ANY {
        /* This is the expected result */
    }
    EM_END;

    /* Done */
    return 0;
}
#endif


emlib_ret_t pool_test(void)
{
    enum { LOOP = 2 };
    int loop;
    int rc;

    rc = capacity_test();
    if (rc) return rc;

    rc = pool_alignment_test();
    if (rc) return rc;

    rc = pool_buf_alignment_test();
    if (rc) return rc;

    for (loop=0; loop<LOOP; ++loop) {
        /* Test that the pool should grow automaticly. */
        rc = drain_test(SIZE, SIZE);
        if (rc != 0) return rc;

        /* Test situation where pool is not allowed to grow. 
         * We expect the test to return correct error.
         */
        rc = drain_test(SIZE, 0);
        if (rc != -40) return rc;
    }

#if 0
    rc = pool_buf_test();
    if (rc != 0)
        return rc;
#endif
    printf("\n\n");
    em_pool_factory_dump(mem, true);
    printf("\n\n");

    return 0;
}

#else
/* To prevent warning about "translation unit is empty"
 * when this test is disabled. 
 */
int dummy_pool_test;
#endif	/* INCLUDE_POOL_TEST */

