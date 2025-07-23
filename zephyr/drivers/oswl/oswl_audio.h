/*
* Copyright (c) 2021, Supper Acme SW Platform
* All rights reserved.
*
* Name：oswl_audio.h
* Identification ：
* Summary：Define the general audio params
*
* Author：mengxiang
* Date：2022-07-09
*
*/

#ifndef __OSWL_AUDIO_H__
#define __OSWL_AUDIO_H__

#include "sa_comm_type.h"
#include "sa_audio.h"

typedef SA_S32 audio_device_t;
typedef SA_S32 audio_io_handle_t;

typedef struct SA_AUDIO_DUMP_CONFIG_S
{
    SA_S32 data_size;
    SA_CHAR *file_name;
} SA_AUDIO_DUMP_CONFIG;


struct SA_AUDIO_STREAM_S
{
    /**
     * Return the sample rate in Hz - eg. 16000
     */
    SA_S32 (*get_sample_rate)(const struct SA_AUDIO_STREAM_S *stream);

    // /**
    //  * Return the sample rate in Hz
    //  */
    // SA_S32 (*set_sample_rate)(struct SA_AUDIO_STREAM_S *stream, AUDIO_SAMPLE_RATE_E rate);

    /**
     * Return size of input/output buffer in bytes for this stream -eg 4800
     */
    SA_S32 (*get_buffer_size)(const struct SA_AUDIO_STREAM_S *stream);

    /**
     * Return the channels mask in terms of AUDIO_SOUND_MODE_E
     */
    SA_S32 (*get_channels)(const struct SA_AUDIO_STREAM_S *stream);

    // /**
    //  * Set the audio format -eg AUDIO_BIT_WIDTH_16
    //  */
    // SA_S32 (*set_format)(const struct SA_AUDIO_STREAM_S *stream, AUDIO_BIT_WIDTH_E format);

    /**
     * Return the audio format in terms of AUDIO_BIT_WIDTH_E
     */
    AUDIO_BIT_WIDTH_E (*get_format)(const struct SA_AUDIO_STREAM_S *stream);
};

typedef struct SA_AUDIO_STREAM_S AUDIO_STREAM_S;

struct SA_AUDIO_STREAM_OUT_S
{
    /**
     * Common methods of the audio stream out. This must be the first member of SA_AUDIO_STREAM_OUT_S
     * as users of this structure will cast a SA_AUDIO_STREAM_S to SA_AUDIO_STREAM_OUT_S pointer in 
     * contexts where it's know the SA_AUDIO_STREAM_S reference an SA_AUDIO_STREAM_OUT_S
     */
    struct SA_AUDIO_STREAM_S common;

    /**
     * Return the audio hardware driver estimated latency in milliseconds
     */
    SA_S32 (*get_latency)(const struct SA_AUDIO_STREAM_OUT_S *stream);

    /**
     * This method serves as adirect interface with hardware,
     * allowing you to directly set the volume.
     */
    SA_S32 (*set_volume)(const struct SA_AUDIO_STREAM_OUT_S *stream, SA_F32 left, SA_F32 right);

    /**
     * Write audio buffer to hardware
     */
    SA_S32 (*write)(const struct SA_AUDIO_STREAM_OUT_S *stream, const SA_VOID *buffer, SA_S32 size, SA_S64 *timeStamp);

    /**
     * Notifies to the audio driver to stop playback however the queued buffers are retained by the hardware.
     */
    SA_S32 (*pause)(const struct SA_AUDIO_STREAM_OUT_S *stream);

    /**
     * Notifies to the audio driver to resume the playback following a pause
     * 
     */
    SA_S32 (*resume)(const struct SA_AUDIO_STREAM_OUT_S *stream);

    /**
     * Notifies to the audio driver to flush the queued data. Stream must already be paused before calling flush()
     * 
     */
    SA_S32 (*flush)(const struct SA_AUDIO_STREAM_OUT_S *stream);

    /**
     * Notifies to the audio driver to start the audio stream data
     * 
     */
    SA_S32 (*start)(const struct SA_AUDIO_STREAM_OUT_S *stream);

    /**
     * Notifies to the audio driver to stop the audio stream data
     * 
     */
    SA_S32 (*stop)(const struct SA_AUDIO_STREAM_OUT_S *stream);
    /**
     * Notifies to the audio driver to check the audio buffer status
     * 
     */
    SA_S32 (*check)(const struct SA_AUDIO_STREAM_OUT_S *stream, SA_S32 milliSec);
};

typedef struct SA_AUDIO_STREAM_OUT_S AUDIO_STREAM_OUT_S;


struct SA_AUDIO_STREAM_IN_S
{
    /**
     * @brief 
     * Common methods of the audio stream in. This must be the first member of SA_AUDIO_STREAM_IN_S
     * as users of this structure will cast a SA_AUDIO_STREAM_S to SA_AUDIO_STREAM_IN_S pointer in 
     * contexts where it's know the SA_AUDIO_STREAM_S reference an SA_AUDIO_STREAM_IN_S
     */
    struct SA_AUDIO_STREAM_S common;

    /**
     * Set the input gain for the audio driver
     */
    SA_S32 (*set_gain)(const struct SA_AUDIO_STREAM_IN_S *stream, SA_F32 gain);

    /**
     * Read audio buffer in from audio driver. Return number of bytes read.
     */
    SA_S32 (*read)(const struct SA_AUDIO_STREAM_IN_S *stream, const SA_VOID *buffer, SA_S32 size, SA_S64 *timeStamp);

    /**
     * Notifies to the audio driver to start the audio stream in data
     * 
     */
    SA_S32 (*start)(const struct SA_AUDIO_STREAM_IN_S *stream);

    /**
     * Notifies to the audio driver to stop the audio stream in data
     * 
     */
    SA_S32 (*stop)(const struct SA_AUDIO_STREAM_IN_S *stream);
    /**
     * Notifies to the audio driver to check the audio stream in data status
     * 
     */
    SA_S32 (*check)(const struct SA_AUDIO_STREAM_IN_S *stream, SA_S32 milliSec);
};

typedef struct SA_AUDIO_STREAM_IN_S AUDIO_STREAM_IN_S;

struct SA_AUDIO_HW_DEVICE_S
{
    /**
     * check to see if the audio hardware interface has been initialized.
     * return 0 on success, -ENODEV on failure.
     */
    SA_S32 (*init_check)(const struct SA_AUDIO_HW_DEVICE_S *dev);

    SA_S32 (*get_error)(const struct SA_AUDIO_HW_DEVICE_S *dev);

    /**
     * This method creates and opens the audio hardware output stream.
     */
    SA_S32 (*open_output_stream)(const struct SA_AUDIO_HW_DEVICE_S *dev, AUDIO_CONFIG_S *config, SA_CHAR *pcm_name, AUDIO_STREAM_OUT_S **stream_out);

    SA_VOID (*close_output_stream)(const struct SA_AUDIO_HW_DEVICE_S *dev, AUDIO_STREAM_OUT_S *stream_out);

     /**
     * This method creates and opens the audio hardware input stream.
     */
    SA_S32 (*open_input_stream)(const struct SA_AUDIO_HW_DEVICE_S *dev, AUDIO_CONFIG_S *config, SA_CHAR *pcm_name, AUDIO_STREAM_IN_S **stream_in);

    SA_VOID (*close_input_stream)(const struct SA_AUDIO_HW_DEVICE_S *dev, AUDIO_STREAM_IN_S *stream_in);

};

typedef struct SA_AUDIO_HW_DEVICE_S AUDIO_HW_DEVICE_S;


extern SA_S32 oswl_audio_hw_device_open(const SA_CHAR *name, struct SA_AUDIO_HW_DEVICE_S **device);
extern SA_S32 oswl_audio_hw_device_close(struct SA_AUDIO_HW_DEVICE_S *device);

extern SA_VOID oswl_audio_dump_init(SA_AUDIO_DUMP_CONFIG *conf);
extern SA_S32 oswl_audio_dump_attach(SA_S8 *data);
extern SA_VOID oswl_audio_dump_exit(SA_VOID);

typedef struct saAIO_CTL_ATTR_S {
    SA_S32  s32SoundDevId;
    SA_U32	resv; /*reserve item*/
} AIO_CTL_ATTR_S;

struct SA_AUDIO_CTL_HW_DEVICE_S
{
     /**
     * This method set and get the codec hardware.
     */
    SA_S32 (*ctl_open)(const struct SA_AUDIO_CTL_HW_DEVICE_S *dev, const AIO_CTL_ATTR_S *pAttr, SA_VOID **ctlHandle);

    SA_S32 (*ctl_cset)(const struct SA_AUDIO_CTL_HW_DEVICE_S *dev, SA_VOID *ctlHandle, SA_U32 cmd, SA_VOID *valArg);

    SA_S32 (*ctl_cget)(const struct SA_AUDIO_CTL_HW_DEVICE_S *dev, SA_VOID *ctlHandle, SA_U32 cmd, SA_VOID *valArg);

    SA_S32 (*ctl_close)(const struct SA_AUDIO_CTL_HW_DEVICE_S *dev,SA_VOID *ctlHandle);
};

typedef struct SA_AUDIO_CTL_HW_DEVICE_S AUDIO_CTL_HW_DEVICE_S;

extern SA_S32 oswl_ctl_device_open(const SA_CHAR *name, struct SA_AUDIO_CTL_HW_DEVICE_S **device);
extern SA_S32 oswl_ctl_device_close(struct SA_AUDIO_CTL_HW_DEVICE_S *device);

extern SA_S32 oswl_audio_timer_enable(SA_U32 interval);
extern SA_S32 oswl_audio_timer_disable(SA_VOID);


#endif
