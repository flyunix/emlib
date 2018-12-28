/*
 * File:    string.h
 * Author:  Liu HongLiang<lhl_nciae@sina.cn>
 * Brief:   Provides string manipulation functions found in ANSI string.h.
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

#ifndef __EM_COMPAT_STRING_H__
#define __EM_COMPAT_STRING_H__


//#if defined(EM_HAS_STRING_H) && PJ_HAS_STRING_H != 0
#   include <string.h>
//#endif

/* For sprintf family */
#include <stdio.h>

#define em_ansi_strcmp		strcmp
#define em_ansi_strncmp		strncmp
#define em_ansi_strlen		strlen
#define em_ansi_strcpy		strcpy
#define em_ansi_strncpy		strncpy
#define em_ansi_strcat		strcat
#define em_ansi_strstr		strstr
#define em_ansi_strchr		strchr
#define em_ansi_strcasecmp	strcasecmp
#define em_ansi_stricmp		strcasecmp
#define em_ansi_strncasecmp	strncasecmp
#define em_ansi_strnicmp	strncasecmp
#define em_ansi_sprintf		sprintf

#define em_ansi_snprintf	snprintf
#define em_ansi_vsprintf	vsprintf
#define em_ansi_vsnprintf	vsnprintf

#endif/*__EM_COMPAT_STRING_H__*/
