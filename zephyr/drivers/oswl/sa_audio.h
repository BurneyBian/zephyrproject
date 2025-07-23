/*
* Copyright (c) 2021, Supper Acme SW Platform
* All rights reserved.
*
* Name：sa_audio.h
* Identification ：
* Summary：Define the general audio params
*
* Author：mengxiang
* Date：2022-07-09
*
*/

#ifndef __SA_AUDIO_H__
#define __SA_AUDIO_H__

#include "sa_comm_type.h"

#define AUDIO_HW_INTERFACE "sa_audio_hw_interface"

typedef enum SA_AUDIO_HW_SOUND_E {
    AUDIO_HW_SOUND_I2S0 = 0,
    AUDIO_HW_SOUND_I2S1,
    AUDIO_HW_SOUND_PDM,
} AUDIO_HW_SOUND_E;

typedef enum SA_AUDIO_STATE_E
{
    AUDIO_STATE_STOP = 0,
    AUDIO_STATE_RUNNING = 1,
    AUDIO_STATE_PAUSE = 2,
    AUDIO_STATE_XRUN = 3,
    AUDIO_STATE_MAX,
} AUDIO_STATE_E;

typedef enum SA_AUDIO_SAMPLE_RATE_E
{
    AUDIO_SAMPLE_RATE_8000 = 8000,      /* 8K samplerate */
    AUDIO_SAMPLE_RATE_12000 = 12000,    /* 12K samplerate */
    AUDIO_SAMPLE_RATE_16000 = 16000,    /* 16K samplerate */
    AUDIO_SAMPLE_RATE_24000 = 24000,    /* 24K samplerate */
    AUDIO_SAMPLE_RATE_32000 = 32000,    /* 32K samplerate */
    AUDIO_SAMPLE_RATE_44100 = 44100,    /* 44.1K samplerate */
    AUDIO_SAMPLE_RATE_48000 = 48000,    /* 48K samplerate */
    AUDIO_SAMPLE_RATE_MAX,
} AUDIO_SAMPLE_RATE_E;

typedef enum SA_AUDIO_BIT_WIDTH_E
{
    AUDIO_BIT_WIDTH_8 = 8,      /* 8bit width */
    AUDIO_BIT_WIDTH_16 = 16,     /* 16bit width */
    AUDIO_BIT_WIDTH_24 = 24,     /* 24bit width */
    AUDIO_BIT_WIDTH_32 = 32,     /* 32bit width */
    AUDIO_BIT_WIDTH_MAX,
} AUDIO_BIT_WIDTH_E;

typedef enum SA_AUDIO_CLK_MODE_E 
{ 
    AUDIO_MODE_I2S_SLAVE = 0,       /* I2S slave mode */ 
    AUDIO_MODE_I2S_MASTER = 1,      /* I2S master mode */ 
    AUDIO_MODE_MAX 
} AUDIO_CLK_MODE_E;

typedef enum SA_AUDIO_SOUND_MODE_E
{
    AUDIO_SOUND_MODE_1_CHAN = 1,      
    AUDIO_SOUND_MODE_2_CHAN = 2,    
    AUDIO_SOUND_MODE_3_CHAN = 3,
    AUDIO_SOUND_MODE_4_CHAN = 4,        
    AUDIO_SOUND_MODE_MAX
} AUDIO_SOUND_MODE_E;

typedef enum SA_AIO_MODE_E
{
	AIO_MODE_I2S_MASTER  = 0,   /* AIO I2S master mode */
	AIO_MODE_I2S_SLAVE,         /* AIO I2S slave mode */
	AI_MODE_PDM_MASTER,			/* AI  PDM master mode */
	AIO_MODE_BUTT
} AIO_MODE_E;

typedef enum
{
	AIO_I2STYPE_I2S = 0,
	AIO_I2STYPE_TDM,
	AIO_PDMTYPE,
} AIO_DTYPE_E;

typedef enum SA_AUDIO_SOUND_PCM_ACCESS_E
{
	AUDIO_SOUND_PCM_NONINTERLEAVED = 0,  
   	AUDIO_SOUND_PCM_INTERLEAVED = 1,   
    AUDIO_SOUND_PCM_BUTT,
} AUDIO_SOUND_PCM_ACCESS_E;


typedef struct SA_AUDIO_CONIFG_S
{
    AUDIO_HW_SOUND_E sound_id;
    AUDIO_SAMPLE_RATE_E sample_rate;//采样率
    AUDIO_BIT_WIDTH_E bits;         //BIT位宽
    SA_S32 format;                  //单声道 双声道 TDM等
    AUDIO_CLK_MODE_E clk_mode;      //时钟master/slave模式选择
    AUDIO_SOUND_PCM_ACCESS_E interleaved; /*交织数据 非交织数据*/
    SA_S32 period_time;             
    SA_S32 period_size;             
    SA_S32 period_count;
    SA_S32 start_threashold;
    SA_S32 stop_threashold;
} AUDIO_CONFIG_S;

/*codec ctl config*/
typedef enum saACODEC_IOCTL_E
{
    SA8900_CTL_ADC_EQ_CONFIG = 0x1,
    SA8900_CTL_DAC_EQ_CONFIG,   
    SA8900_CTL_WINDNOISE_CONFIG,
    SA8900_CTL_BASS_CONFIG,
    SA8900_CTL_TREBLE_CONFIG,
    SA8900_CTL_AO_VOL,
    SA8900_CTL_HARD_REFERENCE_CONFIG,
    SA8900_CTL_AI_MICGAIN_CONFIG,
    
    ACODEC_CTL_BUTT
} ACODEC_IOCTL_E;

typedef enum SA_AUDIO_ERROR_CODE_E
{
    AUDIO_SUCCESS = 0,
    
    //音频基础错误码
    AUDIO_ERROR_CODE_NULL_POINTER = 0xa0000001,

    //音频输入错误码
    AUDIO_ERROR_CODE_PCM_IN_OPEN_FAILED = 0xa0010001,
    AUDIO_ERROR_CODE_PCM_IN_HW_PARAMS_FAILED = 0xa0010002,
    AUDIO_ERROR_CODE_PCM_IN_HW_STATE_ERROR = 0xa0010003,
    AUDIO_ERROR_CODE_PCM_IN_HW_ALSA_INNER_ERROR = 0xa0010004,

    //音频输出错误码
    AUDIO_ERROR_CODE_PCM_OUT_OPEN_FAILED = 0xa0020001,
    AUDIO_ERROR_CODE_PCM_OUT_HW_PARAMS_FAILED = 0xa0020002,
    AUDIO_ERROR_CODE_PCM_OUT_HW_STATE_ERROR = 0xa0020003,
    AUDIO_ERROR_CODE_PCM_OUT_HW_ALSA_INNER_ERROR = 0xa0020004,

    //音频编解码相关错误码

    //音频算法相关错误码

    //音频设备与流相关错误
    AUDIO_ERROR_CODE_DEVICE_ERROR = 0xA0050001,


    AUDIO_ERROR_CODE_MAX
} AUDIO_ERROR_CODE_E;



#endif
