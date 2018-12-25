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


#include "log.h"
#include "test.h"

#define DO_TEST(test) \
    do  \
{\
    EM_LOG(EM_LOG_DEBUG, "Start Run %s TEST ...", #test);\
    rc = test; \
    EM_LOG(EM_LOG_DEBUG, "End Run %s TEST, Result:%s(errno:%d).\n", #test, rc == 0 ? "OK": "FAIL", rc);\
    if(rc != EMBED_SUCC) { \
        goto test_over;\
    }\
}while(0)

void test_main(void)
{
    embed_ret_t rc = EMBED_SUCC;
    EM_LOG(EM_LOG_DEBUG, "Start Run TEST Cases ...\n");


#ifdef  INCLUDE_PTHREAD_LOCK 
    DO_TEST(pthread_mutex_test());
#endif

#ifdef INCLUDE_PTHREAD_NEST_LOCK
    DO_TEST(pthread_nest_mutex_test());
#endif

test_over:
    if(rc == 0) {
        EM_LOG(EM_LOG_DEBUG, "All TEST Cases Run OK!");
    } else {
        EM_LOG(EM_LOG_ERROR, "TEST Cases Run Failed, Pelase Check Errno.");
    }
}
