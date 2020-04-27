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
#include <em/except.h>

#if defined(EM_HAS_SEMAPHORE_H) && EM_HAS_SEMAPHORE_H != 0
#  include <semaphore.h>
#endif

#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>

static const char *module  = "os_core_unix";

#define SIGNATURE1  0xDEAFBEEF
#define SIGNATURE2  0xDEADC0DE


struct em_thread_t
{
    char	    obj_name[EM_MAX_OBJ_NAME];
    pthread_t	    thread;
    em_thread_proc *proc;
    void	   *arg;
    uint32	    signature1;
    uint32	    signature2;

    em_mutex_t	   *suspended_mutex;

#if defined(EM_OS_HAS_CHECK_STACK) && EM_OS_HAS_CHECK_STACK!=0
    uint32	    stk_size;
    uint32	    stk_max_usage;
    char	   *stk_start;
    const char	   *caller_file;
    int		    caller_line;
#endif
};

struct em_atomic_t
{
    em_mutex_t	       *mutex;
    em_atomic_value_t	value;
};

#if defined(EM_HAS_SEMAPHORE) && EM_HAS_SEMAPHORE != 0
struct em_sem_t
{
    sem_t	       *sem;
    char		   obj_name[EM_MAX_OBJ_NAME];
};
#endif /* EM_HAS_SEMAPHORE */

struct em_mutex_t
{
    pthread_mutex_t     mutex;
    char		obj_name[EM_MAX_OBJ_NAME];
#if EM_DEBUG
    int		        nesting_level;
    em_thread_t	       *owner;
    char		owner_name[EM_MAX_OBJ_NAME];
#endif
};

struct em_os_tt_obj_t{
    /*timer task name.*/
    char tt_name[EM_MAX_OBJ_NAME];

    /*timer task id.*/
    timer_t timer_id;

    /*callback function called by expired timer.*/
    void (*expired_cb)(void *args);

    /*args of callback function.*/
    void *args;

    /*initially expire time val.*/
    em_time_val it_value;

    /*timer interval*/
    em_time_val it_interval;

    /*timer task state.*/
    em_tt_state_e tt_state;

};

#if defined(EM_HAS_EVENT_OBJ) && EM_HAS_EVENT_OBJ != 0
struct em_event_t
{
    enum event_state {
        EV_STATE_OFF,
        EV_STATE_SET,
        EV_STATE_PULSED
    } state;

    em_mutex_t		mutex;
    pthread_cond_t	cond;

    em_bool_t		auto_reset;
    unsigned		threads_waiting;
    unsigned		threads_to_release;
};
#endif	/* EM_HAS_EVENT_OBJ */

/*
 * Flag and reference counter for EMLIB instance.
 */
static int initialized;

#if EM_HAS_THREADS
static em_thread_t main_thread;
static long thread_tls_id;
static em_mutex_t critical_section;
#else
#   define MAX_THREADS 32
static int tls_flag[MAX_THREADS];
static void *tls[MAX_THREADS];
#endif

static unsigned atexit_count;
static void (*atexit_func[32])(void);

static emlib_ret_t init_mutex(em_mutex_t *mutex, const char *name, int type);

/*
 * em_init(void).
 * Init EMLIB!
 */
EM_DEF(emlib_ret_t) em_init(void)
{
#if 0
    char dummy_guid[EM_GUID_MAX_LENGTH];
    em_str_t guid;
#endif
    emlib_ret_t rc;

    /* Check if EMLIB have been initialized */
    if (initialized) {
        ++initialized;
        return EM_SUCC;
    }

#if EM_HAS_THREADS
    /* Init this thread's TLS. */
    if ((rc = em_thread_init()) != 0) {
        return rc;
    }

    /* Critical section. */
    if ((rc = init_mutex(&critical_section, "critsec", EM_MUTEX_RECURSE)) != 0)
        return rc;
#endif

#if 0
    /* Init logging */
    em_log_init();

    /* Initialize exception ID for the pool.
     * Must do so after critical section is configured.
     */
    rc = em_exception_id_alloc("EMLIB/No memory", &EM_NO_MEMORY_EXCEPTION);
    if (rc != EM_SUCC)
        return rc;

    /* Init random seed. */
    /* Or probably not. Let application in charge of this */
    /* em_srand( clock() ); */

    /* Startup GUID. */
    guid.ptr = dummy_guid;
    em_generate_unique_string( &guid );

    /* Startup timestamp */
#if defined(EM_HAS_HIGH_RES_TIMER) && EM_HAS_HIGH_RES_TIMER != 0
    {
        em_timestamp dummy_ts;
        if ((rc=em_get_timestamp(&dummy_ts)) != 0) {
            return rc;
        }
    }
#endif
#endif
    /* Flag EMLIB as initialized */
    ++initialized;
    EMLIB_ASSERT(initialized == 1);

    EM_LOG(EM_LOG_DEBUG, "EMLIB %s for POSIX initialized", em_get_version());

    return EM_SUCC;
}

/*
 * em_atexit()
 */
EM_DEF(emlib_ret_t) em_atexit(void (*func)(void))
{
    if (atexit_count >= EM_ARRAY_SIZE(atexit_func))
        return EM_ETOOMANY;

    atexit_func[atexit_count++] = func;
    return EM_SUCC;
}

/*
 * em_shutdown(void)
 */
EM_DEF(void) em_shutdown()
{
    int i;

    /* Only perform shutdown operation when 'initialized' reaches zero */
    EMLIB_ASSERT(initialized > 0);
    if (--initialized != 0)
        return;

    /* Call atexit() functions */
    for (i=atexit_count-1; i>=0; --i) {
        (*atexit_func[i])();
    }
    atexit_count = 0;

    /* Free exception ID */
    if (EM_NO_MEMORY_EXCEPTION != -1) {
        em_exception_id_free(EM_NO_MEMORY_EXCEPTION);
        EM_NO_MEMORY_EXCEPTION = -1;
    }

#if EM_HAS_THREADS
    /* Destroy EMLIB critical section */
    em_mutex_destroy(&critical_section);

    /* Free EMLIB TLS */
    if (thread_tls_id != -1) {
        em_thread_local_free(thread_tls_id);
        thread_tls_id = -1;
    }

    /* Ticket #1132: Assertion when (re)starting EMLIB on different thread */
    em_bzero(&main_thread, sizeof(main_thread));
#endif

    /* Clear static variables */
    em_errno_clear_handlers();
}
/*
 * em_getpid(void)
 */
EM_DEF(uint32) em_getpid(void)
{
    EM_CHECK_STACK();
    return getpid();
}

/*
 * Check if this thread has been registered to EMLIB.
 */
EM_DEF(em_bool_t) em_thread_is_registered(void)
{
#if EM_HAS_THREADS
    return em_thread_local_get(thread_tls_id) != 0;
#else
    EMLIB_ASSERT("em_thread_is_registered() called in non-threading mode!");
    return EM_TRUE;
#endif
}


/*
 * Get thread priority value for the thread.
 */
EM_DEF(int) em_thread_get_prio(em_thread_t *thread)
{
#if EM_HAS_THREADS
    struct sched_param param;
    int policy;
    int rc;

    rc = pthread_getschedparam (thread->thread, &policy, &param);
    if (rc != 0)
        return -1;

    return param.sched_priority;
#else
    EM_UNUSED_ARG(thread);
    return 1;
#endif
}


/*
 * Set the thread priority.
 */
EM_DEF(emlib_ret_t) em_thread_set_prio(em_thread_t *thread,  int prio)
{
#if EM_HAS_THREADS
    struct sched_param param;
    int policy;
    int rc;

    rc = pthread_getschedparam (thread->thread, &policy, &param);
    if (rc != 0)
        return EM_RETURN_OS_ERROR(rc);

    param.sched_priority = prio;

    rc = pthread_setschedparam(thread->thread, policy, &param);
    if (rc != 0)
        return EM_RETURN_OS_ERROR(rc);

    return EM_SUCC;
#else
    EM_UNUSED_ARG(thread);
    EM_UNUSED_ARG(prio);
    EMLIB_ASSERT("em_thread_set_prio() called in non-threading mode!");
    return 1;
#endif
}


/*
 * Get the lowest priority value available on this system.
 */
EM_DEF(int) em_thread_get_prio_min(em_thread_t *thread)
{
    struct sched_param param;
    int policy;
    int rc;

    rc = pthread_getschedparam(thread->thread, &policy, &param);
    if (rc != 0)
        return -1;

#if defined(_POSIX_PRIORITY_SCHEDULING)
    return sched_get_priority_min(policy);
#elif defined __OpenBSD__
    /* Thread prio min/max are declared in OpenBSD private hdr */
    return 0;
#else
    EMLIB_ASSERT("em_thread_get_prio_min() not supported!");
    return 0;
#endif
}


/*
 * Get the highest priority value available on this system.
 */
EM_DEF(int) em_thread_get_prio_max(em_thread_t *thread)
{
    struct sched_param param;
    int policy;
    int rc;

    rc = pthread_getschedparam(thread->thread, &policy, &param);
    if (rc != 0)
        return -1;

#if defined(_POSIX_PRIORITY_SCHEDULING)
    return sched_get_priority_max(policy);
#elif defined __OpenBSD__
    /* Thread prio min/max are declared in OpenBSD private hdr */
    return 31;
#else
    EMLIB_ASSERT("em_thread_get_prio_max() not supported!");
    return 0;
#endif
}


/*
 * Get native thread handle
 */
EM_DEF(void*) em_thread_get_os_handle(em_thread_t *thread)
{
    EMLIB_ASSERT_RETURN(thread, NULL);

#if EM_HAS_THREADS
    return &thread->thread;
#else
    EMLIB_ASSERT("em_thread_is_registered() called in non-threading mode!");
    return NULL;
#endif
}

/*
 * em_thread_register(..)
 */
EM_DEF(emlib_ret_t) em_thread_register ( const char *cstr_thread_name,
        em_thread_desc desc,
        em_thread_t **ptr_thread)
{
#if EM_HAS_THREADS
    char stack_ptr;
    emlib_ret_t rc;
    em_thread_t *thread = (em_thread_t *)desc;
    em_str_t thread_name = em_str((char*)cstr_thread_name);

    /* Size sanity check. */
    if (sizeof(em_thread_desc) < sizeof(em_thread_t)) {
        EMLIB_ASSERT(!"Not enough em_thread_desc size!");
        return EM_EBUG;
    }

    /* Warn if this thread has been registered before */
    if (em_thread_local_get (thread_tls_id) != 0) {
        // 2006-02-26 bennylp:
        //  This wouldn't work in all cases!.
        //  If thread is created by external module (e.g. sound thread),
        //  thread may be reused while the pool used for the thread descriptor
        //  has been deleted by application.
        em_thread_t *thread_ptr = (em_thread_t*)em_thread_local_get (thread_tls_id);
        (void)thread_ptr;
        //return EM_SUCC;
        EM_LOG(EM_LOG_DEBUG, "Info: possibly re-registering existing thread");
    }

    /* On the other hand, also warn if the thread descriptor buffer seem to
     * have been used to register other threads.
     */
    EMLIB_ASSERT(thread->signature1 != SIGNATURE1 ||
            thread->signature2 != SIGNATURE2 ||
            (thread->thread != pthread_self()));
            /*BUG#1:2019-01-05:thead->thread == pthread_self() */

    /* Initialize and set the thread entry. */
    em_bzero(desc, sizeof(struct em_thread_t));
    thread->thread = pthread_self();
    thread->signature1 = SIGNATURE1;
    thread->signature2 = SIGNATURE2;

    if(cstr_thread_name &&em_strlen(&thread_name) < sizeof(thread->obj_name)-1)
        em_ansi_snprintf(thread->obj_name, sizeof(thread->obj_name),
                cstr_thread_name, thread->thread);
    else
        em_ansi_snprintf(thread->obj_name, sizeof(thread->obj_name),
                "thr%p", (void*)thread->thread);

    rc = em_thread_local_set(thread_tls_id, thread);
    if (rc != EM_SUCC) {
        em_bzero(desc, sizeof(struct em_thread_t));
        return rc;
    }

#if defined(EM_OS_HAS_CHECK_STACK) && EM_OS_HAS_CHECK_STACK!=0
    thread->stk_start = &stack_ptr;
    thread->stk_size = 0xFFFFFFFFUL;
    thread->stk_max_usage = 0;
#else
    EM_UNUSED_ARG(stack_ptr);
#endif

    *ptr_thread = thread;
    return EM_SUCC;
#else
    em_thread_t *thread = (em_thread_t*)desc;
    *ptr_thread = thread;
    return EM_SUCC;
#endif
}

/*
 * em_thread_init(void)
 */
emlib_ret_t em_thread_init(void)
{
#if EM_HAS_THREADS
    emlib_ret_t rc;
    em_thread_t *dummy;

    rc = em_thread_local_alloc(&thread_tls_id );
    if (rc != EM_SUCC) {
        return rc;
    }
    return em_thread_register("thr%p", (long*)&main_thread, &dummy);
#else
    EM_LOG(EM_LOG_TRACE, "Thread init error. Threading is not enabled!");
    return EM_EINVALIDOP;
#endif
}

#if EM_HAS_THREADS
/*
 * thread_main()
 *
 * This is the main entry for all threads.
 */
static void *thread_main(void *param)
{
    em_thread_t *rec = (em_thread_t*)param;
    void *result;
    emlib_ret_t rc;

#if defined(EM_OS_HAS_CHECK_STACK) && EM_OS_HAS_CHECK_STACK!=0
    rec->stk_start = (char*)&rec;
#endif

    /* Set current thread id. */
    rc = em_thread_local_set(thread_tls_id, rec);
    if (rc != EM_SUCC) {
        EMLIB_ASSERT_NOLOG(!"Thread TLS ID is not set (em_init() error?)");
    }

    /* Check if suspension is required. */
    if (rec->suspended_mutex) {
        em_mutex_lock(rec->suspended_mutex);
        em_mutex_unlock(rec->suspended_mutex);
    }

    EM_LOG_MOD(EM_LOG_TRACE, rec->obj_name, "Thread started");

    /* Call user's entry! */
    result = (void*)(long)(*rec->proc)(rec->arg);

    /* Done. */
    EM_LOG_MOD(EM_LOG_TRACE, rec->obj_name, "Thread quitting");

    return result;
}
#endif

/*
 * em_thread_create(...)
 */
EM_DEF(emlib_ret_t) em_thread_create( em_pool_t *pool,
        const char *thread_name,
        em_thread_proc *proc,
        void *arg,
        em_size_t stack_size,
        unsigned flags,
        em_thread_t **ptr_thread)
{
#if EM_HAS_THREADS
    em_thread_t *rec;
    pthread_attr_t thread_attr;
    void *stack_addr;
    int rc;

    EM_UNUSED_ARG(stack_addr);

    EM_CHECK_STACK();
    EMLIB_ASSERT_RETURN(pool && proc && ptr_thread, EM_EINVAL);

    /* Create thread record and assign name for the thread */
    rec = (struct em_thread_t*) em_pool_zalloc(pool, sizeof(em_thread_t));
    EMLIB_ASSERT_RETNOLOG(rec, EM_ENOMEM);

    /* Set name. */
    if (!thread_name)
        thread_name = "thr%p";

    if (strchr(thread_name, '%')) {
        em_ansi_snprintf(rec->obj_name, EM_MAX_OBJ_NAME, thread_name, rec);
    } else {
        strncpy(rec->obj_name, thread_name, EM_MAX_OBJ_NAME);
        rec->obj_name[EM_MAX_OBJ_NAME-1] = '\0';
    }

    /* Set default stack size */
    if (stack_size == 0)
        stack_size = EM_THREAD_DEFAULT_STACK_SIZE;

#if defined(EM_OS_HAS_CHECK_STACK) && EM_OS_HAS_CHECK_STACK!=0
    rec->stk_size = stack_size;
    rec->stk_max_usage = 0;
#endif

    /* Emulate suspended thread with mutex. */
    if (flags & EM_THREAD_SUSPENDED) {
        rc = em_mutex_create_simple(pool, NULL, &rec->suspended_mutex);
        if (rc != EM_SUCC) {
            return rc;
        }

        em_mutex_lock(rec->suspended_mutex);
    } else {
        EMLIB_ASSERT_NOLOG(rec->suspended_mutex == NULL);
    }


    /* Init thread attributes */
    pthread_attr_init(&thread_attr);

#if defined(EM_THREAD_SET_STACK_SIZE) && EM_THREAD_SET_STACK_SIZE!=0
    /* Set thread's stack size */
    rc = pthread_attr_setstacksize(&thread_attr, stack_size);
    if (rc != 0)
        return EM_RETURN_OS_ERROR(rc);
#endif	/* EM_THREAD_SET_STACK_SIZE */


#if defined(EM_THREAD_ALLOCATE_STACK) && EM_THREAD_ALLOCATE_STACK!=0
    /* Allocate memory for the stack */
    stack_addr = em_pool_alloc(pool, stack_size);
    EMLIB_ASSERT_RETNOLOG(stack_addr, EM_ENOMEM);

    rc = pthread_attr_setstackaddr(&thread_attr, stack_addr);
    if (rc != 0)
        return EM_RETURN_OS_ERROR(rc);
#endif	/* EM_THREAD_ALLOCATE_STACK */


    /* Create the thread. */
    rec->proc = proc;
    rec->arg = arg;
    rc = pthread_create( &rec->thread, &thread_attr, &thread_main, rec);
    if (rc != 0) {
        return EM_RETURN_OS_ERROR(rc);
    }

    *ptr_thread = rec;

    EM_LOG_MOD(EM_LOG_TRACE, rec->obj_name, "Thread created");
    return EM_SUCC;
#else
    EMLIB_ASSERT(!"Threading is disabled!");
    return EM_EINVALIDOP;
#endif
}

/*
 * em_thread-get_name()
 */
EM_DEF(const char*) em_thread_get_name(em_thread_t *p)
{
#if EM_HAS_THREADS
    em_thread_t *rec = (em_thread_t*)p;

    EM_CHECK_STACK();
    EMLIB_ASSERT_RETURN(p, "");

    return rec->obj_name;
#else
    return "";
#endif
}

/*
 * em_thread_resume()
 */
EM_DEF(emlib_ret_t) em_thread_resume(em_thread_t *p)
{
    emlib_ret_t rc;

    EM_CHECK_STACK();
    EMLIB_ASSERT_RETURN(p, EM_EINVAL);

    rc = em_mutex_unlock(p->suspended_mutex);

    return rc;
}

/*
 * em_thread_this()
 */
EM_DEF(em_thread_t*) em_thread_this(void)
{
#if EM_HAS_THREADS
    em_thread_t *rec = (em_thread_t*)em_thread_local_get(thread_tls_id);

    if (rec == NULL) {
        EMLIB_ASSERT(!"Calling emlib from unknown/external thread. You must "
                "register external threads with em_thread_register() "
                "before calling any emlib functions.");
    }

    /*
     * MUST NOT check stack because this function is called
     * by EM_CHECK_STACK() itself!!!
     *
     */

    return rec;
#else
    EMLIB_ASSERT(!"Threading is not enabled!");
    return NULL;
#endif
}

/*
 * em_thread_join()
 */
EM_DEF(emlib_ret_t) em_thread_join(em_thread_t *p)
{
#if EM_HAS_THREADS
    em_thread_t *rec = (em_thread_t *)p;
    void *ret;
    int result;

    EM_CHECK_STACK();

    if (p == em_thread_this())
        return EM_ECANCELLED;

    EM_LOG_MOD(EM_LOG_TRACE, em_thread_this()->obj_name, "Joining thread %s", p->obj_name);
    result = pthread_join( rec->thread, &ret);

    if (result == 0)
        return EM_SUCC;
    else {
        /* Calling pthread_join() on a thread that no longer exists and
         * getting back ESRCH isn't an error (in this context).
         * Thanks Phil Torre <ptorre@zetron.com>.
         */
        return result==ESRCH ? EM_SUCC : EM_RETURN_OS_ERROR(result);
    }
#else
    EM_CHECK_STACK();
    EMLIB_ASSERT(!"No multithreading support!");
    return EM_EINVALIDOP;
#endif
}

/*
 * em_thread_destroy()
 */
EM_DEF(emlib_ret_t) em_thread_destroy(em_thread_t *p)
{
    EM_CHECK_STACK();

    /* Destroy mutex used to suspend thread */
    if (p->suspended_mutex) {
        em_mutex_destroy(p->suspended_mutex);
        p->suspended_mutex = NULL;
    }

    return EM_SUCC;
}

/*
 * em_thread_sleep()
 */
EM_DEF(emlib_ret_t) em_thread_sleep(unsigned msec)
{
    /* TODO: should change this to something like EM_OS_HAS_NANOSLEEP */
#if defined(EM_RTEMS) && EM_RTEMS!=0
    enum { NANOSEC_PER_MSEC = 1000000 };
    struct timespec req;

    EM_CHECK_STACK();
    req.tv_sec = msec / 1000;
    req.tv_nsec = (msec % 1000) * NANOSEC_PER_MSEC;

    if (nanosleep(&req, NULL) == 0)
        return EM_SUCC;

    return EM_RETURN_OS_ERROR(em_get_native_os_error());
#else
    EM_CHECK_STACK();

    em_set_os_error(0);

    usleep(msec * 1000);

    /* MacOS X (reported on 10.5) seems to always set errno to ETIMEDOUT.
     * It does so because usleep() is declared to return int, and we're
     * supposed to check for errno only when usleep() returns non-zero.
     * Unfortunately, usleep() is declared to return void in other platforms
     * so it's not possible to always check for the return value (unless
     * we add a detection routine in autoconf).
     *
     * As a workaround, here we check if ETIMEDOUT is returned and
     * return successfully if it is.
     */
    if (em_get_native_os_error() == ETIMEDOUT)
        return EM_SUCC;

    return em_get_os_error();

#endif	/* EM_RTEMS */
}

EM_DEF(emlib_ret_t) em_get_threadid_size(void)
{
    return (sizeof(em_thread_t));
}
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

    EMLIB_ASSERT( sizeof(pthread_key_t) <= sizeof(long));
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
    EMLIB_ASSERT(index >= 0 && index < MAX_THREADS);
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
    EMLIB_ASSERT(index >= 0 && index < MAX_THREADS);
    return tls[index];
#endif
}

///////////////////////////////////////////////////////////////////////////////
EM_DEF(void) em_enter_critical_section(void)
{
#if EM_HAS_THREADS
    em_mutex_lock(&critical_section);
#endif
}

EM_DEF(void) em_leave_critical_section(void)
{
#if EM_HAS_THREADS
    em_mutex_unlock(&critical_section);
#endif
}


///////////////////////////////////////////////////////////////////////////////
#if defined(EM_LINUX) && EM_LINUX!=0
DECLS_BEGIN
EM_DECL(int) pthread_mutexattr_settype(pthread_mutexattr_t*,int);
DECLS_END
#endif

static emlib_ret_t init_mutex(em_mutex_t *mutex, const char *name, int type)
{
#if EM_HAS_THREADS
    pthread_mutexattr_t attr;
    int rc;

    EM_CHECK_STACK();

    rc = pthread_mutexattr_init(&attr);
    if (rc != 0)
        return EM_RETURN_OS_ERROR(rc);

    if (type == EM_MUTEX_SIMPLE) {
#if (defined(EM_LINUX) && EM_LINUX!=0) || \
        defined(EM_HAS_PTHREAD_MUTEXATTR_SETTYPE)
        rc = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL);
#elif (defined(EM_RTEMS) && EM_RTEMS!=0) || \
        defined(EM_PTHREAD_MUTEXATTR_T_HAS_RECURSIVE)
        /* Nothing to do, default is simple */
#else
        rc = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL);
#endif
    } else {
#if (defined(EM_LINUX) && EM_LINUX!=0) || \
        defined(EM_HAS_PTHREAD_MUTEXATTR_SETTYPE)
        rc = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
#elif (defined(EM_RTEMS) && EM_RTEMS!=0) || \
        defined(EM_PTHREAD_MUTEXATTR_T_HAS_RECURSIVE)
        // Phil Torre <ptorre@zetron.com>:
        // The RTEMS implementation of POSIX mutexes doesn't include
        // pthread_mutexattr_settype(), so what follows is a hack
        // until I get RTEMS patched to support the set/get functions.
        //
        // More info:
        //   newlib's pthread also lacks pthread_mutexattr_settype(),
        //   but it seems to have mutexattr.recursive.
        //EM_TODO(FIX_RTEMS_RECURSIVE_MUTEX_TYPE)
            attr.recursive = 1;
#else
        rc = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
#endif
    }

    if (rc != 0) {
        return EM_RETURN_OS_ERROR(rc);
    }

    rc = pthread_mutex_init(&mutex->mutex, &attr);
    if (rc != 0) {
        return EM_RETURN_OS_ERROR(rc);
    }

    rc = pthread_mutexattr_destroy(&attr);
    if (rc != 0) {
        emlib_ret_t status = EM_RETURN_OS_ERROR(rc);
        pthread_mutex_destroy(&mutex->mutex);
        return status;
    }

#if EM_DEBUG
    /* Set owner. */
    mutex->nesting_level = 0;
    mutex->owner = NULL;
    mutex->owner_name[0] = '\0';
#endif

    /* Set name. */
    if (!name) {
        name = "mtx%p";
    }
    if (strchr(name, '%')) {
        em_ansi_snprintf(mutex->obj_name, EM_MAX_OBJ_NAME, name, mutex);
    } else {
        strncpy(mutex->obj_name, name, EM_MAX_OBJ_NAME);
        mutex->obj_name[EM_MAX_OBJ_NAME-1] = '\0';
    }

    EM_LOG_MOD(EM_LOG_TRACE, mutex->obj_name, "Mutex created");
    return EM_SUCC;
#else /* EM_HAS_THREADS */
    return EM_SUCC;
#endif
}

/*
 * em_mutex_create()
 */
EM_DEF(emlib_ret_t) em_mutex_create(em_pool_t *pool,
        const char *name,
        int type,
        em_mutex_t **ptr_mutex)
{
#if EM_HAS_THREADS
    emlib_ret_t rc;
    em_mutex_t *mutex;

    EMLIB_ASSERT_RETURN(pool && ptr_mutex, EM_EINVAL);

    mutex = EM_POOL_ALLOC_T(pool, em_mutex_t);
    EMLIB_ASSERT_RETURN(mutex, EM_ENOMEM);

    if ((rc=init_mutex(mutex, name, type)) != EM_SUCC)
        return rc;

    *ptr_mutex = mutex;
    return EM_SUCC;
#else /* EM_HAS_THREADS */
    *ptr_mutex = (em_mutex_t*)1;
    return EM_SUCC;
#endif
}

/*
 * em_mutex_create_simple()
 */
EM_DEF(emlib_ret_t) em_mutex_create_simple( em_pool_t *pool,
        const char *name,
        em_mutex_t **mutex )
{
    return em_mutex_create(pool, name, EM_MUTEX_SIMPLE, mutex);
}

/*
 * em_mutex_create_recursive()
 */
EM_DEF(emlib_ret_t) em_mutex_create_recursive( em_pool_t *pool,
        const char *name,
        em_mutex_t **mutex )
{
    return em_mutex_create(pool, name, EM_MUTEX_RECURSE, mutex);
}

/*
 * em_mutex_lock()
 */
EM_DEF(emlib_ret_t) em_mutex_lock(em_mutex_t *mutex)
{
#if EM_HAS_THREADS
    emlib_ret_t status;

    EM_CHECK_STACK();
    EMLIB_ASSERT_RETURN(mutex, EM_EINVAL);

#if EM_DEBUG
    EM_LOG_MOD(EM_LOG_TRACE,mutex->obj_name, "Mutex: thread %s is waiting (mutex owner=%s)",
                em_thread_this()->obj_name,
                mutex->owner_name);
#else
    EM_LOG_MOD(EM_LOG_TRACE,mutex->obj_name, "Mutex: thread %s is waiting",
                em_thread_this()->obj_name);
#endif

    status = pthread_mutex_lock( &mutex->mutex );


#if EM_DEBUG
    if (status == EM_SUCC) {
        mutex->owner = em_thread_this();
        em_ansi_strcpy(mutex->owner_name, mutex->owner->obj_name);
        ++mutex->nesting_level;
    }

    EM_LOG_MOD(EM_LOG_TRACE,mutex->obj_name,
            status == 0 ?
            "Mutex acquired by thread %s (level=%d)" :
            "Mutex acquisition FAILED by %s (level=%d)",
            em_thread_this()->obj_name,
            mutex->nesting_level);
#else
    EM_LOG_MOD(EM_LOG_TRACE,mutex->obj_name,
                status==0 ? "Mutex acquired by thread %s" : "FAILED by %s",
                em_thread_this()->obj_name);
#endif

    if (status == 0)
        return EM_SUCC;
    else
        return EM_RETURN_OS_ERROR(status);
#else	/* EM_HAS_THREADS */
    EMLIB_ASSERT( mutex == (em_mutex_t*)1 );
    return EM_SUCC;
#endif
}

/*
 * em_mutex_unlock()
 */
EM_DEF(emlib_ret_t) em_mutex_unlock(em_mutex_t *mutex)
{
#if EM_HAS_THREADS
    emlib_ret_t status;

    EM_CHECK_STACK();
    EMLIB_ASSERT_RETURN(mutex, EM_EINVAL);

#if EM_DEBUG
    EMLIB_ASSERT(mutex->owner == em_thread_this());
    if (--mutex->nesting_level == 0) {
        mutex->owner = NULL;
        mutex->owner_name[0] = '\0';
    }

    EM_LOG_MOD(EM_LOG_TRACE,mutex->obj_name, "Mutex released by thread %s (level=%d)",
                em_thread_this()->obj_name,
                mutex->nesting_level);
#else
    EM_LOG_MOD(EM_LOG_TRACE,mutex->obj_name, "Mutex released by thread %s",
                em_thread_this()->obj_name);
#endif

    status = pthread_mutex_unlock( &mutex->mutex );
    if (status == 0)
        return EM_SUCC;
    else
        return EM_RETURN_OS_ERROR(status);

#else /* EM_HAS_THREADS */
    EMLIB_ASSERT( mutex == (em_mutex_t*)1 );
    return EM_SUCC;
#endif
}

/*
 * em_mutex_trylock()
 */
EM_DEF(emlib_ret_t) em_mutex_trylock(em_mutex_t *mutex)
{
#if EM_HAS_THREADS
    int status;

    EM_CHECK_STACK();
    EMLIB_ASSERT_RETURN(mutex, EM_EINVAL);

    EM_LOG_MOD(EM_LOG_TRACE,mutex->obj_name, "Mutex: thread %s is trying",
                em_thread_this()->obj_name);

    status = pthread_mutex_trylock( &mutex->mutex );

    if (status==0) {
#if EM_DEBUG
        mutex->owner = em_thread_this();
        em_ansi_strcpy(mutex->owner_name, mutex->owner->obj_name);
        ++mutex->nesting_level;

        EM_LOG_MOD(EM_LOG_TRACE,mutex->obj_name, "Mutex acquired by thread %s (level=%d)",
                em_thread_this()->obj_name,
                mutex->nesting_level);
#else
        EM_LOG_MOD(EM_LOG_TRACE, mutex->obj_name, "Mutex acquired by thread %s",
                em_thread_this()->obj_name);
#endif
    } else {
        EM_LOG_MOD(EM_LOG_TRACE,mutex->obj_name, "Mutex: thread %s's trylock() failed",
                em_thread_this()->obj_name);
    }

    if (status==0)
        return EM_SUCC;
    else
        return EM_RETURN_OS_ERROR(status);
#else	/* EM_HAS_THREADS */
    EMLIB_ASSERT( mutex == (em_mutex_t*)1);
    return EM_SUCC;
#endif
}

/*
 * em_mutex_destroy()
 */
EM_DEF(emlib_ret_t) em_mutex_destroy(em_mutex_t *mutex)
{
    enum { RETRY = 4 };
    int status = 0;
    unsigned retry;

    EM_CHECK_STACK();
    EMLIB_ASSERT_RETURN(mutex, EM_EINVAL);

#if EM_HAS_THREADS
    EM_LOG_MOD(EM_LOG_TRACE,mutex->obj_name, "Mutex destroyed by thread %s",
            em_thread_this()->obj_name);

    for (retry=0; retry<RETRY; ++retry) {
        status = pthread_mutex_destroy( &mutex->mutex );
        if (status == EM_SUCC)
            break;
        else if (retry<RETRY-1 && status == EBUSY)
            pthread_mutex_unlock(&mutex->mutex);
    }

    if (status == 0)
        return EM_SUCC;
    else {
        return EM_RETURN_OS_ERROR(status);
    }
#else
    EMLIB_ASSERT( mutex == (em_mutex_t*)1 );
    status = EM_SUCC;
    return status;
#endif
}

#if EM_DEBUG
EM_DEF(em_bool_t) em_mutex_is_locked(em_mutex_t *mutex)
{
#if EM_HAS_THREADS
    return mutex->owner == em_thread_this();
#else
    return 1;
#endif
}
#endif

#if defined(EM_OS_HAS_CHECK_STACK) && EM_OS_HAS_CHECK_STACK != 0
/*
 * em_thread_check_stack()
 * Implementation for EM_CHECK_STACK()
 */
EM_DEF(void) em_thread_check_stack(const char *file, int line)
{
    char stk_ptr;
    uint32 usage;
    em_thread_t *thread = em_thread_this();
    
    /* Calculate current usage. */
    usage = (&stk_ptr > thread->stk_start) ? &stk_ptr - thread->stk_start :
		thread->stk_start - &stk_ptr;

    /* Assert if stack usage is dangerously high. */
    EMLIB_ASSERT("STACK OVERFLOW!! " && (usage <= thread->stk_size - 128));

    /* Keep statistic. */
    if (usage > thread->stk_max_usage) {
        thread->stk_max_usage = usage;
        thread->caller_file = file;
        thread->caller_line = line;
    }
}

/*
 * em_thread_get_stack_max_usage()
 */
EM_DEF(uint32) em_thread_get_stack_max_usage(em_thread_t *thread)
{
    return thread->stk_max_usage;
}

/*
 * em_thread_get_stack_info()
 */
EM_DEF(emlib_ret_t) em_thread_get_stack_info( em_thread_t *thread,
					      const char **file,
					      int *line )
{
    EMLIB_ASSERT(thread);

    *file = thread->caller_file;
    *line = thread->caller_line;
    return 0;
}
#endif	/* EM_OS_HAS_CHECK_STACK */

///////////////////////////////////////////////////////////////////////////////
/*
 * em_atomic_create()
 */
EM_DEF(emlib_ret_t) em_atomic_create( em_pool_t *pool,
				      em_atomic_value_t initial,
				      em_atomic_t **ptr_atomic)
{
    emlib_ret_t rc;
    em_atomic_t *atomic_var;

    atomic_var = EM_POOL_ZALLOC_T(pool, em_atomic_t);

    EMLIB_ASSERT_RETURN(atomic_var, EM_ENOMEM);

#if EM_HAS_THREADS
    rc = em_mutex_create(pool, "atm%p", EM_MUTEX_SIMPLE, &atomic_var->mutex);
    if (rc != EM_SUCC)
	return rc;
#endif
    atomic_var->value = initial;

    *ptr_atomic = atomic_var;
    return EM_SUCC;
}

/*
 * em_atomic_destroy()
 */
EM_DEF(emlib_ret_t) em_atomic_destroy( em_atomic_t *atomic_var )
{
    emlib_ret_t status;

    EMLIB_ASSERT_RETURN(atomic_var, EM_EINVAL);
    
#if EM_HAS_THREADS
    status = em_mutex_destroy( atomic_var->mutex );
    if (status == EM_SUCC) {
        atomic_var->mutex = NULL;
    }
    return status;
#else
    return 0;
#endif
}

/*
 * em_atomic_set()
 */
EM_DEF(void) em_atomic_set(em_atomic_t *atomic_var, em_atomic_value_t value)
{
    emlib_ret_t status;

    EM_CHECK_STACK();
    EMLIB_ASSERT_ON_FAIL(atomic_var, return);

#if EM_HAS_THREADS
    status = em_mutex_lock( atomic_var->mutex );
    if (status != EM_SUCC) {
        return;
    }
#endif
    atomic_var->value = value;
#if EM_HAS_THREADS
    em_mutex_unlock( atomic_var->mutex);
#endif
}

/*
 * em_atomic_get()
 */
EM_DEF(em_atomic_value_t) em_atomic_get(em_atomic_t *atomic_var)
{
    em_atomic_value_t oldval;

    EM_CHECK_STACK();

#if EM_HAS_THREADS
    em_mutex_lock( atomic_var->mutex );
#endif
    oldval = atomic_var->value;
#if EM_HAS_THREADS
    em_mutex_unlock( atomic_var->mutex);
#endif
    return oldval;
}

/*
 * em_atomic_inc_and_get()
 */
EM_DEF(em_atomic_value_t) em_atomic_inc_and_get(em_atomic_t *atomic_var)
{
    em_atomic_value_t new_value;

    EM_CHECK_STACK();


#if EM_HAS_THREADS
    em_mutex_lock( atomic_var->mutex );
#endif
    new_value = ++atomic_var->value;
#if EM_HAS_THREADS
    em_mutex_unlock( atomic_var->mutex);
#endif

    return new_value;
}
/*
 * em_atomic_inc()
 */
EM_DEF(void) em_atomic_inc(em_atomic_t *atomic_var)
{
    EMLIB_ASSERT_ON_FAIL(atomic_var, return);
    em_atomic_inc_and_get(atomic_var);
}

/*
 * em_atomic_dec_and_get()
 */
EM_DEF(em_atomic_value_t) em_atomic_dec_and_get(em_atomic_t *atomic_var)
{
    em_atomic_value_t new_value;

    EM_CHECK_STACK();

#if EM_HAS_THREADS
    em_mutex_lock( atomic_var->mutex );
#endif
    new_value = --atomic_var->value;
#if EM_HAS_THREADS
    em_mutex_unlock( atomic_var->mutex);
#endif

    return new_value;
}

/*
 * em_atomic_dec()
 */
EM_DEF(void) em_atomic_dec(em_atomic_t *atomic_var)
{
    EMLIB_ASSERT_ON_FAIL(atomic_var, return);
    em_atomic_dec_and_get(atomic_var);
}

/*
 * em_atomic_add_and_get()
 */
EM_DEF(em_atomic_value_t) em_atomic_add_and_get( em_atomic_t *atomic_var,
                                                 em_atomic_value_t value )
{
    em_atomic_value_t new_value;

#if EM_HAS_THREADS
    em_mutex_lock(atomic_var->mutex);
#endif

    atomic_var->value += value;
    new_value = atomic_var->value;

#if EM_HAS_THREADS
    em_mutex_unlock(atomic_var->mutex);
#endif

    return new_value;
}

/*
 * em_atomic_add()
 */
EM_DEF(void) em_atomic_add( em_atomic_t *atomic_var,
                            em_atomic_value_t value )
{
    EMLIB_ASSERT_ON_FAIL(atomic_var, return);
    em_atomic_add_and_get(atomic_var, value);
}

struct em_rwmutex_t
{
    pthread_rwlock_t rwlock;
};

EM_DEF(emlib_ret_t) em_rwmutex_create(em_pool_t *pool, const char *name,
				      em_rwmutex_t **p_mutex)
{
    em_rwmutex_t *rwm;
    emlib_ret_t status;

    EM_UNUSED_ARG(name);

    rwm = EM_POOL_ALLOC_T(pool, em_rwmutex_t);
    EMLIB_ASSERT_RETURN(rwm, EM_ENOMEM);

    status = pthread_rwlock_init(&rwm->rwlock, NULL);
    if (status != 0)
	return EM_RETURN_OS_ERROR(status);

    *p_mutex = rwm;
    return EM_SUCC;
}

/*
 * Lock the mutex for reading.
 *
 */
EM_DEF(emlib_ret_t) em_rwmutex_lock_read(em_rwmutex_t *mutex)
{
    emlib_ret_t status;

    status = pthread_rwlock_rdlock(&mutex->rwlock);
    if (status != 0)
	return EM_RETURN_OS_ERROR(status);

    return EM_SUCC;
}

/*
 * Lock the mutex for writing.
 *
 */
EM_DEF(emlib_ret_t) em_rwmutex_lock_write(em_rwmutex_t *mutex)
{
    emlib_ret_t status;

    status = pthread_rwlock_wrlock(&mutex->rwlock);
    if (status != 0)
	return EM_RETURN_OS_ERROR(status);

    return EM_SUCC;
}

/*
 * Release read lock.
 *
 */
EM_DEF(emlib_ret_t) em_rwmutex_unlock_read(em_rwmutex_t *mutex)
{
    return em_rwmutex_unlock_write(mutex);
}

/*
 * Release write lock.
 *
 */
EM_DEF(emlib_ret_t) em_rwmutex_unlock_write(em_rwmutex_t *mutex)
{
    emlib_ret_t status;

    status = pthread_rwlock_unlock(&mutex->rwlock);
    if (status != 0)
	return EM_RETURN_OS_ERROR(status);

    return EM_SUCC;
}

/*
 * Destroy reader/writer mutex.
 *
 */
EM_DEF(emlib_ret_t) em_rwmutex_destroy(em_rwmutex_t *mutex)
{
    emlib_ret_t status;

    status = pthread_rwlock_destroy(&mutex->rwlock);
    if (status != 0)
	return EM_RETURN_OS_ERROR(status);

    return EM_SUCC;
}

///////////////////////////////////////////////////////////////////////////////
#if defined(EM_HAS_SEMAPHORE) && EM_HAS_SEMAPHORE != 0

/*
 * em_sem_create()
 */
EM_DEF(emlib_ret_t) em_sem_create( em_pool_t *pool,
				   const char *name,
				   unsigned initial,
				   unsigned max,
                   em_sem_t **ptr_sem)
{
#if EM_HAS_THREADS
    em_sem_t *sem;

    EM_CHECK_STACK();
    EMLIB_ASSERT_RETURN(pool != NULL && ptr_sem != NULL, EM_EINVAL);

    sem = EM_POOL_ALLOC_T(pool, em_sem_t);
    EMLIB_ASSERT_RETURN(sem, EM_ENOMEM);

    sem->sem = EM_POOL_ALLOC_T(pool, sem_t);
    /**
     * Linux Programmer's Manual SEM_INIT(3)
     * tation
     *
     * tation
     *
     *
     * sem_init - initialize an unnamed semaphore
     *
     * int sem_init(sem_t *sem, int pshared, unsigned int value);
     *
     * The pshared argument indicates whether this semaphore is to be shared between the threads of a process, or between processes.
     *
     * If pshared has the value 0, then the semaphore is shared between the threads of a process, and should be located at some address that is visible to all threads (e.g., a global variable, or a variable
     * allocated dynamically on the heap).
     *
     * If pshared is nonzero, then the semaphore is shared between processes, and should be located in a region of shared memory (see shm_open(3), mmap(2), and shmget(2)).  (Since a child created by fork(2)
     * inherits its parent's memory mappings, it can also access the semaphore.)  Any process that can access the shared memory region can operate on the semaphore using sem_post(3), sem_wait(3), etc.
     *
     */
    if (sem_init( sem->sem, 0, initial) != 0)
        return EM_RETURN_OS_ERROR(em_get_native_os_error());

    /* Set name. */
    if (!name) {
        name = "sem%p";
    }
    if (strchr(name, '%')) {
        em_ansi_snprintf(sem->obj_name, EM_MAX_OBJ_NAME, name, sem);
    } else {
        strncpy(sem->obj_name, name, EM_MAX_OBJ_NAME);
        sem->obj_name[EM_MAX_OBJ_NAME-1] = '\0';
    }

    EM_LOG(EM_LOG_TRACE, sem->obj_name, "Semaphore created");

    *ptr_sem = sem;
    return EM_SUCC;
#else
    *ptr_sem = (em_sem_t*)1;
    return EM_SUCC;
#endif
}

/*
 * em_sem_wait()
 */
EM_DEF(emlib_ret_t) em_sem_wait(em_sem_t *sem)
{
#if EM_HAS_THREADS
    int result;

    EM_CHECK_STACK();
    EMLIB_ASSERT_RETURN(sem, EM_EINVAL);

    EM_LOG(EM_LOG_TRACE, sem->obj_name, "Semaphore: thread %s is waiting",
                em_thread_this()->obj_name);

    result = sem_wait( sem->sem );

    if (result == 0) {
        EM_LOG(EM_LOG_TRACE, sem->obj_name, "Semaphore acquired by thread %s",
                    em_thread_this()->obj_name);
    } else {
        EM_LOG(EM_LOG_TRACE, sem->obj_name, "Semaphore: thread %s FAILED to acquire",
                    em_thread_this()->obj_name);
    }

    if (result == 0)
        return EM_SUCC;
    else
        return EM_RETURN_OS_ERROR(em_get_native_os_error());
#else
    em_assert( sem == (em_sem_t*) 1 );
    return EM_SUCC;
#endif
}

/*
 * em_sem_trywait()
 */
EM_DEF(emlib_ret_t) em_sem_trywait(em_sem_t *sem)
{
#if EM_HAS_THREADS
    int result;

    EM_CHECK_STACK();
    EMLIB_ASSERT_RETURN(sem, EM_EINVAL);

    result = sem_trywait( sem->sem );

    if (result == 0) {
        EM_LOG(EM_LOG_TRACE, sem->obj_name, "Semaphore acquired by thread %s",
                    em_thread_this()->obj_name);
    }
    if (result == 0)
        return EM_SUCC;
    else
        return EM_RETURN_OS_ERROR(em_get_native_os_error());
#else
    em_assert( sem == (em_sem_t*)1 );
    return EM_SUCC;
#endif
}

/*
 * em_sem_post()
 */
EM_DEF(emlib_ret_t) em_sem_post(em_sem_t *sem)
{
#if EM_HAS_THREADS
    int result;
    EM_LOG(EM_LOG_TRACE, sem->obj_name, "Semaphore released by thread %s",
                em_thread_this()->obj_name);
    result = sem_post( sem->sem );

    if (result == 0)
        return EM_SUCC;
    else
        return EM_RETURN_OS_ERROR(em_get_native_os_error());
#else
    em_assert( sem == (em_sem_t*) 1);
    return EM_SUCC;
#endif
}

/*
 * em_sem_destroy()
 */
EM_DEF(emlib_ret_t) em_sem_destroy(em_sem_t *sem)
{
#if EM_HAS_THREADS
    int result;

    EM_CHECK_STACK();
    EMLIB_ASSERT_RETURN(sem, EM_EINVAL);

    EM_LOG(EM_LOG_TRACE, sem->obj_name, "Semaphore destroyed by thread %s",
                em_thread_this()->obj_name);
    result = sem_destroy( sem->sem );

    if (result == 0)
        return EM_SUCC;
    else
        return EM_RETURN_OS_ERROR(em_get_native_os_error());
#else
    em_assert( sem == (em_sem_t*) 1 );
    return EM_SUCC;
#endif
}

#endif	/* EM_HAS_SEMAPHORE */

///////////////////////////////////////////////////////////////////////////////
#if defined(EM_HAS_EVENT_OBJ) && EM_HAS_EVENT_OBJ != 0

/**
 *Note: ASYNC-SIGNAL SAFETY
 *The condition functions are not async-signal safe, and should not be called from a signal handler. 
 *In particular, calling pthread_cond_signal or pthread_cond_broadcast from a signal handler may dead‐ lock the calling thread.
 **/

/*
 * em_event_create()
 */
EM_DEF(emlib_ret_t) em_event_create(em_pool_t *pool, const char *name,
				    em_bool_t manual_reset, em_bool_t initial,
                    em_event_t **ptr_event)
{
    em_event_t *event;

    event = EM_POOL_ALLOC_T(pool, em_event_t);

    init_mutex(&event->mutex, name, EM_MUTEX_SIMPLE);
    pthread_cond_init(&event->cond, 0);
    event->auto_reset = !manual_reset;
    event->threads_waiting = 0;

    if (initial) {
        event->state = EV_STATE_SET;
        event->threads_to_release = 1;
    } else {
        event->state = EV_STATE_OFF;
        event->threads_to_release = 0;
    }

    *ptr_event = event;
    return EM_SUCC;
}

static void event_on_one_release(em_event_t *event)
{
    if (event->state == EV_STATE_SET) {
        if (event->auto_reset) {
            event->threads_to_release = 0;
            event->state = EV_STATE_OFF;
        } else {
            /* Manual reset remains on */
        }
    } else {
        if (event->auto_reset) {
            /* Only release one */
            event->threads_to_release = 0;
            event->state = EV_STATE_OFF;
        } else {
            event->threads_to_release--;
            EM_ASSERT(event->threads_to_release >= 0);
            if (event->threads_to_release==0)
                event->state = EV_STATE_OFF;
        }
    }
}

/*
 * em_event_wait()
 */
EM_DEF(emlib_ret_t) em_event_wait(em_event_t *event)
{
    pthread_mutex_lock(&event->mutex.mutex);
    event->threads_waiting++;
    while (event->state == EV_STATE_OFF)
        pthread_cond_wait(&event->cond, &event->mutex.mutex);
    event->threads_waiting--;
    event_on_one_release(event);
    pthread_mutex_unlock(&event->mutex.mutex);
    return EM_SUCC;
}

/*
 * em_event_trywait()
 */
EM_DEF(emlib_ret_t) em_event_trywait(em_event_t *event)
{
    emlib_ret_t status;

    pthread_mutex_lock(&event->mutex.mutex);
    status = event->state != EV_STATE_OFF ? EM_SUCC : -1;
    if (status==EM_SUCC) {
        event_on_one_release(event);
    }
    pthread_mutex_unlock(&event->mutex.mutex);

    return status;
}

/*
 * em_event_set()
 */
EM_DEF(emlib_ret_t) em_event_set(em_event_t *event)
{
    pthread_mutex_lock(&event->mutex.mutex);
    event->threads_to_release = 1;
    event->state = EV_STATE_SET;
    if (event->auto_reset)
        pthread_cond_signal(&event->cond);
    else
        pthread_cond_broadcast(&event->cond);
    pthread_mutex_unlock(&event->mutex.mutex);
    return EM_SUCC;
}

/*
 * em_event_pulse()
 */
EM_DEF(emlib_ret_t) em_event_pulse(em_event_t *event)
{
    pthread_mutex_lock(&event->mutex.mutex);
    if (event->threads_waiting) {
        event->threads_to_release = event->auto_reset ? 1 :
            event->threads_waiting;
        event->state = EV_STATE_PULSED;
        if (event->threads_to_release==1)
            pthread_cond_signal(&event->cond);
        else
            pthread_cond_broadcast(&event->cond);
    }
    pthread_mutex_unlock(&event->mutex.mutex);
    return EM_SUCC;
}

/*
 * em_event_reset()
 */
EM_DEF(emlib_ret_t) em_event_reset(em_event_t *event)
{
    pthread_mutex_lock(&event->mutex.mutex);
    event->state = EV_STATE_OFF;
    event->threads_to_release = 0;
    pthread_mutex_unlock(&event->mutex.mutex);
    return EM_SUCC;
}

/*
 * em_event_destroy()
 */
EM_DEF(emlib_ret_t) em_event_destroy(em_event_t *event)
{
    em_mutex_destroy(&event->mutex);
    pthread_cond_destroy(&event->cond);
    return EM_SUCC;
}

/**
 * em_create_timer_task
 */
static void notify_function(sigval_t val)
{
    em_thread_t *this_thread;
    static em_thread_desc desc;

    em_bzero(desc, sizeof(desc));

    EM_ERROR_CHECK_NORET(em_thread_register("timer_task_thread", desc, &this_thread));

    em_os_tt_obj_t *tt_obj = (em_os_tt_obj_t*)val.sival_ptr;

    EM_LOG(
            EM_LOG_DEBUG, 
            "timer_id:%ld, timer_name:%s, expired, call notify function.", 
            tt_obj->timer_id,
            tt_obj->tt_name
          );

    if(tt_obj->expired_cb) {
        tt_obj->expired_cb(tt_obj->args);
    }
}

static emlib_ret_t  create_timer_task(
            timer_t *timerid,
            em_os_tt_obj_t *tt_obj,
            em_tt_notify_type_e notify_type
            )
{
    EMLIB_ASSERT_RETURN(timerid, EM_EINVAL);
    
    struct sigevent sev;

    switch(notify_type) {
        case TTN_SIGNAL_E:
            break;
        case TTN_THREAD_E:
            sev.sigev_notify = SIGEV_THREAD;
            sev.sigev_notify_function = notify_function;
            sev.sigev_notify_attributes = NULL;
            sev.sigev_value.sival_ptr   = tt_obj;
            break;
        defalut:
            break;
    }

    if(timer_create(CLOCK_MONOTONIC, &sev, timerid) == -1) {
            return EM_RETURN_OS_ERROR(em_get_native_os_error());
    }

    return EM_SUCC;
}

EM_DEF(emlib_ret_t) em_create_timer_task(
        em_pool_t *pool, 
        char      *tt_name,
        em_time_val it_value, 
        em_time_val it_interval,
        em_tt_notify_type_e notify_type,
        void (*expired_cb)(void *args),
        void *args,
        em_os_tt_obj_t **tt_obj
        )
{
    EMLIB_ASSERT_RETURN(pool && expired_cb, EM_EINVAL);

    emlib_ret_t ret;
    em_os_tt_obj_t * os_tt_obj = em_pool_calloc(pool, 1, sizeof(em_os_tt_obj_t));

    if(tt_obj == NULL) {
        return EM_ENOMEM;
    }

    *tt_obj = os_tt_obj;

    /*Set timer task name.*/
    if(tt_name) {
        em_ansi_strncpy(os_tt_obj->tt_name, tt_name, EM_MAX_OBJ_NAME);
    } else {
        em_ansi_snprintf(os_tt_obj->tt_name, EM_MAX_OBJ_NAME, "tt-%ld", pthread_self());
    }
    
    /*Set timer task expired time.*/
    EM_TIME_VAL_SET(os_tt_obj->it_value, it_value) ;        
    EM_TIME_VAL_SET(os_tt_obj->it_interval, it_interval);

    /*Set timer task expired callback function.*/
    os_tt_obj->expired_cb = expired_cb;
    os_tt_obj->args = args;

    /*Set timer task current state.*/
    os_tt_obj->tt_state = TTS_CREATE_E;

    /*Create Posix timer task.*/
    ret = create_timer_task(&os_tt_obj->timer_id, os_tt_obj, notify_type);
    
    if(ret != EM_SUCC) {
        return ret;
    }

    EM_LOG(EM_LOG_DEBUG, "timer task:%s, crete succ.", os_tt_obj->tt_name);
    return EM_SUCC;
}

/**
 * em_os_ttask_start.
 */
EM_DEF(emlib_ret_t) em_os_ttask_start(em_os_tt_obj_t *tt_obj)
{
    EMLIB_ASSERT_RETURN(tt_obj, EM_EINVAL);

    emlib_ret_t ret;
    struct itimerspec new_value;

    new_value.it_value.tv_sec = tt_obj->it_value.sec;
    new_value.it_value.tv_nsec = tt_obj->it_value.msec * 1000000;
    new_value.it_interval.tv_sec = tt_obj->it_interval.sec;
    new_value.it_interval.tv_nsec = tt_obj->it_interval.msec * 1000000;

    EM_CHECK_STACK();

    ret = timer_settime(tt_obj->timer_id, 0, &new_value, NULL);

    if(ret != EM_SUCC) {
        return EM_RETURN_OS_ERROR(em_get_native_os_error());
    }

    tt_obj->tt_state = TTS_RUNNING_E;

    EM_LOG(EM_LOG_DEBUG, "timer task:%s start succ.", tt_obj->tt_name);

    return EM_SUCC;
}

/**
 * em_os_ttask_stop.
 */
EM_DEF(emlib_ret_t) em_os_ttask_stop(em_os_tt_obj_t *tt_obj)
{
    EMLIB_ASSERT_RETURN(tt_obj, EM_EINVAL);

    struct itimerspec new_value, remain;

    emlib_ret_t ret;
    EM_CHECK_STACK();

    memset(&new_value, 0, sizeof(struct itimerspec));

    ret = timer_settime(tt_obj->timer_id, 0, &new_value, &remain);

    if(ret != EM_SUCC) {
        return EM_RETURN_OS_ERROR(em_get_native_os_error());
    }

    /*save remain time when timer task stoped.*/
    tt_obj->it_value.sec = remain.it_value.tv_sec;
    tt_obj->it_value.msec = remain.it_value.tv_nsec / 1000000;

    tt_obj->tt_state = TTS_STOP_E;
    EM_LOG(EM_LOG_DEBUG, "timer task:%s stop succ.", tt_obj->tt_name);

    return EM_SUCC;
}

/**
 * em_os_ttask_resume.
 */
EM_DEF(emlib_ret_t) em_os_ttask_resume(em_os_tt_obj_t *tt_obj)
{
    EMLIB_ASSERT_RETURN(tt_obj, EM_EINVAL);

    emlib_ret_t ret;    
    EM_CHECK_STACK();

    ret = em_os_ttask_start(tt_obj);

    if(ret != EM_SUCC) {
        return ret;
    }

    tt_obj->tt_state = TTS_RUNNING_E;
    EM_LOG(EM_LOG_DEBUG, "timer task:%s resume succ.", tt_obj->tt_name);

    return EM_SUCC;
}

/**
 * em_os_ttask_destory.
 */
EM_DECL(emlib_ret_t) em_os_ttask_destroy(em_os_tt_obj_t *tt_obj)
{
    EMLIB_ASSERT_RETURN(tt_obj, EM_EINVAL);

    emlib_ret_t ret;

    ret = timer_delete(tt_obj->timer_id);

    if(ret != EM_SUCC) {
        return EM_RETURN_OS_ERROR(em_get_native_os_error()) ;
    }

    EM_LOG(EM_LOG_DEBUG, "timer task:%s delete succ.", tt_obj->tt_name);

    return EM_SUCC;
}

/**
 * em_os_ttask_destory.
 */
EM_DECL(emlib_ret_t) em_os_ttask_mod(
            em_os_tt_obj_t *tt_obj,
            em_time_val it_value, 
            em_time_val it_interval)
{
    EMLIB_ASSERT_RETURN(tt_obj, EM_EINVAL);

    emlib_ret_t ret;
    struct itimerspec new_value;

    new_value.it_value.tv_sec = it_value.sec;
    new_value.it_value.tv_nsec = it_value.msec * 1000000;
    new_value.it_interval.tv_sec = it_interval.sec;
    new_value.it_interval.tv_nsec = it_interval.msec * 1000000;

    EM_CHECK_STACK();

    ret = timer_settime(tt_obj->timer_id, 0, &new_value, NULL);

    if(ret != EM_SUCC) {
        return EM_RETURN_OS_ERROR(em_get_native_os_error());
    }


    tt_obj->tt_state = TTS_RUNNING_E;

    EM_LOG(EM_LOG_DEBUG, "timer task:%s mod succ.", tt_obj->tt_name);

    return EM_SUCC;
}

#endif	/* EM_HAS_EVENT_OBJ */
