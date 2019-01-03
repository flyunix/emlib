/*
 * File:    assert.h
 * Author:  Liu HongLiang<lhl_nciae@sina.cn>
 * Brief:   .
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
 * 2018-12-20 Liu HongLiang<lhl_nciae@sina.cn> created.
 *
 */
#ifndef _ASSERT_H_
#define _ASSERT_H_

#include <assert.h>

#include "em/log.h"
#include "em/types.h"

DECLS_BEGIN 

#define em_assert(expr) assert(expr)

/*ASSERT 文本*/
#define EMLIB_ASSERT(expr){\
    if(!(expr)) { \
        EM_LOG(EM_LOG_ERROR, #expr);\
        em_assert((expr));\
    }\
}\

#define EMLIB_ASSERT_ON_FAIL(expr, exec_on_fail) \
    do {\
        em_assert(expr); \
        if(!expr) exec_on_fail;\
    }while(0)

/**
 * @hideinitializer
 * If ther value is non-zero, then 
 * #EMLIB_ASSERT_RETURN macro will evaluate the expression in @a expr during
 * run-time. If the expression yields false, assertion will be triggered
 * and the current function will return with the specified return value.
 *
 */
#define EMLIB_ASSERT_RETURN(expr,retval)    \
    do { \
        if (!(expr)) {em_assert(expr); EM_LOG(EM_LOG_ERROR, #expr); return retval; } \
    } while (0)

#define return_if_fail(p) if(!(p)) \
	{printf("%s:%d Warning: "#p" failed.\n", \
		__func__, __LINE__); return;}
#define return_val_if_fail(p, ret) if(!(p)) \
	{printf("%s:%d Warning: "#p" failed.\n",\
	__func__, __LINE__); return (ret);}

DECLS_END   

#endif/*__ASSERT_H_*/
