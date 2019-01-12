/*
 * File:    errno.h
 * Author:  Liu HongLiang<lhl_nciae@sina.cn>
 * Brief:   Emlib Error Subsystem 
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

#include "em/log.h"
#include "em/errno.h"
#include "em/string.h"
#include "em/assert.h"
#include "em/compat/string.h"

#include <stdarg.h>

static const char *module = "ERRNO";

/* Prototype for platform specific error message, which will be defined 
 * in separate file.
 */
DECLS_BEGIN
EM_DECL(int) platform_strerror(em_os_err_type code, char *buf, em_size_t bufsize );
DECLS_END


#ifndef EMLIB_MAX_ERR_MSG_HANDLER
#	define EMLIB_MAX_ERR_MSG_HANDLER   10
#endif

/* Error message handler. */
static unsigned err_msg_hnd_cnt;
static struct err_msg_hnd
{
    emlib_ret_t	    begin;
    emlib_ret_t	    end;
    em_str_t	  (*strerror)(emlib_ret_t, char*, em_size_t);

} err_msg_hnd[EMLIB_MAX_ERR_MSG_HANDLER];

/* EMLIB's own error codes/messages */
#if defined(EM_HAS_ERROR_STRING) && EM_HAS_ERROR_STRING!=0

static const struct 
{
    int code;
    const char *msg;
} err_str[] = 
{
    EM_BUILD_ERR(EM_EUNKNOWN,      "Unknown Error" ),
    EM_BUILD_ERR(EM_EPENDING,      "Pending operation" ),
    EM_BUILD_ERR(EM_ETOOMANYCONN,  "Too many connecting sockets" ),
    EM_BUILD_ERR(EM_EINVAL,        "Invalid value or argument" ),
    EM_BUILD_ERR(EM_ENAMETOOLONG,  "Name too long" ),
    EM_BUILD_ERR(EM_ENOTFOUND,     "Not found" ),
    EM_BUILD_ERR(EM_ENOMEM,        "Not enough memory" ),
    EM_BUILD_ERR(EM_EBUG,          "BUG DETECTED!" ),
    EM_BUILD_ERR(EM_ETIMEDOUT,     "Operation timed out" ),
    EM_BUILD_ERR(EM_ETOOMANY,      "Too many objects of the specified type"),
    EM_BUILD_ERR(EM_EBUSY,         "Object is busy"),
    EM_BUILD_ERR(EM_ENOTSUP,	   "Option/operation is not supported"),
    EM_BUILD_ERR(EM_EINVALIDOP,	   "Invalid operation"),
    EM_BUILD_ERR(EM_ECANCELLED,    "Operation cancelled"),
    EM_BUILD_ERR(EM_EEXISTS,       "Object already exists" ),
    EM_BUILD_ERR(EM_EEOF,	   "End of file" ),
    EM_BUILD_ERR(EM_ETOOBIG,	   "Size is too big"),
    EM_BUILD_ERR(EM_ERESOLVE,	   "gethostbyname() has returned error"),
    EM_BUILD_ERR(EM_ETOOSMALL,	   "Size is too short"),
    EM_BUILD_ERR(EM_EIGNORED,	   "Ignored"),
    EM_BUILD_ERR(EM_EIPV6NOTSUP,   "IPv6 is not supported"),
    EM_BUILD_ERR(EM_EAFNOTSUP,	   "Unsupported address family"),
    EM_BUILD_ERR(EM_EGONE,	   "Object no longer exists"),
    EM_BUILD_ERR(EM_ESOCKETSTOP,   "Socket is in bad state")
};
#endif	/* EM_HAS_ERROR_STRING */


/*
 * emlib_error()
 *
 * Retrieve message string for emlib's own error code.
 */
static int emlib_error(emlib_ret_t code, char *buf, em_size_t size)
{
    int len;

#if defined(EM_HAS_ERROR_STRING) && EM_HAS_ERROR_STRING!=0
    unsigned i;

    for (i=0; i<sizeof(err_str)/sizeof(err_str[0]); ++i) {
        if (err_str[i].code == code) {
            em_size_t len2 = em_ansi_strlen(err_str[i].msg);
            if (len2 >= size) len2 = size-1;
            em_memcpy(buf, err_str[i].msg, len2);
            buf[len2] = '\0';
            return (int)len2;
        }
    }
#endif

    len = em_ansi_snprintf( buf, size, "Unknown emlib error %d", code);
    if (len < 1 || len >= (int)size)
        len = (int)(size - 1);
    return len;
}

#define IN_RANGE(val,start,end)	    ((val)>=(start) && (val)<(end))

/* Register strerror handle. */
EM_DEF(emlib_ret_t) em_register_strerror( emlib_ret_t start,
        emlib_ret_t space,
        em_error_callback f)
{
    unsigned i;

    /* Check arguments. */
    EMLIB_ASSERT_RETURN((start && space && f), EM_EINVAL);

    /* Check if there aren't too many handlers registered. */
    EMLIB_ASSERT_RETURN((err_msg_hnd_cnt < EM_ARRAY_SIZE(err_msg_hnd)),
            EM_ETOOMANY);

    /* Start error must be greater than EM_ERRNO_START_USER */
    EMLIB_ASSERT_RETURN((start >= EM_ERRNO_START_USER), EM_EEXISTS);

    /* Check that no existing handler has covered the specified range. */
    for (i=0; i<err_msg_hnd_cnt; ++i) {
        if (IN_RANGE(start, err_msg_hnd[i].begin, err_msg_hnd[i].end) ||
                IN_RANGE(start+space-1, err_msg_hnd[i].begin, err_msg_hnd[i].end))
        {
            if (err_msg_hnd[i].begin == start && 
                    err_msg_hnd[i].end == (start+space) &&
                    err_msg_hnd[i].strerror == f)
            {
                /* The same range and handler has already been registered */
                return EM_SUCC;
            }

            return EM_EEXISTS;
        }
    }

    /* Register the handler. */
    err_msg_hnd[err_msg_hnd_cnt].begin = start;
    err_msg_hnd[err_msg_hnd_cnt].end = start + space;
    err_msg_hnd[err_msg_hnd_cnt].strerror = f;

    ++err_msg_hnd_cnt;

    return EM_SUCC;
}

/* Internal emlib function called by em_shutdown() to clear error handlers */
void em_errno_clear_handlers(void)
{
    err_msg_hnd_cnt = 0;
    em_bzero(err_msg_hnd, sizeof(err_msg_hnd));
}


/*
 * em_strerror()
 */
EM_DEF(em_str_t) em_strerror( emlib_ret_t statcode, 
        char *buf, em_size_t bufsize )
{
    int len = -1;
    em_str_t errstr;

    EMLIB_ASSERT(buf && bufsize);

    if (statcode == EM_SUCC) {
        len = em_ansi_snprintf( buf, bufsize, "Success");

    } else if (statcode < EM_ERRNO_START + EM_ERRNO_SPACE_SIZE) {
        len = em_ansi_snprintf( buf, bufsize, "Unknown error %d", statcode);

    } else if (statcode < EM_ERRNO_START_STATUS + EM_ERRNO_SPACE_SIZE) {
        len = emlib_error(statcode, buf, bufsize);

    } else if (statcode < EM_ERRNO_START_SYS + EM_ERRNO_SPACE_SIZE) {
        len = platform_strerror(EM_STATUS_TO_OS(statcode), buf, bufsize);

    } else {
        unsigned i;

        /* Find user handler to get the error message. */
        for (i=0; i<err_msg_hnd_cnt; ++i) {
            if (IN_RANGE(statcode, err_msg_hnd[i].begin, err_msg_hnd[i].end)) {
                return (*err_msg_hnd[i].strerror)(statcode, buf, bufsize);
            }
        }

        /* Handler not found! */
        len = em_ansi_snprintf( buf, bufsize, "Unknown error %d", statcode);
    }

    if (len < 1 || len >= (int)bufsize) {
        len = (int)(bufsize - 1);
        buf[len] = '\0';
    }

    errstr.ptr = buf;
    errstr.slen = len;

    return errstr;
}

#if EM_LOG_MAX_LEVEL >= 1
static void invoke_log(const char *sender, int level, const char *format, ...)
{
    va_list arg;
    va_start(arg, format);
    EM_LOG_MOD(level, sender, format, arg);
    va_end(arg);
}

static void em_perror_imp(int log_level, const char *sender, 
        emlib_ret_t status,
        const char *title_fmt, va_list marker)
{
    char titlebuf[EM_PERROR_TITLE_BUF_SIZE];
    char errmsg[EM_ERR_MSG_SIZE];
    int len;

    /* Build the title */
    len = em_ansi_vsnprintf(titlebuf, sizeof(titlebuf), title_fmt, marker);
    if (len < 0 || len >= (int)sizeof(titlebuf))
        em_ansi_strcpy(titlebuf, "Error");

    /* Get the error */
    em_strerror(status, errmsg, sizeof(errmsg));

    /* Send to log */
    invoke_log(sender, log_level, "%s: %s", titlebuf, errmsg);
}

EM_DEF(void) em_perror(int log_level, const char *sender, emlib_ret_t status,
        const char *title_fmt, ...)
{
    va_list marker;
    va_start(marker, title_fmt);
    em_perror_imp(log_level, sender, status, title_fmt, marker);
    va_end(marker);
}

EM_DEF(void) em_perror_1(const char *sender, emlib_ret_t status,
        const char *title_fmt, ...)
{
    va_list marker;
    va_start(marker, title_fmt);
    em_perror_imp(1, sender, status, title_fmt, marker);
    va_end(marker);
}

#else /* #if EM_LOG_MAX_LEVEL >= 1 */
EM_DEF(void) em_perror(int log_level, const char *sender, emlib_ret_t status,
        const char *title_fmt, ...)
{
}
#endif	/* #if EM_LOG_MAX_LEVEL >= 1 */


#if EM_LOG_MAX_LEVEL >= 2
EM_DEF(void) em_perror_2(const char *sender, emlib_ret_t status,
        const char *title_fmt, ...)
{
    va_list marker;
    va_start(marker, title_fmt);
    em_perror_imp(2, sender, status, title_fmt, marker);
    va_end(marker);
}
#endif

#if EM_LOG_MAX_LEVEL >= 3
EM_DEF(void) em_perror_3(const char *sender, emlib_ret_t status,
        const char *title_fmt, ...)
{
    va_list marker;
    va_start(marker, title_fmt);
    em_perror_imp(3, sender, status, title_fmt, marker);
    va_end(marker);
}
#endif

#if EM_LOG_MAX_LEVEL >= 4
EM_DEF(void) em_perror_4(const char *sender, emlib_ret_t status,
        const char *title_fmt, ...)
{
    va_list marker;
    va_start(marker, title_fmt);
    em_perror_imp(4, sender, status, title_fmt, marker);
    va_end(marker);
}
#endif

#if EM_LOG_MAX_LEVEL >= 5
EM_DEF(void) em_perror_5(const char *sender, emlib_ret_t status,
        const char *title_fmt, ...)
{
    va_list marker;
    va_start(marker, title_fmt);
    em_perror_imp(5, sender, status, title_fmt, marker);
    va_end(marker);
}
#endif

#if EM_LOG_MAX_LEVEL >= 6
EM_DEF(void) em_perror_6(const char *sender, emlib_ret_t status,
        const char *title_fmt, ...)
{
    va_list marker;
    va_start(marker, title_fmt);
    em_perror_imp(6, sender, status, title_fmt, marker);
    va_end(marker);
}
#endif

