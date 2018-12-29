/*
 * File:    string.c
 * Author:  Liu HongLiang<lhl_nciae@sina.cn>
 * Brief:   TEST Module for C String 
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
 * 2018-12-25 Liu HongLiang<lhl_nciae@sina.cn> created.
 *
 */
#include "test.h"
#include "em/list.h"

/**
 * \page page_emlib_list_test Test: Linked List
 *
 * This file provides implementation of \b list_test(). It tests the
 * functionality of the linked-list API.
 *
 * \section list_test_sec Scope of the Test
 *
 * API tested:
 *  - em_list_init()
 *  - em_list_insert_head()
 *  - em_list_insert_tail()
 *  - em_list_insert_before()
 *  - em_list_insert_after()
 *  - em_list_merge_last()
 *  - em_list_empty()
 *  - em_list_insert_nodes_before()
 *  - em_list_erase()
 *  - em_list_find_node()
 *  - em_list_search()
 *
 */

#ifdef INCLUDE_LIST_TEST

#include "emlib.h"

typedef struct list_node
{
    EM_DECL_LIST_MEMBER(struct list_node);
    int value;
} list_node;

static int compare_node(void *value, const em_list_type *nd)
{
    list_node *node = (list_node*)nd;
    return ((long)(em_ssize_t)value == node->value) ? 0 : -1;
}

#define EM_SIGNED_ARRAY_SIZE(a)	((int)EMLIB_ARRAY_SIZE(a))

emlib_ret_t list_test(void)
{
    list_node nodes[4];    // must be even number of nodes
    list_node list;
    list_node list2;
    list_node *p;
    int i; // don't change to unsigned!

    //
    // Test insert_before().
    //
    list.value = (unsigned)-1;
    em_list_init(&list);
    for (i=0; i<EM_SIGNED_ARRAY_SIZE(nodes); ++i) {
        nodes[i].value = i;
        em_list_insert_before(&list, &nodes[i]);
    }
    // check.
    for (i=0, p=list.next; i<EM_SIGNED_ARRAY_SIZE(nodes); ++i, p=p->next) {
        EMLIB_ASSERT(p->value == i);
        if (p->value != i) {
            return -1;
        }
    }

    //
    // Test insert_tail().
    //
    list.value = (unsigned)-1;
    em_list_init(&list);
    for (i=0; i<EM_SIGNED_ARRAY_SIZE(nodes); ++i) {
        nodes[i].value = i;
        em_list_insert_tail(&list, &nodes[i]);
    }
    // check.
    for (i=0, p=list.next; i<EM_SIGNED_ARRAY_SIZE(nodes); ++i, p=p->next) {
        EMLIB_ASSERT(p->value == i);
        if (p->value != i) {
            return -1;
        }
    }


    //
    // Test insert_after()
    //
    em_list_init(&list);
    for (i=EM_SIGNED_ARRAY_SIZE(nodes)-1; i>=0; --i) {
        em_list_insert_after(&list, &nodes[i]);
    }
    // check.
    for (i=0, p=list.next; i<EM_SIGNED_ARRAY_SIZE(nodes); ++i, p=p->next) {
        EMLIB_ASSERT(p->value == i);
        if (p->value != i) {
            return -1;
        }
    }

    //
    // Test insert_head()
    //
    em_list_init(&list);
    for (i=EM_SIGNED_ARRAY_SIZE(nodes)-1; i>=0; --i) {
        em_list_insert_head(&list, &nodes[i]);
    }
    // check.
    for (i=0, p=list.next; i<EM_SIGNED_ARRAY_SIZE(nodes); ++i, p=p->next) {
        EMLIB_ASSERT(p->value == i);
        if (p->value != i) {
            return -1;
        }
    }
    //
    // Test merge_last()
    //
    // Init lists
    em_list_init(&list);
    em_list_init(&list2);
    for (i=0; i<EM_SIGNED_ARRAY_SIZE(nodes)/2; ++i) {
        em_list_insert_before(&list, &nodes[i]);
    }
    for (i=EM_SIGNED_ARRAY_SIZE(nodes)/2; i<EM_SIGNED_ARRAY_SIZE(nodes); ++i) {
        em_list_insert_before(&list2, &nodes[i]);
    }
    // merge
    em_list_merge_last(&list, &list2);
    // check.
    for (i=0, p=list.next; i<EM_SIGNED_ARRAY_SIZE(nodes); ++i, p=p->next) {
        EMLIB_ASSERT(p->value == i);
        if (p->value != i) {
            return -1;
        }
    }
    // check list is empty
    EMLIB_ASSERT( em_list_empty(&list2) );
    if (!em_list_empty(&list2)) {
        return -1;
    }

    // 
    // Check merge_first()
    //
    em_list_init(&list);
    em_list_init(&list2);
    for (i=0; i<EM_SIGNED_ARRAY_SIZE(nodes)/2; ++i) {
        em_list_insert_before(&list, &nodes[i]);
    }
    for (i=EM_SIGNED_ARRAY_SIZE(nodes)/2; i<EM_SIGNED_ARRAY_SIZE(nodes); ++i) {
        em_list_insert_before(&list2, &nodes[i]);
    }
    // merge
    em_list_merge_first(&list2, &list);
    // check (list2).
    for (i=0, p=list2.next; i<EM_SIGNED_ARRAY_SIZE(nodes); ++i, p=p->next) {
        EMLIB_ASSERT(p->value == i);
        if (p->value != i) {
            return -1;
        }
    }
    // check list is empty
    EMLIB_ASSERT( em_list_empty(&list) );
    if (!em_list_empty(&list)) {
        return -1;
    }

    //
    // Test insert_nodes_before()
    //
    // init list
    em_list_init(&list);
    for (i=0; i<EM_SIGNED_ARRAY_SIZE(nodes)/2; ++i) {
        em_list_insert_before(&list, &nodes[i]);
    }
    // chain remaining nodes
    em_list_init(&nodes[EM_SIGNED_ARRAY_SIZE(nodes)/2]);
    for (i=EM_SIGNED_ARRAY_SIZE(nodes)/2+1; i<EM_SIGNED_ARRAY_SIZE(nodes); ++i) {
        em_list_insert_before(&nodes[EM_SIGNED_ARRAY_SIZE(nodes)/2], &nodes[i]);
    }
    // insert nodes
    em_list_insert_nodes_before(&list, &nodes[EM_SIGNED_ARRAY_SIZE(nodes)/2]);
    // check
    for (i=0, p=list.next; i<EM_SIGNED_ARRAY_SIZE(nodes); ++i, p=p->next) {
        EMLIB_ASSERT(p->value == i);
        if (p->value != i) {
            return -1;
        }
    }

    // erase test.
    em_list_init(&list);
    for (i=0; i<EM_SIGNED_ARRAY_SIZE(nodes); ++i) {
        nodes[i].value = i;
        em_list_insert_before(&list, &nodes[i]);
    }
    for (i=EM_SIGNED_ARRAY_SIZE(nodes)-1; i>=0; --i) {
        int j;
        em_list_erase(&nodes[i]);
        for (j=0, p=list.next; j<i; ++j, p=p->next) {
            EMLIB_ASSERT(p->value == j);
            if (p->value != j) {
                return -1;
            }
        }
    }

    // find and search
    em_list_init(&list);
    for (i=0; i<EM_SIGNED_ARRAY_SIZE(nodes); ++i) {
        nodes[i].value = i;
        em_list_insert_before(&list, &nodes[i]);
    }
    for (i=0; i<EM_SIGNED_ARRAY_SIZE(nodes); ++i) {
        p = (list_node*) em_list_find_node(&list, &nodes[i]);
        EMLIB_ASSERT( p == &nodes[i] );
        if (p != &nodes[i]) {
            return -1;
        }
        p = (list_node*) em_list_search(&list, (void*)(em_ssize_t)i, 
                &compare_node);
        EMLIB_ASSERT( p == &nodes[i] );
        if (p != &nodes[i]) {
            return -1;
        }
    }
    return 0;
}

#else
/* To prevent warning about "translation unit is empty"
 * when this test is disabled. 
 */
int dummy_list_test;
#endif	/* INCLUDE_LIST_TEST */


