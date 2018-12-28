/*
 * File:    lock.c
 * Author:  Liu HongLiang<lhl_nciae@sina.cn>
 * Brief:   Abstract lock module interface.
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

#include "em/lock.h"

inline emlib_ret_t em_lock(em_locker *thiz)
{
    return_if_fail((thiz != NULL) && (thiz->lock));

    return thiz->lock(thiz);
}

inline emlib_ret_t em_trylock(em_locker *thiz)
{
    return_val_if_fail((thiz != NULL) && (thiz->trylock), EM_FAIL);

    return thiz->trylock(thiz);
}

inline emlib_ret_t em_unlock(em_locker *thiz) 
{
    return_if_fail((thiz != NULL) && (thiz->unlock));

    return thiz->unlock(thiz);
}


inline void em_lock_destroy(em_locker *thiz)
{
    return_if_fail(thiz != NULL);
    
    thiz->destroy(thiz);
}

