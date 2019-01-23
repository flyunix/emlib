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

#define ASSERT(expr) 

#ifndef NASSERT
#       define EM_ASSERT(expr) assert(expr)
#else
#       define EM_ASSERT(expr) ASSERT(expr)
#endif

/*ASSERT 文本*/
#define EMLIB_ASSERT(expr){\
    if(!(expr)) { \
        EM_LOG(EM_LOG_ERROR, #expr);\
        EM_ASSERT((expr));\
    }\
}\

#define EMLIB_ASSERT_NOLOG(expr){\
    if(!(expr)) { \
        EM_ASSERT((expr));\
        printf("EMLIB_ASSERT_NOLOG:%s\n", #expr);\
    }\
}\

#define EMLIB_ASSERT_ON_FAIL(expr, exec_on_fail) \
    do {\
        EM_ASSERT(expr); \
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
        if (!(expr)) {EM_ASSERT(expr); EM_LOG(EM_LOG_ERROR, #expr); return retval; } \
    } while (0)

#define EMLIB_ASSERT_RETNOLOG(expr,retval)    \
    do { \
        if (!(expr)) {EM_ASSERT(expr); printf("EMLIB_ASSERT_RETNOLOG:%s\n", #expr);return retval; } \
    } while (0)

#define return_if_fail(p) if(!(p)) \
	{printf("%s:%d Warning: "#p" failed.\n", \
		__func__, __LINE__); return;}
#define return_val_if_fail(p, ret) if(!(p)) \
	{printf("%s:%d Warning: "#p" failed.\n",\
	__func__, __LINE__); return (ret);}

#define TERRNO() (-__LINE__)

#define __EM_ERROR__(func) \
emlib_ret_t ret;\
ret = func; \
if(ret != EM_SUCC) { \
    char err_buf[1024] = {0};\
    em_strerror(ret, err_buf, sizeof(err_buf));\
    EM_LOG(EM_LOG_ERROR, #func" failed:%s", err_buf); \

#define EM_ERROR_CHECK(func) \
    do { \
        __EM_ERROR__(func)\
        return ret; \
    } \
}while(0)

#define EM_ERROR_CHECK_TEST(func) \
    do { \
        __EM_ERROR__(func)\
        return TERRNO(); \
    } \
}while(0)

#define EM_ERROR_CHECK_NORET(func) \
    do { \
        __EM_ERROR__(func)\
        return;\
    }\
}while(0)

DECLS_END   

#endif/*__ASSERT_H_*/
