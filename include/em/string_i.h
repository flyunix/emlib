/*
 * File:    string_i.h
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
#include "types.h"
#include "assert.h"
#include "pool.h"

EM_IDEF(em_str_t) em_str(char *str)
{
    em_str_t dst;
    dst.ptr = str;
    dst.slen = str ? em_ansi_strlen(str) : 0;
    dst.blen = 0;

    return dst;
}

EM_IDEF(em_str_t*) em_str_new(em_pool_t *pool,
        em_size_t blen)
{
    em_str_t *str = (em_str_t*)em_pool_alloc(pool, sizeof(em_str_t) + blen);

    if(str == NULL)
        return str;

    str->blen = blen;
    str->slen = 0; 
    str->ptr  = (char*)str + sizeof(em_str_t);

    return str;
}

EM_IDEF(em_str_t*) em_strdup(em_pool_t *pool,
        em_str_t *dst,
        const em_str_t *src)
{
    /* Without this, destination will be corrupted */
    if (dst == src)
        return dst;

    if (src->slen) {
        dst->ptr = (char*)em_pool_alloc(pool, src->slen);
        em_memcpy(dst->ptr, src->ptr, src->slen);
    }
    dst->slen = src->slen;
    return dst;
}
 
EM_IDEF(em_str_t*) em_strdup_with_null( em_pool_t *pool,
        em_str_t *dst,
        const em_str_t *src)
{
    dst->ptr = (char*)em_pool_alloc(pool, src->slen+1);
    if (src->slen) {
        em_memcpy(dst->ptr, src->ptr, src->slen);
    }
    dst->slen = src->slen;
    dst->ptr[dst->slen] = '\0';
    return dst;
}

EM_IDEF(em_str_t*) em_strdup2(em_pool_t *pool,
        em_str_t *dst,
        const char *src)
{
    dst->slen = src ? em_ansi_strlen(src) : 0;
    if (dst->slen) {
        dst->ptr = (char*)em_pool_alloc(pool, dst->slen);
        em_memcpy(dst->ptr, src, dst->slen);
    } else {
        dst->ptr = NULL;
    }
    return dst;
}

EM_IDEF(em_str_t*) em_strdup2_with_null( em_pool_t *pool,
        em_str_t *dst,
        const char *src)
{
    dst->slen = src ? em_ansi_strlen(src) : 0;
    dst->ptr = (char*)em_pool_alloc(pool, dst->slen+1);
    if (dst->slen) {
        em_memcpy(dst->ptr, src, dst->slen);
    }
    dst->ptr[dst->slen] = '\0';
    return dst;
}

EM_IDEF(em_str_t) em_strdup3(em_pool_t *pool, const char *src)
{
    em_str_t temp;
    em_strdup2(pool, &temp, src);
    return temp;
}

EM_IDEF(em_str_t*) em_strcpy(em_str_t *dst, const em_str_t *src)
{
    EMLIB_ASSERT_RETURN(dst->blen >= src->slen, NULL);

    dst->slen = src->slen;
    if (src->slen > 0)
        em_memcpy(dst->ptr, src->ptr, src->slen);
    return dst;
}

EM_IDEF(em_str_t*) em_strcpy2(em_str_t *dst, const char *src)
{
    em_ssize_t len = src ? em_ansi_strlen(src) : 0;

    EMLIB_ASSERT_RETURN(dst->blen >= len, NULL);

    if (len > 0) {
        em_memcpy(dst->ptr, src, dst->slen);
        dst->slen = len;
    }

    return dst;
}

EM_IDEF(em_str_t*) em_strncpy( em_str_t *dst, const em_str_t *src, 
        em_ssize_t max)
{
    EMLIB_ASSERT_RETURN(max >= 0, NULL);
    if (max > src->slen) max = src->slen;
    EMLIB_ASSERT_RETURN(dst->blen >= max, NULL);
    if (max > 0)
        em_memcpy(dst->ptr, src->ptr, max);
    dst->slen = max;
    return dst;
}

EM_IDEF(em_str_t*) em_strncpy_with_null( em_str_t *dst, const em_str_t *src,
        em_ssize_t max)
{
    EMLIB_ASSERT_RETURN(max >= 0, NULL);

    if (max <= src->slen)
        max = max-1;
    else
        max = src->slen;

    EMLIB_ASSERT_RETURN(dst->blen >= max, NULL);

    em_memcpy(dst->ptr, src->ptr, max);
    dst->ptr[max] = '\0';
    dst->slen = max;
    return dst;
}


EM_IDEF(int) em_strcmp( const em_str_t *str1, const em_str_t *str2)
{
    if (str1->slen == 0) {
        return str2->slen==0 ? 0 : -1;
    } else if (str2->slen == 0) {
        return 1;
    } else {
        em_size_t min = (str1->slen < str2->slen)? str1->slen : str2->slen;
        int res = em_memcmp(str1->ptr, str2->ptr, min);
        if (res == 0) {
            return (str1->slen < str2->slen) ? -1 :
                (str1->slen == str2->slen ? 0 : 1);
        } else {
            return res;
        }
    }
}

EM_IDEF(int) em_strncmp( const em_str_t *str1, const em_str_t *str2, 
        em_size_t len)
{
    em_str_t copy1, copy2;

    if (len < (unsigned)str1->slen) {
        copy1.ptr = str1->ptr;
        copy1.slen = len;
        str1 = &copy1;
    }

    if (len < (unsigned)str2->slen) {
        copy2.ptr = str2->ptr;
        copy2.slen = len;
        str2 = &copy2;
    }

    return em_strcmp(str1, str2);
}

EM_IDEF(int) em_strncmp2( const em_str_t *str1, const char *str2, 
        em_size_t len)
{
    em_str_t copy2;

    if (str2) {
        copy2.ptr = (char*)str2;
        copy2.slen = em_ansi_strlen(str2);
    } else {
        copy2.slen = 0;
    }

    return em_strncmp(str1, &copy2, len);
}

EM_IDEF(int) em_strcmp2( const em_str_t *str1, const char *str2 )
{
    em_str_t copy2;

    if (str2) {
        copy2.ptr = (char*)str2;
        copy2.slen = em_ansi_strlen(str2);
    } else {
        copy2.ptr = NULL;
        copy2.slen = 0;
    }

    return em_strcmp(str1, &copy2);
}

EM_IDEF(int) em_stricmp( const em_str_t *str1, const em_str_t *str2)
{
    if (str1->slen == 0) {
        return str2->slen==0 ? 0 : -1;
    } else if (str2->slen == 0) {
        return 1;
    } else {
        em_size_t min = (str1->slen < str2->slen)? str1->slen : str2->slen;
        int res = em_ansi_strnicmp(str1->ptr, str2->ptr, min);
        if (res == 0) {
            return (str1->slen < str2->slen) ? -1 :
                (str1->slen == str2->slen ? 0 : 1);
        } else {
            return res;
        }
    }
}

#if defined(EM_HAS_STRICMP_ALNUM) && EM_HAS_STRICMP_ALNUM!=0
EM_IDEF(int) strnicmp_alnum( const char *str1, const char *str2,
        int len)
{
    if (len==0)
        return 0;
    else {
        register const uint32 *p1 = (uint32*)str1, 
                 *p2 = (uint32*)str2;
        while (len > 3 && (*p1 & 0x5F5F5F5F)==(*p2 & 0x5F5F5F5F))
            ++p1, ++p2, len-=4;

        if (len > 3)
            return -1;
#if defined(EM_IS_LITTLE_ENDIAN) && EM_IS_LITTLE_ENDIAN!=0
        else if (len==3)
            return ((*p1 & 0x005F5F5F)==(*p2 & 0x005F5F5F)) ? 0 : -1;
        else if (len==2)
            return ((*p1 & 0x00005F5F)==(*p2 & 0x00005F5F)) ? 0 : -1;
        else if (len==1)
            return ((*p1 & 0x0000005F)==(*p2 & 0x0000005F)) ? 0 : -1;
#else
        else if (len==3)
            return ((*p1 & 0x5F5F5F00)==(*p2 & 0x5F5F5F00)) ? 0 : -1;
        else if (len==2)
            return ((*p1 & 0x5F5F0000)==(*p2 & 0x5F5F0000)) ? 0 : -1;
        else if (len==1)
            return ((*p1 & 0x5F000000)==(*p2 & 0x5F000000)) ? 0 : -1;
#endif
        else 
            return 0;
    }
}

EM_IDEF(int) em_stricmp_alnum(const em_str_t *str1, const em_str_t *str2)
{
    register int len = str1->slen;

    if (len != str2->slen) {
        return (len < str2->slen) ? -1 : 1;
    } else if (len == 0) {
        return 0;
    } else {
        register const uint32 *p1 = (uint32*)str1->ptr, 
                 *p2 = (uint32*)str2->ptr;
        while (len > 3 && (*p1 & 0x5F5F5F5F)==(*p2 & 0x5F5F5F5F))
            ++p1, ++p2, len-=4;

        if (len > 3)
            return -1;
#if defined(EM_IS_LITTLE_ENDIAN) && EM_IS_LITTLE_ENDIAN!=0
        else if (len==3)
            return ((*p1 & 0x005F5F5F)==(*p2 & 0x005F5F5F)) ? 0 : -1;
        else if (len==2)
            return ((*p1 & 0x00005F5F)==(*p2 & 0x00005F5F)) ? 0 : -1;
        else if (len==1)
            return ((*p1 & 0x0000005F)==(*p2 & 0x0000005F)) ? 0 : -1;
#else
        else if (len==3)
            return ((*p1 & 0x5F5F5F00)==(*p2 & 0x5F5F5F00)) ? 0 : -1;
        else if (len==2)
            return ((*p1 & 0x5F5F0000)==(*p2 & 0x5F5F0000)) ? 0 : -1;
        else if (len==1)
            return ((*p1 & 0x5F000000)==(*p2 & 0x5F000000)) ? 0 : -1;
#endif
        else 
            return 0;
    }
}
#endif	/* EM_HAS_STRICMP_ALNUM */

EM_IDEF(int) em_stricmp2( const em_str_t *str1, const char *str2)
{
    em_str_t copy2;

    if (str2) {
        copy2.ptr = (char*)str2;
        copy2.slen = em_ansi_strlen(str2);
    } else {
        copy2.ptr = NULL;
        copy2.slen = 0;
    }

    return em_stricmp(str1, &copy2);
}

EM_IDEF(int) em_strnicmp( const em_str_t *str1, const em_str_t *str2, 
        em_size_t len)
{
    em_str_t copy1, copy2;

    if (len < (unsigned)str1->slen) {
        copy1.ptr = str1->ptr;
        copy1.slen = len;
        str1 = &copy1;
    }

    if (len < (unsigned)str2->slen) {
        copy2.ptr = str2->ptr;
        copy2.slen = len;
        str2 = &copy2;
    }

    return em_stricmp(str1, str2);
}

EM_IDEF(int) em_strnicmp2( const em_str_t *str1, const char *str2, 
        em_size_t len)
{
    em_str_t copy2;

    if (str2) {
        copy2.ptr = (char*)str2;
        copy2.slen = em_ansi_strlen(str2);
    } else {
        copy2.slen = 0;
    }

    return em_strnicmp(str1, &copy2, len);
}

EM_IDEF(em_str_t*) em_strcat(em_str_t *dst, const em_str_t *src)
{
    EMLIB_ASSERT_RETURN(dst->blen >= (dst->slen + src->slen), NULL);

    if (src->slen) {
        em_memcpy(dst->ptr + dst->slen, src->ptr, src->slen);
        dst->slen += src->slen;
    }
}

EM_IDEF(void) em_strcat2(em_str_t *dst, const char *str)
{
    em_size_t len = str? em_ansi_strlen(str) : 0;
    EMLIB_ASSERT(dst->blen >= (dst->slen + len));

    if (len) {
        em_memcpy(dst->ptr + dst->slen, str, len);
        dst->slen += len;
    }
}

EM_IDEF(em_str_t*) em_strtrim( em_str_t *str )
{
    em_strltrim(str);
    em_strrtrim(str);
    return str;
}

