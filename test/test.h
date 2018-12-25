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

#include "types.h"

DECLS_BEGIN

#define INCLUDE_PTHREAD_LOCK 
#define INCLUDE_PTHREAD_NEST_LOCK

extern embed_ret_t pthread_mutex_test(void);
extern embed_ret_t pthread_nest_mutex_test(void);

void test_main(void);

DECLS_END
#endif
