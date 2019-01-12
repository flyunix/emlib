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
#ifndef __EM_RAND_H__
#define __EM_RAND_H__

/**
 * @file rand.h
 */

#include <em/config.h>

EM_BEGIN_DECL


/**
 * @defgroup EM_RAND Random Number Generator
 * @ingroup EM_MISC
 * @{
 * This module contains functions for generating random numbers.
 * This abstraction is needed not only because not all platforms have
 * \a rand() and \a srand(), but also on some platforms \a rand()
 * only has 16-bit randomness, which is not good enough.
 */

/**
 * Put in seed to random number generator.
 *
 * @param seed	    Seed value.
 */
EM_DECL(void) em_srand(unsigned int seed);


/**
 * Generate random integer with 32bit randomness.
 *
 * @return a random integer.
 */
EM_DECL(int) em_rand(void);


/** @} */


EM_END_DECL


#endif	/* __EM_RAND_H__ */

