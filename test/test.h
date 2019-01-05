/*
 * File:    test.h
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

#ifndef __TEST_H__
#define __TEST_H__

#include "em/types.h"

DECLS_BEGIN

#define TERRNO() (-__LINE__)

#define INCLUDE_PTHREAD_LOCK      1
#define INCLUDE_PTHREAD_NEST_LOCK 1
#define INCLUDE_CSTR              1
#define INCLUDE_STRING_TEST       1
#define INCLUDE_LIST_TEST         1
#define INCLUDE_POOL_TEST         1
#define INCLUDE_EXCEPTION_TEST    1
#define INCLUDE_THREAD_TEST       1


extern emlib_ret_t pthread_mutex_test(void);
extern emlib_ret_t pthread_nest_mutex_test(void);
extern emlib_ret_t cstr_test(void);
extern emlib_ret_t string_test(void);
extern emlib_ret_t list_test(void);
extern emlib_ret_t pool_test(void);
extern emlib_ret_t exception_test(void);
extern emlib_ret_t thread_test(void);

extern em_pool_factory *mem;

int test_main(int log_level);
extern void  app_perror(const char *msg, emlib_ret_t err);

DECLS_END
#endif
