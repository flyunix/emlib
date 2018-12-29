/*
 * File:    list.h
 * Author:  Liu HongLiang<lhl_nciae@sina.cn>
 * Brief:   Linked List data structure.
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
 * 2018-12-28 Liu HongLiang<lhl_nciae@sina.cn> created.
 *
 */
#ifndef __EM_LIST_H__
#define __EM_LIST_H__

/**
 * @file list.h
 * @brief Linked List data structure.
 */

#include "em/types.h"

DECLS_BEGIN
/*
 * @defgroup EM_DS Data Structure.
 */

/**
 * @defgroup EM_LIST Linked List
 * @ingroup EM_DS
 * @{
 *
 * List in EMLIB is implemented as doubly-linked list, and it won't require
 * dynamic memory allocation (just as all emlib data structures). The list here
 * should be viewed more like a low level C list instead of high level C++ list
 * (which normally are easier to use but require dynamic memory allocations),
 * therefore all caveats with C list apply here too (such as you can NOT put
 * a node in more than one lists).
 *
 * \section _list_example_sec Examples
 *
 * See below for examples on how to manipulate linked list:
 *  - @ref page_emlib_samples_list_c
 *  - @ref page_emlib_list_test
 */


/**
 * Use this macro in the start of the structure declaration to declare that
 * the structure can be used in the linked list operation. This macro simply
 * declares additional member @a prev and @a next to the structure.
 * @hideinitializer
 */
#define EM_DECL_LIST_MEMBER(type)                       \
                                   /** List @a prev. */ \
                                   type *prev;          \
                                   /** List @a next. */ \
                                   type *next 


/**
 * This structure describes generic list node and list. The owner of this list
 * must initialize the 'value' member to an appropriate value (typically the
 * owner itself).
 */
struct em_list
{
    EM_DECL_LIST_MEMBER(void);
} EM_ATTR_MAY_ALIAS; /* may_alias avoids warning with gcc-4.4 -Wall -O2 */


/**
 * Initialize the list.
 * Initially, the list will have no member, and function em_list_empty() will
 * always return nonzero (which indicates TRUE) for the newly initialized 
 * list.
 *
 * @param node The list head.
 */
EM_INLINE(void) em_list_init(em_list_type * node)
{
    ((em_list*)node)->next = ((em_list*)node)->prev = node;
}


/**
 * Check that the list is empty.
 *
 * @param node	The list head.
 *
 * @return Non-zero if the list is empty, or zero if it is not empty.
 *
 */
EM_INLINE(int) em_list_empty(const em_list_type * node)
{
    return ((em_list*)node)->next == node;
}

/**
 * Insert the node to the list tail
 *
 * @param pos	The element to which the node will be inserted tail. 
 * @param node	The element to be inserted.
 *
 * @return void.
 */
EM_IDECL(void ) em_list_insert_tail(em_list_type *list_head, em_list_type *node);

/**
 * Insert the node to the list head 
 *
 * @param pos	The element to which the node will be inserted head. 
 * @param node	The element to be inserted.
 *
 * @return void.
 */
EM_IDECL(void) em_list_insert_head(em_list_type *list_head, em_list_type *node);

/**
 * Insert the node to the list before the specified element position.
 *
 * @param pos	The element to which the node will be inserted before. 
 * @param node	The element to be inserted.
 *
 * @return void.
 */
EM_IDECL(void)	em_list_insert_before(em_list_type *pos, em_list_type *node);


/**
 * Insert the node to the back of the list. This is just an alias for
 * #em_list_insert_before().
 *
 * @param list	The list. 
 * @param node	The element to be inserted.
 */
EM_INLINE(void) em_list_push_back(em_list_type *list, em_list_type *node)
{
    em_list_insert_before(list, node);
}


/**
 * Inserts all nodes in \a nodes to the target list.
 *
 * @param lst	    The target list.
 * @param nodes	    Nodes list.
 */
EM_IDECL(void) em_list_insert_nodes_before(em_list_type *lst,
        em_list_type *nodes);

/**
 * Insert a node to the list after the specified element position.
 *
 * @param pos	    The element in the list which will precede the inserted 
 *		    element.
 * @param node	    The element to be inserted after the position element.
 *
 * @return void.
 */
EM_IDECL(void) em_list_insert_after(em_list_type *pos, em_list_type *node);


/**
 * Insert the node to the front of the list. This is just an alias for
 * #em_list_insert_after().
 *
 * @param list	The list. 
 * @param node	The element to be inserted.
 */
EM_INLINE(void) em_list_push_front(em_list_type *list, em_list_type *node)
{
    em_list_insert_after(list, node);
}


/**
 * Insert all nodes in \a nodes to the target list.
 *
 * @param lst	    The target list.
 * @param nodes	    Nodes list.
 */
EM_IDECL(void) em_list_insert_nodes_after(em_list_type *lst,
        em_list_type *nodes);


/**
 * Remove elements from the source list, and insert them to the destination
 * list. The elements of the source list will occupy the
 * front elements of the target list. Note that the node pointed by \a list2
 * itself is not considered as a node, but rather as the list descriptor, so
 * it will not be inserted to the \a list1. The elements to be inserted starts
 * at \a list2->next. If \a list2 is to be included in the operation, use
 * \a em_list_insert_nodes_before.
 *
 * @param list1	The destination list.
 * @param list2	The source list.
 *
 * @return void.
 */
EM_IDECL(void) em_list_merge_first(em_list_type *list1, em_list_type *list2);


/**
 * Remove elements from the second list argument, and insert them to the list 
 * in the first argument. The elements from the second list will be appended
 * to the first list. Note that the node pointed by \a list2
 * itself is not considered as a node, but rather as the list descriptor, so
 * it will not be inserted to the \a list1. The elements to be inserted starts
 * at \a list2->next. If \a list2 is to be included in the operation, use
 * \a em_list_insert_nodes_before.
 *
 * @param list1	    The element in the list which will precede the inserted 
 *		    element.
 * @param list2	    The element in the list to be inserted.
 *
 * @return void.
 */
EM_IDECL(void) em_list_merge_last( em_list_type *list1, em_list_type *list2);


/**
 * Erase the node from the list it currently belongs.
 *
 * @param node	    The element to be erased.
 */
EM_IDECL(void) em_list_erase(em_list_type *node);


/**
 * Find node in the list.
 *
 * @param list	    The list head.
 * @param node	    The node element to be searched.
 *
 * @return The node itself if it is found in the list, or NULL if it is not 
 *         found in the list.
 */
EM_IDECL(em_list_type*) em_list_find_node(em_list_type *list, 
        em_list_type *node);


/**
 * Search the list for the specified value, using the specified comparison
 * function. This function iterates on nodes in the list, started with the
 * first node, and call the user supplied comparison function until the
 * comparison function returns ZERO.
 *
 * @param list	    The list head.
 * @param value	    The user defined value to be passed in the comparison 
 *		    function
 * @param comp	    The comparison function, which should return ZERO to 
 *		    indicate that the searched value is found.
 *
 * @return The first node that matched, or NULL if it is not found.
 */
EM_IDECL(em_list_type*) em_list_search(em_list_type *list, void *value,
        int (*comp)(void *value, const em_list_type *node));


/**
 * Traverse the list to get the number of elements in the list.
 *
 * @param list	    The list head.
 *
 * @return	    Number of elements.
 */
EM_IDECL(em_size_t) em_list_size(const em_list_type *list);

/**
 * @}
 */

#if _FUNCTIONS_ARE_INLINED
#  include "list_i.h"
#endif

DECLS_END
#endif	/* __EM_LIST_H__ */

