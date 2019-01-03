/* $Id: pool_caching.c 5170 2015-08-25 08:45:46Z nanang $ */
/* 
 * Copyright (C) 2008-2011 Teluu Inc. (http://www.teluu.com)
 * Copyright (C) 2003-2008 Benny Prijono <benny@prijono.org>
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
 */
#include <em/pool.h>
#include <em/log.h>
#include <em/string.h>
#include <em/assert.h>
#include <em/lock.h>
#include <em/os.h>
#include <em/pool_buf.h>
#include <em/pthread_lock.h>

#if !EM_HAS_POOL_ALT_API

static const char *module = "POOL_CACHING";

static em_pool_t* cpool_create_pool(em_pool_factory *pf, 
				    const char *name,
				    em_size_t initial_size, 
				    em_size_t increment_sz,
				    em_pool_callback *callback);
static void cpool_release_pool(em_pool_factory *pf, em_pool_t *pool);
static void cpool_dump_status(em_pool_factory *factory, em_bool_t detail );
static em_bool_t cpool_on_block_alloc(em_pool_factory *f, em_size_t sz);
static void cpool_on_block_free(em_pool_factory *f, em_size_t sz);


static em_size_t pool_sizes[EM_CACHING_POOL_ARRAY_SIZE] = 
{
    256, 512, 1024, 2048, 4096, 8192, 12288, 16384, 
    20480, 24576, 28672, 32768, 40960, 49152, 57344, 65536
};

/* Index where the search for size should begin.
 * Start with pool_sizes[5], which is 8192.
 */
#define START_SIZE  5


EM_DEF(void) em_caching_pool_init( em_caching_pool *cp, 
				   const em_pool_factory_policy *policy,
				   em_size_t max_capacity)
{
    int i;
    em_pool_t *pool;

    EM_CHECK_STACK();

    em_bzero(cp, sizeof(*cp));

    cp->max_capacity = max_capacity;
    em_list_init(&cp->used_list);
    for (i=0; i<EM_CACHING_POOL_ARRAY_SIZE; ++i)
        em_list_init(&cp->free_list[i]);

    if (policy == NULL) {
        policy = &em_pool_factory_default_policy;
    }

    em_memcpy(&cp->factory.policy, policy, sizeof(em_pool_factory_policy));
    cp->factory.create_pool = &cpool_create_pool;
    cp->factory.release_pool = &cpool_release_pool;
    cp->factory.dump_status = &cpool_dump_status;
    cp->factory.on_block_alloc = &cpool_on_block_alloc;
    cp->factory.on_block_free = &cpool_on_block_free;

    pool = em_pool_create_on_buf("cachingpool", cp->pool_buf, sizeof(cp->pool_buf));
    cp->locker = pthread_lock_create(pool);

    EMLIB_ASSERT(cp->locker != NULL);
}

EM_DEF(void) em_caching_pool_destroy( em_caching_pool *cp )
{
    int i;
    em_pool_t *pool;

    EM_CHECK_STACK();

    /* Delete all pool in free list */
    for (i=0; i < EM_CACHING_POOL_ARRAY_SIZE; ++i) {
        em_pool_t *next;
        pool = (em_pool_t*) cp->free_list[i].next;
        for (; pool != (void*)&cp->free_list[i]; pool = next) {
            next = pool->next;
            em_list_erase(pool);
            em_pool_destroy_int(pool);
        }
    }

    /* Delete all pools in used list */
    pool = (em_pool_t*) cp->used_list.next;
    while (pool != (em_pool_t*) &cp->used_list) {
        em_pool_t *next = pool->next;
        em_list_erase(pool);
        EM_LOG_MOD(EM_LOG_DEBUG,pool->obj_name, 
                "Pool is not released by application, releasing now");
        em_pool_destroy_int(pool);
        pool = next;
    }

    if (cp->locker) {
        em_lock_destroy(cp->locker);
    }
}

static em_pool_t* cpool_create_pool(em_pool_factory *pf, 
        const char *name, 
        em_size_t initial_size, 
        em_size_t increment_sz, 
        em_pool_callback *callback)
{
    em_caching_pool *cp = (em_caching_pool*)pf;
    em_pool_t *pool;
    int idx;

    EM_CHECK_STACK();

    em_lock(cp->locker);

    /* Use pool factory's policy when callback is NULL */
    if (callback == NULL) {
        callback = pf->policy.callback;
    }

    /* Search the suitable size for the pool. 
     * We'll just do linear search to the size array, as the array size itself
     * is only a few elements. Binary search I suspect will be less efficient
     * for this purpose.
     */
    if (initial_size <= pool_sizes[START_SIZE]) {
        for (idx=START_SIZE-1; 
                idx >= 0 && pool_sizes[idx] >= initial_size;
                --idx)
            ;
        ++idx;
    } else {
        for (idx=START_SIZE+1; 
                idx < EM_CACHING_POOL_ARRAY_SIZE && 
                pool_sizes[idx] < initial_size;
                ++idx)
            ;
    }

    /* Check whether there's a pool in the list. */
    if (idx==EM_CACHING_POOL_ARRAY_SIZE || em_list_empty(&cp->free_list[idx])) {
        /* No pool is available. */
        /* Set minimum size. */
        if (idx < EM_CACHING_POOL_ARRAY_SIZE)
            initial_size =  pool_sizes[idx];

        /* Create new pool */
        pool = em_pool_create_int(&cp->factory, name, initial_size, 
                increment_sz, callback);
        if (!pool) {
            em_unlock(cp->locker);
            return NULL;
        }

    } else {
        /* Get one pool from the list. */
        pool = (em_pool_t*) cp->free_list[idx].next;
        em_list_erase(pool);

        /* Initialize the pool. */
        em_pool_init_int(pool, name, increment_sz, callback);

        /* Update pool manager's free capacity. */
        if (cp->capacity > em_pool_get_capacity(pool)) {
            cp->capacity -= em_pool_get_capacity(pool);
        } else {
            cp->capacity = 0;
        }

        EM_LOG_MOD(EM_LOG_TRACE, pool->obj_name, "pool reused, size=%u", pool->capacity);
    }

    /* Put in used list. */
    em_list_insert_before( &cp->used_list, pool );

    /* Mark factory data */
    pool->factory_data = (void*) (em_ssize_t) idx;

    /* Increment used count. */
    ++cp->used_count;

    em_unlock(cp->locker);
    return pool;
}

static void cpool_release_pool( em_pool_factory *pf, em_pool_t *pool)
{
    em_caching_pool *cp = (em_caching_pool*)pf;
    em_size_t pool_capacity;
    unsigned i;

    EM_CHECK_STACK();

    EMLIB_ASSERT_ON_FAIL(pf && pool, return);

    em_lock(cp->locker);

#if PJ_SAFE_POOL
    /* Make sure pool is still in our used list */
    if (em_list_find_node(&cp->used_list, pool) != pool) {
        em_assert(!"Attempt to destroy pool that has been destroyed before");
        return;
    }
#endif

    /* Erase from the used list. */
    em_list_erase(pool);

    /* Decrement used count. */
    --cp->used_count;

    pool_capacity = em_pool_get_capacity(pool);

    /* Destroy the pool if the size is greater than our size or if the total
     * capacity in our recycle list (plus the size of the pool) exceeds 
     * maximum capacity.
     . */
    if (pool_capacity > pool_sizes[EM_CACHING_POOL_ARRAY_SIZE-1] ||
            cp->capacity + pool_capacity > cp->max_capacity)
    {
        em_pool_destroy_int(pool);
        em_unlock(cp->locker);
        return;
    }

    /* Reset pool. */
    EM_LOG_MOD(EM_LOG_TRACE, pool->obj_name, "recycle(): cap=%d, used=%d(%d%%)", 
            pool_capacity, em_pool_get_used_size(pool), 
            em_pool_get_used_size(pool)*100/pool_capacity);
    em_pool_reset(pool);

    pool_capacity = em_pool_get_capacity(pool);

    /*
     * Otherwise put the pool in our recycle list.
     */
    i = (unsigned) (unsigned long) (em_ssize_t) pool->factory_data;

    em_assert(i<EM_CACHING_POOL_ARRAY_SIZE);
    if (i >= EM_CACHING_POOL_ARRAY_SIZE ) {
        /* Something has gone wrong with the pool. */
        em_pool_destroy_int(pool);
        em_unlock(cp->locker);
        return;
    }

    em_list_insert_after(&cp->free_list[i], pool);
    cp->capacity += pool_capacity;

    em_unlock(cp->locker);
}

static void cpool_dump_status(em_pool_factory *factory, em_bool_t detail )
{
    em_caching_pool *cp = (em_caching_pool*)factory;

    em_lock(cp->locker);

    EM_LOG_MOD(EM_LOG_INFO, "cachpool", "Dumping caching pool:");
    EM_LOG_MOD(EM_LOG_INFO, "cachpool", "Capacity=%u, max_capacity=%u, used_cnt=%u", \
            cp->capacity, cp->max_capacity, cp->used_count);
    if (detail) {
        em_pool_t *pool = (em_pool_t*) cp->used_list.next;
        em_size_t total_used = 0, total_capacity = 0;
        EM_LOG_MOD(EM_LOG_INFO,"cachpool", "Dumping all active pools:");
        while (pool != (void*)&cp->used_list) {
            em_size_t pool_capacity = em_pool_get_capacity(pool);
            EM_LOG_MOD(EM_LOG_INFO,"cachpool", "   %16s: %8d of %8d (%d%%) used", 
                    em_pool_getobjname(pool), 
                    em_pool_get_used_size(pool), 
                    pool_capacity,
                    em_pool_get_used_size(pool)*100/pool_capacity);
            total_used += em_pool_get_used_size(pool);
            total_capacity += pool_capacity;
            pool = pool->next;
        }
        if (total_capacity) {
            EM_LOG_MOD(EM_LOG_INFO,"cachpool", "  Total %9d of %9d (%d %%) used!",
                    total_used, total_capacity,
                    total_used * 100 / total_capacity);
        }
    }

    em_unlock(cp->locker);
}


static em_bool_t cpool_on_block_alloc(em_pool_factory *f, em_size_t sz)
{
    em_caching_pool *cp = (em_caching_pool*)f;

    //Can't lock because mutex is not recursive
    //if (cp->mutex) pj_mutex_lock(cp->mutex);

    cp->used_size += sz;
    if (cp->used_size > cp->peak_used_size)
        cp->peak_used_size = cp->used_size;

    //if (cp->mutex) pj_mutex_unlock(cp->mutex);

    return EM_TRUE;
}


static void cpool_on_block_free(em_pool_factory *f, em_size_t sz)
{
    em_caching_pool *cp = (em_caching_pool*)f;

    //pj_mutex_lock(cp->mutex);
    cp->used_size -= sz;
    //pj_mutex_unlock(cp->mutex);
}


#endif	/* EM_HAS_POOL_ALT_API */

