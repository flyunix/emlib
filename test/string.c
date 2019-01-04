/*
 * File:    string.c
 * Author:  Liu HongLiang<lhl_nciae@sina.cn>
 * Brief:   TEST Module for C String 
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
 * 2018-12-25 Liu HongLiang<lhl_nciae@sina.cn> created.
 *
 */

#include "em/string.h"
#include "em/pool.h"
#include "em/log.h"
#include "test.h"

/**
 * \page page_emlib_string_test Test: String
 *
 * This file provides implementation of \b string_test(). It tests the
 * functionality of the string API.
 *
 * \section sleep_test_sec Scope of the Test
 *
 * API tested:
 *  - em_str()
 *  - em_str_new()
 *  - em_strcmp()
 *  - em_strcmp2()
 *  - em_stricmp()
 *  - em_strlen()
 *  - em_strncmp()
 *  - em_strnicmp()
 *  - em_strchr()
 *  - em_strdup()
 *  - em_strdup2()
 *  - em_strcpy()
 *  - em_strcat()
 *  - em_strtrim()
 *  - em_utoa()
 *  - em_strtoul()
 *  - em_strtoul2()
 *  - em_create_random_string()
 *  - ... and mode..
 */

#if INCLUDE_STRING_TEST

#define HELLO_WORLD	"Hello World"
#define HELLO_WORLD_LEN	11
#define JUST_HELLO	"Hello"
#define JUST_HELLO_LEN	5
#define UL_VALUE	3456789012UL

#define SNULL 0

emlib_ret_t string_test(void)
{
    const em_str_t hello_world = { HELLO_WORLD, HELLO_WORLD_LEN };
    const em_str_t just_hello = { JUST_HELLO, JUST_HELLO_LEN };
    em_str_t s1, s2, *s3, s4, s5, *s6, *s7;
    enum { RCOUNT = 10, RLEN = 16 };
    em_str_t random[RCOUNT];
    em_pool_t *pool;
    int i;

    pool = em_pool_create(mem, "string_test", 4096, 0, SNULL);
    if (!pool) return -5;

    /* 
     * em_str(), em_str_init(), em_strcmp(), em_stricmp(), em_strlen(), 
     * em_strncmp(), em_strchr() 
     */
    s1 = em_str(HELLO_WORLD);
    if (em_strcmp(&s1, &hello_world) != 0)
        return -10;
    if (em_stricmp(&s1, &hello_world) != 0)
        return -20;
    if (em_strcmp(&s1, &just_hello) <= 0)
        return -30;
    if (em_stricmp(&s1, &just_hello) <= 0)
        return -40;
    if (em_strlen(&s1) != strlen(HELLO_WORLD))
        return -50;
    if (em_strncmp(&s1, &hello_world, 5) != 0)
        return -60;
    if (em_strnicmp(&s1, &hello_world, 5) != 0)
        return -70;
    if (em_strchr(&s1, HELLO_WORLD[1]) != s1.ptr+1)
        return -80;

    s6 = em_str_new(pool, 256);
    if(em_strcpy(s6, &s1) == NULL) 
        return TERRNO();
    if(em_strcmp(s6, &hello_world) != 0) 
        return TERRNO();

    s7 = em_str_new(pool, s1.slen);
    if(em_strcpy(s7, &s1) == NULL)
        return TERRNO();
    if(em_strcmp(s6, &hello_world) != 0) 
        return TERRNO();

    /* 
     * em_strdup() 
     */
    if (!em_strdup(pool, &s2, &s1))
        return -100;
    if (em_strcmp(&s1, &s2) != 0)
        return -110;

    /* 
     * em_strcpy(), em_strcat() 
     */
    s3 = em_str_new(pool, 256);
    if(em_strcpy(s3, &s2) == NULL)
        return TERRNO();
    if(em_strcat(s3, &just_hello) == NULL)
        return TERRNO();

    if (em_strcmp2(s3, HELLO_WORLD JUST_HELLO) != 0)
        return -210;

    /* 
     * em_strdup2(), em_strtrim(). 
     */
    em_strdup2(pool, &s4, " " HELLO_WORLD "\t ");
    em_strtrim(&s4);
    if (em_strcmp2(&s4, HELLO_WORLD) != 0)
        return -250;

    /* 
     * em_utoa() 
     */
    s5.ptr = (char*) em_pool_alloc(pool, 16);
    if (!s5.ptr)
        return -270;
    s5.slen = em_utoa(UL_VALUE, s5.ptr);

    /* 
     * em_strtoul() 
     */
    if (em_strtoul(&s5) != UL_VALUE)
        return -280;

    /*
     * em_strtoul2()
     */
    s5 = em_str("123456");

    em_strtoul2(&s5, SNULL, 10);	/* Crash test */

    if (em_strtoul2(&s5, &s4, 10) != 123456UL)
        return -290;
    if (s4.slen != 0)
        return -291;
    if (em_strtoul2(&s5, &s4, 16) != 0x123456UL)
        return -292;

    s5 = em_str("0123ABCD");
    if (em_strtoul2(&s5, &s4, 10) != 123)
        return -293;
    if (s4.slen != 4)
        return -294;
    if (s4.ptr == SNULL || *s4.ptr != 'A')
        return -295;
    if (em_strtoul2(&s5, &s4, 16) != 0x123ABCDUL)
        return -296;
    if (s4.slen != 0)
        return -297;

#if 0
    /* 
     * pj_create_random_string() 
     * Check that no duplicate strings are returned.
     */
    for (i=0; i<RCOUNT; ++i) {
        int j;

        random[i].ptr = (char*) em_pool_alloc(pool, RLEN);
        if (!random[i].ptr)
            return -320;

        random[i].slen = RLEN;
        em_create_random_string(random[i].ptr, RLEN);

        for (j=0; j<i; ++j) {
            if (em_strcmp(&random[i], &random[j])==0)
                return -330;
        }
    }
#endif

    /* Done. */
    em_pool_release(pool);

    return 0;
}

#else
/* To prevent warning about "translation unit is empty"
 * when this test is disabled. 
 */
int dummy_string_test;
#endif	/* INCLUDE_STRING_TEST */

