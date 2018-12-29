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

/* Internal */
EM_INLINE(void) em_link_node(em_list_type *prev, em_list_type *next)
{
    ((em_list*)prev)->next = next;
    ((em_list*)next)->prev = prev;
}

EM_IDEF(void ) em_list_insert_tail(em_list_type *list_head, em_list_type *node)
{
    em_list_insert_before(list_head, node);
}

EM_IDEF(void ) em_list_insert_head(em_list_type *list_head, em_list_type *node)
{
    em_list_insert_after(list_head, node);
}


EM_IDEF(void) em_list_insert_after(em_list_type *pos, em_list_type *node)
{
    ((em_list*)node)->prev = pos;
    ((em_list*)node)->next = ((em_list*)pos)->next;
    ((em_list*) ((em_list*)pos)->next) ->prev = node;
    ((em_list*)pos)->next = node;
}


EM_IDEF(void) em_list_insert_before(em_list_type *pos, em_list_type *node)
{
    em_list_insert_after(((em_list*)pos)->prev, node);
}


EM_IDEF(void) em_list_insert_nodes_after(em_list_type *pos, em_list_type *lst)
{
    em_list *lst_last = (em_list *) ((em_list*)lst)->prev;
    em_list *pos_next = (em_list *) ((em_list*)pos)->next;

    em_link_node(pos, lst);
    em_link_node(lst_last, pos_next);
}

EM_IDEF(void) em_list_insert_nodes_before(em_list_type *pos, em_list_type *lst)
{
    em_list_insert_nodes_after(((em_list*)pos)->prev, lst);
}

EM_IDEF(void) em_list_merge_last(em_list_type *lst1, em_list_type *lst2)
{
    if (!em_list_empty(lst2)) {
        em_link_node(((em_list*)lst1)->prev, ((em_list*)lst2)->next);
        em_link_node(((em_list*)lst2)->prev, lst1);
        em_list_init(lst2);
    }
}

EM_IDEF(void) em_list_merge_first(em_list_type *lst1, em_list_type *lst2)
{
    if (!em_list_empty(lst2)) {
        em_link_node(((em_list*)lst2)->prev, ((em_list*)lst1)->next);
        em_link_node(((em_list*)lst1), ((em_list*)lst2)->next);
        em_list_init(lst2);
    }
}

EM_IDEF(void) em_list_erase(em_list_type *node)
{
    em_link_node( ((em_list*)node)->prev, ((em_list*)node)->next);

    /* It'll be safer to init the next/prev fields to itself, to
     * prevent multiple erase() from corrupting the list. See
     * ticket #520 for one sample bug.
     */
    em_list_init(node);
}


EM_IDEF(em_list_type*) em_list_find_node(em_list_type *list, em_list_type *node)
{
    em_list *p = (em_list *) ((em_list*)list)->next;
    while (p != list && p != node)
        p = (em_list *) p->next;

    return p==node ? p : NULL;
}


EM_IDEF(em_list_type*) em_list_search(em_list_type *list, void *value,
        int (*comp)(void *value, const em_list_type *node))
{
    em_list *p = (em_list *) ((em_list*)list)->next;
    while (p != list && (*comp)(value, p) != 0)
        p = (em_list *) p->next;

    return p==list ? NULL : p;
}


EM_IDEF(em_size_t) em_list_size(const em_list_type *list)
{
    const em_list *node = (const em_list*) ((const em_list*)list)->next;
    em_size_t count = 0;

    while (node != list) {
        ++count;
        node = (em_list*)node->next;
    }

    return count;
}

