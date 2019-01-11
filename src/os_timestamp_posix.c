/*
 * File:    os_timestamp_posix.c
 * Author:  Liu HongLiang<lhl_nciae@sina.cn>
 * Brief:   OS time common API.
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
 * 2019-01-10 Liu HongLiang<lhl_nciae@sina.cn> created.
 *
 */
#include <em/os.h>
#include <em/errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#if defined(EM_HAS_UNISTD_H) && EM_HAS_UNISTD_H != 0
#   include <unistd.h>

#   if defined(_POSIX_TIMERS) && _POSIX_TIMERS > 0 && \
       defined(_POSIX_MONOTONIC_CLOCK)
#       define USE_POSIX_TIMERS 1
#   endif

#endif

#if defined(USE_POSIX_TIMERS) && USE_POSIX_TIMERS != 0
#include <sys/time.h>
#include <time.h>
#include <errno.h>

#define NSEC_PER_SEC	1000000000

EM_DEF(emlib_ret_t) em_get_timestamp(em_timestamp *ts)
{
    struct timespec tp;

    if (clock_gettime(CLOCK_MONOTONIC, &tp) != 0) {
        return EM_RETURN_OS_ERROR(em_get_native_os_error());
    }

    ts->u64 = tp.tv_sec;
    ts->u64 *= NSEC_PER_SEC;
    ts->u64 += tp.tv_nsec;

    return EM_SUCC;
}

EM_DEF(emlib_ret_t) em_get_timestamp_freq(em_timestamp *freq)
{
    freq->u32.hi = 0;
    freq->u32.lo = NSEC_PER_SEC;

    return EM_SUCC;
}

#else
#include <sys/time.h>
#include <errno.h>

#define USEC_PER_SEC	1000000

EM_DEF(emlib_ret_t) em_get_timestamp(em_timestamp *ts)
{
    struct timeval tv;

    if (gettimeofday(&tv, NULL) != 0) {
        return EM_RETURN_OS_ERROR(em_get_native_os_error());
    }

    ts->u64 = tv.tv_sec;
    ts->u64 *= USEC_PER_SEC;
    ts->u64 += tv.tv_usec;

    return EM_SUCC;
}

EM_DEF(emlib_ret_t) em_get_timestamp_freq(em_timestamp *freq)
{
    freq->u32.hi = 0;
    freq->u32.lo = USEC_PER_SEC;

    return EM_SUCC;
}

#endif
