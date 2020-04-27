/*
 * File:    log.h
 * Author:  Liu HongLiang<lhl_nciae@sina.cn>
 * Brief:   log manage.
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
 * 2018-12-20 Liu HongLiang<lhl_nciae@sina.cn> created.
 *
 */

#ifndef _LOG_H_
#define _LOG_H_

#include "types.h"

DECLS_BEGIN

typedef enum _LOG_LEVEL {
    EM_LOG_FATAL = 0,
    EM_LOG_ERROR,
    EM_LOG_WARN,
    EM_LOG_INFO,
    EM_LOG_DEBUG,
    EM_LOG_TRACE
}EM_LOG_LEVEL;

extern void _em_log(const char *func, int line, int level, const char * module, const char *fmt, ...);
extern void v_em_log(const char *func, int line, int level, const char *module, const char *fmt, va_list args);

#define EM_LOG(level, fmt, args...) EM_LOG_MOD(level, module, fmt, ##args)

#define EM_LOG_MOD(level, module, fmt, args...) \
    do{\
        _em_log(__func__, __LINE__, level, module, fmt, ##args);\
    }while(0)

void em_log_set_log_level(EM_LOG_LEVEL log_level);

void app_perror(const char *msg, emlib_ret_t rc);

void printx(int8 *data, uint32 data_len);

DECLS_END

#endif /*_LOG_H_*/
