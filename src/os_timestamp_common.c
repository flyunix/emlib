/*
 * File:    os_timestamp_common.c
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
 * 2019-01-11 Liu HongLiang<lhl_nciae@sina.cn> created.
 *
 */
#include <em/os.h>
#include <em/compat/high_precision.h>

#if defined(EM_HAS_HIGH_RES_TIMER) && EM_HAS_HIGH_RES_TIMER != 0

#define U32MAX  (0xFFFFFFFFUL)
#define NANOSEC (1000000000UL)
#define USEC    (1000000UL)
#define MSEC    (1000)

#define u64tohighprec(u64)	((em_highprec_t)((int64)(u64)))

static em_highprec_t get_elapsed( const em_timestamp *start,
                                  const em_timestamp *stop )
{
#if defined(EM_HAS_INT64) && EM_HAS_INT64!=0
    return u64tohighprec(stop->u64 - start->u64);
#else
    em_highprec_t elapsed_hi, elapsed_lo;

    elapsed_hi = stop->u32.hi - start->u32.hi;
    elapsed_lo = stop->u32.lo - start->u32.lo;

    /* elapsed_hi = elapsed_hi * U32MAX */
    em_highprec_mul(elapsed_hi, U32MAX);

    return elapsed_hi + elapsed_lo;
#endif
}

static em_highprec_t elapsed_msec( const em_timestamp *start,
                                   const em_timestamp *stop )
{
    em_timestamp ts_freq;
    em_highprec_t freq, elapsed;

    if (em_get_timestamp_freq(&ts_freq) != EM_SUCC)
        return 0;

    /* Convert frequency timestamp */
#if defined(EM_HAS_INT64) && EM_HAS_INT64!=0
    freq = u64tohighprec(ts_freq.u64);
#else
    freq = ts_freq.u32.hi;
    em_highprec_mul(freq, U32MAX);
    freq += ts_freq.u32.lo;
#endif

    /* Avoid division by zero. */
    if (freq == 0) freq = 1;

    /* Get elapsed time in cycles. */
    elapsed = get_elapsed(start, stop);

    /* usec = elapsed * MSEC / freq */
    em_highprec_mul(elapsed, MSEC);
    em_highprec_div(elapsed, freq);

    return elapsed;
}

static em_highprec_t elapsed_usec( const em_timestamp *start,
                                   const em_timestamp *stop )
{
    em_timestamp ts_freq;
    em_highprec_t freq, elapsed;

    if (em_get_timestamp_freq(&ts_freq) != EM_SUCC)
        return 0;

    /* Convert frequency timestamp */
#if defined(EM_HAS_INT64) && EM_HAS_INT64!=0
    freq = u64tohighprec(ts_freq.u64);
#else
    freq = ts_freq.u32.hi;
    em_highprec_mul(freq, U32MAX);
    freq += ts_freq.u32.lo;
#endif

    /* Avoid division by zero. */
    if (freq == 0) freq = 1;

    /* Get elapsed time in cycles. */
    elapsed = get_elapsed(start, stop);

    /* usec = elapsed * USEC / freq */
    em_highprec_mul(elapsed, USEC);
    em_highprec_div(elapsed, freq);

    return elapsed;
}

EM_DEF(uint32) em_elapsed_nanosec( const em_timestamp *start,
                                        const em_timestamp *stop )
{
    em_timestamp ts_freq;
    em_highprec_t freq, elapsed;

    if (em_get_timestamp_freq(&ts_freq) != EM_SUCC)
        return 0;

    /* Convert frequency timestamp */
#if defined(EM_HAS_INT64) && EM_HAS_INT64!=0
    freq = u64tohighprec(ts_freq.u64);
#else
    freq = ts_freq.u32.hi;
    em_highprec_mul(freq, U32MAX);
    freq += ts_freq.u32.lo;
#endif

    /* Avoid division by zero. */
    if (freq == 0) freq = 1;

    /* Get elapsed time in cycles. */
    elapsed = get_elapsed(start, stop);

    /* usec = elapsed * USEC / freq */
    em_highprec_mul(elapsed, NANOSEC);
    em_highprec_div(elapsed, freq);

    return (uint32)elapsed;
}

EM_DEF(uint32) em_elapsed_usec( const em_timestamp *start,
                                     const em_timestamp *stop )
{
    return (uint32)elapsed_usec(start, stop);
}

EM_DEF(uint32) em_elapsed_msec( const em_timestamp *start,
                                     const em_timestamp *stop )
{
    return (uint32)elapsed_msec(start, stop);
}

EM_DEF(uint64) em_elapsed_msec64(const em_timestamp *start,
                                      const em_timestamp *stop )
{
    return (uint64)elapsed_msec(start, stop);
}

EM_DEF(em_time_val) em_elapsed_time( const em_timestamp *start,
                                     const em_timestamp *stop )
{
    em_highprec_t elapsed = elapsed_msec(start, stop);
    em_time_val tv_elapsed;

    if (EM_HIGHPREC_VALUE_IS_ZERO(elapsed)) {
        tv_elapsed.sec = tv_elapsed.msec = 0;
        return tv_elapsed;
    } else {
        em_highprec_t sec, msec;

        sec = elapsed;
        em_highprec_div(sec, MSEC);
        tv_elapsed.sec = (long)sec;

        msec = elapsed;
        em_highprec_mod(msec, MSEC);
        tv_elapsed.msec = (long)msec;

        return tv_elapsed;
    }
}

EM_DEF(uint32) em_elapsed_cycle( const em_timestamp *start,
                                      const em_timestamp *stop )
{
    return stop->u32.lo - start->u32.lo;
}

EM_DEF(emlib_ret_t) em_gettickcount(em_time_val *tv)
{
    em_timestamp ts, start;
    emlib_ret_t status;

    if ((status = em_get_timestamp(&ts)) != EM_SUCC)
        return status;

    em_set_timestamp32(&start, 0, 0);
    *tv = em_elapsed_time(&start, &ts);

    return EM_SUCC;
}

#endif  /* EM_HAS_HIGH_RES_TIMER */

