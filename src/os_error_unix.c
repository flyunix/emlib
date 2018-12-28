/*
 * File:    os_error_unix.c
 * Author:  Liu HongLiang<lhl_nciae@sina.cn>
 * Brief:   Error fo Unix. 
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
 * 2018-12-25 Liu HongLiang<lhl_nciae@sina.cn> created.
 *
 */
#include "em/errno.h"
#include "em/string.h"

#include <errno.h>

EM_DEF(emlib_ret_t) em_get_os_error(void)
{
    return EM_STATUS_FROM_OS(errno);
}

EM_DEF(void) em_set_os_error(emlib_ret_t code)
{
    errno = EM_STATUS_TO_OS(code);
}

EM_DEF(emlib_ret_t) em_get_netos_error(void)
{
    return EM_STATUS_FROM_OS(errno);
}

EM_DEF(void) em_set_netos_error(emlib_ret_t code)
{
    errno = EM_STATUS_TO_OS(code);
}

/* 
 * platform_strerror()
 *
 * Platform specific error message. This file is called by em_strerror() 
 * in errno.c 
 */
int platform_strerror( em_os_err_type os_errcode, 
                       char *buf, em_size_t bufsize)
{
    const char *syserr = strerror(os_errcode);
    em_size_t len = syserr ? strlen(syserr) : 0;

    if (len >= bufsize) len = bufsize - 1;
    if (len > 0)
	em_memcpy(buf, syserr, len);
    buf[len] = '\0';
    return len;
}


