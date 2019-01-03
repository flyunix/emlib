/*
 * File:    string.c
 * Author:  Liu HongLiang<lhl_nciae@sina.cn>
 * Brief:   emlib String instand of C String.
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
#include "em/string.h"
#include "em/errno.h"
#include "em/assert.h"
#include "em/pool.h"
#include "em/ctype.h"
#include "em/limits.h"

static const char * module = "STRING";

#if EM_FUNCTIONS_ARE_INLINED==0
#  include "em/string_i.h"
#endif


EM_DEF(em_ssize_t) em_strspn(const em_str_t *str, const em_str_t *set_char)
{
    em_ssize_t i, j, count = 0;
    for (i = 0; i < str->slen; i++) {
        if (count != i) 
            break;

        for (j = 0; j < set_char->slen; j++) {
            if (str->ptr[i] == set_char->ptr[j])
                count++;
        }
    }
    return count;
}


EM_DEF(em_ssize_t) em_strspn2(const em_str_t *str, const char *set_char)
{
    em_ssize_t i, j, count = 0;
    for (i = 0; i < str->slen; i++) {
        if (count != i)
            break;

        for (j = 0; set_char[j] != 0; j++) {
            if (str->ptr[i] == set_char[j])
                count++;
        }
    }
    return count;
}


EM_DEF(em_ssize_t) em_strcspn(const em_str_t *str, const em_str_t *set_char)
{
    em_ssize_t i, j;
    for (i = 0; i < str->slen; i++) {
        for (j = 0; j < set_char->slen; j++) {
            if (str->ptr[i] == set_char->ptr[j])
                return i;
        }
    }
    return i;
}


EM_DEF(em_ssize_t) em_strcspn2(const em_str_t *str, const char *set_char)
{
    em_ssize_t i, j;
    for (i = 0; i < str->slen; i++) {
        for (j = 0; set_char[j] != 0; j++) {
            if (str->ptr[i] == set_char[j])
                return i;
        }
    }
    return i;
}


EM_DEF(em_ssize_t) em_strtok(const em_str_t *str, const em_str_t *delim,
        em_str_t *tok, em_size_t start_idx)
{    
    em_ssize_t str_idx;

    tok->slen = 0;
    if ((str->slen == 0) || ((em_size_t)str->slen < start_idx)) {
        return str->slen;
    }

    tok->ptr = str->ptr + start_idx;
    tok->slen = str->slen - start_idx;

    str_idx = em_strspn(tok, delim);
    if (start_idx+str_idx == (em_size_t)str->slen) {
        return str->slen;
    }    
    tok->ptr += str_idx;
    tok->slen -= str_idx;

    tok->slen = em_strcspn(tok, delim);
    return start_idx + str_idx;
}


EM_DEF(em_ssize_t) em_strtok2(const em_str_t *str, const char *delim,
        em_str_t *tok, em_size_t start_idx)
{
    em_ssize_t str_idx;

    tok->slen = 0;
    if ((str->slen == 0) || ((em_size_t)str->slen < start_idx)) {
        return str->slen;
    }

    tok->ptr = str->ptr + start_idx;
    tok->slen = str->slen - start_idx;

    str_idx = em_strspn2(tok, delim);
    if (start_idx + str_idx == (em_size_t)str->slen) {
        return str->slen;
    }
    tok->ptr += str_idx;
    tok->slen -= str_idx;

    tok->slen = em_strcspn2(tok, delim);
    return start_idx + str_idx;
}


EM_DEF(char*) em_strstr(const em_str_t *str, const em_str_t *substr)
{
    const char *s, *ends;

    /* Special case when substr is zero */
    if (substr->slen == 0) {
        return (char*)str->ptr;
    }

    s = str->ptr;
    ends = str->ptr + str->slen - substr->slen;
    for (; s<=ends; ++s) {
        if (em_ansi_strncmp(s, substr->ptr, substr->slen)==0)
            return (char*)s;
    }
    return NULL;
}


EM_DEF(char*) em_stristr(const em_str_t *str, const em_str_t *substr)
{
    const char *s, *ends;

    /* Special case when substr is zero */
    if (substr->slen == 0) {
        return (char*)str->ptr;
    }

    s = str->ptr;
    ends = str->ptr + str->slen - substr->slen;
    for (; s<=ends; ++s) {
        if (em_ansi_strnicmp(s, substr->ptr, substr->slen)==0)
            return (char*)s;
    }
    return NULL;
}


EM_DEF(em_str_t*) em_strltrim( em_str_t *str )
{
    char *end = str->ptr + str->slen;
    register char *p = str->ptr;
    while (p < end && em_isspace(*p))
        ++p;
    str->slen -= (p - str->ptr);
    str->ptr = p;
    return str;
}

EM_DEF(em_str_t*) em_strrtrim( em_str_t *str )
{
    char *end = str->ptr + str->slen;
    register char *p = end - 1;
    while (p >= str->ptr && em_isspace(*p))
        --p;
    str->slen -= ((end - p) - 1);
    return str;
}

#if 0
EM_DEF(char*) em_create_random_string(char *str, em_size_t len)
{
    unsigned i;
    char *p = str;

    //EM_CHECK_STACK();

    for (i=0; i<len/8; ++i) {
        em_uint32_t val = em_rand();
        em_val_to_hex_digit( (val & 0xFF000000) >> 24, p+0 );
        em_val_to_hex_digit( (val & 0x00FF0000) >> 16, p+2 );
        em_val_to_hex_digit( (val & 0x0000FF00) >>  8, p+4 );
        em_val_to_hex_digit( (val & 0x000000FF) >>  0, p+6 );
        p += 8;
    }
    for (i=i * 8; i<len; ++i) {
        *p++ = em_hex_digits[ em_rand() & 0x0F ];
    }
    return str;
}
#endif

EM_DEF(long) em_strtol(const em_str_t *str)
{
    //EM_CHECK_STACK();

    if (str->slen > 0 && (str->ptr[0] == '+' || str->ptr[0] == '-')) {
        em_str_t s;

        s.ptr = str->ptr + 1;
        s.slen = str->slen - 1;
        return (str->ptr[0] == '-'? -(long)em_strtoul(&s) : em_strtoul(&s));
    } else
        return em_strtoul(str);
}


EM_DEF(emlib_ret_t) em_strtol2(const em_str_t *str, long *value)
{
    em_str_t s;
    unsigned long retval = 0;
    em_bool_t is_negative = EM_FALSE;
    int rc = 0;

    //EM_CHECK_STACK();

    if (!str || !value) {
        return EM_EINVAL;
    }

    s = *str;
    em_strltrim(&s);

    if (s.slen == 0)
        return EM_EINVAL;

    if (s.ptr[0] == '+' || s.ptr[0] == '-') {
        is_negative = (s.ptr[0] == '-');
        s.ptr += 1;
        s.slen -= 1;
    }

    rc = em_strtoul3(&s, &retval, 10);
    if (rc == EM_EINVAL) {
        return rc;
    } else if (rc != EM_SUCC) {
        *value = is_negative ? EM_MINLONG : EM_MAXLONG;
        return is_negative ? EM_ETOOSMALL : EM_ETOOBIG;
    }

    if (retval > EM_MAXLONG && !is_negative) {
        *value = EM_MAXLONG;
        return EM_ETOOBIG;
    }

    if (retval > (EM_MAXLONG + 1UL) && is_negative) {
        *value = EM_MINLONG;
        return EM_ETOOSMALL;
    }

    *value = is_negative ? -(long)retval : retval;

    return EM_SUCC;
}

EM_DEF(unsigned long) em_strtoul(const em_str_t *str)
{
    unsigned long value;
    unsigned i;

    //EM_CHECK_STACK();

    value = 0;
    for (i=0; i<(unsigned)str->slen; ++i) {
        if (!em_isdigit(str->ptr[i]))
            break;
        value = value * 10 + (str->ptr[i] - '0');
    }
    return value;
}

EM_DEF(unsigned long) em_strtoul2(const em_str_t *str, em_str_t *endptr,
        unsigned base)
{
    unsigned long value;
    unsigned i;

    //EM_CHECK_STACK();

    value = 0;
    if (base <= 10) {
        for (i=0; i<(unsigned)str->slen; ++i) {
            unsigned c = (str->ptr[i] - '0');
            if (c >= base)
                break;
            value = value * base + c;
        }
    } else if (base == 16) {
        for (i=0; i<(unsigned)str->slen; ++i) {
            if (!em_isxdigit(str->ptr[i]))
                break;
            value = value * 16 + em_hex_digit_to_val(str->ptr[i]);
        }
    } else {
        em_assert(!"Unsupported base");
        i = 0;
        value = 0xFFFFFFFFUL;
    }

    if (endptr) {
        endptr->ptr = str->ptr + i;
        endptr->slen = str->slen - i;
    }

    return value;
}

EM_DEF(emlib_ret_t) em_strtoul3(const em_str_t *str, unsigned long *value,
        unsigned base)
{
    em_str_t s;
    unsigned i;

    //EM_CHECK_STACK();

    if (!str || !value) {
        return EM_EINVAL;
    }

    s = *str;
    em_strltrim(&s);

    if (s.slen == 0 || s.ptr[0] < '0' ||
            (base <= 10 && (unsigned)s.ptr[0] > ('0' - 1) + base) ||
            (base == 16 && !em_isxdigit(s.ptr[0])))
    {
        return EM_EINVAL;
    }

    *value = 0;
    if (base <= 10) {
        for (i=0; i<(unsigned)s.slen; ++i) {
            unsigned c = s.ptr[i] - '0';
            if (s.ptr[i] < '0' || (unsigned)s.ptr[i] > ('0' - 1) + base) {
                break;
            }
            if (*value > EM_MAXULONG / base) {
                *value = EM_MAXULONG;
                return EM_ETOOBIG;
            }

            *value *= base;
            if ((EM_MAXULONG - *value) < c) {
                *value = EM_MAXULONG;
                return EM_ETOOBIG;
            }
            *value += c;
        }
    } else if (base == 16) {
        for (i=0; i<(unsigned)s.slen; ++i) {
            unsigned c = em_hex_digit_to_val(s.ptr[i]);
            if (!em_isxdigit(s.ptr[i]))
                break;

            if (*value > EM_MAXULONG / base) {
                *value = EM_MAXULONG;
                return EM_ETOOBIG;
            }
            *value *= base;
            if ((EM_MAXULONG - *value) < c) {
                *value = EM_MAXULONG;
                return EM_ETOOBIG;
            }
            *value += c;
        }
    } else {
        em_assert(!"Unsupported base");
        return EM_EINVAL;
    }
    return EM_SUCC;
}

EM_DEF(float) em_strtof(const em_str_t *str)
{
    em_str_t part;
    char *pdot;
    float val;

    if (str->slen == 0)
        return 0;

    pdot = (char*)em_memchr(str->ptr, '.', str->slen);
    part.ptr = str->ptr;
    part.slen = pdot ? pdot - str->ptr : str->slen;

    if (part.slen)
        val = (float)em_strtol(&part);
    else
        val = 0;

    if (pdot) {
        part.ptr = pdot + 1;
        part.slen = (str->ptr + str->slen - pdot - 1);
        if (part.slen) {
            em_str_t endptr;
            float fpart, fdiv;
            int i;
            fpart = (float)em_strtoul2(&part, &endptr, 10);
            fdiv = 1.0;
            for (i=0; i<(part.slen - endptr.slen); ++i)
                fdiv = fdiv * 10;
            if (val >= 0)
                val += (fpart / fdiv);
            else
                val -= (fpart / fdiv);
        }
    }
    return val;
}

EM_DEF(int) em_utoa(unsigned long val, char *buf)
{
    return em_utoa_pad(val, buf, 0, 0);
}

EM_DEF(int) em_utoa_pad( unsigned long val, char *buf, int min_dig, int pad)
{
    char *p;
    int len;

    //EM_CHECK_STACK();

    p = buf;
    do {
        unsigned long digval = (unsigned long) (val % 10);
        val /= 10;
        *p++ = (char) (digval + '0');
    } while (val > 0);

    len = (int)(p-buf);
    while (len < min_dig) {
        *p++ = (char)pad;
        ++len;
    }
    *p-- = '\0';

    do {
        char temp = *p;
        *p = *buf;
        *buf = temp;
        --p;
        ++buf;
    } while (buf < p);

    return len;
}
