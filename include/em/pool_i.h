/*
 * File:    mpool.h
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

#include "em/string.h"

EM_IDEF(em_size_t) em_pool_get_capacity( em_pool_t *pool )
{
    return pool->capacity;
}

EM_IDEF(em_size_t) em_pool_get_used_size( em_pool_t *pool )
{
    em_pool_block *b = pool->block_list.next;
    em_size_t used_size = sizeof(em_pool_t);
    while (b != &pool->block_list) {
        used_size += (b->cur - b->buf) + sizeof(em_pool_block);
        b = b->next;
    }
    return used_size;
}

EM_IDEF(void*) em_pool_alloc_from_block( em_pool_block *block, em_size_t size )
{
    /* The operation below is valid for size==0. 
     * When size==0, the function will return the pointer to the pool
     * memory address, but no memory will be allocated.
     */
    if (size & (EM_POOL_ALIGNMENT-1)) {
        size = (size + EM_POOL_ALIGNMENT) & ~(EM_POOL_ALIGNMENT-1);
    }
    if ((em_size_t)(block->end - block->cur) >= size) {
        void *ptr = block->cur;
        block->cur += size;
        return ptr;
    }
    return NULL;
}

EM_IDEF(void*) em_pool_alloc( em_pool_t *pool, em_size_t size)
{
    void *ptr = em_pool_alloc_from_block(pool->block_list.next, size);
    if (!ptr)
        ptr = em_pool_allocate_find(pool, size);
    return ptr;
}


EM_IDEF(void*) em_pool_calloc( em_pool_t *pool, em_size_t count, em_size_t size)
{
    void *buf = em_pool_alloc( pool, size*count);
    if (buf)
        em_bzero(buf, size * count);
    return buf;
}

EM_IDEF(const char *) em_pool_getobjname( const em_pool_t *pool )
{
    return pool->obj_name;
}

EM_IDEF(em_pool_t*) em_pool_create( em_pool_factory *f, 
        const char *name,
        em_size_t initial_size, 
        em_size_t increment_size,
        em_pool_callback *callback)
{
    return (*f->create_pool)(f, name, initial_size, increment_size, callback);
}

EM_IDEF(void) em_pool_release( em_pool_t *pool )
{
    if (pool->factory->release_pool)
        (*pool->factory->release_pool)(pool->factory, pool);
}


EM_IDEF(void) em_pool_safe_release( em_pool_t **ppool )
{
    em_pool_t *pool = *ppool;
    *ppool = NULL;
    if (pool)
        em_pool_release(pool);
}
