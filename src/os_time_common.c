/*
 * File:    os_time_common.c
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
 * 2019-01-09 Liu HongLiang<lhl_nciae@sina.cn> created.
 *
 */
#include <em/os.h>
#include <em/compat/time.h>
#include <em/errno.h>


///////////////////////////////////////////////////////////////////////////////

EM_DEF(emlib_ret_t) em_time_decode(const em_time_val *tv, em_parsed_time *pt)
{
    struct tm local_time;

    EM_CHECK_STACK();

#if defined(EM_HAS_LOCALTIME_R) && EM_HAS_LOCALTIME_R != 0
    localtime_r((time_t*)&tv->sec, &local_time);
#else
    /* localtime() is NOT thread-safe. */
    local_time = *localtime((time_t*)&tv->sec);
#endif

    pt->year = local_time.tm_year+1900;
    pt->mon = local_time.tm_mon;
    pt->day = local_time.tm_mday;
    pt->hour = local_time.tm_hour;
    pt->min = local_time.tm_min;
    pt->sec = local_time.tm_sec;
    pt->wday = local_time.tm_wday;
    pt->msec = tv->msec;

    return EM_SUCC;
}

/**
 * Encode parsed time to time value.
 */
EM_DEF(emlib_ret_t) em_time_encode(const em_parsed_time *pt, em_time_val *tv)
{
    struct tm local_time;

    local_time.tm_year = pt->year - 1900;
    local_time.tm_mon = pt->mon;
    local_time.tm_mday = pt->day;
    local_time.tm_hour = pt->hour;
    local_time.tm_min = pt->min;
    local_time.tm_sec = pt->sec;
    local_time.tm_isdst = 0;

    tv->sec = mktime(&local_time);
    tv->msec = pt->msec;

    return EM_SUCC;
}


/**
 * Convert local time to GMT.
 */
EM_DEF(emlib_ret_t) em_time_local_to_gmt(em_time_val *tv)
{
    EM_UNUSED_ARG(tv);
    return EM_EBUG;
}

/**
 * Convert GMT to local time.
 */
EM_DEF(emlib_ret_t) em_time_gmt_to_local(em_time_val *tv)
{
    EM_UNUSED_ARG(tv);
    return EM_EBUG;
}

EM_DEF(void) em_time_val_normalize(em_time_val *t)
{
    EM_CHECK_STACK();

    if (t->msec >= 1000) {
        t->sec += (t->msec / 1000);
        t->msec = (t->msec % 1000);
    } else if (t->msec <= -1000) {
        do {
            t->sec--;
            t->msec += 1000;
        } while (t->msec <= -1000);
    }

    if (t->sec >= 1 && t->msec < 0) {
        t->sec--;
        t->msec += 1000;

    } else if (t->sec < 0 && t->msec > 0) {
        t->sec++;
        t->msec -= 1000;
    }
}
