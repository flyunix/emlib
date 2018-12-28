/*
 * File:    limits.h
 * Author:  Liu HongLiang<lhl_nciae@sina.cn>
 * Brief:  Common min and max values 
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
#ifndef __EM_LIMITS_H__
#define __EM_LIMITS_H__


//#include <pj/compat/limits.h>

#define LONG_MAX     2147483647L
#define LONG_MIN      (-LONG_MAX - 1L)
#define ULONG_MAX    4294967295UL

/** Maximum value for signed 32-bit integer. */
#define EM_MAXINT32	0x7fffffff

/** Minimum value for signed 32-bit integer. */
#define EM_MININT32	0x80000000

/** Maximum value for unsigned 16-bit integer. */
#define EM_MAXUINT16	0xffff

/** Maximum value for unsigned char. */
#define EM_MAXUINT8	0xff

/** Maximum value for long. */
#define EM_MAXLONG	LONG_MAX

/** Minimum value for long. */
#define EM_MINLONG	LONG_MIN

/** Minimum value for unsigned long. */
#define EM_MAXULONG	ULONG_MAX

#endif  /* __EM_LIMITS_H__ */
