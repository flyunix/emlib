/*
 * File:    string.h
 * Author:  Liu HongLiang<lhl_nciae@sina.cn>
 * Brief:   C type helper macros. 
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

#ifndef __EM_CTYPE_H__
#define __EM_CTYPE_H__

#include "types.h"
#include "compat/ctype.h"

DECLS_BEGIN

/**
 * @defgroup em_ctype ctype - Character Type
 * @ingroup EM_MISC
 * @{
 *
 * This module contains several inline functions/macros for testing or
 * manipulating character types. It is provided in EMLIB because EMLIB
 * must not depend to LIBC.
 */

/** 
 * Returns a non-zero value if either isalpha or isdigit is true for c.
 * @param c     The integer character to test.
 * @return      Non-zero value if either isalpha or isdigit is true for c.
 */
EM_INLINE(int) em_isalnum(unsigned char c) { return isalnum(c); }

/** 
 * Returns a non-zero value if c is a particular representation of an 
 * alphabetic character.
 * @param c     The integer character to test.
 * @return      Non-zero value if c is a particular representation of an 
 *              alphabetic character.
 */
EM_INLINE(int) em_isalpha(unsigned char c) { return isalpha(c); }

/** 
 * Returns a non-zero value if c is a particular representation of an 
 * ASCII character.
 * @param c     The integer character to test.
 * @return      Non-zero value if c is a particular representation of 
 *              an ASCII character.
 */
EM_INLINE(int) em_isascii(unsigned char c) { return c<128; }

/** 
 * Returns a non-zero value if c is a particular representation of 
 * a decimal-digit character.
 * @param c     The integer character to test.
 * @return      Non-zero value if c is a particular representation of 
 *              a decimal-digit character.
 */
EM_INLINE(int) em_isdigit(unsigned char c) { return isdigit(c); }

/** 
 * Returns a non-zero value if c is a particular representation of 
 * a space character (0x09 - 0x0D or 0x20).
 * @param c     The integer character to test.
 * @return      Non-zero value if c is a particular representation of 
 *              a space character (0x09 - 0x0D or 0x20).
 */
EM_INLINE(int) em_isspace(unsigned char c) { return isspace(c); }

/** 
 * Returns a non-zero value if c is a particular representation of 
 * a lowercase character.
 * @param c     The integer character to test.
 * @return      Non-zero value if c is a particular representation of 
 *              a lowercase character.
 */
EM_INLINE(int) em_islower(unsigned char c) { return islower(c); }


/** 
 * Returns a non-zero value if c is a particular representation of 
 * a uppercase character.
 * @param c     The integer character to test.
 * @return      Non-zero value if c is a particular representation of 
 *              a uppercase character.
 */
EM_INLINE(int) em_isupper(unsigned char c) { return isupper(c); }

/**
 * Returns a non-zero value if c is a either a space (' ') or horizontal
 * tab ('\\t') character.
 * @param c     The integer character to test.
 * @return      Non-zero value if c is a either a space (' ') or horizontal
 *              tab ('\\t') character.
 */
EM_INLINE(int) em_isblank(unsigned char c) { return (c==' ' || c=='\t'); }

/**
 * Converts character to lowercase.
 * @param c     The integer character to convert.
 * @return      Lowercase character of c.
 */
EM_INLINE(int) em_tolower(unsigned char c) { return tolower(c); }

/**
 * Converts character to uppercase.
 * @param c     The integer character to convert.
 * @return      Uppercase character of c.
 */
EM_INLINE(int) em_toupper(unsigned char c) { return toupper(c); }

/**
 * Returns a non-zero value if c is a particular representation of 
 * an hexadecimal digit character.
 * @param c     The integer character to test.
 * @return      Non-zero value if c is a particular representation of 
 *              an hexadecimal digit character.
 */
EM_INLINE(int) em_isxdigit(unsigned char c){ return isxdigit(c); }

/**
 * Array of hex digits, in lowerspace.
 */
/*extern char em_hex_digits[];*/
#define em_hex_digits	"0123456789abcdef"

/**
 * Convert a value to hex representation.
 * @param value	    Integral value to convert.
 * @param p	    Buffer to hold the hex representation, which must be
 *		    at least two bytes length.
 */
EM_INLINE(void) em_val_to_hex_digit(unsigned value, char *p)
{
    *p++ = em_hex_digits[ (value & 0xF0) >> 4 ];
    *p   = em_hex_digits[ (value & 0x0F) ];
}

/**
 * Convert hex digit c to integral value.
 * @param c	The hex digit character.
 * @return	The integral value between 0 and 15.
 */
EM_INLINE(unsigned) em_hex_digit_to_val(unsigned char c)
{
    if (c <= '9')
	return (c-'0') & 0x0F;
    else if (c <= 'F')
	return  (c-'A'+10) & 0x0F;
    else
	return (c-'a'+10) & 0x0F;
}

DECLS_END

#endif/*__EM_CTYPE_H__*/
