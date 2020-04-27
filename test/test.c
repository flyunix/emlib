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
 * but WITHOUT ANY WARRANTY; without even the implied warranty of * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

/*
 * History:
 * ================================================================
 * 2018-12-24 Liu HongLiang<lhl_nciae@sina.cn> created.
 *
 */

#include "em/os.h"
#include "em/log.h"
#include "em/errno.h"
#include "em/string.h"

#include "test.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>


static const char* module = "TEST_CASE";
#define START_LINE "\n-----------------------------------------------------------------------------------------------------\n"
#define END_LINE   "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"

#define DO_TEST(test) \
    do  \
{\
    fprintf(stdout, "%s", START_LINE);\
    fprintf(stdout, "%s Start ...\n", test.tc_name);\
    rc = (*(test.tc))(); \
    fprintf(stdout, "%s End, Result:%s(errno:%d).\n", test.tc_name, rc == 0 ? "OK": "FAIL", rc);\
    fprintf(stdout, "%s", END_LINE);\
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
    //BUILD_TC_CASE(TC_ENABLE, sock_test),
    BUILD_TC_CASE(TC_ENABLE, timer_task_test),
    /*
    BUILD_TC_CASE(TC_ENABLE, cstr_test),
    BUILD_TC_CASE(TC_ENABLE, string_test),
    BUILD_TC_CASE(TC_ENABLE, list_test),
    BUILD_TC_CASE(TC_ENABLE, pool_test),
    BUILD_TC_CASE(TC_ENABLE, exception_test),
    BUILD_TC_CASE(TC_ENABLE, sleep_test),
    BUILD_TC_CASE(TC_ENABLE, thread_test),
    BUILD_TC_CASE(TC_ENABLE, timer_test)
    */
};

em_pool_factory *mem;

int test_main(int log_level)
{
    int32 count = 0;
    em_caching_pool caching_pool;
    emlib_ret_t rc = EM_SUCC;
    int32 tc_cnt = EM_ARRAY_SIZE(tc_array);

    mem = &caching_pool.factory;

    /*Emlib init.*/
    em_init();

    em_caching_pool_init( &caching_pool, NULL, 65536/*64kbytes*/);

    em_log_set_log_level(log_level);    

    EM_LOG(EM_LOG_INFO, "Hello, %s.", "Emlib.");
    EM_LOG(EM_LOG_INFO, "Enjoy, It!");
    em_msleep(500);

    EM_LOG(EM_LOG_INFO, "Start Run TEST Cases ...\n");

    int fd = open("/dev/abc", O_RDWR);
    printf("fd = %d.\n", errno);

    em_perror(EM_LOG_ERROR, "TEST", EM_STATUS_FROM_OS(errno), "open failed.");

    for (int32 i = 0; i < tc_cnt; i++) {
        if(tc_array[i].flag) {
            DO_TEST(tc_array[i]);
            count++;
        }
    }

test_over:
    if(rc == 0) {
        fprintf(stdout, "All TEST Cases Run OK!");
    } else {
        EM_LOG(EM_LOG_ERROR, "TEST Cases Run Failed, Pelase Check Errno.");
    }

    em_caching_pool_destroy(&caching_pool);

    return rc;
}
