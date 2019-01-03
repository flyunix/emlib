/*
 * File:    log.c
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
 * 2018-12-21 Liu HongLiang<lhl_nciae@sina.cn> created.
 *
 */

#ifndef __LOG_H__
#define __LOG_H__

#include "em/log.h"
#include "em/assert.h"

#include <stdarg.h>

static const char *ltexts[] = { "FATAL", "ERROR", " WARN", 
    " INFO", "DEBUG", "TRACE"};

static EM_LOG_LEVEL _log_level = EM_LOG_DEBUG;

void _em_log(const char *func, int line, int level, const char *module, const char *fmt, ...)
{
    return_if_fail((func != NULL) && (fmt != NULL))

    if(level <= _log_level) {
        va_list list;
        va_start(list, fmt);
        printf("[%-s] %-s:%-s:%-d ", ltexts[level], module, func, line);
        vprintf(fmt, list);
        printf("\n");
        va_end(list);
    }
}

void em_log_set_log_level(EM_LOG_LEVEL log_level)
{
    _log_level = log_level;
}

#endif
