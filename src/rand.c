/*
 * File:    rand.c
 * Author:  Liu HongLiang<lhl_nciae@sina.cn>
 * Brief:   Random Number Generator.
 * The timer scheduling implementation here is based on ACE library's 
 * ACE_Timer_Heap, with only little modification to suit our library's style
 * (I even left most of the comments in the original source).
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
 * 2019-01-12 Liu HongLiang<lhl_nciae@sina.cn> created.
 *
 */
#include <em/rand.h>
#include <em/os.h>
#include <em/compat/rand.h>

EM_DEF(void) em_srand(unsigned int seed)
{
    EM_CHECK_STACK();
    platform_srand(seed);
}

EM_DEF(int) em_rand(void)
{
    EM_CHECK_STACK();
    return platform_rand();
}

