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
#ifndef __EM_COMPAT_CTYPE_H__
#define __EM_COMPAT_CTYPE_H__

#if defined(PJ_HAS_CTYPE_H) && PJ_HAS_CTYPE_H != 0
#  include <ctype.h>
#else
#  define isalnum(c)	    (isalpha(c) || isdigit(c))
#  define isalpha(c)	    (islower(c) || isupper(c))
#  define isascii(c)	    (((unsigned char)(c))<=0x7f)
#  define isdigit(c)	    ((c)>='0' && (c)<='9')
#  define isspace(c)	    ((c)==' '  || (c)=='\t' ||\
			     (c)=='\n' || (c)=='\r' || (c)=='\v')
#  define islower(c)	    ((c)>='a' && (c)<='z')
#  define isupper(c)	    ((c)>='A' && (c)<='Z')
#  define isxdigit(c)	    (isdigit(c) || (tolower(c)>='a'&&tolower(c)<='f'))
#  define tolower(c)	    (((c) >= 'A' && (c) <= 'Z') ? (c)+('a'-'A') : (c))
#  define toupper(c)	    (((c) >= 'a' && (c) <= 'z') ? (c)-('a'-'A') : (c))
#endif


#endif	/* __PJ_COMPAT_CTYPE_H__ */
