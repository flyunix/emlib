/*
 * File:    lock.c
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
 * 2018-12-24 Liu HongLiang<lhl_nciae@sina.cn> created.
 *
 */

#include "em/os.h"
#include "em/lock.h"
#include "em/errno.h"

static  const char *module = "LOCK";

typedef void LOCK_OBJ;

typedef emlib_ret_t (*lock_func)(LOCK_OBJ *lock_obj);

struct em_lock_t {
    LOCK_OBJ  *lock_obj;

    lock_func lock;
    lock_func trylock;
    lock_func unlock;
    lock_func destroy;
};

/******************************************************************************
 * Implementation of lock object with mutex.
 */
static em_lock_t mutex_lock_template = 
{
    NULL,
    (lock_func) &em_mutex_lock,
    (lock_func) &em_mutex_trylock,
    (lock_func) &em_mutex_unlock,
    (lock_func) &em_mutex_destroy
};

static emlib_ret_t create_mutex_lock( em_pool_t *pool,
        const char *name,
        int type,
        em_lock_t **lock )
{
    em_lock_t *p_lock;
    em_mutex_t *mutex;
    emlib_ret_t rc;

    EMLIB_ASSERT_RETURN(pool && lock, EM_EINVAL);

    p_lock = EM_POOL_ALLOC_T(pool, em_lock_t);
    if (!p_lock)
        return EM_ENOMEM;

    em_memcpy(p_lock, &mutex_lock_template, sizeof(em_lock_t));
    rc = em_mutex_create(pool, name, type, &mutex);
    if (rc != EM_SUCC)
        return rc;

    p_lock->lock_obj = mutex;
    *lock = p_lock;
    return EM_SUCC;
}

EM_DEF(emlib_ret_t) em_lock_create_simple_mutex( em_pool_t *pool,
        const char *name,
        em_lock_t **lock )
{
    return create_mutex_lock(pool, name, EM_MUTEX_SIMPLE, lock);
}

EM_DEF(emlib_ret_t) em_lock_create_recursive_mutex( em_pool_t *pool,
        const char *name,
        em_lock_t **lock )
{
    return create_mutex_lock(pool, name, EM_MUTEX_RECURSE, lock);
}

inline emlib_ret_t em_lock(em_lock_t *thiz)
{
    return_val_if_fail((thiz != NULL) && (thiz->lock), EM_FAIL);

    return thiz->lock(thiz->lock_obj);
}

inline emlib_ret_t em_trylock(em_lock_t *thiz)
{
    return_val_if_fail((thiz != NULL) && (thiz->trylock), EM_FAIL);

    return thiz->trylock(thiz->lock_obj);
}

inline emlib_ret_t em_unlock(em_lock_t *thiz) 
{
    return_val_if_fail((thiz != NULL) && (thiz->unlock), EM_FAIL);

    return thiz->unlock(thiz->lock_obj);
}


inline void em_lock_destroy(em_lock_t *thiz)
{
    return_if_fail(thiz != NULL);

    thiz->destroy(thiz->lock_obj);
}

/******************************************************************************
 * Group lock
 */

/* Individual lock in the group lock */
typedef struct grp_lock_item
{
    EM_DECL_LIST_MEMBER(struct grp_lock_item);
    int		 prio;
    em_lock_t	*lock;

} grp_lock_item;

/* Destroy callbacks */
typedef struct grp_destroy_callback
{
    EM_DECL_LIST_MEMBER(struct grp_destroy_callback);
    void	*comp;
    void	(*handler)(void*);
} grp_destroy_callback;

#if EM_GRP_LOCK_DEBUG
/* Store each add_ref caller */
typedef struct grp_lock_ref
{
    EM_DECL_LIST_MEMBER(struct grp_lock_ref);
    const char	*file;
    int		 line;
} grp_lock_ref;
#endif

/* The group lock */
struct em_grp_lock_t
{
    em_lock_t	 	 base;

    em_pool_t		*pool;
    em_atomic_t		*ref_cnt;
    em_lock_t		*own_lock;

    em_thread_t		*owner;
    int			 owner_cnt;

    grp_lock_item	 lock_list;
    grp_destroy_callback destroy_list;

#if EM_GRP_LOCK_DEBUG
    grp_lock_ref	 ref_list;
    grp_lock_ref	 ref_free_list;
#endif
};


EM_DEF(void) em_grp_lock_config_default(em_grp_lock_config *cfg)
{
    em_bzero(cfg, sizeof(*cfg));
}

static void grp_lock_set_owner_thread(em_grp_lock_t *glock)
{
    if (!glock->owner) {
        glock->owner = em_thread_this();
        glock->owner_cnt = 1;
    } else {
        EM_ASSERT(glock->owner == em_thread_this());
        glock->owner_cnt++;
    }
}

static void grp_lock_unset_owner_thread(em_grp_lock_t *glock)
{
    EMLIB_ASSERT(glock->owner == em_thread_this());
    EMLIB_ASSERT(glock->owner_cnt > 0);
    if (--glock->owner_cnt <= 0) {
        glock->owner = NULL;
        glock->owner_cnt = 0;
    }
}

static emlib_ret_t grp_lock_acquire(LOCK_OBJ *p)
{
    em_grp_lock_t *glock = (em_grp_lock_t*)p;
    grp_lock_item *lck;

    EMLIB_ASSERT(em_atomic_get(glock->ref_cnt) > 0);

    lck = glock->lock_list.next;
    while (lck != &glock->lock_list) {
        em_lock(lck->lock);
        lck = lck->next;
    }
    grp_lock_set_owner_thread(glock);
    em_grp_lock_add_ref(glock);
    return EM_SUCC;
}

static emlib_ret_t grp_lock_tryacquire(LOCK_OBJ *p)
{
    em_grp_lock_t *glock = (em_grp_lock_t*)p;
    grp_lock_item *lck;

    EMLIB_ASSERT(em_atomic_get(glock->ref_cnt) > 0);

    lck = glock->lock_list.next;
    while (lck != &glock->lock_list) {
        emlib_ret_t status = em_trylock(lck->lock);
        if (status != EM_SUCC) {
            lck = lck->prev;
            while (lck != &glock->lock_list) {
                em_lock(lck->lock);
                lck = lck->prev;
            }
            return status;
        }
        lck = lck->next;
    }
    grp_lock_set_owner_thread(glock);
    em_grp_lock_add_ref(glock);
    return EM_SUCC;
}

static emlib_ret_t grp_lock_release(LOCK_OBJ *p)
{
    em_grp_lock_t *glock = (em_grp_lock_t*)p;
    grp_lock_item *lck;

    grp_lock_unset_owner_thread(glock);

    lck = glock->lock_list.prev;
    while (lck != &glock->lock_list) {
        em_lock(lck->lock);
        lck = lck->prev;
    }
    return em_grp_lock_dec_ref(glock);
}

static emlib_ret_t grp_lock_add_handler( em_grp_lock_t *glock,
        em_pool_t *pool,
        void *comp,
        void (*destroy)(void *comp),
        em_bool_t acquire_lock)
{
    grp_destroy_callback *cb;

    if (acquire_lock)
        grp_lock_acquire(glock);

    if (pool == NULL)
        pool = glock->pool;

    cb = EM_POOL_ZALLOC_T(pool, grp_destroy_callback);
    cb->comp = comp;
    cb->handler = destroy;
    em_list_push_back(&glock->destroy_list, cb);

    if (acquire_lock)
        grp_lock_release(glock);

    return EM_SUCC;
}

static emlib_ret_t grp_lock_destroy(LOCK_OBJ *p)
{
    em_grp_lock_t *glock = (em_grp_lock_t*)p;
    em_pool_t *pool = glock->pool;
    grp_lock_item *lck;
    grp_destroy_callback *cb;

    if (!glock->pool) {
        /* already destroyed?! */
        return EM_EINVAL;
    }

    /* Release all chained locks */
    lck = glock->lock_list.next;
    while (lck != &glock->lock_list) {
        if (lck->lock != glock->own_lock) {
            int i;
            for (i=0; i<glock->owner_cnt; ++i)
                em_unlock(lck->lock);
        }
        lck = lck->next;
    }

    /* Call callbacks */
    cb = glock->destroy_list.next;
    while (cb != &glock->destroy_list) {
        grp_destroy_callback *next = cb->next;
        cb->handler(cb->comp);
        cb = next;
    }

    em_lock_destroy(glock->own_lock);
    em_atomic_destroy(glock->ref_cnt);
    glock->pool = NULL;
    em_pool_release(pool);

    return EM_SUCC;
}


EM_DEF(emlib_ret_t) em_grp_lock_create( em_pool_t *pool,
        const em_grp_lock_config *cfg,
        em_grp_lock_t **p_grp_lock)
{
    em_grp_lock_t *glock;
    grp_lock_item *own_lock;
    emlib_ret_t status;

    EMLIB_ASSERT_RETURN(pool && p_grp_lock, EM_EINVAL);

    EM_UNUSED_ARG(cfg);

    pool = em_pool_create(pool->factory, "glck%p", 512, 512, NULL);
    if (!pool)
        return EM_ENOMEM;

    glock = EM_POOL_ZALLOC_T(pool, em_grp_lock_t);
    glock->base.lock_obj = glock;
    glock->base.lock = &grp_lock_acquire;
    glock->base.trylock = &grp_lock_tryacquire;
    glock->base.unlock = &grp_lock_release;
    glock->base.destroy = &grp_lock_destroy;

    glock->pool = pool;
    em_list_init(&glock->lock_list);
    em_list_init(&glock->destroy_list);
#if EM_GRP_LOCK_DEBUG
    em_list_init(&glock->ref_list);
    em_list_init(&glock->ref_free_list);
#endif

    status = em_atomic_create(pool, 0, &glock->ref_cnt);
    if (status != EM_SUCC)
        goto on_error;

    status = em_lock_create_recursive_mutex(pool, pool->obj_name,
            &glock->own_lock);
    if (status != EM_SUCC)
        goto on_error;

    own_lock = EM_POOL_ZALLOC_T(pool, grp_lock_item);
    own_lock->lock = glock->own_lock;
    em_list_push_back(&glock->lock_list, own_lock);

    *p_grp_lock = glock;
    return EM_SUCC;

on_error:
    grp_lock_destroy(glock);
    return status;
}

EM_DEF(emlib_ret_t) em_grp_lock_create_w_handler( em_pool_t *pool,
        const em_grp_lock_config *cfg,
        void *member,
        void (*handler)(void *member),
        em_grp_lock_t **p_grp_lock)
{
    emlib_ret_t status;

    status = em_grp_lock_create(pool, cfg, p_grp_lock);
    if (status == EM_SUCC) {
        grp_lock_add_handler(*p_grp_lock, pool, member, handler, EM_FALSE);
    }

    return status;
}

EM_DEF(emlib_ret_t) em_grp_lock_destroy( em_grp_lock_t *grp_lock)
{
    return grp_lock_destroy(grp_lock);
}

EM_DEF(emlib_ret_t) em_grp_lock_acquire( em_grp_lock_t *grp_lock)
{
    return grp_lock_acquire(grp_lock);
}

EM_DEF(emlib_ret_t) em_grp_lock_tryacquire( em_grp_lock_t *grp_lock)
{
    return grp_lock_tryacquire(grp_lock);
}

EM_DEF(emlib_ret_t) em_grp_lock_release( em_grp_lock_t *grp_lock)
{
    return grp_lock_release(grp_lock);
}

EM_DEF(emlib_ret_t) em_grp_lock_replace( em_grp_lock_t *old_lock,
        em_grp_lock_t *new_lock)
{
    grp_destroy_callback *ocb;

    /* Move handlers from old to new */
    ocb = old_lock->destroy_list.next;
    while (ocb != &old_lock->destroy_list) {
        grp_destroy_callback *ncb;

        ncb = EM_POOL_ALLOC_T(new_lock->pool, grp_destroy_callback);
        ncb->comp = ocb->comp;
        ncb->handler = ocb->handler;
        em_list_push_back(&new_lock->destroy_list, ncb);

        ocb = ocb->next;
    }

    em_list_init(&old_lock->destroy_list);

    grp_lock_destroy(old_lock);
    return EM_SUCC;
}

EM_DEF(emlib_ret_t) em_grp_lock_add_handler( em_grp_lock_t *glock,
        em_pool_t *pool,
        void *comp,
        void (*destroy)(void *comp))
{
    return grp_lock_add_handler(glock, pool, comp, destroy, EM_TRUE);
}

EM_DEF(emlib_ret_t) em_grp_lock_del_handler( em_grp_lock_t *glock,
        void *comp,
        void (*destroy)(void *comp))
{
    grp_destroy_callback *cb;

    grp_lock_acquire(glock);

    cb = glock->destroy_list.next;
    while (cb != &glock->destroy_list) {
        if (cb->comp == comp && cb->handler == destroy)
            break;
        cb = cb->next;
    }

    if (cb != &glock->destroy_list)
        em_list_erase(cb);

    grp_lock_release(glock);
    return EM_SUCC;
}

static emlib_ret_t grp_lock_add_ref(em_grp_lock_t *glock)
{
    em_atomic_inc(glock->ref_cnt);
    return EM_SUCC;
}

static emlib_ret_t grp_lock_dec_ref(em_grp_lock_t *glock)
{
    int cnt; /* for debugging */
    if ((cnt=em_atomic_dec_and_get(glock->ref_cnt)) == 0) {
        grp_lock_destroy(glock);
        return EM_EGONE;
    }
    EMLIB_ASSERT(cnt > 0);
    return EM_SUCC;
}

#if EM_GRP_LOCK_DEBUG
static emlib_ret_t grp_lock_dec_ref_dump(em_grp_lock_t *glock)
{
    emlib_ret_t status;

    status = grp_lock_dec_ref(glock);
    if (status == EM_SUCC) {
        em_grp_lock_dump(glock);
    } else if (status == EM_EGONE) {
        EM_LOG(EM_LOG_DEBUG, "Group lock %p destroyed.", glock);
    }

    return status;
}

EM_DEF(emlib_ret_t) em_grp_lock_add_ref_dbg(em_grp_lock_t *glock,
        const char *file,
        int line)
{
    grp_lock_ref *ref;
    emlib_ret_t status;

    em_enter_critical_section();
    if (!em_list_empty(&glock->ref_free_list)) {
        ref = glock->ref_free_list.next;
        em_list_erase(ref);
    } else {
        ref = em_pool_alloc_T(glock->pool, grp_lock_ref);
    }

    ref->file = file;
    ref->line = line;
    em_list_push_back(&glock->ref_list, ref);

    em_leave_critical_section();

    status = grp_lock_add_ref(glock);

    if (status != EM_SUCC) {
        em_enter_critical_section();
        em_list_erase(ref);
        em_list_push_back(&glock->ref_free_list, ref);
        em_leave_critical_section();
    }

    return status;
}

EM_DEF(emlib_ret_t) em_grp_lock_dec_ref_dbg(em_grp_lock_t *glock,
        const char *file,
        int line)
{
    grp_lock_ref *ref;

    EM_UNUSED_ARG(line);

    em_enter_critical_section();
    /* Find the same source file */
    ref = glock->ref_list.next;
    while (ref != &glock->ref_list) {
        if (strcmp(ref->file, file) == 0) {
            em_list_erase(ref);
            em_list_push_back(&glock->ref_free_list, ref);
            break;
        }
        ref = ref->next;
    }
    em_leave_critical_section();

    if (ref == &glock->ref_list) {
        EM_LOG(EM_LOG_WARN, "em_grp_lock_dec_ref_dbg() could not find "
                    "matching ref for %s", file);
    }

    return grp_lock_dec_ref_dump(glock);
}
#else
EM_DEF(emlib_ret_t) em_grp_lock_add_ref(em_grp_lock_t *glock)
{
    return grp_lock_add_ref(glock);
}

EM_DEF(emlib_ret_t) em_grp_lock_dec_ref(em_grp_lock_t *glock)
{
    return grp_lock_dec_ref(glock);
}
#endif

EM_DEF(int) em_grp_lock_get_ref(em_grp_lock_t *glock)
{
    return em_atomic_get(glock->ref_cnt);
}

EM_DEF(emlib_ret_t) em_grp_lock_chain_lock( em_grp_lock_t *glock,
        em_lock_t *lock,
        int pos)
{
    grp_lock_item *lck, *new_lck;
    int i;

    grp_lock_acquire(glock);

    for (i=0; i<glock->owner_cnt; ++i)
        em_lock(lock);

    lck = glock->lock_list.next;
    while (lck != &glock->lock_list) {
        if (lck->prio >= pos)
            break;
        lck = lck->next;
    }

    new_lck = EM_POOL_ZALLOC_T(glock->pool, grp_lock_item);
    new_lck->prio = pos;
    new_lck->lock = lock;
    em_list_insert_before(lck, new_lck);

    /* this will also release the new lock */
    grp_lock_release(glock);
    return EM_SUCC;
}

EM_DEF(emlib_ret_t) em_grp_lock_unchain_lock( em_grp_lock_t *glock,
        em_lock_t *lock)
{
    grp_lock_item *lck;

    grp_lock_acquire(glock);

    lck = glock->lock_list.next;
    while (lck != &glock->lock_list) {
        if (lck->lock == lock)
            break;
        lck = lck->next;
    }

    if (lck != &glock->lock_list) {
        int i;

        em_list_erase(lck);
        for (i=0; i<glock->owner_cnt; ++i)
            em_unlock(lck->lock);
    }

    grp_lock_release(glock);
    return EM_SUCC;
}

EM_DEF(void) em_grp_lock_dump(em_grp_lock_t *grp_lock)
{
#if EM_GRP_LOCK_DEBUG
    grp_lock_ref *ref;
    char info_buf[1000];
    em_str_t info;

    info.ptr = info_buf;
    info.slen = 0;

    grp_lock_add_ref(grp_lock);
    em_enter_critical_section();

    ref = grp_lock->ref_list.next;
    while (ref != &grp_lock->ref_list && info.slen < sizeof(info_buf)) {
        char *start = info.ptr + info.slen;
        int max_len = sizeof(info_buf) - info.slen;
        int len;

        len = em_ansi_snprintf(start, max_len, "\t%s:%d\n", ref->file, ref->line);
        if (len < 1 || len >= max_len) {
            len = strlen(ref->file);
            if (len > max_len - 1)
                len = max_len - 1;

            memcpy(start, ref->file, len);
            start[len++] = '\n';
        }

        info.slen += len;

        ref = ref->next;
    }

    if (ref != &grp_lock->ref_list) {
        int i;
        for (i=0; i<4; ++i)
            info_buf[sizeof(info_buf)-i-1] = '.';
    }
    info.ptr[info.slen-1] = '\0';

    em_leave_critical_section();

    EM_LOG(EM_LOG_DEBUG, "Group lock %p, ref_cnt=%d. Reference holders:\n%s",
            grp_lock, em_grp_lock_get_ref(grp_lock)-1, info.ptr);

    grp_lock_dec_ref(grp_lock);
#else
    EM_LOG(EM_LOG_DEBUG, "Group lock %p, ref_cnt=%d.",
                grp_lock, em_grp_lock_get_ref(grp_lock));
#endif
}
