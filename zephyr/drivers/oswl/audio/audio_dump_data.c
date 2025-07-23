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
#include "oswl_audio.h"
#include "audio_queue.h"

typedef struct saAUDIO_DUMP_HANDLD_S
{
    SA_S32 dataSize;   //当前场景下每次压入的数据块长度
    SA_CHAR *fileName;  //当前场景下需要DUMP的文件名
    oswl_thread_t dumpThread;
    oswl_mutex_t dumpMutex;
    SA_S32 dumpRunning;
    sa_audio_list_t *dataQueue;
} AUDIO_DUMP_HANDLE_S;

static AUDIO_DUMP_HANDLE_S audioDumpHandle = {0};

SA_VOID oswl_audio_dump_exit(SA_VOID)
{
    audioDumpHandle.dumpRunning = 0;
}

SA_S32 oswl_audio_dump_attach(SA_S8 *data)
{
    SA_S32 size = 0;
    if (NULL == data) {
        return -1;
    }

    SA_S32 *temp_data = NULL;
    temp_data = (SA_S32 *)rt_noncache_malloc(audioDumpHandle.dataSize);
    if (SA_NULL == temp_data) {
        OSWL_LOG_E("temp_data is null\n");
        return -1;
    }
    memcpy((SA_S8 *)temp_data, data, audioDumpHandle.dataSize);

    oswl_mutex_lock(&audioDumpHandle.dumpMutex);
    size = sa_audio_list_check_size(audioDumpHandle.dataQueue);
    if (size > 500) {
        OSWL_LOG_E("audio_attach_audio_result_dump_task size overrun!\n");
    }
    sa_audio_list_push_back(audioDumpHandle.dataQueue, temp_data);
    oswl_mutex_unlock(&audioDumpHandle.dumpMutex);
    return 0;
}

static void sa_audio_dump_task(void* param)
{
    SA_S32 size = 0;
    SA_S32 sa_fd = 0;
    SA_S32 ret = 0;

    oswl_mutex_init(&audioDumpHandle.dumpMutex);
    audioDumpHandle.dataQueue = sa_audio_list_new();

    OSWL_LOG_D("-----audio_data_task in-----\n");

    sa_fd = open(audioDumpHandle.fileName, O_RDWR | O_CREAT | O_TRUNC);
    if (sa_fd < 0) {
        OSWL_LOG_E("sa_audio_dump_task fileName open err!\n");
        return;
    }

    audioDumpHandle.dumpRunning = 1;

    while (audioDumpHandle.dumpRunning) {
        oswl_mutex_lock(&audioDumpHandle.dumpMutex);
        size = sa_audio_list_check_size(audioDumpHandle.dataQueue);
        oswl_mutex_unlock(&audioDumpHandle.dumpMutex);

        if (size > 0) {
            SA_S32 *my_data = NULL;
            oswl_mutex_lock(&audioDumpHandle.dumpMutex);
            sa_audio_list_pop_front(audioDumpHandle.dataQueue, &my_data);
            oswl_mutex_unlock(&audioDumpHandle.dumpMutex);
            if (SA_NULL == my_data) {
                OSWL_LOG_E("my_data NULL\n");
                continue;
            }
            ret = write(sa_fd, (char *)my_data, audioDumpHandle.dataSize);
            if (ret <= 0) {
                OSWL_LOG_E("write err!\n");
            }
            rt_noncache_free(my_data);
        } else {
            oswl_mdelay(100);
        }
    }

    close(sa_fd);
    sa_audio_list_free(audioDumpHandle.dataQueue);
    oswl_mutex_destory(&audioDumpHandle.dumpMutex);
}

SA_VOID oswl_audio_dump_init(SA_AUDIO_DUMP_CONFIG *conf)
{
    OSWL_LOG_I("oswl_audio_dump_init 0325\n");
    memset(&audioDumpHandle, 0, sizeof(AUDIO_DUMP_HANDLE_S));

    audioDumpHandle.dataSize = conf->data_size;
    audioDumpHandle.fileName = conf->file_name;
    
    audioDumpHandle.dumpThread.callback = sa_audio_dump_task;
    audioDumpHandle.dumpThread.name = "adump";
    audioDumpHandle.dumpThread.params = NULL;
    audioDumpHandle.dumpThread.stack_size = 4096;
    audioDumpHandle.dumpThread.priority = 20;
    audioDumpHandle.dumpThread.tick = 20;
    oswl_thread_init(&audioDumpHandle.dumpThread);
}