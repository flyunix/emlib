/*
 * File:    ctype.h
 * Author:  Liu HongLiang<lhl_nciae@sina.cn>
 * Brief:   Provides ctype function family. 
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
 * 2018-12-27 Liu HongLiang<lhl_nciae@sina.cn> created.
 *
 */
#ifndef __EM_COMPAT_ERRNO_H__
#define __EM_COMPAT_ERRNO_H__

#if defined(EM_HAS_ERRNO_VAR) && EM_HAS_ERRNO_VAR!= 0
    typedef int em_os_err_type;
#   define em_get_native_os_error()	    (errno)
#   define em_get_native_netos_error()	    (errno)
#else
#   error "Please define how to get errno for this platform here!"
#endif

#endif	/* __EM_COMPAT_ERRNO_H__ */

