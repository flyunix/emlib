/*
 * File:    time.h
 * Author:  Liu HongLiang<lhl_nciae@sina.cn>
 * Brief:   Include OS time API declaration.
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
#ifndef __EM_COMPAT_TIME_H__
#define __EM_COMPAT_TIME_H__

/**
 * @file time.h
 * @brief Provides ftime() and localtime() etc functions.
 */

#if defined(EM_HAS_TIME_H) && EM_HAS_TIME_H != 0
#  include <time.h>
#endif

#if defined(EM_HAS_SYS_TIME_H) && EM_HAS_SYS_TIME_H != 0
#  include <sys/time.h>
#endif

#if defined(EM_HAS_SYS_TIMEB_H) && EM_HAS_SYS_TIMEB_H != 0
//#  include <sys/timeb.h>
#endif


#endif	/* __EM_COMPAT_TIME_H__ */

