/*
 * File:    lock.h
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
 * 2018-12-21 Liu HongLiang<lhl_nciae@sina.cn> created.
 *
 */

#ifndef __EM_LOCK_H__
#define __EM_LOCK_H__

#include "em/types.h"
#include "em/assert.h"

DECLS_BEGIN

emlib_ret_t em_lock(em_lock_t *thiz);

emlib_ret_t em_trylock(em_lock_t *thiz);

emlib_ret_t em_unlock(em_lock_t *thiz);

void em_lock_destroy(em_lock_t *thiz);

/**
 * Create simple, non recursive mutex lock object.
 *
 * @param pool	    Memory pool.
 * @param name	    Lock object's name.
 * @param lock	    Pointer to store the returned handle.
 *
 * @return	    EM_SUCC or the appropriate error code.
 */
EM_DECL(emlib_ret_t) em_lock_create_simple_mutex( em_pool_t *pool,
						  const char *name,
						  em_lock_t **lock );

/**
 * Create recursive mutex lock object.
 *
 * @param pool	    Memory pool.
 * @param name	    Lock object's name.
 * @param lock	    Pointer to store the returned handle.
 *
 * @return	    EM_SUCC or the appropriate error code.
 */
EM_DECL(emlib_ret_t) em_lock_create_recursive_mutex( em_pool_t *pool,
						     const char *name,
						     em_lock_t **lock );


/**
 * @defgroup EM_GRP_LOCK Group Lock
 * @ingroup EM_LOCK
 * @{
 *
 * Group lock is a synchronization object to manage concurrency among members
 * within the same logical group. Example of such groups are:
 *
 *   - dialog, which has members such as the dialog itself, an invite session,
 *     and several transactions
 *   - ICE, which has members such as ICE stream transport, ICE session, STUN
 *     socket, TURN socket, and down to ioqueue key
 *
 * Group lock has three functions:
 *
 *   - mutual exclusion: to protect resources from being accessed by more than
 *     one threads at the same time
 *   - session management: to make sure that the resource is not destroyed
 *     while others are still using or about to use it.
 *   - lock coordinator: to provide uniform lock ordering among more than one
 *     lock objects, which is necessary to avoid deadlock.
 *
 * The requirements of the group lock are:
 *
 *    - must satisfy all the functions above
 *    - must allow members to join or leave the group (for example,
 *      transaction may be added or removed from a dialog)
 *    - must be able to synchronize with external lock (for example, a dialog
 *      lock must be able to sync itself with EMSUA lock)
 */

/**
 * Settings for creating the group lock.
 */
typedef struct em_grp_lock_config
{
    /**
     * Creation flags, currently must be zero.
     */
    unsigned	flags;

} em_grp_lock_config;


/**
 * Initialize the config with the default values.
 *
 * @param cfg		The config to be initialized.
 */
EM_DECL(void) em_grp_lock_config_default(em_grp_lock_config *cfg);

/**
 * Create a group lock object. Initially the group lock will have reference
 * counter of one.
 *
 * @param pool		The group lock only uses the pool parameter to get
 * 			the pool factory, from which it will create its own
 * 			pool.
 * @param cfg		Optional configuration.
 * @param p_grp_lock	Pointer to receive the newly created group lock.
 *
 * @return		EM_SUCCESS or the appropriate error code.
 */
EM_DECL(emlib_ret_t) em_grp_lock_create(em_pool_t *pool,
                                        const em_grp_lock_config *cfg,
                                        em_grp_lock_t **p_grp_lock);

/**
 * Create a group lock object, with the specified destructor handler, to be
 * called by the group lock when it is about to be destroyed. Initially the
 * group lock will have reference counter of one.
 *
 * @param pool		The group lock only uses the pool parameter to get
 * 			the pool factory, from which it will create its own
 * 			pool.
 * @param cfg		Optional configuration.
 * @param member	A pointer to be passed to the handler.
 * @param handler	The destroy handler.
 * @param p_grp_lock	Pointer to receive the newly created group lock.
 *
 * @return		EM_SUCCESS or the appropriate error code.
 */
EM_DECL(emlib_ret_t) em_grp_lock_create_w_handler(em_pool_t *pool,
                                        	  const em_grp_lock_config *cfg,
                                        	  void *member,
                                                  void (*handler)(void *member),
                                        	  em_grp_lock_t **p_grp_lock);

/**
 * Forcibly destroy the group lock, ignoring the reference counter value.
 *
 * @param grp_lock	The group lock.
 *
 * @return		EM_SUCCESS or the appropriate error code.
 */
EM_DECL(emlib_ret_t) em_grp_lock_destroy( em_grp_lock_t *grp_lock);

/**
 * Move the contents of the old lock to the new lock and destroy the
 * old lock.
 *
 * @param old_lock	The old group lock to be destroyed.
 * @param new_lock	The new group lock.
 *
 * @return		EM_SUCCESS or the appropriate error code.
 */
EM_DECL(emlib_ret_t) em_grp_lock_replace(em_grp_lock_t *old_lock,
                                         em_grp_lock_t *new_lock);

/**
 * Acquire lock on the specified group lock.
 *
 * @param grp_lock	The group lock.
 *
 * @return		EM_SUCCESS or the appropriate error code.
 */
EM_DECL(emlib_ret_t) em_grp_lock_acquire( em_grp_lock_t *grp_lock);

/**
 * Acquire lock on the specified group lock if it is available, otherwise
 * return immediately wihout waiting.
 *
 * @param grp_lock	The group lock.
 *
 * @return		EM_SUCCESS or the appropriate error code.
 */
EM_DECL(emlib_ret_t) em_grp_lock_tryacquire( em_grp_lock_t *grp_lock);

/**
 * Release the previously held lock. This may cause the group lock
 * to be destroyed if it is the last one to hold the reference counter.
 * In that case, the function will return EM_EGONE.
 *
 * @param grp_lock	The group lock.
 *
 * @return		EM_SUCCESS or the appropriate error code.
 */
EM_DECL(emlib_ret_t) em_grp_lock_release( em_grp_lock_t *grp_lock);

/**
 * Add a destructor handler, to be called by the group lock when it is
 * about to be destroyed.
 *
 * @param grp_lock	The group lock.
 * @param pool		Pool to allocate memory for the handler.
 * @param member	A pointer to be passed to the handler.
 * @param handler	The destroy handler.
 *
 * @return		EM_SUCCESS or the appropriate error code.
 */
EM_DECL(emlib_ret_t) em_grp_lock_add_handler(em_grp_lock_t *grp_lock,
                                             em_pool_t *pool,
                                             void *member,
                                             void (*handler)(void *member));

/**
 * Remove previously registered handler. All parameters must be the same
 * as when the handler was added.
 *
 * @param grp_lock	The group lock.
 * @param member	A pointer to be passed to the handler.
 * @param handler	The destroy handler.
 *
 * @return		EM_SUCCESS or the appropriate error code.
 */
EM_DECL(emlib_ret_t) em_grp_lock_del_handler(em_grp_lock_t *grp_lock,
                                             void *member,
                                             void (*handler)(void *member));

/**
 * Increment reference counter to prevent the group lock grom being destroyed.
 *
 * @param grp_lock	The group lock.
 *
 * @return		EM_SUCCESS or the appropriate error code.
 */
#if !EM_GRP_LOCK_DEBUG
EM_DECL(emlib_ret_t) em_grp_lock_add_ref(em_grp_lock_t *grp_lock);

#define em_grp_lock_add_ref_dbg(grp_lock, x, y) em_grp_lock_add_ref(grp_lock)

#else

#define em_grp_lock_add_ref(g)	em_grp_lock_add_ref_dbg(g, __FILE__, __LINE__)

EM_DECL(emlib_ret_t) em_grp_lock_add_ref_dbg(em_grp_lock_t *grp_lock,
                                             const char *file,
                                             int line);
#endif

/**
 * Decrement the reference counter. When the counter value reaches zero, the
 * group lock will be destroyed and all destructor handlers will be called.
 *
 * @param grp_lock	The group lock.
 *
 * @return		EM_SUCCESS or the appropriate error code.
 */
#if !EM_GRP_LOCK_DEBUG
EM_DECL(emlib_ret_t) em_grp_lock_dec_ref(em_grp_lock_t *grp_lock);

#define em_grp_lock_dec_ref_dbg(grp_lock, x, y) em_grp_lock_dec_ref(grp_lock)
#else

#define em_grp_lock_dec_ref(g)	em_grp_lock_dec_ref_dbg(g, __FILE__, __LINE__)

EM_DECL(emlib_ret_t) em_grp_lock_dec_ref_dbg(em_grp_lock_t *grp_lock,
                                             const char *file,
                                             int line);

#endif

/**
 * Get current reference count value. This normally is only used for
 * debugging purpose.
 *
 * @param grp_lock	The group lock.
 *
 * @return		The reference count value.
 */
EM_DECL(int) em_grp_lock_get_ref(em_grp_lock_t *grp_lock);


/**
 * Dump group lock info for debugging purpose. If group lock debugging is
 * enabled (via EM_GRP_LOCK_DEBUG) macro, this will print the group lock
 * reference counter value along with the source file and line. If
 * debugging is disabled, this will only print the reference counter.
 *
 * @param grp_lock	The group lock.
 */
EM_DECL(void) em_grp_lock_dump(em_grp_lock_t *grp_lock);


/**
 * Synchronize an external lock with the group lock, by adding it to the
 * list of locks to be acquired by the group lock when the group lock is
 * acquired.
 *
 * The ''pos'' argument specifies the lock order and also the relative
 * position with regard to lock ordering against the group lock. Locks with
 * lower ''pos'' value will be locked first, and those with negative value
 * will be locked before the group lock (the group lock's ''pos'' value is
 * zero).
 *
 * @param grp_lock	The group lock.
 * @param ext_lock	The external lock
 * @param pos		The position.
 *
 * @return		EM_SUCCESS or the appropriate error code.
 */
EM_DECL(emlib_ret_t) em_grp_lock_chain_lock(em_grp_lock_t *grp_lock,
                                            em_lock_t *ext_lock,
                                            int pos);

/**
 * Remove an external lock from group lock's list of synchronized locks.
 *
 * @param grp_lock	The group lock.
 * @param ext_lock	The external lock
 *
 * @return		EM_SUCCESS or the appropriate error code.
 */
EM_DECL(emlib_ret_t) em_grp_lock_unchain_lock(em_grp_lock_t *grp_lock,
                                              em_lock_t *ext_lock);


/** @} */
DECLS_END

#endif
