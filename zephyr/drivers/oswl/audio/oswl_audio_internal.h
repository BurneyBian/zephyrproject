/*
* Copyright (c) 2021, Supper Acme SW Platform
* All rights reserved.
*
* Name：sa_lapi_audio.h
* Identification ：
* Summary：Define the general audio params
*
* Author：mengxiang
* Date：2022-07-09
*
*/

#ifndef __OSWL_AUDIO_INTERNAL_H__
#define __OSWL_AUDIO_INTERNAL_H__

#include "sa_comm_type.h"
#include "oswl_audio.h"
#include "oswl.h"
#include <rtthread.h>
#include <rtdevice.h>

#ifdef BSP_USING_STAR_DRLESS
#define SA_AUDIO_MQ_MSG_NUM  200
#else
#define SA_AUDIO_MQ_MSG_NUM  200
#endif

typedef struct saGENERIC_AUDIO_HW_DEVICE_S
{
    AUDIO_HW_DEVICE_S device;
    SA_BOOL isInit;
} GENERIC_AUDIO_HW_DEVICE_S;

typedef struct saGENERIC_AUDIO_STREAM_OUT_S
{
    AUDIO_STREAM_OUT_S stream;
    GENERIC_AUDIO_HW_DEVICE_S *dev;
    AUDIO_CONFIG_S configs;
    AUDIO_STATE_E state;
    SA_CHAR *outName;
    rt_device_t speakerDev;
} GENERIC_AUDIO_STREAM_OUT_S;

typedef struct saGENERIC_AUDIO_STREAM_IN_S
{
    AUDIO_STREAM_IN_S stream;
    GENERIC_AUDIO_HW_DEVICE_S *dev;
    AUDIO_CONFIG_S configs;
    AUDIO_STATE_E state;
    SA_CHAR *inName;
    rt_device_t recordDev;
} GENERIC_AUDIO_STREAM_IN_S;

typedef struct saGENERIC_AUDIO_CTL_HW_DEVICE_S
{
    AUDIO_CTL_HW_DEVICE_S device;
    SA_BOOL isInit;
} GENERIC_AUDIO_CTL_HW_DEVICE_S;

typedef struct saGENERIC_AUDIO_CTL_S
{
    GENERIC_AUDIO_CTL_HW_DEVICE_S *dev;
    AUDIO_STATE_E state;
    //snd_ctl_t *handle;
    SA_CHAR *devName;
    SA_S32 card;
    SA_S32 quiet;
    SA_S32 keep_handle;
} GENERIC_AUDIO_CTL_S;



#endif
