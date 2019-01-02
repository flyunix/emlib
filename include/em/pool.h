/*
 * File:    pool.h
 * Author:  Liu HongLiang<lhl_nciae@sina.cn>
 * Brief:   Memory Pool
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
 * 2019-01-02 Liu HongLiang<lhl_nciae@sina.cn> created.
 *
 */
#include "em/list.h"
#include "em/lock.h"

/* See if we use pool's alternate API.
 * The alternate API is used e.g. to implement pool debugging.
 */
#if EM_HAS_POOL_ALT_API
#  include "em/pool_alt.h"
#endif


#ifndef __em_pool_H__
#define __em_pool_H__

/**
 * @file pool.h
 * @brief Memory Pool.
 */
DECLS_BEGIN

/**
 * @defgroup em_pool_GROUP Fast Memory Pool
 * @brief
 * Memory pools allow dynamic memory allocation comparable to malloc or the 
 * new in operator C++. Those implementations are not desirable for very
 * high performance applications or real-time systems, because of the 
 * performance bottlenecks and it suffers from fragmentation issue.
 *
 * \section em_pool_INTRO_SEC emlbi's Memory Pool
 * \subsection em_pool_ADVANTAGE_SUBSEC Advantages
 * 
 * emlbi's pool has many advantages over traditional malloc/new operator and
 * over other memory pool implementations, because:
 *  - unlike other memory pool implementation, it allows allocation of
 *    memory chunks of different sizes,
 *  - it's very very fast. 
 *    \n
 *    Memory chunk allocation is not only an O(1) 
 *    operation, but it's also very simple (just 
 *    few pointer arithmetic operations) and it doesn't require locking 
 *    any mutex,
 *  - it's memory efficient.
 *    \n
 *    Pool doesn't keep track individual memory chunks allocated by
 *    applications, so there is no additional overhead needed for each
 *    memory allocation (other than possible additional of few bytes, up to
 *    em_pool_ALIGNMENT-1, for aligning the memory). 
 *    But see the @ref em_pool_CAVEATS_SUBSEC below.
 *  - it prevents memory leaks. 
 *    \n
 *    Memory pool inherently has garbage collection functionality. In fact, 
 *    there is no need to free the chunks allocated from the memory pool.
 *    All chunks previously allocated from the pool will be freed once the
 *    pool itself is destroyed. This would prevent memory leaks that haunt
 *    programmers for decades, and it provides additional performance 
 *    advantage over traditional malloc/new operator.
 *
 * Even more, emlbi's memory pool provides some additional usability and
 * flexibility for applications:
 *  - memory leaks are easily traceable, since memory pool is assigned name,
 *    and application can inspect what pools currently active in the system.
 *  - by design, memory allocation from a pool is not thread safe. We assumed
 *    that a pool will be owned by a higher level object, and thread safety 
 *    should be handled by that object. This enables very fast pool operations
 *    and prevents unnecessary locking operations,
 *  - by default, the memory pool API behaves more like C++ new operator, 
 *    in that it will throw EM_NO_MEMORY_EXCEPTION exception (see 
 *    @ref EM_EXCEPT) when memory chunk allocation fails. This enables failure
 *    handling to be done on more high level function (instead of checking
 *    the result of em_pool_alloc() everytime). If application doesn't like
 *    this, the default behavior can be changed on global basis by supplying 
 *    different policy to the pool factory.
 *  - any memory allocation backend allocator/deallocator may be used. By
 *    default, the policy uses malloc() and free() to manage the pool's block,
 *    but application may use different strategy, for example to allocate
 *    memory blocks from a globally static memory location.
 *
 *
 * \subsection em_pool_PERFORMANCE_SUBSEC Performance
 * 
 * The result of emlbi's memory design and careful implementation is a
 * memory allocation strategy that can speed-up the memory allocations
 * and deallocations by up to <b>30 times</b> compared to standard
 * malloc()/free() (more than 150 million allocations per second on a
 * P4/3.0GHz Linux machine).
 *
 * (Note: your mileage may vary, of course. You can see how much emlbi's
 *  pool improves the performance over malloc()/free() in your target
 *  system by running emlbi-test application).
 *
 *
 * \subsection em_pool_CAVEATS_SUBSEC Caveats
 *
 * There are some caveats though!
 *
 * When creating pool, emlbi requires applications to specify the initial
 * pool size, and as soon as the pool is created, emlbi allocates memory
 * from the system by that size. Application designers MUST choose the 
 * initial pool size carefully, since choosing too big value will result in
 * wasting system's memory.
 *
 * But the pool can grow. Application designer can specify how the
 * pool will grow in size, by specifying the size increment when creating
 * the pool.
 *
 * The pool, however, <b>cannot</b> shrink! Since there is <b>no</b> 
 * function to deallocate memory chunks, there is no way for the pool to 
 * release back unused memory to the system. 
 * Application designers must be aware that constant memory allocations 
 * from pool that has infinite life-time may cause the memory usage of 
 * the application to grow over time.
 *
 *
 * \section em_pool_USING_SEC Using Memory Pool
 *
 * This section describes how to use emlbi's memory pool framework.
 * As we hope the readers will witness, emlbi's memory pool API is quite
 * straightforward. 
 *
 * \subsection em_pool_USING_F Create Pool Factory
 * First, application needs to initialize a pool factory (this normally
 * only needs to be done once in one application). emlbi provides
 * a pool factory implementation called caching pool (see @ref 
 * EM_CACHING_POOL), and it is initialized by calling #em_caching_pool_init().
 *
 * \subsection em_pool_USING_P Create The Pool
 * Then application creates the pool object itself with #em_pool_create(),
 * specifying among other thing the pool factory where the pool should
 * be created from, the pool name, initial size, and increment/expansion
 * size.
 *
 * \subsection em_pool_USING_M Allocate Memory as Required
 * Then whenever application needs to allocate dynamic memory, it would
 * call #em_pool_alloc(), #em_pool_calloc(), or #em_pool_zalloc() to
 * allocate memory chunks from the pool.
 *
 * \subsection em_pool_USING_DP Destroy the Pool
 * When application has finished with the pool, it should call 
 * #em_pool_release() to release the pool object back to the factory. 
 * Depending on the types of the factory, this may release the memory back 
 * to the operating system.
 *
 * \subsection em_pool_USING_Dc Destroy the Pool Factory
 * And finally, before application quites, it should deinitialize the
 * pool factory, to make sure that all memory blocks allocated by the
 * factory are released back to the operating system. After this, of 
 * course no more memory pool allocation can be requested.
 *
 * \subsection em_pool_USING_EX Example
 * Below is a sample complete program that utilizes emlbi's memory pool.
 *
 * \code

   #include <emlbi.h>

   #define THIS_FILE    "pool_sample.c"

   static void my_perror(const char *title, emlib_ret_t status)
   {
        char errmsg[EM_ERR_MSG_SIZE];

	em_strerror(status, errmsg, sizeof(errmsg));
	LOG(1,(THIS_FILE, "%s: %s [status=%d]", title, errmsg, status));
   }

   static void pool_demo_1(em_pool_factory *pfactory)
   {
	unsigned i;
	em_pool_t *pool;

	// Must create pool before we can allocate anything
	pool = em_pool_create(pfactory,	 // the factory
			      "pool1",	 // pool's name
			      4000,	 // initial size
			      4000,	 // increment size
			      NULL);	 // use default callback.
	if (pool == NULL) {
	    my_perror("Error creating pool", EM_ENOMEM);
	    return;
	}

	// Demo: allocate some memory chunks
	for (i=0; i<1000; ++i) {
	    void *p;

	    p = em_pool_alloc(pool, (em_rand()+1) % 512);

	    // Do something with p
	    ...

	    // Look! No need to free p!!
	}

	// Done with silly demo, must free pool to release all memory.
	em_pool_release(pool);
   }

   int main()
   {
	em_caching_pool cp;
	emlib_ret_t status;

        // Must init emlbi before anything else
	status = em_init();
	if (status != EM_SUCC) {
	    my_perror("Error initializing emlbi", status);
	    return 1;
	}

	// Create the pool factory, in this case, a caching pool,
	// using default pool policy.
	em_caching_pool_init(&cp, NULL, 1024*1024 );

	// Do a demo
	pool_demo_1(&cp.factory);

	// Done with demos, destroy caching pool before exiting app.
	em_caching_pool_destroy(&cp);

	return 0;
   }

   \endcode
 *
 * More information about pool factory, the pool object, and caching pool
 * can be found on the Module Links below.
 */


/**
 * @defgroup EM_POOL Memory Pool Object
 * @ingroup em_pool_GROUP
 * @brief
 * The memory pool is an opaque object created by pool factory.
 * Application uses this object to request a memory chunk, by calling
 * #em_pool_alloc(), #em_pool_calloc(), or #em_pool_zalloc(). 
 * When the application has finished using
 * the pool, it must call #em_pool_release() to free all the chunks previously
 * allocated and release the pool back to the factory.
 *
 * A memory pool is initialized with an initial amount of memory, which is
 * called a block. Pool can be configured to dynamically allocate more memory 
 * blocks when it runs out of memory. 
 *
 * The pool doesn't keep track of individual memory allocations
 * by user, and the user doesn't have to free these indidual allocations. This
 * makes memory allocation simple and very fast. All the memory allocated from
 * the pool will be destroyed when the pool itself is destroyed.
 *
 * \section em_pool_tHREADING_SEC More on Threading Policies
 * - By design, memory allocation from a pool is not thread safe. We assumed 
 *   that a pool will be owned by an object, and thread safety should be 
 *   handled by that object. Thus these functions are not thread safe: 
 *	- #em_pool_alloc, 
 *	- #em_pool_calloc, 
 *	- and other pool statistic functions.
 * - Threading in the pool factory is decided by the policy set for the
 *   factory when it was created.
 *
 * \section em_pool_EXAMPLES_SEC Examples
 *
 * For some sample codes on how to use the pool, please see:
 *  - @ref page_emlbi_pool_test
 *
 * @{
 */

/**
 * The type for function to receive callback from the pool when it is unable
 * to allocate memory. The elegant way to handle this condition is to throw
 * exception, and this is what is expected by most of this library 
 * components.
 */
typedef void em_pool_callback(em_pool_t *pool, em_size_t size);

/**
 * This class, which is used internally by the pool, describes a single 
 * block of memory from which user memory allocations will be allocated from.
 */
typedef struct em_pool_block
{
    EM_DECL_LIST_MEMBER(struct em_pool_block);  /**< List's prev and next.  */
    unsigned char    *buf;                      /**< Start of buffer.       */
    unsigned char    *cur;                      /**< Current alloc ptr.     */
    unsigned char    *end;                      /**< End of buffer.         */
} em_pool_block;


/**
 * This structure describes the memory pool. Only implementors of pool factory
 * need to care about the contents of this structure.
 */
struct em_pool_t
{
    EM_DECL_LIST_MEMBER(struct em_pool_t);  /**< Standard list elements.    */

    /** Pool name */
    char	    obj_name[EM_MAX_OBJ_NAME];

    /** Pool factory. */
    em_pool_factory *factory;

    /** Data put by factory */
    void	    *factory_data;

    /** Current capacity allocated by the pool. */
    em_size_t	    capacity;

    /** Size of memory block to be allocated when the pool runs out of memory */
    em_size_t	    increment_size;

    /** List of memory blocks allcoated by the pool. */
    em_pool_block   block_list;

    /** The callback to be called when the pool is unable to allocate memory. */
    em_pool_callback *callback;

};


/**
 * Guidance on how much memory required for initial pool administrative data.
 */
#define EM_POOL_SIZE	        (sizeof(struct em_pool_t))

/** 
 * Pool memory alignment (must be power of 2). 
 */
#ifndef EM_POOL_ALIGNMENT
#   define EM_POOL_ALIGNMENT    4
#endif

/**
 * Create a new pool from the pool factory. This wrapper will call create_pool
 * member of the pool factory.
 *
 * @param factory	    The pool factory.
 * @param name		    The name to be assigned to the pool. The name should 
 *			    not be longer than EM_MAX_OBJ_NAME (32 chars), or 
 *			    otherwise it will be truncated.
 * @param initial_size	    The size of initial memory blocks taken by the pool.
 *			    Note that the pool will take 68+20 bytes for 
 *			    administrative area from this block.
 * @param increment_size    the size of each additional blocks to be allocated
 *			    when the pool is running out of memory. If user 
 *			    requests memory which is larger than this size, then 
 *			    an error occurs.
 *			    Note that each time a pool allocates additional block, 
 *			    it needs em_pool_SIZE more to store some 
 *			    administrative info.
 * @param callback	    Callback to be called when error occurs in the pool.
 *			    If this value is NULL, then the callback from pool
 *			    factory policy will be used.
 *			    Note that when an error occurs during pool creation, 
 *			    the callback itself is not called. Instead, NULL 
 *			    will be returned.
 *
 * @return                  The memory pool, or NULL.
 */
EM_IDECL(em_pool_t*) em_pool_create(em_pool_factory *factory, 
				    const char *name,
				    em_size_t initial_size, 
				    em_size_t increment_size,
				    em_pool_callback *callback);

/**
 * Release the pool back to pool factory.
 *
 * @param pool	    Memory pool.
 */
EM_IDECL(void) em_pool_release( em_pool_t *pool );


/**
 * Release the pool back to pool factory and set the pool pointer to zero.
 *
 * @param ppool	    Pointer to memory pool.
 */
EM_IDECL(void) em_pool_safe_release( em_pool_t **ppool );


/**
 * Get pool object name.
 *
 * @param pool the pool.
 *
 * @return pool name as NULL terminated string.
 */
EM_IDECL(const char *) em_pool_getobjname( const em_pool_t *pool );

/**
 * Reset the pool to its state when it was initialized.
 * This means that if additional blocks have been allocated during runtime, 
 * then they will be freed. Only the original block allocated during 
 * initialization is retained. This function will also reset the internal 
 * counters, such as pool capacity and used size.
 *
 * @param pool the pool.
 */
EM_DECL(void) em_pool_reset( em_pool_t *pool );


/**
 * Get the pool capacity, that is, the system storage that have been allocated
 * by the pool, and have been used/will be used to allocate user requests.
 * There's no guarantee that the returned value represent a single
 * contiguous block, because the capacity may be spread in several blocks.
 *
 * @param pool	the pool.
 *
 * @return the capacity.
 */
EM_IDECL(em_size_t) em_pool_get_capacity( em_pool_t *pool );

/**
 * Get the total size of user allocation request.
 *
 * @param pool	the pool.
 *
 * @return the total size.
 */
EM_IDECL(em_size_t) em_pool_get_used_size( em_pool_t *pool );

/**
 * Allocate storage with the specified size from the pool.
 * If there's no storage available in the pool, then the pool can allocate more
 * blocks if the increment size is larger than the requested size.
 *
 * @param pool	    the pool.
 * @param size	    the requested size.
 *
 * @return pointer to the allocated memory.
 *
 * @see em_pool_ALLOC_T
 */
EM_IDECL(void*) em_pool_alloc( em_pool_t *pool, em_size_t size);

/**
 * Allocate storage  from the pool, and initialize it to zero.
 * This function behaves like em_pool_alloc(), except that the storage will
 * be initialized to zero.
 *
 * @param pool	    the pool.
 * @param count	    the number of elements in the array.
 * @param elem	    the size of individual element.
 *
 * @return pointer to the allocated memory.
 */
EM_IDECL(void*) em_pool_calloc( em_pool_t *pool, em_size_t count, 
				em_size_t elem);


/**
 * Allocate storage from the pool and initialize it to zero.
 *
 * @param pool	    The pool.
 * @param size	    The size to be allocated.
 *
 * @return	    Pointer to the allocated memory.
 *
 * @see em_pool_ZALLOC_T
 */
EM_INLINE(void*) em_pool_zalloc(em_pool_t *pool, em_size_t size)
{
    return em_pool_calloc(pool, 1, size);
}


/**
 * This macro allocates memory from the pool and returns the instance of
 * the specified type. It provides a stricker type safety than em_pool_alloc()
 * since the return value of this macro will be type-casted to the specified
 * type.
 *
 * @param pool	    The pool
 * @param type	    The type of object to be allocated
 *
 * @return	    Memory buffer of the specified type.
 */
#define em_pool_ALLOC_T(pool,type) \
	    ((type*)em_pool_alloc(pool, sizeof(type)))

/**
 * This macro allocates memory from the pool, zeroes the buffer, and 
 * returns the instance of the specified type. It provides a stricker type 
 * safety than em_pool_zalloc() since the return value of this macro will be 
 * type-casted to the specified type.
 *
 * @param pool	    The pool
 * @param type	    The type of object to be allocated
 *
 * @return	    Memory buffer of the specified type.
 */
#define em_pool_ZALLOC_T(pool,type) \
	    ((type*)em_pool_zalloc(pool, sizeof(type)))

/*
 * Internal functions
 */
EM_IDECL(void*) em_pool_alloc_from_block(em_pool_block *block, em_size_t size);
EM_DECL(void*) em_pool_allocate_find(em_pool_t *pool, em_size_t size);


	
/**
 * @}	// EM_POOL
 */

/* **************************************************************************/
/**
 * @defgroup em_pool_factory Pool Factory and Policy
 * @ingroup em_pool_GROUP
 * @brief
 * A pool object must be created through a factory. A factory not only provides
 * generic interface functions to create and release pool, but also provides 
 * strategy to manage the life time of pools. One sample implementation, 
 * \a em_caching_pool, can be set to keep the pools released by application for
 * future use as long as the total memory is below the limit.
 * 
 * The pool factory interface declared in emlbi is designed to be extensible.
 * Application can define its own strategy by creating it's own pool factory
 * implementation, and this strategy can be used even by existing library
 * without recompilation.
 *
 * \section em_pool_factory_ITF Pool Factory Interface
 * The pool factory defines the following interface:
 *  - \a policy: the memory pool factory policy.
 *  - \a create_pool(): create a new memory pool.
 *  - \a release_pool(): release memory pool back to factory.
 *
 * \section em_pool_factory_POL Pool Factory Policy.
 *
 * A pool factory only defines functions to create and release pool and how
 * to manage pools, but the rest of the functionalities are controlled by
 * policy. A pool policy defines:
 *  - how memory block is allocated and deallocated (the default implementation
 *    allocates and deallocate memory by calling malloc() and free()).
 *  - callback to be called when memory allocation inside a pool fails (the
 *    default implementation will throw EM_NO_MEMORY_EXCEPTION exception).
 *  - concurrency when creating and releasing pool from/to the factory.
 *
 * A pool factory can be given different policy during creation to make
 * it behave differently. For example, caching pool factory can be configured
 * to allocate and deallocate from a static/contiguous/preallocated memory 
 * instead of using malloc()/free().
 * 
 * What strategy/factory and what policy to use is not defined by emlbi, but
 * instead is left to application to make use whichever is most efficient for
 * itself.
 *
 * The pool factory policy controls the behaviour of memory factories, and
 * defines the following interface:
 *  - \a block_alloc(): allocate memory block from backend memory mgmt/system.
 *  - \a block_free(): free memory block back to backend memory mgmt/system.
 * @{
 */

/* We unfortunately don't have support for factory policy options as now,
   so we keep this commented at the moment.
enum em_pool_factory_OPTION
{
    em_pool_factory_SERIALIZE = 1
};
*/

/**
 * This structure declares pool factory interface.
 */
typedef struct em_pool_factory_policy
{
    /**
     * Allocate memory block (for use by pool). This function is called
     * by memory pool to allocate memory block.
     * 
     * @param factory	Pool factory.
     * @param size	The size of memory block to allocate.
     *
     * @return		Memory block.
     */
    void* (*block_alloc)(em_pool_factory *factory, em_size_t size);

    /**
     * Free memory block.
     *
     * @param factory	Pool factory.
     * @param mem	Memory block previously allocated by block_alloc().
     * @param size	The size of memory block.
     */
    void (*block_free)(em_pool_factory *factory, void *mem, em_size_t size);

    /**
     * Default callback to be called when memory allocation fails.
     */
    em_pool_callback *callback;

    /**
     * Option flags.
     */
    unsigned flags;

} em_pool_factory_policy;

/**
 * \def EM_NO_MEMORY_EXCEPTION
 * This constant denotes the exception number that will be thrown by default
 * memory factory policy when memory allocation fails.
 *
 * @see EM_NO_MEMORY_EXCEPTION()
 */
EM_DECL_DATA(int) EM_NO_MEMORY_EXCEPTION;

/**
 * Get #EM_NO_MEMORY_EXCEPTION constant.
 */ 
EM_DECL(int) em_NO_MEMORY_EXCEPTION(void);

/**
 * This global variable points to default memory pool factory policy.
 * The behaviour of the default policy is:
 *  - block allocation and deallocation use malloc() and free().
 *  - callback will raise EM_NO_MEMORY_EXCEPTION exception.
 *  - access to pool factory is not serialized (i.e. not thread safe).
 *
 * @see em_pool_factory_get_default_policy
 */
EM_DECL_DATA(em_pool_factory_policy) em_pool_factory_default_policy;


/**
 * Get the default pool factory policy.
 *
 * @return the pool policy.
 */
EM_DECL(const em_pool_factory_policy*) em_pool_factory_get_default_policy(void);


/**
 * This structure contains the declaration for pool factory interface.
 */
struct em_pool_factory
{
    /**
     * Memory pool policy.
     */
    em_pool_factory_policy policy;

    /**
    * Create a new pool from the pool factory.
    *
    * @param factory	The pool factory.
    * @param name	the name to be assigned to the pool. The name should 
    *			not be longer than EM_MAX_OBJ_NAME (32 chars), or 
    *			otherwise it will be truncated.
    * @param initial_size the size of initial memory blocks taken by the pool.
    *			Note that the pool will take 68+20 bytes for 
    *			administrative area from this block.
    * @param increment_size the size of each additional blocks to be allocated
    *			when the pool is running out of memory. If user 
    *			requests memory which is larger than this size, then 
    *			an error occurs.
    *			Note that each time a pool allocates additional block, 
    *			it needs 20 bytes (equal to sizeof(em_pool_block)) to 
    *			store some administrative info.
    * @param callback	Cllback to be called when error occurs in the pool.
    *			Note that when an error occurs during pool creation, 
    *			the callback itself is not called. Instead, NULL 
    *			will be returned.
    *
    * @return the memory pool, or NULL.
    */
    em_pool_t*	(*create_pool)( em_pool_factory *factory,
				const char *name,
				em_size_t initial_size, 
				em_size_t increment_size,
				em_pool_callback *callback);

    /**
     * Release the pool to the pool factory.
     *
     * @param factory	The pool factory.
     * @param pool	The pool to be released.
    */
    void (*release_pool)( em_pool_factory *factory, em_pool_t *pool );

    /**
     * Dump pool status to log.
     *
     * @param factory	The pool factory.
     */
    void (*dump_status)( em_pool_factory *factory, em_bool_t detail );

    /**
     * This is optional callback to be called by allocation policy when
     * it allocates a new memory block. The factory may use this callback
     * for example to keep track of the total number of memory blocks
     * currently allocated by applications.
     *
     * @param factory	    The pool factory.
     * @param size	    Size requested by application.
     *
     * @return		    MUST return EM_TRUE, otherwise the block
     *                      allocation is cancelled.
     */
    em_bool_t (*on_block_alloc)(em_pool_factory *factory, em_size_t size);

    /**
     * This is optional callback to be called by allocation policy when
     * it frees memory block. The factory may use this callback
     * for example to keep track of the total number of memory blocks
     * currently allocated by applications.
     *
     * @param factory	    The pool factory.
     * @param size	    Size freed.
     */
    void (*on_block_free)(em_pool_factory *factory, em_size_t size);

};

/**
 * This function is intended to be used by pool factory implementors.
 * @param factory           Pool factory.
 * @param name              Pool name.
 * @param initial_size      Initial size.
 * @param increment_size    Increment size.
 * @param callback          Callback.
 * @return                  The pool object, or NULL.
 */
EM_DECL(em_pool_t*) em_pool_create_int(	em_pool_factory *factory, 
					const char *name,
					em_size_t initial_size, 
					em_size_t increment_size,
					em_pool_callback *callback);

/**
 * This function is intended to be used by pool factory implementors.
 * @param pool              The pool.
 * @param name              Pool name.
 * @param increment_size    Increment size.
 * @param callback          Callback function.
 */
EM_DECL(void) em_pool_init_int( em_pool_t *pool, 
				const char *name,
				em_size_t increment_size,
				em_pool_callback *callback);

/**
 * This function is intended to be used by pool factory implementors.
 * @param pool      The memory pool.
 */
EM_DECL(void) em_pool_destroy_int( em_pool_t *pool );


/**
 * Dump pool factory state.
 * @param pf	    The pool factory.
 * @param detail    Detail state required.
 */
EM_INLINE(void) em_pool_factory_dump( em_pool_factory *pf,
				      em_bool_t detail )
{
    (*pf->dump_status)(pf, detail);
}

/**
 *  @}	// em_pool_factory
 */

/* **************************************************************************/

/**
 * @defgroup EM_CACHING_POOL Caching Pool Factory
 * @ingroup em_pool_GROUP
 * @brief
 * Caching pool is one sample implementation of pool factory where the
 * factory can reuse memory to create a pool. Application defines what the 
 * maximum memory the factory can hold, and when a pool is released the
 * factory decides whether to destroy the pool or to keep it for future use.
 * If the total amount of memory in the internal cache is still within the
 * limit, the factory will keep the pool in the internal cache, otherwise the
 * pool will be destroyed, thus releasing the memory back to the system.
 *
 * @{
 */

/**
 * Number of unique sizes, to be used as index to the free list.
 * Each pool in the free list is organized by it's size.
 */
#define EM_CACHING_POOL_ARRAY_SIZE	16

/**
 * Declaration for caching pool. Application doesn't normally need to
 * care about the contents of this struct, it is only provided here because
 * application need to define an instance of this struct (we can not allocate
 * the struct from a pool since there is no pool factory yet!).
 */
struct em_caching_pool 
{
    /** Pool factory interface, must be declared first. */
    em_pool_factory factory;

    /** Current factory's capacity, i.e. number of bytes that are allocated
     *  and available for application in this factory. The factory's
     *  capacity represents the size of all pools kept by this factory
     *  in it's free list, which will be returned to application when it
     *  requests to create a new pool.
     */
    em_size_t	    capacity;

    /** Maximum size that can be held by this factory. Once the capacity
     *  has exceeded @a max_capacity, further #em_pool_release() will
     *  flush the pool. If the capacity is still below the @a max_capacity,
     *  #em_pool_release() will save the pool to the factory's free list.
     */
    em_size_t       max_capacity;

    /**
     * Number of pools currently held by applications. This number gets
     * incremented everytime #em_pool_create() is called, and gets
     * decremented when #em_pool_release() is called.
     */
    em_size_t       used_count;

    /**
     * Total size of memory currently used by application.
     */
    em_size_t	    used_size;

    /**
     * The maximum size of memory used by application throughout the life
     * of the caching pool.
     */
    em_size_t	    peak_used_size;

    /**
     * Lists of pools in the cache, indexed by pool size.
     */
    em_list	    free_list[EM_CACHING_POOL_ARRAY_SIZE];

    /**
     * List of pools currently allocated by applications.
     */
    em_list	    used_list;

    /**
     * Internal pool.
     */
    char	    pool_buf[256 * (sizeof(size_t) / 4)];

    /**
     * Mutex.
     */
    em_locker *locker;
};



/**
 * Initialize caching pool.
 *
 * @param ch_pool	The caching pool factory to be initialized.
 * @param policy	Pool factory policy.
 * @param max_capacity	The total capacity to be retained in the cache. When
 *			the pool is returned to the cache, it will be kept in
 *			recycling list if the total capacity of pools in this
 *			list plus the capacity of the pool is still below this
 *			value.
 */
EM_DECL(void) em_caching_pool_init( em_caching_pool *ch_pool, 
				    const em_pool_factory_policy *policy,
				    em_size_t max_capacity);


/**
 * Destroy caching pool, and release all the pools in the recycling list.
 *
 * @param ch_pool	The caching pool.
 */
EM_DECL(void) em_caching_pool_destroy( em_caching_pool *ch_pool );

/**
 * @}	// em_caching_pool
 */

#if EM_FUNCTIONS_ARE_INLINED
#    include "pool_i.h"
#endif

DECLS_END   
#endif	/* __em_pool_H__ */

