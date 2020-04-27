/*
 * File:    config.c
 * Author:  Liu HongLiang<lhl_nciae@sina.cn>
 * Brief:   Emlib config.
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

#include "em/config.h"

//static const char *id = "config.c";

#define EM_MAKE_VERSION_1(a,b,c,d) 	#a "." #b "." #c d
#define EM_MAKE_VERSION_2(a,b,c,d) 	EM_MAKE_VERSION_1(a, b, c, d)

EM_DEF_DATA(const char*) EM_VERSION = EM_MAKE_VERSION_2(EM_VERSION_NUM_MAJOR,
						         EM_VERSION_NUM_MINOR,
						         EM_VERSION_NUM_REV,
						         EM_VERSION_NUM_EXTRA);

/*
 * Get EMLIB version string.
 */
EM_DEF(const char*) em_get_version(void)
{
    return EM_VERSION;
}

