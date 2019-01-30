/*
 * File:    rand.h
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
#ifndef __EM_COMPAT_RAND_H__
#define __EM_COMPAT_RAND_H__

/**
 * @file rand.h
 * @brief Provides platform_rand() and platform_srand() functions.
 */

#if defined(EM_HAS_STDLIB_H) && EM_HAS_STDLIB_H != 0
/*
 * Use stdlib based rand() and srand().
 */
#  include <stdlib.h>
#  define platform_srand    srand
#  if defined(RAND_MAX) && RAND_MAX <= 0xFFFF
/*
 * When rand() is only 16 bit strong, double the strength
 * by calling it twice!
 */
EM_INLINE(int) platform_rand(void)
{
    return ((rand() & 0xFFFF) << 16) | (rand() & 0xFFFF);
}
#  else
#      define platform_rand rand
#  endif

#else
#  warning "platform_rand() is not implemented"
#  define platform_rand()	1
#  define platform_srand(seed)

#endif


#endif	/* __EM_COMPAT_RAND_H__ */

