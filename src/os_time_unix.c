/*
 * File:    os_time_unix.c
 * Author:  Liu HongLiang<lhl_nciae@sina.cn>
 * Brief:   Linux time wrapper.
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
 * 2019-01-09 Liu HongLiang<lhl_nciae@sina.cn> created.
 *
 */

#include <em/os.h>
#include <em/errno.h>
#include <em/compat/time.h>

const static char *module = "OS_TIME_UNIX";

#if defined(EM_HAS_UNISTD_H) && EM_HAS_UNISTD_H!=0
#    include <unistd.h>
#endif

#include <errno.h>

#define S2S     1
#define S2NS    1000000000UL

#define USEC2S  1000000UL
#define MSEC2S  1000UL

#define USEC2NS 1000UL
#define MSEC2NS 1000000UL



#define TIME2SV_T(time, sv, T2S, T2NS)  \
    do {\
        if(time > 0) { \
            sv.sv_sec = time / T2S; \
            sv.sv_nsec = (time % T2S) * T2NS;\
        } else { \
            sv.sv_sec = 0;sv.sv_nsec = 0;    \
        }\
    }while(0)

#define TIMEADD(ts, sv) \
    do {\
        ts.tv_sec  += sv.sv_sec; \
        uint64 nsec = ts.tv_nsec + sv.sv_nsec;\
        if( nsec >= S2NS ) {\
            ts.tv_sec  += 1;\
            ts.tv_nsec = nsec - S2NS;\
        } else { \
            ts.tv_nsec = nsec;\
        }\
    }while(0)

///////////////////////////////////////////////////////////////////////////////

EM_DEF(emlib_ret_t) em_gettimeofday(em_time_val *p_tv)
{
    struct timeval the_time;
    int rc;

    EM_CHECK_STACK();

    rc = gettimeofday(&the_time, NULL);
    if (rc != 0)
        return EM_RETURN_OS_ERROR(em_get_native_os_error());

    p_tv->sec = the_time.tv_sec;
    p_tv->msec = the_time.tv_usec / 1000;
    return EM_SUCC;
}

#if (_XOPEN_SOURCE >= 600 || _POSIX_C_SOURCE >= 201112L)
static emlib_ret_t _em_sleep(em_sleep_val sv)
{
    EMLIB_ASSERT_RETURN(sv.sv_sec >= 0 && sv.sv_nsec >= 0, EM_EINVAL);

    emlib_ret_t ret = EM_FAIL;
    struct timespec ts;
    EM_CHECK_STACK();

    ret = clock_gettime(CLOCK_MONOTONIC, &ts);
    if(ret != EM_SUCC) {
        return EM_RETURN_OS_ERROR(em_get_native_os_error());
    }

    TIMEADD(ts, sv);
    ret = EM_FAIL;
    while(ret != EM_SUCC) {
        ret = clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &ts, NULL);
        if(ret > 0 && ret != EINTR) 
            return EM_RETURN_OS_ERROR(em_get_native_os_error());
    }

    return EM_SUCC;
}

#else
EM_DEF(emlib_ret_t) _em_sleep(em_sleep_val sv)
{
    EMLIB_ASSERT_RETURN(sv.sv_sec >= 0, EM_EINVAL);   
    emlib_ret_t ret;
    EM_CHECK_STACK();
    struct timespec ts;
    ts.tv_sec = sv.sv_sec;
    ts.tv_nsec = sv.sv_nsec;

    ret = nanosleep(&ts, NULL);

    if(ret != EM_SUCC) {
        return EM_RETURN_OS_ERROR(em_get_native_os_error());
    }

    return EM_SUCC;
}
#endif

EM_DEF(emlib_ret_t) em_sleep(uint32 sec)
{
    EMLIB_ASSERT_RETURN(sec >= 0, EM_EINVAL);

    em_sleep_val sv;
    EM_CHECK_STACK();

    TIME2SV_T(sec, sv, S2S, S2NS);
    return _em_sleep(sv);
}

EM_DEF(emlib_ret_t) em_usleep(uint32 usec)
{
    EMLIB_ASSERT_RETURN(usec >= 0, EM_EINVAL);     

    em_sleep_val sv;
    EM_CHECK_STACK();

    TIME2SV_T(usec, sv, USEC2S, USEC2NS);

    return _em_sleep(sv);
}

EM_DEF(emlib_ret_t)em_msleep(uint32 msec)
{
    EMLIB_ASSERT_RETURN(msec >= 0, EM_EINVAL);     

    em_sleep_val sv;

    EM_CHECK_STACK();
    TIME2SV_T(msec, sv, MSEC2S, MSEC2NS);

    return _em_sleep(sv);
}

