/*
 * File:    exception.c
 * Author:  Liu HongLiang<lhl_nciae@sina.cn>
 * Brief:   C exception.
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
 * 2019-01-04 Liu HongLiang<lhl_nciae@sina.cn> created.
 *
 */
#include <em/except.h>
#include <em/os.h>
#include <em/assert.h>
#include <em/log.h>
#include <em/errno.h>
#include <em/string.h>

static const char* module = "EXCEPTION";

static long thread_local_id = -1;

#if defined(EM_HAS_EXCEPTION_NAMES) && EM_HAS_EXCEPTION_NAMES != 0
static const char *exception_id_names[EM_MAX_EXCEPTION_ID];
#else
/*
 * Start from 1 (not 0)!!!
 * Exception 0 is reserved for normal path of setjmp()!!!
 */
static int last_exception_id = 1;
#endif  /* EM_HAS_EXCEPTION_NAMES */


#if !defined(EM_EXCEPTION_USE_WIN32_SEH) || EM_EXCEPTION_USE_WIN32_SEH==0
EM_DEF(void) em_throw_exception_(int exception_id)
{
    struct em_exception_state_t *handler;

    handler = (struct em_exception_state_t*) 
        em_thread_local_get(thread_local_id);
    if (handler == NULL) {
        EM_LOG_MOD(EM_LOG_ERROR,"except.c", "!!!FATAL: unhandled exception %s!\n", 
                em_exception_id_name(exception_id));
        em_assert(handler != NULL);
        /* This will crash the system! */
    }
    em_pop_exception_handler_(handler);
    em_longjmp(handler->state, exception_id);
}

static void exception_cleanup(void)
{
    if (thread_local_id != -1) {
        em_thread_local_free(thread_local_id);
        thread_local_id = -1;
    }

#if defined(EM_HAS_EXCEPTION_NAMES) && EM_HAS_EXCEPTION_NAMES != 0
    {
        unsigned i;
        for (i=0; i<EM_MAX_EXCEPTION_ID; ++i)
            exception_id_names[i] = NULL;
    }
#else
    last_exception_id = 1;
#endif
}

EM_DEF(void) em_push_exception_handler_(struct em_exception_state_t *rec)
{
    struct em_exception_state_t *parent_handler = NULL;

    if (thread_local_id == -1) {
        em_thread_local_alloc(&thread_local_id);
        em_assert(thread_local_id != -1);
        em_atexit(&exception_cleanup);
    }
    parent_handler = (struct em_exception_state_t *)
        em_thread_local_get(thread_local_id);
    rec->prev = parent_handler;
    em_thread_local_set(thread_local_id, rec);
}

EM_DEF(void) em_pop_exception_handler_(struct em_exception_state_t *rec)
{
    struct em_exception_state_t *handler;

    handler = (struct em_exception_state_t *)
        em_thread_local_get(thread_local_id);
    if (handler && handler==rec) {
        em_thread_local_set(thread_local_id, handler->prev);
    }
}
#endif

#if defined(EM_HAS_EXCEPTION_NAMES) && EM_HAS_EXCEPTION_NAMES != 0
EM_DEF(emlib_ret_t) em_exception_id_alloc( const char *name,
        em_exception_id_t *id)
{
    unsigned i;

    em_enter_critical_section();

    /*
     * Start from 1 (not 0)!!!
     * Exception 0 is reserved for normal path of setjmp()!!!
     */
    for (i=1; i<EM_MAX_EXCEPTION_ID; ++i) {
        if (exception_id_names[i] == NULL) {
            exception_id_names[i] = name;
            *id = i;
            em_leave_critical_section();
            return EM_SUCC;
        }
    }

    em_leave_critical_section();
    return EM_ETOOMANY;
}

EM_DEF(emlib_ret_t) em_exception_id_free( em_exception_id_t id )
{
    /*
     * Start from 1 (not 0)!!!
     * Exception 0 is reserved for normal path of setjmp()!!!
     */
    EMLIB_ASSERT_RETURN(id>0 && id<EM_MAX_EXCEPTION_ID, EM_EINVAL);

    em_enter_critical_section();
    exception_id_names[id] = NULL;
    em_leave_critical_section();

    return EM_SUCC;

}

EM_DEF(const char*) em_exception_id_name(em_exception_id_t id)
{
    static char unknown_name[32];

    /*
     * Start from 1 (not 0)!!!
     * Exception 0 is reserved for normal path of setjmp()!!!
     */
    EMLIB_ASSERT_RETURN(id>0 && id<EM_MAX_EXCEPTION_ID, "<Invalid ID>");

    if (exception_id_names[id] == NULL) {
        em_ansi_snprintf(unknown_name, sizeof(unknown_name), 
                "exception %d", id);
        return unknown_name;
    }

    return exception_id_names[id];
}

#else   /* EM_HAS_EXCEPTION_NAMES */
EM_DEF(emlib_ret_t) em_exception_id_alloc( const char *name,
        em_exception_id_t *id)
{
    EMLIB_ASSERT_RETURN(last_exception_id < EM_MAX_EXCEPTION_ID-1, EM_ETOOMANY);

    *id = last_exception_id++;
    return EM_SUCC;
}

EM_DEF(emlib_ret_t) em_exception_id_free( em_exception_id_t id )
{
    return EM_SUCC;
}

EM_DEF(const char*) em_exception_id_name(em_exception_id_t id)
{
    return "";
}

#endif  /* EM_HAS_EXCEPTION_NAMES */


