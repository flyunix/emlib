/*
 * File:    high_precision.h
 * Author:  Liu HongLiang<lhl_nciae@sina.cn>
 * Brief:   high precision declaration.
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
 * 2019-01-11 Liu HongLiang<lhl_nciae@sina.cn> created.
 *
 */
#ifndef __EM_COMPAT_HIGH_PRECISION_H__
#define __EM_COMPAT_HIGH_PRECISION_H__


#if defined(EM_HAS_FLOATING_POINT) && EM_HAS_FLOATING_POINT != 0
    /*
     * The first choice for high precision math is to use double.
     */
#   include <math.h>
    typedef double em_highprec_t;

#   define EM_HIGHPREC_VALUE_IS_ZERO(a)     (a==0)
#   define em_highprec_mod(a,b)             (a=fmod(a,b))

#elif defined(EM_HAS_INT64) && EM_HAS_INT64 != 0
    /*
     * Next choice is to use 64-bit arithmatics.
     */
    typedef int64 em_highprec_t;

#else
#   warning "High precision math is not available"

    /*
     * Last, fallback to 32-bit arithmetics.
     */
    typedef int32 em_highprec_t;

#endif

/**
 * @def em_highprec_mul
 * em_highprec_mul(a1, a2) - High Precision Multiplication
 * Multiply a1 and a2, and store the result in a1.
 */
#ifndef em_highprec_mul
#   define em_highprec_mul(a1,a2)   (a1 = a1 * a2)
#endif

/**
 * @def em_highprec_div
 * em_highprec_div(a1, a2) - High Precision Division
 * Divide a2 from a1, and store the result in a1.
 */
#ifndef em_highprec_div
#   define em_highprec_div(a1,a2)   (a1 = a1 / a2)
#endif

/**
 * @def em_highprec_mod
 * em_highprec_mod(a1, a2) - High Precision Modulus
 * Get the modulus a2 from a1, and store the result in a1.
 */
#ifndef em_highprec_mod
#   define em_highprec_mod(a1,a2)   (a1 = a1 % a2)
#endif


/**
 * @def EM_HIGHPREC_VALUE_IS_ZERO(a)
 * Test if the specified high precision value is zero.
 */
#ifndef EM_HIGHPREC_VALUE_IS_ZERO
#   define EM_HIGHPREC_VALUE_IS_ZERO(a)     (a==0)
#endif


#endif	/* __EM_COMPAT_HIGH_PRECISION_H__ */

