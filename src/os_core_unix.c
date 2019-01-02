/*
 * File:    os_core_unix.c
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
/*
 * Contributors:
 * - Thanks for Zetron, Inc. (Phil Torre, ptorre@zetron.com) for donating
 *   the RTEMS port.
 */
#ifndef _GNU_SOURCE
#   define _GNU_SOURCE
#endif
#include <em/os.h>
#include <em/assert.h>
#include <em/pool.h>
#include <em/log.h>
#include <em/string.h>
#include <em/errno.h>


#include <unistd.h>
#include <errno.h>

#include <pthread.h>

static const char *module  = "os_core_unix";

///////////////////////////////////////////////////////////////////////////////
/*
 * em_thread_local_alloc()
 */
EM_DEF(emlib_ret_t) em_thread_local_alloc(long *p_index)
{
#if EM_HAS_THREADS
    pthread_key_t key;
    int rc;

    EMLIB_ASSERT_RETURN(p_index != NULL, EM_EINVAL);

    em_assert( sizeof(pthread_key_t) <= sizeof(long));
    if ((rc=pthread_key_create(&key, NULL)) != 0)
        return EM_RETURN_OS_ERROR(rc);

    *p_index = key;
    return EM_SUCC;
#else
    int i;
    for (i=0; i<MAX_THREADS; ++i) {
        if (tls_flag[i] == 0)
            break;
    }
    if (i == MAX_THREADS)
        return EM_ETOOMANY;

    tls_flag[i] = 1;
    tls[i] = NULL;

    *p_index = i;
    return EM_SUCC;
#endif
}

/*
 * em_thread_local_free()
 */
EM_DEF(void) em_thread_local_free(long index)
{
    EM_CHECK_STACK();
#if EM_HAS_THREADS
    pthread_key_delete(index);
#else
    tls_flag[index] = 0;
#endif
}

/*
 * em_thread_local_set()
 */
EM_DEF(emlib_ret_t) em_thread_local_set(long index, void *value)
{
    //Can't check stack because this function is called in the
    //beginning before main thread is initialized.
    //EM_CHECK_STACK();
#if EM_HAS_THREADS
    int rc=pthread_setspecific(index, value);
    return rc==0 ? EM_SUCC : EM_RETURN_OS_ERROR(rc);
#else
    em_assert(index >= 0 && index < MAX_THREADS);
    tls[index] = value;
    return EM_SUCC;
#endif
}

EM_DEF(void*) em_thread_local_get(long index)
{
    //Can't check stack because this function is called
    //by EM_CHECK_STACK() itself!!!
    //EM_CHECK_STACK();
#if EM_HAS_THREADS
    return pthread_getspecific(index);
#else
    em_assert(index >= 0 && index < MAX_THREADS);
    return tls[index];
#endif
}

