/*
 * File:    exception.h
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
 * 2010-01-04 Liu HongLiang<lhl_nciae@sina.cn> created.
 *
 */

#ifndef __EM_EXCEPTION_H__
#define __EM_EXCEPTION_H__

/**
 * @file except.h
 * @brief Exception Handling in C.
 */

#include <em/types.h>
#include <em/log.h>


DECLS_BEGIN

/**
 * @defgroup EM_EXCEPT Exception Handling
 * @ingroup EM_MISC
 * @{
 *
 * \section em_except_sample_sec Quick Example
 *
 * For the impatient, take a look at some examples:
 *  - @ref page_emlib_samples_except_c
 *  - @ref page_emlib_exception_test
 *
 * \section em_except_except Exception Handling
 *
 * This module provides exception handling syntactically similar to C++ in
 * C language. Otherwise it will use setjmp() and longjmp().
 *
 * On some platforms where setjmp/longjmp is not available, setjmp/longjmp 
 * implementation is provided. See <em/compat/setjmp.h> for compatibility.
 *
 * The exception handling mechanism is completely thread safe, so the exception
 * thrown by one thread will not interfere with other thread.
 *
 * The exception handling constructs are similar to C++. The blocks will be
 * constructed similar to the following sample:
 *
 * \verbatim
   #define NO_MEMORY     1
   #define SYNTAX_ERROR  2
  
   int sample1()
   {
      EM_USE_EXCEPTION;  // declare local exception stack.
  
      EM_TRY {
        ...// do something..
      }
      EM_CATCH(NO_MEMORY) {
        ... // handle exception 1
      }
      EM_END;
   }

   int sample2()
   {
      EM_USE_EXCEPTION;  // declare local exception stack.
  
      EM_TRY {
        ...// do something..
      }
      EM_CATCH_ANY {
         if (EM_GET_EXCEPTION() == NO_MEMORY)
	    ...; // handle no memory situation
	 else if (EM_GET_EXCEPTION() == SYNTAX_ERROR)
	    ...; // handle syntax error
      }
      EM_END;
   }
   \endverbatim
 *
 * The above sample uses hard coded exception ID. It is @b strongly
 * recommended that applications request a unique exception ID instead
 * of hard coded value like above.
 *
 * \section em_except_reg Exception ID Allocation
 *
 * To ensure that exception ID (number) are used consistently and to
 * prevent ID collisions in an application, it is strongly suggested that 
 * applications allocate an exception ID for each possible exception
 * type. As a bonus of this process, the application can identify
 * the name of the exception when the particular exception is thrown.
 *
 * Exception ID management are performed with the following APIs:
 *  - #em_exception_id_alloc().
 *  - #em_exception_id_free().
 *  - #em_exception_id_name().
 *
 *
 * EMLIB itself automatically allocates one exception id, i.e.
 * #EM_NO_MEMORY_EXCEPTION which is declared in <em/pool.h>. This exception
 * ID is raised by default pool policy when it fails to allocate memory.
 *
 * CAVEATS:
 *  - unlike C++ exception, the scheme here won't call destructors of local
 *    objects if exception is thrown. Care must be taken when a function
 *    hold some resorce such as pool or mutex etc.
 *  - You CAN NOT make nested exception in one single function without using
 *    a nested EM_USE_EXCEPTION. Samples:
  \verbatim
	void wrong_sample()
	{
	    EM_USE_EXCEPTION;

	    EM_TRY {
		// Do stuffs
		...
	    }
	    EM_CATCH_ANY {
		// Do other stuffs
		....
		..

		// The following block is WRONG! You MUST declare 
		// EM_USE_EXCEPTION once again in this block.
		EM_TRY {
		    ..
		}
		EM_CATCH_ANY {
		    ..
		}
		EM_END;
	    }
	    EM_END;
	}

  \endverbatim

 *  - You MUST NOT exit the function inside the EM_TRY block. The correct way
 *    is to return from the function after EM_END block is executed. 
 *    For example, the following code will yield crash not in this code,
 *    but rather in the subsequent execution of EM_TRY block:
  \verbatim
        void wrong_sample()
	{
	    EM_USE_EXCEPTION;

	    EM_TRY {
		// do some stuffs
		...
		return;	        <======= DO NOT DO THIS!
	    }
	    EM_CATCH_ANY {
	    }
	    EM_END;
	}
  \endverbatim
  
 *  - You can not provide more than EM_CATCH or em_CATCH_ANY nor use EM_CATCH
 *    and EM_CATCH_ANY for a single em_TRY.
 *  - Exceptions will always be caught by the first handler (unlike C++ where
 *    exception is only caught if the type matches.

 * \section EM_EX_KEYWORDS Keywords
 *
 * \subsection EM_THROW em_THROW(expression)
 * Throw an exception. The expression thrown is an integer as the result of
 * the \a expression. This keyword can be specified anywhere within the 
 * program.
 *
 * \subsection EM_USE_EXCEPTION em_USE_EXCEPTION
 * Specify this in the variable definition section of the function block 
 * (or any blocks) to specify that the block has \a EM_TRY/em_CATCH exception 
 * block. 
 * Actually, this is just a macro to declare local variable which is used to
 * push the exception state to the exception stack.
 * Note: you must specify EM_USE_EXCEPTION as the last statement in the
 * local variable declarations, since it may evaluate to nothing.
 *
 * \subsection EM_TRY em_TRY
 * The \a EM_TRY keyword is typically followed by a block. If an exception is
 * thrown in this block, then the execution will resume to the \a EM_CATCH 
 * handler.
 *
 * \subsection EM_CATCH em_CATCH(expression)
 * The \a EM_CATCH is normally followed by a block. This block will be executed
 * if the exception being thrown is equal to the expression specified in the
 * \a EM_CATCH.
 *
 * \subsection EM_CATCH_ANY em_CATCH_ANY
 * The \a EM_CATCH is normally followed by a block. This block will be executed
 * if any exception was raised in the TRY block.
 *
 * \subsection EM_END em_END
 * Specify this keyword to mark the end of \a EM_TRY / \a em_CATCH blocks.
 *
 * \subsection EM_GET_EXCEPTION em_GET_EXCEPTION(void)
 * Get the last exception thrown. This macro is normally called inside the
 * \a EM_CATCH or \a em_CATCH_ANY block, altough it can be used anywhere where
 * the \a EM_USE_EXCEPTION definition is in scope.
 *
 * 
 * \section em_except_examples_sec Examples
 *
 * For some examples on how to use the exception construct, please see:
 *  - @ref page_emlib_samples_except_c
 *  - @ref page_emlib_exception_test
 */

#  include <setjmp.h>
   typedef jmp_buf em_jmp_buf;
#  ifndef em_setjmp
#    define em_setjmp(buf)	setjmp(buf)
#  endif
#  ifndef em_longjmp
#    define em_longjmp(buf,d)	longjmp(buf,d)
#  endif

/**
 * Allocate a unique exception id.
 * Applications don't have to allocate a unique exception ID before using
 * the exception construct. However, by doing so it ensures that there is
 * no collisions of exception ID.
 *
 * As a bonus, when exception number is acquired through this function,
 * the library can assign name to the exception (only if 
 * EM_HAS_EXCEPTION_NAMES is enabled (default is yes)) and find out the
 * exception name when it catches an exception.
 *
 * @param name      Name to be associated with the exception ID.
 * @param id        Pointer to receive the ID.
 *
 * @return          EM_SUCCESS on success or em_ETOOMANY if the library 
 *                  is running out out ids.
 */
EM_DECL(emlib_ret_t) em_exception_id_alloc(const char *name,
                                           em_exception_id_t *id);

/**
 * Free an exception id.
 *
 * @param id        The exception ID.
 *
 * @return          EM_SUCCESS or the appropriate error code.
 */
EM_DECL(emlib_ret_t) em_exception_id_free(em_exception_id_t id);

/**
 * Retrieve name associated with the exception id.
 *
 * @param id        The exception ID.
 *
 * @return          The name associated with the specified ID.
 */
EM_DECL(const char*) em_exception_id_name(em_exception_id_t id);


/*****************************************************************************
 **
 ** IMPLEMENTATION OF EXCEPTION USING GENERIC SETJMP/LONGJMP
 **
 ****************************************************************************/

/**
 * This structure (which should be invisible to user) manages the TRY handler
 * stack.
 */
struct em_exception_state_t
{    
    em_jmp_buf state;                   /**< jmp_buf.                    */
    struct em_exception_state_t *prev;  /**< Previous state in the list. */
};

/**
 * Throw exception.
 * @param id    Exception Id.
 */
EM_DECL_NO_RETURN(void) 
em_throw_exception_(em_exception_id_t id) EM_ATTR_NORETURN;

/**
 * Push exception handler.
 */
EM_DECL(void) em_push_exception_handler_(struct em_exception_state_t *rec);

/**
 * Pop exception handler.
 */
EM_DECL(void) em_pop_exception_handler_(struct em_exception_state_t *rec);

/**
 * Declare that the function will use exception.
 * @hideinitializer
 */
#define EM_USE_EXCEPTION    struct em_exception_state_t em_x_except__; int em_x_code__

/**
 * Start exception specification block.
 * @hideinitializer
 */
#define EM_TRY		    if (1) { \
				em_push_exception_handler_(&em_x_except__); \
				em_x_code__ = em_setjmp(em_x_except__.state); \
				if (em_x_code__ == 0)
/**
 * Catch the specified exception Id.
 * @param id    The exception number to catch.
 * @hideinitializer
 */
#define EM_CATCH(id)	    else if (em_x_code__ == (id))

/**
 * Catch any exception number.
 * @hideinitializer
 */
#define EM_CATCH_ANY	    else

/**
 * End of exception specification block.
 * @hideinitializer
 */
#define EM_END			em_pop_exception_handler_(&em_x_except__); \
			    } else {}

/**
 * Throw exception.
 * @param exception_id  The exception number.
 * @hideinitializer
 */
#define EM_THROW(exception_id)	em_throw_exception_(exception_id)

/**
 * Get current exception.
 * @return      Current exception code.
 * @hideinitializer
 */
#define EM_GET_EXCEPTION()	(em_x_code__)


DECLS_END


#endif	/* __EM_EXCEPTION_H__ */

