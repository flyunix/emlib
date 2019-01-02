/*
 * File:    pool.h
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
#include "em/log.h"
//#include "em/except.h"
#include "em/assert.h"
#include "em/os.h"

#if !EM_HAS_POOL_ALT_API

/* Include inline definitions when inlining is disabled. */
#if !EM_FUNCTIONS_ARE_INLINED
#  include "em/pool_i.h"
#endif

#define ALIGN_PTR(PTR,ALIGNMENT)    (PTR + (-(em_ssize_t)(PTR) & (ALIGNMENT-1)))

EM_DEF_DATA(int) EM_NO_MEMORY_EXCEPTION;

EM_DEF(int) em_NO_MEMORY_EXCEPTION()
{
    return EM_NO_MEMORY_EXCEPTION;
}

/*
 * Create new block.
 * Create a new big chunk of memory block, from which user allocation will be
 * taken from.
 */
static em_pool_block *em_pool_create_block( em_pool_t *pool, em_size_t size)
{
    em_pool_block *block;

    EM_CHECK_STACK();
    em_assert(size >= sizeof(em_pool_block));

    EM_LOG_MOD(EM_LOG_DEBUG, pool->obj_name, "create_block(sz=%u), cur.cap=%u, cur.used=%u", 
            size, pool->capacity, em_pool_get_used_size(pool));

    /* Request memory from allocator. */
    block = (em_pool_block*) 
        (*pool->factory->policy.block_alloc)(pool->factory, size);
    if (block == NULL) {
        (*pool->callback)(pool, size);
        return NULL;
    }

    /* Add capacity. */
    pool->capacity += size;

    /* Set start and end of buffer. */
    block->buf = ((unsigned char*)block) + sizeof(em_pool_block);
    block->end = ((unsigned char*)block) + size;

    /* Set the start pointer, aligning it as needed */
    block->cur = ALIGN_PTR(block->buf, EM_POOL_ALIGNMENT);

    /* Insert in the front of the list. */
    em_list_insert_after(&pool->block_list, block);

    EM_LOG_MOD(EM_LOG_DEBUG, pool->obj_name,"block created, buffer=%p-%p",block->buf, block->end);

    return block;
}

/*
 * Allocate memory chunk for user from available blocks.
 * This will iterate through block list to find space to allocate the chunk.
 * If no space is available in all the blocks, a new block might be created
 * (depending on whether the pool is allowed to resize).
 */
EM_DEF(void*) em_pool_allocate_find(em_pool_t *pool, em_size_t size)
{
    em_pool_block *block = pool->block_list.next;
    void *p;
    em_size_t block_size;

    EM_CHECK_STACK();

    while (block != &pool->block_list) {
        p = em_pool_alloc_from_block(block, size);
        if (p != NULL)
            return p;
        block = block->next;
    }
    /* No available space in all blocks. */

    /* If pool is configured NOT to expand, return error. */
    if (pool->increment_size == 0) {
        EM_LOG_MOD(EM_LOG_DEBUG, pool->obj_name, "Can't expand pool to allocate %u bytes "
                    "(used=%u, cap=%u)",
                    size, em_pool_get_used_size(pool), pool->capacity);
        (*pool->callback)(pool, size);
        return NULL;
    }

    /* If pool is configured to expand, but the increment size
     * is less than the required size, expand the pool by multiple
     * increment size. Also count the size wasted due to aligning
     * the block.
     */
    if (pool->increment_size < 
            size + sizeof(em_pool_block) + EM_POOL_ALIGNMENT) 
    {
        em_size_t count;
        count = (size + pool->increment_size + sizeof(em_pool_block) +
                EM_POOL_ALIGNMENT) / 
            pool->increment_size;
        block_size = count * pool->increment_size;

    } else {
        block_size = pool->increment_size;
    }

    EM_LOG_MOD(EM_LOG_DEBUG, pool->obj_name, 
                "%u bytes requested, resizing pool by %u bytes (used=%u, cap=%u)",
                size, block_size, em_pool_get_used_size(pool), pool->capacity);

    block = em_pool_create_block(pool, block_size);
    if (!block)
        return NULL;

    p = em_pool_alloc_from_block(block, size);
    em_assert(p != NULL);
#if EM_DEBUG
    if (p == NULL) {
        EM_UNUSED_ARG(p);
    }
#endif
    return p;
}

/*
 * Internal function to initialize pool.
 */
EM_DEF(void) em_pool_init_int(  em_pool_t *pool, 
        const char *name,
        em_size_t increment_size,
        em_pool_callback *callback)
{
    EM_CHECK_STACK();

    pool->increment_size = increment_size;
    pool->callback = callback;

    if (name) {
        if (strchr(name, '%') != NULL) {
            em_ansi_snprintf(pool->obj_name, sizeof(pool->obj_name), 
                    name, pool);
        } else {
            em_ansi_strncpy(pool->obj_name, name, EM_MAX_OBJ_NAME);
            pool->obj_name[EM_MAX_OBJ_NAME-1] = '\0';
        }
    } else {
        pool->obj_name[0] = '\0';
    }
}

/*
 * Create new memory pool.
 */
EM_DEF(em_pool_t*) em_pool_create_int( em_pool_factory *f, const char *name,
        em_size_t initial_size, 
        em_size_t increment_size,
        em_pool_callback *callback)
{
    em_pool_t *pool;
    em_pool_block *block;
    uint8 *buffer;

    EM_CHECK_STACK();

    /* Size must be at least sizeof(em_pool)+sizeof(em_pool_block) */
    EMLIB_ASSERT_RETURN(initial_size >= sizeof(em_pool_t)+sizeof(em_pool_block),
            NULL);

    /* If callback is NULL, set calback from the policy */
    if (callback == NULL)
        callback = f->policy.callback;

    /* Allocate initial block */
    buffer = (uint8*) (*f->policy.block_alloc)(f, initial_size);
    if (!buffer)
        return NULL;

    /* Set pool administrative data. */
    pool = (em_pool_t*)buffer;
    em_bzero(pool, sizeof(*pool));

    em_list_init(&pool->block_list);
    pool->factory = f;

    /* Create the first block from the memory. */
    block = (em_pool_block*) (buffer + sizeof(*pool));
    block->buf = ((unsigned char*)block) + sizeof(em_pool_block);
    block->end = buffer + initial_size;

    /* Set the start pointer, aligning it as needed */
    block->cur = ALIGN_PTR(block->buf, EM_POOL_ALIGNMENT);

    em_list_insert_after(&pool->block_list, block);

    em_pool_init_int(pool, name, increment_size, callback);

    /* Pool initial capacity and used size */
    pool->capacity = initial_size;

    EM_LOG_MOD(EM_LOG_DEBUG, pool->obj_name, "pool created, size=%u", pool->capacity);
    return pool;
}

/*
 * Reset the pool to the state when it was created.
 * All blocks will be deallocated except the first block. All memory areas
 * are marked as free.
 */
static void reset_pool(em_pool_t *pool)
{
    em_pool_block *block;

    EM_CHECK_STACK();

    block = pool->block_list.prev;
    if (block == &pool->block_list)
        return;

    /* Skip the first block because it is occupying the same memory
       as the pool itself.
       */
    block = block->prev;

    while (block != &pool->block_list) {
        em_pool_block *prev = block->prev;
        em_list_erase(block);
        (*pool->factory->policy.block_free)(pool->factory, block, 
                block->end - (unsigned char*)block);
        block = prev;
    }

    block = pool->block_list.next;

    /* Set the start pointer, aligning it as needed */
    block->cur = ALIGN_PTR(block->buf, EM_POOL_ALIGNMENT);

    pool->capacity = block->end - (unsigned char*)pool;
}

/*
 * The public function to reset pool.
 */
EM_DEF(void) em_pool_reset(em_pool_t *pool)
{
    EM_LOG_MOD(EM_LOG_DEBUG, pool->obj_name, "reset(): cap=%d, used=%d(%d%%)", 
                pool->capacity, em_pool_get_used_size(pool), 
                em_pool_get_used_size(pool)*100/pool->capacity);

    reset_pool(pool);
}

/*
 * Destroy the pool.
 */
EM_DEF(void) em_pool_destroy_int(em_pool_t *pool)
{
    em_size_t initial_size;

    EM_LOG_MOD(EM_LOG_DEBUG, pool->obj_name, "destroy(): cap=%d, used=%d(%d%%), block0=%p-%p", 
                pool->capacity, em_pool_get_used_size(pool), 
                em_pool_get_used_size(pool)*100/pool->capacity,
                ((em_pool_block*)pool->block_list.next)->buf, 
                ((em_pool_block*)pool->block_list.next)->end);

    reset_pool(pool);
    initial_size = ((em_pool_block*)pool->block_list.next)->end - 
        (unsigned char*)pool;
    if (pool->factory->policy.block_free)
        (*pool->factory->policy.block_free)(pool->factory, pool, initial_size);
}


#endif	/* EM_HAS_POOL_ALT_API */

