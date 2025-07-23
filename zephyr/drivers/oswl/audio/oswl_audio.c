/*
* Copyright (c) 2021, Supper Acme SW Platform
* All rights reserved.
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


#include "oswl_audio_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

static SA_S32 audioDeviceRefCount = 0; //对于音频设备以及音频流来说，只支持单例场景，不支持多线程打开多个音频设备，在设备管理层也不支持多线程操作多个流
static SA_S32 audioErrorCode = 0;

//根据音频BIT位宽属性来转换对应一帧音频帧的字节数
static SA_S32 audio_stream_bits_to_bytes(AUDIO_BIT_WIDTH_E bits)
{
	SA_S32 bytes = 0;
	switch (bits) 
	{
	case AUDIO_BIT_WIDTH_8:
		bytes = 1;
		break;
	case AUDIO_BIT_WIDTH_16:
		bytes = 2;
		break;
	case AUDIO_BIT_WIDTH_24:
		bytes = 3;
		break;
	case AUDIO_BIT_WIDTH_32:
		bytes = 4;
		break;
	default:
		bytes = 1;
		break;
	}

	return bytes;
}

//根据音频模式获取对应的音频声道数
static SA_S32 audio_stream_sound_mode_to_channels(AUDIO_SOUND_MODE_E mode)
{
	SA_S32 bytes = 0;
	switch (mode) 
	{
	case AUDIO_SOUND_MODE_1_CHAN:
		bytes = 1;
		break;
	case AUDIO_SOUND_MODE_2_CHAN:
		bytes = 2;
		break;
	case AUDIO_SOUND_MODE_3_CHAN:
		bytes = 3;
		break;
	case AUDIO_SOUND_MODE_4_CHAN:
		bytes = 4;
		break;
	default:
		bytes = 1;
		break;
	}

	return bytes;
}

//获取音频数据一帧的大小，1帧就是一个采样点的大小 = 采样位宽字节数 * 声道数
static SA_S32 audio_stream_frame_size(AUDIO_CONFIG_S *config)
{
	SA_S32 size = 0;
	SA_S32 bitWidth = 0;
	SA_S32 channels = 0;

	if (SA_NULL == config) {
		audioErrorCode = AUDIO_ERROR_CODE_NULL_POINTER;
		return -1;
	}

	bitWidth = audio_stream_bits_to_bytes(config->bits);
	channels = audio_stream_sound_mode_to_channels(config->format);

	size = bitWidth * channels;

	return size;
}

/********************************************
 * audio output stream functions
 * ******************************************/
static SA_S32 out_get_sample_rate(const struct SA_AUDIO_STREAM_S *stream)
{
	struct saGENERIC_AUDIO_STREAM_OUT_S *out = (struct saGENERIC_AUDIO_STREAM_OUT_S *)stream;
	if (SA_NULL == out) {
		OSWL_LOG_E("out_get_sample_rate input NULL!");
		audioErrorCode = AUDIO_ERROR_CODE_NULL_POINTER;
		return -1;
	}
	return out->configs.sample_rate;
}

static SA_S32 out_get_channels(const struct SA_AUDIO_STREAM_S *stream)
{
	SA_S32 channels = 0;
	struct saGENERIC_AUDIO_STREAM_OUT_S *out = (struct saGENERIC_AUDIO_STREAM_OUT_S *)stream;
	if (SA_NULL == out) {
		OSWL_LOG_E("out_get_sample_rate input NULL!");
		audioErrorCode = AUDIO_ERROR_CODE_NULL_POINTER;
		return -1;
	}

	channels = audio_stream_sound_mode_to_channels(out->configs.format);
	return channels;
}

static SA_S32 out_get_buffer_size(const struct SA_AUDIO_STREAM_S *stream) {
	SA_S32 size = 0;
	SA_S32 frameSize = 0;

    struct saGENERIC_AUDIO_STREAM_OUT_S *out = (struct saGENERIC_AUDIO_STREAM_OUT_S *)stream;

	if (SA_NULL == out) {
		OSWL_LOG_E("out_get_sample_rate input NULL!");
		audioErrorCode = AUDIO_ERROR_CODE_NULL_POINTER;
		return -1;
	}

	frameSize = audio_stream_frame_size(&out->configs);

	size = ((out->configs.sample_rate * frameSize)/1000) * out->configs.period_time * out->configs.period_count;

    return size;
}

static AUDIO_BIT_WIDTH_E out_get_format(const struct SA_AUDIO_STREAM_S *stream)
{
	struct saGENERIC_AUDIO_STREAM_OUT_S *out = (struct saGENERIC_AUDIO_STREAM_OUT_S *)stream;

	if (SA_NULL == out) {
		OSWL_LOG_E("out_get_format input NULL!");
		audioErrorCode = AUDIO_ERROR_CODE_NULL_POINTER;
		return -1;
	}

	return out->configs.bits;
}

//这里会对alsa-lib进行初始化
static SA_S32 out_start(const struct SA_AUDIO_STREAM_OUT_S *stream)
{
	SA_S32 ret = 0;
	SA_S32 s32DevType = AUDIO_STREAM_REPLAY;
	struct rt_audio_caps playBackCaps = {0};

	struct saGENERIC_AUDIO_STREAM_OUT_S *out = (struct saGENERIC_AUDIO_STREAM_OUT_S *)stream;
	if (AUDIO_STATE_RUNNING == out->state) {
		OSWL_LOG_E("out_start out state error!");
		audioErrorCode = AUDIO_ERROR_CODE_PCM_OUT_HW_STATE_ERROR;
		return -1;
	}

	out->speakerDev = rt_device_find(out->outName);
	if (NULL == out->speakerDev) {
		OSWL_LOG_E("out_start rt_device_find err!\n");
		return -1;
	}

	ret = rt_device_control(out->speakerDev, AUDIO_CTL_OPEN, &s32DevType);
	if (ret < 0) {
		OSWL_LOG_E("%s: rt_device_control for replay device open error!\n", __func__);
		return -1;
	}

#if 0
	ret = rt_device_open(out->speakerDev, RT_DEVICE_OFLAG_WRONLY);
	if (ret < 0) {
		OSWL_LOG_E("out_start rt_device_open error!\n");
		return -1;
	}
#endif
	OSWL_LOG_I("out_start rt_device_control %s ok!\n", out->outName);

	// 配置音频设备相关的音频参数
    playBackCaps.main_type               = AUDIO_TYPE_OUTPUT; // 输入类型 录音设备
    playBackCaps.sub_type                = AUDIO_DSP_PARAM;   // 设置所有音频参数信息
    playBackCaps.udata.config.samplerate = out->configs.sample_rate;  // 采样率
    playBackCaps.udata.config.channels   = audio_stream_sound_mode_to_channels(out->configs.format); // 采样通道
    playBackCaps.udata.config.samplebits = out->configs.bits;  // 采样位数 
	playBackCaps.udata.config.master     = out->configs.clk_mode;// 主从模式
	if (out->configs.format >= AUDIO_SOUND_MODE_4_CHAN) {
		playBackCaps.udata.config.interface  = 0;  //TDM
	} else {
		playBackCaps.udata.config.interface  = 1;  //I2S
	}
	OSWL_LOG_I("%s: main type %d, sub type: %d, sr %d, ch %d, bitwidth %d, master %d, interface %d\n", __func__, playBackCaps.main_type, playBackCaps.sub_type, playBackCaps.udata.config.samplerate,
		playBackCaps.udata.config.channels, playBackCaps.udata.config.samplebits, playBackCaps.udata.config.master, playBackCaps.udata.config.interface);
    ret = rt_device_control(out->speakerDev, AUDIO_CTL_CONFIGURE, &playBackCaps);
	if (ret < 0) {
		OSWL_LOG_E("out_start rt_device_control error!\n");
		rt_device_control(out->speakerDev, AUDIO_CTL_CLOSE, &s32DevType);
		return -1;
	}
	
	OSWL_LOG_I("out_start init ok!");
	out->state = AUDIO_STATE_RUNNING;

	return 0;
}

SA_S32 out_stop(const struct SA_AUDIO_STREAM_OUT_S *stream)
{
	SA_S32 ret = 0;
	SA_S32 s32DevType = AUDIO_STREAM_REPLAY;
	struct saGENERIC_AUDIO_STREAM_OUT_S *out = (struct saGENERIC_AUDIO_STREAM_OUT_S *)stream;

	if (AUDIO_STATE_RUNNING != out->state) {
		OSWL_LOG_E("out_stop audio state error!");
		audioErrorCode = AUDIO_ERROR_CODE_PCM_OUT_HW_STATE_ERROR;
		return -1;
	}

	ret = rt_device_control(out->speakerDev, AUDIO_CTL_CLOSE, &s32DevType);
	if (ret < 0) {
		OSWL_LOG_E("%s: rt_device_control for playback device closing error!\n", __func__);
		return -1;
	}
	out->state = AUDIO_STATE_STOP;

	return 0;
}

SA_S32 out_check(const struct SA_AUDIO_STREAM_OUT_S *stream, SA_S32 milliSec)
{
	struct saGENERIC_AUDIO_STREAM_OUT_S *out = (struct saGENERIC_AUDIO_STREAM_OUT_S *)stream;
	SA_S32 avail;
	SA_S32 frameSize;

	if (AUDIO_STATE_RUNNING != out->state) {
		SA_ERR("out_stop audio state error!");
		audioErrorCode = AUDIO_ERROR_CODE_PCM_OUT_HW_STATE_ERROR;
		return -1;
	}
	frameSize = audio_stream_frame_size(&out->configs);
	avail = ((out->configs.sample_rate * frameSize)/1000) * out->configs.period_time * out->configs.period_count;

	return avail;
}

SA_S32 out_get_latency(const struct SA_AUDIO_STREAM_OUT_S *stream)
{
	SA_S32 latency = 0;
	struct saGENERIC_AUDIO_STREAM_OUT_S *out = (struct saGENERIC_AUDIO_STREAM_OUT_S *)stream;
	if (AUDIO_STATE_RUNNING != out->state) {
		OSWL_LOG_E("out_get_latency audio state error!");
		audioErrorCode = AUDIO_ERROR_CODE_PCM_OUT_HW_STATE_ERROR;
		return -1;
	}

	latency = out->configs.period_time * out->configs.period_count;

	return latency;
}

SA_S32 out_set_volume(const struct SA_AUDIO_STREAM_OUT_S *stream, SA_F32 left, SA_F32 right)
{
	struct saGENERIC_AUDIO_STREAM_OUT_S *out = (struct saGENERIC_AUDIO_STREAM_OUT_S *)stream;

	if (AUDIO_STATE_RUNNING != out->state) {
		OSWL_LOG_E("out_set_volume audio state error!");
		audioErrorCode = AUDIO_ERROR_CODE_PCM_OUT_HW_STATE_ERROR;
		return -1;
	}

	return 0;
}

SA_S32 out_write(const struct SA_AUDIO_STREAM_OUT_S *stream, const SA_VOID *buffer, SA_S32 size, SA_S64 *timeStamp)
{
	SA_S32 ret = 0;
	SA_S32 writeSize = 0;

	struct saGENERIC_AUDIO_STREAM_OUT_S *out = (struct saGENERIC_AUDIO_STREAM_OUT_S *)stream;

	if (AUDIO_STATE_RUNNING != out->state) {
		OSWL_LOG_E("out_write audio state error!\n");
		audioErrorCode = AUDIO_ERROR_CODE_PCM_OUT_HW_STATE_ERROR;
		return -1;
	}

	if (SA_NULL == out) {
		OSWL_LOG_E("out_write stream NULL error!\n");
		audioErrorCode = AUDIO_ERROR_CODE_NULL_POINTER;
		return -1;
	}

	ret = rt_device_write(out->speakerDev, 0, (SA_S8 *)buffer, size);
	if (ret < 0) {
		OSWL_LOG_E("out_write rt_device_write error!\n");
	}

	writeSize = ret / audio_stream_frame_size(&out->configs);

	return writeSize;
}

SA_S32 out_flush(const struct SA_AUDIO_STREAM_OUT_S *stream)
{
	struct saGENERIC_AUDIO_STREAM_OUT_S *out = (struct saGENERIC_AUDIO_STREAM_OUT_S *)stream;
	if (SA_NULL == out) {
		OSWL_LOG_E("out_flush stream NULL error!");
		audioErrorCode = AUDIO_ERROR_CODE_NULL_POINTER;
		return -1;
	}

	return 0;
}

SA_S32 out_pause(const struct SA_AUDIO_STREAM_OUT_S *stream)
{
	struct saGENERIC_AUDIO_STREAM_OUT_S *out = (struct saGENERIC_AUDIO_STREAM_OUT_S *)stream;
	if (SA_NULL == out) {
		OSWL_LOG_E("out_pause stream NULL error!");
		audioErrorCode = AUDIO_ERROR_CODE_NULL_POINTER;
		return -1;
	}

	if (AUDIO_STATE_RUNNING != out->state) {
		OSWL_LOG_E("out_pause state error!");
		audioErrorCode = AUDIO_ERROR_CODE_PCM_OUT_HW_STATE_ERROR;
		return -1;
	}

	out->state = AUDIO_STATE_PAUSE;
	return 0;
}

SA_S32 out_resume(const struct SA_AUDIO_STREAM_OUT_S *stream)
{
	struct saGENERIC_AUDIO_STREAM_OUT_S *out = (struct saGENERIC_AUDIO_STREAM_OUT_S *)stream;
	if (SA_NULL == out) {
		OSWL_LOG_E("out_resume stream NULL error!");
		audioErrorCode = AUDIO_ERROR_CODE_NULL_POINTER;
		return -1;
	}

	if (AUDIO_STATE_PAUSE != out->state) {
		OSWL_LOG_E("out_resume state error!");
		audioErrorCode = AUDIO_ERROR_CODE_PCM_OUT_HW_STATE_ERROR;
		return -1;		
	}

	out->state = AUDIO_STATE_RUNNING;

	return 0;
}

/********************************************
 * audio input stream functions
 * ******************************************/
static SA_S32 in_get_sample_rate(const struct SA_AUDIO_STREAM_S *stream)
{
	struct saGENERIC_AUDIO_STREAM_IN_S *in = (struct saGENERIC_AUDIO_STREAM_IN_S *)stream;
	if (SA_NULL == in) {
		OSWL_LOG_E("in_get_sample_rate input NULL!");
		audioErrorCode = AUDIO_ERROR_CODE_NULL_POINTER;
		return -1;
	}
	return in->configs.sample_rate;
}

static SA_S32 in_get_channels(const struct SA_AUDIO_STREAM_S *stream)
{
	SA_S32 channels = 0;
	struct saGENERIC_AUDIO_STREAM_IN_S *in = (struct saGENERIC_AUDIO_STREAM_IN_S *)stream;
	if (SA_NULL == in) {
		OSWL_LOG_E("in_get_channels input NULL!");
		audioErrorCode = AUDIO_ERROR_CODE_NULL_POINTER;
		return -1;
	}

	channels = audio_stream_sound_mode_to_channels(in->configs.format);
	return channels;
}

static SA_S32 in_get_buffer_size(const struct SA_AUDIO_STREAM_S *stream) {
	SA_S32 size = 0;
	SA_S32 frameSize = 0;

    struct saGENERIC_AUDIO_STREAM_IN_S *in = (struct saGENERIC_AUDIO_STREAM_IN_S *)stream;

	if (SA_NULL == in) {
		OSWL_LOG_E("in_get_buffer_size input NULL!");
		audioErrorCode = AUDIO_ERROR_CODE_NULL_POINTER;
		return -1;
	}

	frameSize = audio_stream_frame_size(&in->configs);

	size = ((in->configs.sample_rate * frameSize)/1000) * in->configs.period_time * in->configs.period_count;

    return size;
}

static AUDIO_BIT_WIDTH_E in_get_format(const struct SA_AUDIO_STREAM_S *stream)
{
	struct saGENERIC_AUDIO_STREAM_IN_S *in = (struct saGENERIC_AUDIO_STREAM_IN_S *)stream;

	if(SA_NULL == in) {
		OSWL_LOG_E("in_get_format input NULL!");
		audioErrorCode = AUDIO_ERROR_CODE_NULL_POINTER;
		return -1;
	}

	return in->configs.format;
}

SA_S32 in_set_gain(const struct SA_AUDIO_STREAM_IN_S *stream, SA_F32 gain)
{
	struct saGENERIC_AUDIO_STREAM_IN_S *in = (struct saGENERIC_AUDIO_STREAM_IN_S *)stream;

	if (SA_NULL == in) {
		OSWL_LOG_E("in_set_gain input NULL!");
		audioErrorCode = AUDIO_ERROR_CODE_NULL_POINTER;
		return -1;
	}

	return 0;
}

SA_S32 in_read(const struct SA_AUDIO_STREAM_IN_S *stream, const SA_VOID *buffer, SA_S32 size, SA_S64 *timeStamp)
{
	//SA_S32 ret = 0;
	SA_S32 dataSize = 0;

	struct saGENERIC_AUDIO_STREAM_IN_S *in = (struct saGENERIC_AUDIO_STREAM_IN_S *)stream;

	if (AUDIO_STATE_RUNNING != in->state) {
		OSWL_LOG_E("in_read audio state error!");
		audioErrorCode = AUDIO_ERROR_CODE_PCM_IN_HW_STATE_ERROR;
		return -1;
	}

	if (SA_NULL == in) {
		OSWL_LOG_E("in_read stream NULL error!");
		audioErrorCode = AUDIO_ERROR_CODE_NULL_POINTER;
		return -1;
	}

	dataSize = rt_device_read(in->recordDev, 0, (void *)buffer, size); //这里返回的是实际的数据长度
	dataSize = dataSize / audio_stream_frame_size(&in->configs);//实际应该返回pointNumPerFrame

	return dataSize;
}

//这里会对alsa-lib进行初始化
static SA_S32 in_start(const struct SA_AUDIO_STREAM_IN_S *stream)
{
	SA_S32 ret = 0;
	SA_S32 s32DevType = AUDIO_STREAM_RECORD;
	struct rt_audio_caps caps = {0};
	struct saGENERIC_AUDIO_STREAM_IN_S *in = (struct saGENERIC_AUDIO_STREAM_IN_S *)stream;

	if (AUDIO_STATE_RUNNING == in->state) {
		OSWL_LOG_E("in_start in state error!\n");
		audioErrorCode = AUDIO_ERROR_CODE_PCM_IN_HW_STATE_ERROR;
		return -1;
	}
	OSWL_LOG_D("in_start in!\n");

	in->recordDev = rt_device_find(in->inName);
	if (NULL == in->recordDev) {
		OSWL_LOG_E("in_start rt_device_find err!\n");
		return -1;
	}

	ret = rt_device_control(in->recordDev, AUDIO_CTL_OPEN, &s32DevType);
	if (ret < 0) {
		OSWL_LOG_E("%s: rt_device_control for recorder device open error!\n", __func__);
		return -1;
	}

#if 0
	ret = rt_device_open(in->recordDev, RT_DEVICE_OFLAG_RDONLY);
	if (ret < 0) {
		OSWL_LOG_E("in_start rt_device_open error!\n");
		return -1;
	}
#endif
	OSWL_LOG_I("in_start rt_device_control %s ok!\n", in->inName);

	//2，配置音频设备相关的音频参数
    caps.main_type               = AUDIO_TYPE_INPUT;   // 输入类型  录音设备 
    caps.sub_type                = AUDIO_DSP_PARAM;    // 设置所有音频参数信息
    caps.udata.config.samplerate = in->configs.sample_rate;         // 采样率
    caps.udata.config.channels   = audio_stream_sound_mode_to_channels(in->configs.format); //采样通道
    caps.udata.config.samplebits = in->configs.bits;         // 采样位数
	caps.udata.config.master     = in->configs.clk_mode;             // 主从模式
	if (in->configs.format >= AUDIO_SOUND_MODE_4_CHAN) {
		caps.udata.config.interface  = 0;  //TDM
	} else {
		caps.udata.config.interface  = 1;  //I2S
	}
	OSWL_LOG_I("%s: main type %d, sub type: %d, sr %d, ch %d, bitwidth %d, master %d, interface %d\n", __func__, caps.main_type, caps.sub_type, caps.udata.config.samplerate,
		caps.udata.config.channels, caps.udata.config.samplebits, caps.udata.config.master, caps.udata.config.interface);
    ret = rt_device_control(in->recordDev, AUDIO_CTL_CONFIGURE, &caps);
	if (ret < 0) {
		OSWL_LOG_E("rt_device_control error!\n");
		rt_device_control(in->recordDev, AUDIO_CTL_CLOSE, &s32DevType);
		return -1;
	}

	OSWL_LOG_I("in_start init ok!\n");
	in->state = AUDIO_STATE_RUNNING;

	return 0;
}

SA_S32 in_stop(const struct SA_AUDIO_STREAM_IN_S *stream)
{
	SA_S32 ret = 0;
	SA_S32 s32DevType = AUDIO_STREAM_RECORD;
	struct saGENERIC_AUDIO_STREAM_IN_S *in = (struct saGENERIC_AUDIO_STREAM_IN_S *)stream;

	if (AUDIO_STATE_RUNNING != in->state) {
		OSWL_LOG_E("in_stop audio state error!\n");
		audioErrorCode = AUDIO_ERROR_CODE_PCM_IN_HW_STATE_ERROR;
		return -1;
	}

	ret = rt_device_control(in->recordDev, AUDIO_CTL_CLOSE, &s32DevType);
	if (ret < 0) {
		OSWL_LOG_E("%s: rt_device_control for recorder device closing error!\n", __func__);
		return -1;
	}

	in->state = AUDIO_STATE_STOP;

	return 0;
}


SA_S32 in_check(const struct SA_AUDIO_STREAM_IN_S *stream, SA_S32 milliSec)
{
	struct saGENERIC_AUDIO_STREAM_IN_S *in = (struct saGENERIC_AUDIO_STREAM_IN_S *)stream;
	SA_S32 avail;
	SA_S32 frameSize;

	if (AUDIO_STATE_RUNNING != in->state) {
		SA_ERR("in_stop audio state error!\n");
		audioErrorCode = AUDIO_ERROR_CODE_PCM_IN_HW_STATE_ERROR;
		return -1;
	}

	frameSize = audio_stream_frame_size(&in->configs);

	avail = ((in->configs.sample_rate * frameSize)/1000) * in->configs.period_time * in->configs.period_count;

	return avail;
}



/********************************************
 * audio hw device functions
 * ******************************************/
static SA_S32 adev_init_check(const struct SA_AUDIO_HW_DEVICE_S *dev)
{
	return SA_SUCCESS;
}

static SA_S32 adev_get_error_code(const struct SA_AUDIO_HW_DEVICE_S *dev)
{
	return audioErrorCode;
}

static SA_S32 adev_open_output_stream(const struct SA_AUDIO_HW_DEVICE_S *dev, AUDIO_CONFIG_S *config, SA_CHAR *pcm_name, AUDIO_STREAM_OUT_S **stream_out)
{
	struct saGENERIC_AUDIO_HW_DEVICE_S *adev = (struct saGENERIC_AUDIO_HW_DEVICE_S *)dev;

	GENERIC_AUDIO_STREAM_OUT_S *out = SA_NULL;

	if ((SA_NULL == dev) || (SA_NULL == config) || (SA_NULL == pcm_name) || (SA_NULL == stream_out)) {
		OSWL_LOG_E("adev_open_output_stream params NULL\n");
		return -1;
	}

	out = (GENERIC_AUDIO_STREAM_OUT_S *)oswl_malloc(sizeof(GENERIC_AUDIO_STREAM_OUT_S));
	if (SA_NULL == out) {
		OSWL_LOG_E("adev_open_output_stream out malloc error!\n");
		audioErrorCode = AUDIO_ERROR_CODE_NULL_POINTER;
		return -1;
	}

	out->stream.get_latency = out_get_latency;
	out->stream.set_volume = out_set_volume;
	out->stream.write = out_write;
	out->stream.pause = out_pause;
	out->stream.resume = out_resume;
	out->stream.flush = out_flush;
	out->stream.start = out_start;
	out->stream.stop = out_stop;
	out->stream.check = out_check;
	out->stream.common.get_sample_rate = out_get_sample_rate;
	out->stream.common.get_buffer_size = out_get_buffer_size;
	out->stream.common.get_channels = out_get_channels;
	out->stream.common.get_format = out_get_format;

	out->dev = adev;
	out->state = AUDIO_STATE_STOP;
	out->outName = pcm_name;

	memcpy(&out->configs, config, sizeof(AUDIO_CONFIG_S));

	*stream_out = &out->stream;

	return AUDIO_SUCCESS;
}

static SA_VOID adev_close_output_stream(const struct SA_AUDIO_HW_DEVICE_S *dev, AUDIO_STREAM_OUT_S *stream_out)
{
	struct saGENERIC_AUDIO_HW_DEVICE_S *adev = (struct saGENERIC_AUDIO_HW_DEVICE_S *)dev;
	if ((SA_NULL == dev) || (SA_NULL == stream_out)) {
		OSWL_LOG_E("adev_close_output_stream params NULL\n");
		return;
	}
	adev = adev;
	GENERIC_AUDIO_STREAM_OUT_S *out = (GENERIC_AUDIO_STREAM_OUT_S *)stream_out;

	oswl_free(out);
	return;
}

static SA_S32 adev_open_input_stream(const struct SA_AUDIO_HW_DEVICE_S *dev, AUDIO_CONFIG_S *config, SA_CHAR *pcm_name, AUDIO_STREAM_IN_S **stream_in)
{
	struct saGENERIC_AUDIO_HW_DEVICE_S *adev = (struct saGENERIC_AUDIO_HW_DEVICE_S *)dev;

	GENERIC_AUDIO_STREAM_IN_S *in  = SA_NULL;
	in = (GENERIC_AUDIO_STREAM_IN_S *)oswl_malloc(sizeof(GENERIC_AUDIO_STREAM_IN_S));
	if (SA_NULL == in) {
		OSWL_LOG_E("adev_open_input_stream in malloc error!\n");
		audioErrorCode = AUDIO_ERROR_CODE_NULL_POINTER;
		return -1;
	}

	in->stream.set_gain = in_set_gain;
	in->stream.read = in_read;
	in->stream.start = in_start;
	in->stream.stop = in_stop;
	in->stream.check = in_check;
	in->stream.common.get_sample_rate = in_get_sample_rate;
	in->stream.common.get_buffer_size = in_get_buffer_size;
	in->stream.common.get_channels = in_get_channels;
	in->stream.common.get_format = in_get_format;

	in->dev = adev;
	in->inName = pcm_name;
	in->state = AUDIO_STATE_STOP;

	memcpy(&in->configs, config, sizeof(AUDIO_CONFIG_S));

	OSWL_LOG_I("config->sample_rate %d \n", config->sample_rate);
	OSWL_LOG_I("config->bits %d \n", config->bits);
	OSWL_LOG_I("config->format %d \n", config->format);
	OSWL_LOG_I("config->clk_mode %d \n", config->clk_mode);

	OSWL_LOG_I("in->configs->sample_rate %d \n", in->configs.sample_rate);
	OSWL_LOG_I("in->configs->bits %d \n", in->configs.bits);
	OSWL_LOG_I("in->configs->format %d \n", in->configs.format);
	OSWL_LOG_I("in->configs->clk_mode %d \n", in->configs.clk_mode);

	*stream_in = &in->stream;
	return AUDIO_SUCCESS;
}

RTM_EXPORT(adev_open_input_stream);

static SA_VOID adev_close_input_stream(const struct SA_AUDIO_HW_DEVICE_S *dev, AUDIO_STREAM_IN_S *stream_in)
{
	if (SA_NULL == dev) {
		return;
	}
	//struct saGENERIC_AUDIO_HW_DEVICE_S *adev = (struct saGENERIC_AUDIO_HW_DEVICE_S *)dev;

	GENERIC_AUDIO_STREAM_IN_S *in = (GENERIC_AUDIO_STREAM_IN_S *)stream_in;
	oswl_free(in);

	return;
}

RTM_EXPORT(adev_close_input_stream);

SA_S32 oswl_audio_hw_device_open(const SA_CHAR *name, struct SA_AUDIO_HW_DEVICE_S **device)
{
	static GENERIC_AUDIO_HW_DEVICE_S *adev;

	if ((SA_NULL == name) || (SA_NULL == device)) {
		OSWL_LOG_E("oswl_audio_hw_device_open input params NULL!\n");
		return -1;
	}

	if (0 != strcmp(name, AUDIO_HW_INTERFACE)) {
		OSWL_LOG_E("oswl_audio_hw_device_open name error!\n");
		audioErrorCode = AUDIO_ERROR_CODE_DEVICE_ERROR;
		return -1;
	}

	if (0 != audioDeviceRefCount) {
		OSWL_LOG_E("oswl_audio_hw_device_open device exits\n");
		audioErrorCode = AUDIO_ERROR_CODE_DEVICE_ERROR;
		return -1;
	}

	adev = (GENERIC_AUDIO_HW_DEVICE_S *)oswl_malloc(sizeof(GENERIC_AUDIO_HW_DEVICE_S));
	if (SA_NULL == adev) {
		OSWL_LOG_E("oswl_audio_hw_device_open adev malloc error!\n");
		audioErrorCode = AUDIO_ERROR_CODE_NULL_POINTER;
		return -1;
	}

	adev->device.init_check = adev_init_check;
	adev->device.open_output_stream = adev_open_output_stream;
	adev->device.close_output_stream = adev_close_output_stream;
	adev->device.open_input_stream = adev_open_input_stream;
	adev->device.close_input_stream = adev_close_input_stream;
	adev->device.get_error = adev_get_error_code;

	*device = &adev->device;
	adev->isInit = SA_TRUE;
	audioDeviceRefCount++;

	return AUDIO_SUCCESS;
}

RTM_EXPORT(oswl_audio_hw_device_open);

SA_S32 oswl_audio_hw_device_close(struct SA_AUDIO_HW_DEVICE_S *device)
{
	struct saGENERIC_AUDIO_HW_DEVICE_S *adev = (struct saGENERIC_AUDIO_HW_DEVICE_S *)device;

	if (0 == audioDeviceRefCount) {
		OSWL_LOG_E("oswl_audio_hw_device_close audioDeviceRefCount error!\n");
		audioErrorCode = AUDIO_ERROR_CODE_DEVICE_ERROR;
		return -1;
	}

	oswl_free(adev);
	audioDeviceRefCount = 0;

	return AUDIO_SUCCESS;
}

RTM_EXPORT(oswl_audio_hw_device_close);

#ifdef __cplusplus
}
#endif
