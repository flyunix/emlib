/*
 * File:    unicode.h
 * Author:  Liu HongLiang<lhl_nciae@sina.cn>
 * Brief:   Provides Unicode conversion for Unicode OSes 
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
 * 2019-01-29 Liu HongLiang<lhl_nciae@sina.cn> created.
 *
 */
#ifndef __EM_UNICODE_H__
#define __EM_UNICODE_H__

#include <em/types.h>


/**
 * @defgroup EM_UNICODE Unicode Support
 * @ingroup EM_MISC
 * @{
 */

DECLS_BEGIN

/**
 * @file unicode.h
 * @brief Provides Unicode conversion for Unicode OSes
 */

/**
 * Convert ANSI strings to Unicode strings.
 *
 * @param str		    The ANSI string to be converted.
 * @param len		    The length of the input string.
 * @param wbuf		    Buffer to hold the Unicode string output.
 * @param wbuf_count	    Buffer size, in number of elements (not bytes).
 *
 * @return		    The Unicode string, NULL terminated.
 */
EM_DECL(wchar_t*) em_ansi_to_unicode(const char *str, int len,
        wchar_t *wbuf, int wbuf_count);


/**
 * Convert Unicode string to ANSI string.
 *
 * @param wstr		    The Unicode string to be converted.
 * @param len		    The length of the input string.
 * @param buf		    Buffer to hold the ANSI string output.
 * @param buf_size	    Size of the output buffer.
 *
 * @return		    The ANSI string, NULL terminated.
 */
EM_DECL(char*) em_unicode_to_ansi(const wchar_t *wstr, em_ssize_t len,
        char *buf, int buf_size);


#if defined(EM_NATIVE_STRING_IS_UNICODE) && EM_NATIVE_STRING_IS_UNICODE!=0

/**
 * This macro is used to declare temporary Unicode buffer for ANSI to 
 * Unicode conversion, and should be put in declaration section of a block.
 * When EM_NATIVE_STRING_IS_UNICODE macro is not defined, this 
 * macro will expand to nothing.
 */
#   define EM_DECL_UNICODE_TEMP_BUF(buf,size)   wchar_t buf[size];

/**
 * This macro will convert ANSI string to native, when the platform's
 * native string is Unicode (EM_NATIVE_STRING_IS_UNICODE is non-zero).
 */
#   define EM_STRING_TO_NATIVE(s,buf,max)	em_ansi_to_unicode( \
        s, strlen(s), \
        buf, max)

/**
 * This macro is used to declare temporary ANSI buffer for Unicode to 
 * ANSI conversion, and should be put in declaration section of a block.
 * When EM_NATIVE_STRING_IS_UNICODE macro is not defined, this 
 * macro will expand to nothing.
 */
#   define EM_DECL_ANSI_TEMP_BUF(buf,size)	char buf[size];


/**
 * This macro will convert Unicode string to ANSI, when the platform's
 * native string is Unicode (EM_NATIVE_STRING_IS_UNICODE is non-zero).
 */
#   define EM_NATIVE_TO_STRING(cs,buf,max)	em_unicode_to_ansi( \
        cs, wcslen(cs), \
        buf, max)

#else

/**
 * This macro is used to declare temporary Unicode buffer for ANSI to 
 * Unicode conversion, and should be put in declaration section of a block.
 * When EM_NATIVE_STRING_IS_UNICODE macro is not defined, this 
 * macro will expand to nothing.
 */
#   define EM_DECL_UNICODE_TEMP_BUF(var,size)
/**
 * This macro will convert ANSI string to native, when the platform's
 * native string is Unicode (EM_NATIVE_STRING_IS_UNICODE is non-zero).
 */
#   define EM_STRING_TO_NATIVE(s,buf,max)	((char*)s)
/**
 * This macro is used to declare temporary ANSI buffer for Unicode to 
 * ANSI conversion, and should be put in declaration section of a block.
 * When EM_NATIVE_STRING_IS_UNICODE macro is not defined, this 
 * macro will expand to nothing.
 */
#   define EM_DECL_ANSI_TEMP_BUF(buf,size)
/**
 * This macro will convert Unicode string to ANSI, when the platform's
 * native string is Unicode (EM_NATIVE_STRING_IS_UNICODE is non-zero).
 */
#   define EM_NATIVE_TO_STRING(cs,buf,max)	((char*)(const char*)cs)

#endif


DECLS_END

/*
 * @}
 */


#endif	/* __EM_UNICODE_H__ */
