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

#ifndef __EM_ERRNO_H__
#define __EM_ERRNO_H__

#include "em/types.h"
#include "em/compat/errno.h"

#include <stdarg.h>

DECLS_BEGIN

/**
 * @defgroup EM_ERRNO Error Subsystem
 * @{
 *
 * The EMLIB Error Subsystem is a framework to unify all error codes
 * produced by all components into a single error space, and provide
 * uniform set of APIs to access them. With this framework, any error
 * codes are encoded as emlib_ret_t value. The framework is extensible,
 * application may register new error spaces to be recognized by
 * the framework.
 *
 * @section EM_ERRNO_retval Return Values
 *
 * All functions that returns @a emlib_ret_t returns @a EM_SUCC if the
 * operation was completed successfully, or non-zero value to indicate 
 * error. If the error came from operating system, then the native error
 * code is translated/folded into EMLIB's error namespace by using
 * #EM_STATUS_FROM_OS() macro. The function will do this automatically
 * before returning the error to caller.
 *
 * @section err_services Retrieving and Displaying Error Messages
 *
 * The framework provides the following APIs to retrieve and/or display
 * error messages:
 *
 *   - #em_strerror(): this is the base API to retrieve error string
 *      description for the specified emlib_ret_t error code.
 *
 *   - #EM_PERROR() macro: use this macro similar to EM_LOG to format
 *      an error message and display them to the log
 *
 *   - #em_perror(): this function is similar to EM_PERROR() but unlike
 *      #EM_PERROR(), this function will always be included in the
 *      link process. Due to this reason, prefer to use #EM_PERROR()
 *      if the application is concerned about the executable size.
 *
 * Application MUST NOT pass native error codes (such as error code from
 * functions like GetLastError() or errno) to EMLIB functions expecting
 * @a emlib_ret_t.
 *
 * @section err_extending Extending the Error Space
 *
 * Application may register new error space to be recognized by the
 * framework by using #em_register_strerror(). Use the range started
 * from EM_ERRNO_START_USER to avoid conflict with existing error
 * spaces.
 *
 */

/**
 * Guidelines on error message length.
 */
#define EM_ERR_MSG_SIZE  80

/**
 * Buffer for title string of #EM_PERROR().
 */
#ifndef EM_PERROR_TITLE_BUF_SIZE
#   define EM_PERROR_TITLE_BUF_SIZE	120
#endif


/**
 * Get the last platform error/status, folded into emlib_ret_t.
 * @return	OS dependent error code, folded into emlib_ret_t.
 * @remark	This function gets errno, or calls GetLastError() function and
 *		convert the code into emlib_ret_t with EM_STATUS_FROM_OS. Do
 *		not call this for socket functions!
 * @see	em_get_netos_error()
 */
EM_DECL(emlib_ret_t) em_get_os_error(void);

/**
 * Set last error.
 * @param code	emlib_ret_t
 */
EM_DECL(void) em_set_os_error(emlib_ret_t code);

/**
 * Get the last error from socket operations.
 * @return	Last socket error, folded into emlib_ret_t.
 */
EM_DECL(emlib_ret_t) em_get_netos_error(void);

/**
 * Set error code.
 * @param code	emlib_ret_t.
 */
EM_DECL(void) em_set_netos_error(emlib_ret_t code);


/**
 * Get the error message for the specified error code. The message
 * string will be NULL terminated.
 *
 * @param statcode  The error code.
 * @param buf	    Buffer to hold the error message string.
 * @param bufsize   Size of the buffer.
 *
 * @return	    The error message as NULL terminated string,
 *                  wrapped with em_str_t.
 */
EM_DECL(em_str_t) em_strerror( emlib_ret_t statcode, 
			       char *buf, em_size_t bufsize);

/**
 * A utility macro to print error message pertaining to the specified error 
 * code to the log. This macro will construct the error message title 
 * according to the 'title_fmt' argument, and add the error string pertaining
 * to the error code after the title string. A colon (':') will be added 
 * automatically between the title and the error string.
 *
 * This function is similar to em_perror() function, but has the advantage
 * that the function call can be omitted from the link process if the
 * log level argument is below EM_LOG_MAX_LEVEL threshold.
 *
 * Note that the title string constructed from the title_fmt will be built on
 * a string buffer which size is EM_PERROR_TITLE_BUF_SIZE, which normally is
 * allocated from the stack. By default this buffer size is small (around
 * 120 characters). Application MUST ensure that the constructed title string
 * will not exceed this limit, since not all platforms support truncating
 * the string.
 *
 * @see em_perror()
 *
 * @param level	    The logging verbosity level, valid values are 0-6. Lower
 *		    number indicates higher importance, with level zero 
 *		    indicates fatal error. Only numeral argument is 
 *		    permitted (e.g. not variable).
 * @param arg	    Enclosed 'printf' like arguments, with the following
 *		    arguments:
 *		     - the sender (NULL terminated string),
 *		     - the error code (emlib_ret_t)
 *		     - the format string (title_fmt), and 
 *		     - optional variable number of arguments suitable for the 
 *		       format string.
 *
 * Sample:
 * \verbatim
   EM_PERROR(2, (__FILE__, em_EBUSY, "Error making %s", "coffee"));
   \endverbatim
 * @hideinitializer
 */
#define EM_PERROR(level,arg)	do { \
				    em_perror_wrapper_##level(arg); \
				} while (0)

/**
 * A utility function to print error message pertaining to the specified error 
 * code to the log. This function will construct the error message title 
 * according to the 'title_fmt' argument, and add the error string pertaining
 * to the error code after the title string. A colon (':') will be added 
 * automatically between the title and the error string.
 *
 * Unlike the EM_PERROR() macro, this function takes the \a log_level argument
 * as a normal argument, unlike in EM_PERROR() where a numeral value must be
 * given. However this function will always be linked to the executable,
 * unlike EM_PERROR() which can be omitted when the level is below the 
 * EM_LOG_MAX_LEVEL.
 *
 * Note that the title string constructed from the title_fmt will be built on
 * a string buffer which size is EM_PERROR_TITLE_BUF_SIZE, which normally is
 * allocated from the stack. By default this buffer size is small (around
 * 120 characters). Application MUST ensure that the constructed title string
 * will not exceed this limit, since not all platforms support truncating
 * the string.
 *
 * @see EM_PERROR()
 */
EM_DECL(void) em_perror(int log_level, const char *sender, emlib_ret_t status,
		        const char *title_fmt, ...);


/**
 * Type of callback to be specified in #em_register_strerror()
 *
 * @param e	    The error code to lookup.
 * @param msg	    Buffer to store the error message.
 * @param max	    Length of the buffer.
 *
 * @return	    The error string.
 */
typedef em_str_t (*em_error_callback)(emlib_ret_t e, char *msg, em_size_t max);


/**
 * Register strerror message handler for the specified error space.
 * Application can register its own handler to supply the error message
 * for the specified error code range. This handler will be called
 * by #em_strerror().
 *
 * @param start_code	The starting error code where the handler should
 *			be called to retrieve the error message.
 * @param err_space	The size of error space. The error code range then
 *			will fall in start_code to start_code+err_space-1
 *			range.
 * @param f		The handler to be called when #em_strerror() is
 *			supplied with error code that falls into this range.
 *
 * @return		EM_SUCC or the specified error code. The 
 *			registration may fail when the error space has been
 *			occupied by other handler, or when there are too many
 *			handlers registered to EMLIB.
 */
EM_DECL(emlib_ret_t) em_register_strerror(emlib_ret_t start_code,
					  emlib_ret_t err_space,
					  em_error_callback f);

/**
 * @hideinitializer
 * Return platform os error code folded into emlib_ret_t code. This is
 * the macro that is used throughout the library for all EMLIB's functions
 * that returns error from operating system. Application may override
 * this macro to reduce size (e.g. by defining it to always return 
 * #EM_EUNKNOWN).
 *
 * Note:
 *  This macro MUST return non-zero value regardless whether zero is
 *  passed as the argument. The reason is to protect logic error when
 *  the operating system doesn't report error codes properly.
 *
 * @param os_code   Platform OS error code. This value may be evaluated
 *		    more than once.
 * @return	    The platform os error code folded into emlib_ret_t.
 */
#ifndef EM_RETURN_OS_ERROR
#   define EM_RETURN_OS_ERROR(os_code)   (os_code ? \
        EM_STATUS_FROM_OS(os_code) : -1)
#endif


/**
 * @hideinitializer
 * Fold a platform specific error into an emlib_ret_t code.
 *
 * @param e	The platform os error code.
 * @return	emlib_ret_t
 * @warning	Macro implementation; the syserr argument may be evaluated
 *		multiple times.
 */
#if EM_NATIVE_ERR_POSITIVE
#   define EM_STATUS_FROM_OS(e) (e == 0 ? EM_SUCC : e + EM_ERRNO_START_SYS)
#else
#   define EM_STATUS_FROM_OS(e) (e == 0 ? EM_SUCC : EM_ERRNO_START_SYS - e)
#endif

/**
 * @hideinitializer
 * Fold an emlib_ret_t code back to the native platform defined error.
 *
 * @param e	The emlib_ret_t folded platform os error code.
 * @return	em_os_err_type
 * @warning	macro implementation; the statcode argument may be evaluated
 *		multiple times.  If the statcode was not created by 
 *		em_get_os_error or EM_STATUS_FROM_OS, the results are undefined.
 */
#if EM_NATIVE_ERR_POSITIVE
#   define EM_STATUS_TO_OS(e) (e == 0 ? EM_SUCC : e - EM_ERRNO_START_SYS)
#else
#   define EM_STATUS_TO_OS(e) (e == 0 ? EM_SUCC : EM_ERRNO_START_SYS - e)
#endif


/**
 * @defgroup em_errnum EMLIB's Own Error Codes
 * @ingroup EM_ERRNO
 * @{
 */

/**
 * Use this macro to generate error message text for your error code,
 * so that they look uniformly as the rest of the libraries.
 *
 * @param code	The error code
 * @param msg	The error test.
 */
#ifndef EM_BUILD_ERR
#   define EM_BUILD_ERR(code,msg) { code, msg " (" #code ")" }
#endif


/**
 * @hideinitializer
 * Unknown error has been reported.
 */
#define EM_EUNKNOWN	    (EM_ERRNO_START_STATUS + 1)	/* 70001 */
/**
 * @hideinitializer
 * The operation is pending and will be completed later.
 */
#define EM_EPENDING	    (EM_ERRNO_START_STATUS + 2)	/* 70002 */
/**
 * @hideinitializer
 * Too many connecting sockets.
 */
#define EM_ETOOMANYCONN	    (EM_ERRNO_START_STATUS + 3)	/* 70003 */
/**
 * @hideinitializer
 * Invalid argument.
 */
#define EM_EINVAL	    (EM_ERRNO_START_STATUS + 4)	/* 70004 */
/**
 * @hideinitializer
 * Name too long (eg. hostname too long).
 */
#define EM_ENAMETOOLONG	    (EM_ERRNO_START_STATUS + 5)	/* 70005 */
/**
 * @hideinitializer
 * Not found.
 */
#define EM_ENOTFOUND	    (EM_ERRNO_START_STATUS + 6)	/* 70006 */
/**
 * @hideinitializer
 * Not enough memory.
 */
#define EM_ENOMEM	    (EM_ERRNO_START_STATUS + 7)	/* 70007 */
/**
 * @hideinitializer
 * Bug detected!
 */
#define EM_EBUG             (EM_ERRNO_START_STATUS + 8)	/* 70008 */
/**
 * @hideinitializer
 * Operation timed out.
 */
#define EM_ETIMEDOUT        (EM_ERRNO_START_STATUS + 9)	/* 70009 */
/**
 * @hideinitializer
 * Too many objects.
 */
#define EM_ETOOMANY         (EM_ERRNO_START_STATUS + 10)/* 70010 */
/**
 * @hideinitializer
 * Object is busy.
 */
#define EM_EBUSY            (EM_ERRNO_START_STATUS + 11)/* 70011 */
/**
 * @hideinitializer
 * The specified option is not supported.
 */
#define EM_ENOTSUP	    (EM_ERRNO_START_STATUS + 12)/* 70012 */
/**
 * @hideinitializer
 * Invalid operation.
 */
#define EM_EINVALIDOP	    (EM_ERRNO_START_STATUS + 13)/* 70013 */
/**
 * @hideinitializer
 * Operation is cancelled.
 */
#define EM_ECANCELLED	    (EM_ERRNO_START_STATUS + 14)/* 70014 */
/**
 * @hideinitializer
 * Object already exists.
 */
#define EM_EEXISTS          (EM_ERRNO_START_STATUS + 15)/* 70015 */
/**
 * @hideinitializer
 * End of file.
 */
#define EM_EEOF		    (EM_ERRNO_START_STATUS + 16)/* 70016 */
/**
 * @hideinitializer
 * Size is too big.
 */
#define EM_ETOOBIG	    (EM_ERRNO_START_STATUS + 17)/* 70017 */
/**
 * @hideinitializer
 * Error in gethostbyname(). This is a generic error returned when
 * gethostbyname() has returned an error.
 */
#define EM_ERESOLVE	    (EM_ERRNO_START_STATUS + 18)/* 70018 */
/**
 * @hideinitializer
 * Size is too small.
 */
#define EM_ETOOSMALL	    (EM_ERRNO_START_STATUS + 19)/* 70019 */
/**
 * @hideinitializer
 * Ignored
 */
#define EM_EIGNORED	    (EM_ERRNO_START_STATUS + 20)/* 70020 */
/**
 * @hideinitializer
 * IPv6 is not supported
 */
#define EM_EIPV6NOTSUP	    (EM_ERRNO_START_STATUS + 21)/* 70021 */
/**
 * @hideinitializer
 * Unsupported address family
 */
#define EM_EAFNOTSUP	    (EM_ERRNO_START_STATUS + 22)/* 70022 */
/**
 * @hideinitializer
 * Object no longer exists
 */
#define EM_EGONE	    (EM_ERRNO_START_STATUS + 23)/* 70023 */
/**
 * @hideinitializer
 * Socket is stopped
 */
#define EM_ESOCKETSTOP	    (EM_ERRNO_START_STATUS + 24)/* 70024 */

/** @} */   /* em_errnum */

/** @} */   /* EM_ERRNO */


/**
 * EM_ERRNO_START is where EMLIB specific error values start.
 */
#define EM_ERRNO_START		20000

/**
 * EM_ERRNO_SPACE_SIZE is the maximum number of errors in one of 
 * the error/status range below.
 */
#define EM_ERRNO_SPACE_SIZE	50000

/**
 * EM_ERRNO_START_STATUS is where EMLIB specific status codes start.
 * Effectively the error in this class would be 70000 - 119000.
 */
#define EM_ERRNO_START_STATUS	(EM_ERRNO_START + EM_ERRNO_SPACE_SIZE)

/**
 * EM_ERRNO_START_SYS converts platform specific error codes into
 * emlib_ret_t values.
 * Effectively the error in this class would be 120000 - 169000.
 */
#define EM_ERRNO_START_SYS	(EM_ERRNO_START_STATUS + EM_ERRNO_SPACE_SIZE)

/**
 * EM_ERRNO_START_USER are reserved for applications that use error
 * codes along with EMLIB codes.
 * Effectively the error in this class would be 170000 - 219000.
 */
#define EM_ERRNO_START_USER	(EM_ERRNO_START_SYS + EM_ERRNO_SPACE_SIZE)


/*
 * Below are list of error spaces that have been taken so far:
 *  - EMSIP_ERRNO_START		(EM_ERRNO_START_USER)
 *  - EMMEDIA_ERRNO_START	(EM_ERRNO_START_USER + EM_ERRNO_SPACE_SIZE)
 *  - EMSIP_SIMPLE_ERRNO_START	(EM_ERRNO_START_USER + EM_ERRNO_SPACE_SIZE*2)
 *  - EMLIB_UTIL_ERRNO_START	(EM_ERRNO_START_USER + EM_ERRNO_SPACE_SIZE*3)
 *  - EMNATH_ERRNO_START	(EM_ERRNO_START_USER + EM_ERRNO_SPACE_SIZE*4)
 *  - EMMEDIA_AUDIODEV_ERRNO_START (EM_ERRNO_START_USER + EM_ERRNO_SPACE_SIZE*5)
 *  - EM_SSL_ERRNO_START	   (EM_ERRNO_START_USER + EM_ERRNO_SPACE_SIZE*6)
 *  - EMMEDIA_VIDEODEV_ERRNO_START (EM_ERRNO_START_USER + EM_ERRNO_SPACE_SIZE*7)
 */

/* Internal */
void EM_ERRNO_clear_handlers(void);


/****** Internal for EM_PERROR *******/

/**
 * @def em_perror_wrapper_1(arg)
 * Internal function to write log with verbosity 1. Will evaluate to
 * empty expression if EM_LOG_MAX_LEVEL is below 1.
 * @param arg       Log expression.
 */
#if EM_LOG_MAX_LEVEL >= 1
    #define em_perror_wrapper_1(arg)	em_perror_1 arg
    /** Internal function. */
    EM_DECL(void) em_perror_1(const char *sender, emlib_ret_t status, 
			      const char *title_fmt, ...);
#else
    #define em_perror_wrapper_1(arg)
#endif

/**
 * @def em_perror_wrapper_2(arg)
 * Internal function to write log with verbosity 2. Will evaluate to
 * empty expression if EM_LOG_MAX_LEVEL is below 2.
 * @param arg       Log expression.
 */
#if EM_LOG_MAX_LEVEL >= 2
    #define em_perror_wrapper_2(arg)	em_perror_2 arg
    /** Internal function. */
    EM_DECL(void) em_perror_2(const char *sender, emlib_ret_t status, 
			      const char *title_fmt, ...);
#else
    #define em_perror_wrapper_2(arg)
#endif

/**
 * @def em_perror_wrapper_3(arg)
 * Internal function to write log with verbosity 3. Will evaluate to
 * empty expression if EM_LOG_MAX_LEVEL is below 3.
 * @param arg       Log expression.
 */
#if EM_LOG_MAX_LEVEL >= 3
    #define em_perror_wrapper_3(arg)	em_perror_3 arg
    /** Internal function. */
    EM_DECL(void) em_perror_3(const char *sender, emlib_ret_t status, 
			      const char *title_fmt, ...);
#else
    #define em_perror_wrapper_3(arg)
#endif

/**
 * @def em_perror_wrapper_4(arg)
 * Internal function to write log with verbosity 4. Will evaluate to
 * empty expression if EM_LOG_MAX_LEVEL is below 4.
 * @param arg       Log expression.
 */
#if EM_LOG_MAX_LEVEL >= 4
    #define em_perror_wrapper_4(arg)	em_perror_4 arg
    /** Internal function. */
    EM_DECL(void) em_perror_4(const char *sender, emlib_ret_t status, 
			      const char *title_fmt, ...);
#else
    #define em_perror_wrapper_4(arg)
#endif

/**
 * @def em_perror_wrapper_5(arg)
 * Internal function to write log with verbosity 5. Will evaluate to
 * empty expression if EM_LOG_MAX_LEVEL is below 5.
 * @param arg       Log expression.
 */
#if EM_LOG_MAX_LEVEL >= 5
    #define em_perror_wrapper_5(arg)	em_perror_5 arg
    /** Internal function. */
    EM_DECL(void) em_perror_5(const char *sender, emlib_ret_t status, 
			      const char *title_fmt, ...);
#else
    #define em_perror_wrapper_5(arg)
#endif

/**
 * @def em_perror_wrapper_6(arg)
 * Internal function to write log with verbosity 6. Will evaluate to
 * empty expression if EM_LOG_MAX_LEVEL is below 6.
 * @param arg       Log expression.
 */
#if EM_LOG_MAX_LEVEL >= 6
    #define em_perror_wrapper_6(arg)	em_perror_6 arg
    /** Internal function. */
    EM_DECL(void) em_perror_6(const char *sender, emlib_ret_t status, 
			      const char *title_fmt, ...);
#else
    #define em_perror_wrapper_6(arg)
#endif

DECLS_END

#endif	/* __EM_ERRNO_H__ */

