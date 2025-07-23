/*
* Copyright (c) 2021, Supper Acme SW Platform
* All rights reserved.
*
* Name：lapi_anne_internal.h
* Identification ：
* Summary：Define anne lapi interface
*
* Author：mengxiang
* Date：2022-05-16
*
*/

#ifndef __AUDIO_QUEUE_H__
#define __AUDIO_QUEUE_H__

#include "sa_comm_type.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct sa_audio_list_node_s sa_audio_list_node_t;
typedef struct sa_audio_list_s sa_audio_list_t;

struct sa_audio_list_s {
	int size;
	sa_audio_list_node_t *head;
	sa_audio_list_node_t *tail;
};

struct sa_audio_list_node_s {
	sa_audio_list_node_t *prev;
	sa_audio_list_node_t *next;
	SA_S32 *data;
};

extern sa_audio_list_t *sa_audio_list_new();
extern int sa_audio_list_check_size(sa_audio_list_t *list);
extern int sa_audio_list_pop_front(sa_audio_list_t *list, SA_S32 **item);
extern int sa_audio_list_push_back(sa_audio_list_t *list, SA_S32 *item);
extern int sa_audio_list_clear(sa_audio_list_t *list);
extern int sa_audio_list_free(sa_audio_list_t *list);


#ifdef __cplusplus
}
#endif

#endif // end of __AUDIO_QUEUE_H__
