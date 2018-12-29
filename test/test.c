/*
 * File:    test.c
 * Author:  Liu HongLiang<lhl_nciae@sina.cn>
 * Brief:   TEST Module for emlib
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
 * 2018-12-24 Liu HongLiang<lhl_nciae@sina.cn> created.
 *
 */


#include "em/log.h"
#include "em/string.h"

#include "test.h"

#define DO_TEST(test) \
do  \
{\
    EM_LOG(EM_LOG_DEBUG, "%s TEST Start.", test.tc_name);\
    rc = (*(test.tc))(); \
    EM_LOG(EM_LOG_DEBUG, "%s TEST End, Result:%s(errno:%d).\n", test.tc_name, rc == 0 ? "OK": "FAIL", rc);\
    if(rc != EM_SUCC) { \
        goto test_over;\
    }\
}while(0)

#define BUILD_TC_CASE(f, t) \
{\
    .flag = f,\
    .tc_name = #t,\
    .tc = t\
}

#define TC_ENABLE  1
#define TC_DISABLE 0

struct test_case {
    int8        flag;//0:disable, 1:enable
    int8*       tc_name; 
    emlib_ret_t (*tc)(void);
} tc_array[] = {
    BUILD_TC_CASE(TC_ENABLE, pthread_mutex_test),
    BUILD_TC_CASE(TC_ENABLE, pthread_nest_mutex_test),
    BUILD_TC_CASE(TC_ENABLE, cstr_test),
    BUILD_TC_CASE(TC_ENABLE, string_test)
};

void test_main(void)
{
    int32 count = 0;
    int32 tc_cnt = EMLIB_ARRAY_SIZE(tc_array);

    emlib_ret_t rc = EM_SUCC;

    EM_LOG(EM_LOG_DEBUG, "Start Run TEST Cases ...\n");

    for (int32 i = 0; i < tc_cnt; i++) {
        if(tc_array[i].flag) {
            DO_TEST(tc_array[i]);
            count++;
        }
    }

#if 0
#ifdef  INCLUDE_PTHREAD_LOCK 
    DO_TEST(pthread_mutex_test());
#endif

#ifdef INCLUDE_PTHREAD_NEST_LOCK
    DO_TEST(pthread_nest_mutex_test());
#endif

#ifdef INCLUDE_CSTR
    DO_TEST(cstr_test());
#endif

#ifdef INCLUDE_STRING_TEST
    DO_TEST(string_test());
#endif
#endif

test_over:
    if(rc == 0) {
        EM_LOG(EM_LOG_DEBUG, "All TEST Cases Run OK!");
    } else {
        EM_LOG(EM_LOG_ERROR, "TEST Cases Run Failed, Pelase Check Errno.");
    }
}
