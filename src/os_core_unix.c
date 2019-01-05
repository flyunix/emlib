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


#include <unistd.h>
#include <errno.h>

#include <pthread.h>

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
        //*thread_ptr = (em_thread_t*)em_thread_local_get (thread_tls_id);
        //return EM_SUCC;
        EM_LOG(EM_LOG_DEBUG, "Info: possibly re-registering existing thread");
    }

    /* On the other hand, also warn if the thread descriptor buffer seem to
     * have been used to register other threads.
     */
    EMLIB_ASSERT(thread->signature1 != SIGNATURE1 ||
            thread->signature2 != SIGNATURE2 ||
            /*BUG#1:2019-01-05:thead->thread == pthread_self()
             * */
            (thread->thread != pthread_self()));


      
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
        EMLIB_ASSERT(!"Thread TLS ID is not set (em_init() error?)");
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
    EMLIB_ASSERT_RETURN(rec, EM_ENOMEM);

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
        EMLIB_ASSERT(rec->suspended_mutex == NULL);
    }


    /* Init thread attributes */
    pthread_attr_init(&thread_attr);

#if defined(EM_THREAD_SET_STACK_SIZE) && EM_THREAD_SET_STACK_SIZE!=0
    /* Set thread's stack size */
    rc = pthread_attr_setstacksize(&thread_attr, stack_size);
    if (rc != 0)
        return em_RETURN_OS_ERROR(rc);
#endif	/* EM_THREAD_SET_STACK_SIZE */


#if defined(EM_THREAD_ALLOCATE_STACK) && EM_THREAD_ALLOCATE_STACK!=0
    /* Allocate memory for the stack */
    stack_addr = em_pool_alloc(pool, stack_size);
    EMLIB_ASSERT_RETURN(stack_addr, EM_ENOMEM);

    rc = pthread_attr_setstackaddr(&thread_attr, stack_addr);
    if (rc != 0)
        return em_RETURN_OS_ERROR(rc);
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
                (status==0 ?
                 "Mutex acquired by thread %s (level=%d)" :
                 "Mutex acquisition FAILED by %s (level=%d)"),
                em_thread_this()->obj_name,
                mutex->nesting_level);
#else
    EM_LOG_MOD(EM_LOG_TRACE,mutex->obj_name,
                (status==0 ? "Mutex acquired by thread %s" : "FAILED by %s"),
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
