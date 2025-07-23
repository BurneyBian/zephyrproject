/*
* Copyright (c) 2021, Supper Acme SW Platform
* All rights reserved.
*
* Name：audio_codec_config.h
* Identification ：
* Summary：Define the audio codec configure command
*
* Author：zhengyong
* Date：2023-08-16
*
*/

#ifndef __AUDIO_CODEC_CONFIG_H__
#define __AUDIO_CODEC_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "sa_comm_type.h"

typedef struct saACODEC_CTL_VAL_INFO_S {
    SA_S32 s32NumArg;
    SA_S32 s32Argv[64];
} ACODEC_CTL_VAL_INFO_S;

#pragma pack(1)
typedef struct saAUDIO_CODEC_EQ_CONFIG_S
{
    union
    {
        struct
        {
            SA_S8 L_EQ_EN:1;
            SA_S8 L_BAND0_EQ_EN:1;
            SA_S8 R_EQ_EN:1;
            SA_S8 R_BAND0_EQ_EN:1;
            SA_S8 reserved0:4;
        }ctrl;
        SA_S8 s8val;
    }BAND_CTRL;

    union
    {
        struct
        {
            SA_S8 fine_gain:4;
            SA_S8 pre_gain:3;
            SA_S8 reserved1:1;
        }gain;
        SA_S8 s8val;
    }BAND0_GAIN;

    union
    {
        struct
        {
            SA_S8 fine_gain:4;
            SA_S8 pre_gain:3;
            SA_S8 reserved1:1;
        }gain;
        SA_S8 s8val;
    }BAND1_GAIN;

    union
    {
        struct
        {
            SA_S8 fine_gain:4;
            SA_S8 pre_gain:3;
            SA_S8 reserved1:1;
        }gain;
        SA_S8 s8val;
    }BAND2_GAIN;

    union
    {
        struct
        {
            SA_S8 fine_gain:4;
            SA_S8 pre_gain:3;
            SA_S8 reserved1:1;
        }gain;
        SA_S8 s8val;
    }BAND3_GAIN;

    union
    {
        struct
        {
            SA_S8 fine_gain:4;
            SA_S8 pre_gain:3;
            SA_S8 reserved1:1;
        }gain;
        SA_S8 s8val;
    }BAND4_GAIN;

    union
    {
        struct
        {
            SA_S8 fine_gain:4;
            SA_S8 pre_gain:3;
            SA_S8 reserved1:1;
        }gain;
        SA_S8 s8val;
    }BAND5_GAIN;
    union
    {
        struct
        {
            SA_S8 fine_gain:4;
            SA_S8 pre_gain:3;
            SA_S8 reserved1:1;
        }gain;
        SA_S8 s8val;
    }BAND6_GAIN;

    union
    {
        struct
        {
            SA_S8 fine_gain:4;
            SA_S8 pre_gain:3;
            SA_S8 reserved1:1;
        }gain;
        SA_S8 s8val;
    }BAND7_GAIN;

    union
    {
        struct
        {
            SA_S8 fine_gain:4;
            SA_S8 pre_gain:3;
            SA_S8 reserved1:1;
        }gain;
        SA_S8 s8val;
    }BAND8_GAIN;

    union
    {
        struct
        {
            SA_S8 fine_gain:4;
            SA_S8 pre_gain:3;
            SA_S8 reserved1:1;
        }gain;
        SA_S8 s8val;
    }BAND9_GAIN;
}AUDIO_CODEC_EQ_CONFIG_S;
#pragma pack()

typedef AUDIO_CODEC_EQ_CONFIG_S AUDIO_CODEC_DAC_EQ_CONFIG_S;
typedef AUDIO_CODEC_EQ_CONFIG_S AUDIO_CODEC_ADC_EQ_CONFIG_S;

#pragma pack(1)
typedef struct saAUDIO_CODEC_BASS_CONFIG_S
{
    union
    {
        struct
        {
            SA_S8 BASS:4;
            SA_S8 reserved:2;
            SA_S8 BC:1;
            SA_S8 BB:1;
        }ctrl;
        SA_S8 s8val;
    }BASS_CTRL;
}AUDIO_CODEC_BASS_CONFIG_S;
#pragma pack()

#pragma pack(1)
typedef struct saAUDIO_CODEC_TREBLE_CONFIG_S
{
    union
    {
        struct
        {
            SA_S8 TRBL:4;
            SA_S8 reserved0:2;
            SA_S8 TC:1;
            SA_S8 reserved1:1;
        }ctrl;
        SA_S8 s8val;
    }TREBLE_CTRL;
}AUDIO_CODEC_TREBLE_CONFIG_S;
#pragma pack()

#pragma pack(1)
typedef struct saAUDIO_CODEC_FILTER_CONFIG_S
{
    union
    {
        struct
        {
            SA_S8 s8Filter1Enable:1;
            SA_S8 s8Filter2Enable:1;
            SA_S8 s8Filter3Enable:1;
            SA_S8 s8Filter4Enable:1;
            SA_S8 reserved0:4;
        }ctrl;
        SA_S8 s8Val;
    }R3F;

    SA_S8 R40;
    SA_S8 R41;
    SA_S8 R42;
    SA_S8 R43;
    SA_S8 R44;
    SA_S8 R45;
    SA_S8 R46;
    SA_S8 R47;
    SA_S8 R48;
    SA_S8 R49;
    SA_S8 R4A;
    SA_S8 R4B;
    SA_S8 R4C;
    SA_S8 R4D;
    SA_S8 R4E;

    SA_S8 R4F;
    SA_S8 R50;
    SA_S8 R51;
    SA_S8 R52;
    SA_S8 R53;
    SA_S8 R54;
    SA_S8 R55;
    SA_S8 R56;
    SA_S8 R57;
    SA_S8 R58;
    SA_S8 R59;
    SA_S8 R5A;
    SA_S8 R5B;
    SA_S8 R5C;
    SA_S8 R5D;

    SA_S8 R5E;
    SA_S8 R5F;
    SA_S8 R60;
    SA_S8 R61;
    SA_S8 R62;
    SA_S8 R63;
    SA_S8 R64;
    SA_S8 R65;
    SA_S8 R66;
    SA_S8 R67;
    SA_S8 R68;
    SA_S8 R69;
    SA_S8 R6A;
    SA_S8 R6B;
    SA_S8 R6C;

    SA_S8 R6D;
    SA_S8 R6E;
    SA_S8 R6F;
    SA_S8 R70;
    SA_S8 R71;
    SA_S8 R72;
    SA_S8 R73;
    SA_S8 R74;
    SA_S8 R75;
    SA_S8 R76;
    SA_S8 R77;
    SA_S8 R78;
    SA_S8 R79;
    SA_S8 R7A;
    SA_S8 R7B;
}AUDIO_CODEC_FILTER_CONFIG_S;
#pragma pack()


#ifdef __cplusplus
}
#endif


#endif


