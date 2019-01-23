

/*
 * File:    cstr_test.c
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
 * ================================================================
 * 2018-12-25 Liu HongLiang<lhl_nciae@sina.cn> created.
 *
 */
#include <string.h>

#include "test.h"
#include "em/cstr.h"
#include "em/assert.h"

emlib_ret_t cstr_test(void)
{
    cstr _cs, *cs;
    cs = &_cs;

    int8 buff[1024];

    cstr_init_stack(cs, buff, sizeof(buff));

    //EM_LOG(EM_LOG_DEBUG, "test put int8.");
    int8 c;
    cstr_put_int8(cs, 32);
    cstr_get_int8(cs, &c);
    if(c != 32) {
        return TERRNO(); 
    }

    //printf("test put uint16.\n");
    uint16 s;
    cstr_put_int16(cs, 35745);
    cstr_get_uint16(cs, &s);
    if(s != 35745) {
        return TERRNO(); 
    }

    //printf("test put uint32.\n");
    uint32 i;
    cstr_put_int32(cs, 35712345);
    cstr_get_uint32(cs, &i);
    if(i != 35712345) {
        return TERRNO();
    }
    //printf("i = %u\n", i);

    //printf("test put string.\n");
    char *str = "cstr is C-String library.";
    char string[100] = {0};
    cstr_put_str(cs, str, strlen(str));
    cstr_get_str(cs, string, strlen(str));
    //printf("string = %s\n", string);
    if(strncmp(string, str, strlen(str)) != 0) {
        return TERRNO();
    }

    return 0;
}
