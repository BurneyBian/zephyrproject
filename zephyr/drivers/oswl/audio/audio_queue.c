/*
* Copyright (c) 2021, Supper Acme SW Platform
* All rights reserved.
*
* Name anne_tool.c
* Identification 
* Summary Define anne tool
*
* Author mengxiang
* Date 2022-05-16
*
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <math.h>

#include "oswl.h"
#include "audio_queue.h"
#include <rtthread.h>


sa_audio_list_t *sa_audio_list_new()
{
    sa_audio_list_t *list = rt_noncache_malloc(sizeof(sa_audio_list_t));
    if (NULL == list) {
        return NULL;
    }

    list->size = 0;
    list->head = rt_noncache_malloc(sizeof(sa_audio_list_node_t));
    memset(list->head, 0, sizeof(sa_audio_list_node_t));
    list->tail = rt_noncache_malloc(sizeof(sa_audio_list_node_t));
    memset(list->tail, 0, sizeof(sa_audio_list_node_t));

    list->head->prev = NULL;
    list->head->next = list->tail;

    list->tail->prev = list->head;
    list->tail->next = NULL;

    return list;
}

int sa_audio_list_check_size(sa_audio_list_t *list)
{
    if (NULL == list) {
        return 0;
    }
    return list->size;
}

int sa_audio_list_pop_front(sa_audio_list_t *list, SA_S32 **item)
{
    if ((NULL == list) || (NULL == item)) {
        OSWL_LOG_E("sa_audio_list_pop_front NULL\n");
        return -1;
    }
    if (list->size <= 0) {
        OSWL_LOG_E("sa_audio_list_pop_front size < 0\n");
        return -1;
    }

    sa_audio_list_node_t *first = list->head->next;
    *item = first->data;
    list->head->next = first->next;
    first->next->prev = list->head;

    list->size--;
    rt_noncache_free(first);
    return 0;
}

int sa_audio_list_push_back(sa_audio_list_t *list, SA_S32 *item)
{
    if (NULL == list) {
        return -1;
    }
    sa_audio_list_node_t *node = (sa_audio_list_node_t *)rt_noncache_malloc(sizeof(sa_audio_list_node_t));
    sa_audio_list_node_t *last = list->tail->prev;

    node->data = item;
    node->prev = last;
    node->next = list->tail;
    list->tail->prev = node;
    last->next = node;

    list->size++;
    return 0;
}

int sa_audio_list_clear(sa_audio_list_t *list)
{
    if (NULL == list) {
        return -1;
    }

    sa_audio_list_node_t *node = list->head->next;
    sa_audio_list_node_t *tmp;

    //node = list->head;
    while (node != list->tail) {
        rt_noncache_free(node->data);
        tmp = node;
        node = node->next;
        rt_noncache_free(tmp);
    }

    list->size = 0;
    list->head->next = list->tail;
    list->tail->prev = list->head;
    return 0;
}

int sa_audio_list_free(sa_audio_list_t *list)
{
    if (NULL == list) {
        return -1;
    }

    sa_audio_list_clear(list);
    rt_noncache_free(list->head);
    rt_noncache_free(list->tail);
    rt_noncache_free(list);
    return 0;
}
