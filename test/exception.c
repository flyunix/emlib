/*
 * File:    exception.c
 * Author:  Liu HongLiang<lhl_nciae@sina.cn>
 * Brief:   TEST Module for C Exception.
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
 * 2019-01-04 Liu HongLiang<lhl_nciae@sina.cn> created.
 *
 */
#include "emlib.h"
#include "test.h"

/**
 * \page page_EMlib_exception_test Test: Exception Handling
 *
 * This file provides implEMentation of \b exception_test(). It tests the
 * functionality of the exception handling API.
 *
 * @note This test use static ID not acquired through proper registration.
 * This is not recommended, since it may create ID collissions.
 *
 * \section exception_test_sec Scope of the Test
 *
 * Some scenarios tested:
 *  - no exception situation
 *  - basic TRY/CATCH
 *  - multiple exception handlers
 *  - default handlers
 *
 *
 * This file is <b>EMlib-test/exception.c</b>
 *
 * \include EMlib-test/exception.c
 */


#if INCLUDE_EXCEPTION_TEST

#include <emlib.h>

#ifdef	_MSC_VER
#pragma warning(disable:4702) // warning C4702: unreachable code
#endif

#define	ID_1	1
#define ID_2	2

static int throw_id_1(void)
{
    EM_THROW( ID_1 );
    EM_UNREACHED(return -1;)
}

static int throw_id_2(void)
{
    EM_THROW( ID_2 );
    EM_UNREACHED(return -1;)
}

static int try_catch_test(void)
{
    EM_USE_EXCEPTION;
    int rc = -200;

    EM_TRY {
        EM_THROW(ID_1);
    }
    EM_CATCH_ANY {
        rc = 0;
    }
    EM_END;
    return rc;
}

static int throw_in_handler(void)
{
    EM_USE_EXCEPTION;
    int rc = 0;

    EM_TRY {
        EM_THROW(ID_1);
    }
    EM_CATCH_ANY {
        if (EM_GET_EXCEPTION() != ID_1)
            rc = -300;
        else
            EM_THROW(ID_2);
    }
    EM_END;
    return rc;
}

static int return_in_handler(void)
{
    EM_USE_EXCEPTION;

    EM_TRY {
        EM_THROW(ID_1);
    }
    EM_CATCH_ANY {
        return 0;
    }
    EM_END;
    return -400;
}


static int test(void)
{
    int rc = 0;
    EM_USE_EXCEPTION;

    /*
     * No exception situation.
     */
    EM_TRY {
        EM_UNUSED_ARG(rc);
    }
    EM_CATCH_ANY {
        rc = -3;
    }
    EM_END;

    if (rc != 0)
        return rc;


    /*
     * Basic TRY/CATCH
     */ 
    EM_TRY {
        rc = throw_id_1();

        // should not reach here.
        rc = -10;
    }
    EM_CATCH_ANY {
        int id = EM_GET_EXCEPTION();
        if (id != ID_1) {
            EM_LOG_MOD(EM_LOG_INFO,"", "...error: got unexpected exception %d (%s)", 
                        id, em_exception_id_name(id));
            if (!rc) rc = -20;
        }
    }
    EM_END;

    if (rc != 0)
        return rc;

    /*
     * Multiple exceptions handlers
     */
    EM_TRY {
        rc = throw_id_2();
        // should not reach here.
        rc = -25;
    }
    EM_CATCH_ANY {
        switch (EM_GET_EXCEPTION()) {
            case ID_1:
                if (!rc) rc = -30;
                break;
            case ID_2:
                if (!rc) rc = 0;
                break;
            default:
                if (!rc) rc = -40;
                break;
        }
    }
    EM_END;

    if (rc != 0)
        return rc;

    /*
     * Test default handler.
     */
    EM_TRY {
        rc = throw_id_1();
        // should not reach here
        rc = -50;
    }
    EM_CATCH_ANY {
        switch (EM_GET_EXCEPTION()) {
            case ID_1:
                if (!rc) rc = 0;
                break;
            default:
                if (!rc) rc = -60;
                break;
        }
    }
    EM_END;

    if (rc != 0)
        return rc;

    /*
     * Nested handlers
     */
    EM_TRY {
        rc = try_catch_test();
    }
    EM_CATCH_ANY {
        rc = -70;
    }
    EM_END;

    if (rc != 0)
        return rc;

    /*
     * Throwing exception inside handler
     */
    rc = -80;
    EM_TRY {
        int rc2;
        rc2 = throw_in_handler();
        if (rc2)
            rc = rc2;
    }
    EM_CATCH_ANY {
        if (EM_GET_EXCEPTION() == ID_2) {
            rc = 0;
        } else {
            rc = -90;
        }
    }
    EM_END;

    if (rc != 0)
        return rc;


    /*
     * Return from handler. Returning from the function inside a handler
     * should be okay (though returning from the function inside the
     * EM_TRY block IS NOT OKAY!!). We want to test to see if handler
     * is cleaned up properly, but not sure how to do this.
     */
    EM_TRY {
        int rc2;
        rc2 = return_in_handler();
        if (rc2)
            rc = rc2;
    }
    EM_CATCH_ANY {
        rc = -100;
    }
    EM_END;


    return 0;
}

emlib_ret_t exception_test(void)
{
    int i, rc;
    enum { LOOP = 10 };

    for (i=0; i<LOOP; ++i) {
        if ((rc=test()) != 0) {
            EM_LOG_MOD(EM_LOG_INFO,"", "...failed at i=%d (rc=%d)", i, rc);
            return rc;
        }
    }
    return 0;
}

#else
/* To prevent warning about "translation unit is EMpty"
 * when this test is disabled. 
 */
int dummy_exception_test;
#endif	/* INCLUDE_EXCEPTION_TEST */


