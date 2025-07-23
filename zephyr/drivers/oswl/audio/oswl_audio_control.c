/*
* Copyright (c) 2023, Supper Acme SW Platform
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

#include "sa_comm_type.h"
#include "oswl_audio_internal.h"
#include "audio_codec_config.h"

#define ARRAY_SIZE(a) sizeof(a)/sizeof(a[0]) 

static SA_S32 audioDeviceRefCount = 0; 
static SA_S32 audioErrorCode = 0;

static SA_S8 s8EqPreGainTable[8] = {-30, -24, -18, -12, -6, 0, 6, 12};
static SA_S8 s8EqFineGainTable[16] = {-55, -50, -45, -40, -35, -30, -25, -20, -15, -10, -5, 0, 0, 0, 0, 0};
static SA_S8 s8BassIntensityTable0[15] = {90, 90, 75, 60, 45, 30, 15, 0, -15, -30, -45, -60, -60, -60, -60};
static SA_S8 s8BassIntensityTable1[15] = {15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
static SA_S8 s8TrebleIntensityTable[15] = {90, 90, 75, 60, 45, 30, 15, 0, -15, -30, -45, -60, -60, -60, -60};

static SA_S32 is_value_in_array(SA_S8 value, SA_S8 *array, SA_U32 len)
{
	SA_U32 i;

	for(i=0;i<len;i++) {
		if(array[i] == value) {
			return SA_SUCCESS;
		}
	}

	return SA_FAILURE;
}

static SA_S8 value_to_index(SA_S8 value, SA_S8 *array, SA_U32 len)
{
	SA_U32 i;

	for(i=0;i<len;i++) {
		if(array[i] == value) {
			return i;
		}
	}

	return SA_FAILURE;
}

static SA_S32 adev_ctl_cget(const struct SA_AUDIO_CTL_HW_DEVICE_S *dev, SA_VOID *ctl_handle, SA_U32 cmd, SA_VOID *valArg)
{
	SA_S32 i = 0;
	SA_S32 j = 0;
	SA_S32 ret = 0;
	SA_S8 *pchar;
	ACODEC_CTL_VAL_INFO_S *arg;
	rt_device_t device_handle;
	struct rt_audio_caps caps = {0};
	AUDIO_CODEC_DAC_EQ_CONFIG_S *codec_rx_eq = NULL;
	AUDIO_CODEC_ADC_EQ_CONFIG_S *codec_tx_eq = NULL;
	AUDIO_CODEC_BASS_CONFIG_S *codec_rx_bass = NULL;
	AUDIO_CODEC_TREBLE_CONFIG_S *codec_rx_treble = NULL;
	AUDIO_CODEC_FILTER_CONFIG_S *codec_tx_filter = NULL;

	if(NULL == ctl_handle) {
		OSWL_LOG_E("%s: ctl handle is NULL\n", __func__);
		return SA_FAILURE;
	}
	if(NULL == valArg) {
		OSWL_LOG_E("%s: valArg is NULL\n", __func__);
		return SA_FAILURE;
	}

	device_handle = (rt_device_t)ctl_handle;
	arg = (ACODEC_CTL_VAL_INFO_S *)valArg;
	memset(&caps, 0, sizeof(caps));

	switch (cmd)
	{
	case SA8900_CTL_ADC_EQ_CONFIG:
		if(sizeof(AUDIO_CODEC_ADC_EQ_CONFIG_S) > sizeof(caps.udata.data)) {
			OSWL_LOG_E("%s: AUDIO_CODEC_ADC_EQ_CONFIG_S size(%d) is bigger than caps data size(%d)\n", __func__, sizeof(AUDIO_CODEC_ADC_EQ_CONFIG_S), sizeof(caps.udata.data));
			return SA_FAILURE;
		}

		caps.main_type = AUDIO_TYPE_INPUT|AUDIO_TYPE_MIXER;
		caps.sub_type  = AUDIO_MIXER_EQUALIZER;
		ret = rt_device_control(device_handle, AUDIO_CTL_GETCAPS, &caps);
		if (ret < 0) {
			OSWL_LOG_E("%s: rt_device_control error(%d) for codec_tx_eq config\n", __func__, ret);
			return SA_FAILURE;
		}

		codec_tx_eq = (AUDIO_CODEC_ADC_EQ_CONFIG_S *)caps.udata.data;

		OSWL_LOG_I("%s: codec_tx_eq enable status %d, %d, %d, %d\n", __func__, 
			(SA_U8)codec_tx_eq->BAND_CTRL.ctrl.L_BAND0_EQ_EN&0x01, (SA_U8)codec_tx_eq->BAND_CTRL.ctrl.L_EQ_EN&0x01,
			(SA_U8)codec_tx_eq->BAND_CTRL.ctrl.R_BAND0_EQ_EN&0x01, (SA_U8)codec_tx_eq->BAND_CTRL.ctrl.R_EQ_EN&0x01);
		if(((SA_U8)1 == ((SA_U8)codec_tx_eq->BAND_CTRL.ctrl.L_EQ_EN & 1)) && ((SA_U8)1 == ((SA_U8)codec_tx_eq->BAND_CTRL.ctrl.R_EQ_EN & 1))) {
			arg->s32Argv[0] = 1;
		} else {
			arg->s32Argv[0] = 0;
		}

		i = 1;
		arg->s32Argv[i++] = s8EqPreGainTable[codec_tx_eq->BAND0_GAIN.gain.pre_gain & 0x07];
		arg->s32Argv[i++] = s8EqPreGainTable[codec_tx_eq->BAND1_GAIN.gain.pre_gain & 0x07];
		arg->s32Argv[i++] = s8EqPreGainTable[codec_tx_eq->BAND2_GAIN.gain.pre_gain & 0x07];
		arg->s32Argv[i++] = s8EqPreGainTable[codec_tx_eq->BAND3_GAIN.gain.pre_gain & 0x07];
		arg->s32Argv[i++] = s8EqPreGainTable[codec_tx_eq->BAND4_GAIN.gain.pre_gain & 0x07];
		arg->s32Argv[i++] = s8EqPreGainTable[codec_tx_eq->BAND5_GAIN.gain.pre_gain & 0x07];
		arg->s32Argv[i++] = s8EqPreGainTable[codec_tx_eq->BAND6_GAIN.gain.pre_gain & 0x07];
		arg->s32Argv[i++] = s8EqPreGainTable[codec_tx_eq->BAND7_GAIN.gain.pre_gain & 0x07];
		arg->s32Argv[i++] = s8EqPreGainTable[codec_tx_eq->BAND8_GAIN.gain.pre_gain & 0x07];
		arg->s32Argv[i++] = s8EqPreGainTable[codec_tx_eq->BAND9_GAIN.gain.pre_gain & 0x07];

		arg->s32Argv[i++] = s8EqFineGainTable[codec_tx_eq->BAND0_GAIN.gain.fine_gain & 0x0f];
		arg->s32Argv[i++] = s8EqFineGainTable[codec_tx_eq->BAND1_GAIN.gain.fine_gain & 0x0f];
		arg->s32Argv[i++] = s8EqFineGainTable[codec_tx_eq->BAND2_GAIN.gain.fine_gain & 0x0f];
		arg->s32Argv[i++] = s8EqFineGainTable[codec_tx_eq->BAND3_GAIN.gain.fine_gain & 0x0f];
		arg->s32Argv[i++] = s8EqFineGainTable[codec_tx_eq->BAND4_GAIN.gain.fine_gain & 0x0f];
		arg->s32Argv[i++] = s8EqFineGainTable[codec_tx_eq->BAND5_GAIN.gain.fine_gain & 0x0f];
		arg->s32Argv[i++] = s8EqFineGainTable[codec_tx_eq->BAND6_GAIN.gain.fine_gain & 0x0f];
		arg->s32Argv[i++] = s8EqFineGainTable[codec_tx_eq->BAND7_GAIN.gain.fine_gain & 0x0f];
		arg->s32Argv[i++] = s8EqFineGainTable[codec_tx_eq->BAND8_GAIN.gain.fine_gain & 0x0f];
		arg->s32Argv[i++] = s8EqFineGainTable[codec_tx_eq->BAND9_GAIN.gain.fine_gain & 0x0f];
		arg->s32NumArg = i;
		break;
	case SA8900_CTL_DAC_EQ_CONFIG:
		if(sizeof(AUDIO_CODEC_DAC_EQ_CONFIG_S) > sizeof(caps.udata.data)) {
			OSWL_LOG_E("%s: AUDIO_CODEC_DAC_EQ_CONFIG_S size(%d) is bigger than caps data size(%d)\n", __func__, sizeof(AUDIO_CODEC_DAC_EQ_CONFIG_S), sizeof(caps.udata.data));
			return SA_FAILURE;
		}

		caps.main_type = AUDIO_TYPE_OUTPUT|AUDIO_TYPE_MIXER;
		caps.sub_type  = AUDIO_MIXER_EQUALIZER;
		ret = rt_device_control(device_handle, AUDIO_CTL_GETCAPS, &caps);
		if (ret < 0) {
			OSWL_LOG_E("%s: rt_device_control error(%d) for codec_rx_eq config\n", __func__, ret);
			return SA_FAILURE;
		}

		codec_rx_eq = (AUDIO_CODEC_DAC_EQ_CONFIG_S *)caps.udata.data;

		OSWL_LOG_I("%s: codec_rx_eq enable status %d, %d, %d, %d\n", __func__, 
			(SA_U8)codec_rx_eq->BAND_CTRL.ctrl.L_BAND0_EQ_EN&0x01, (SA_U8)codec_rx_eq->BAND_CTRL.ctrl.L_EQ_EN&0x01,
			(SA_U8)codec_rx_eq->BAND_CTRL.ctrl.R_BAND0_EQ_EN&0x01, (SA_U8)codec_rx_eq->BAND_CTRL.ctrl.R_EQ_EN&0x01);
		if(((SA_U8)1 == ((SA_U8)codec_rx_eq->BAND_CTRL.ctrl.L_EQ_EN & 1)) && ((SA_U8)1 == ((SA_U8)codec_rx_eq->BAND_CTRL.ctrl.R_EQ_EN & 1))) {
			arg->s32Argv[0] = 1;
		} else {
			arg->s32Argv[0] = 0;
		}

		i = 1;
		arg->s32Argv[i++] = s8EqPreGainTable[codec_rx_eq->BAND0_GAIN.gain.pre_gain & 0x07];
		arg->s32Argv[i++] = s8EqPreGainTable[codec_rx_eq->BAND1_GAIN.gain.pre_gain & 0x07];
		arg->s32Argv[i++] = s8EqPreGainTable[codec_rx_eq->BAND2_GAIN.gain.pre_gain & 0x07];
		arg->s32Argv[i++] = s8EqPreGainTable[codec_rx_eq->BAND3_GAIN.gain.pre_gain & 0x07];
		arg->s32Argv[i++] = s8EqPreGainTable[codec_rx_eq->BAND4_GAIN.gain.pre_gain & 0x07];
		arg->s32Argv[i++] = s8EqPreGainTable[codec_rx_eq->BAND5_GAIN.gain.pre_gain & 0x07];
		arg->s32Argv[i++] = s8EqPreGainTable[codec_rx_eq->BAND6_GAIN.gain.pre_gain & 0x07];
		arg->s32Argv[i++] = s8EqPreGainTable[codec_rx_eq->BAND7_GAIN.gain.pre_gain & 0x07];
		arg->s32Argv[i++] = s8EqPreGainTable[codec_rx_eq->BAND8_GAIN.gain.pre_gain & 0x07];
		arg->s32Argv[i++] = s8EqPreGainTable[codec_rx_eq->BAND9_GAIN.gain.pre_gain & 0x07];

		arg->s32Argv[i++] = s8EqFineGainTable[codec_rx_eq->BAND0_GAIN.gain.fine_gain & 0x0f];
		arg->s32Argv[i++] = s8EqFineGainTable[codec_rx_eq->BAND1_GAIN.gain.fine_gain & 0x0f];
		arg->s32Argv[i++] = s8EqFineGainTable[codec_rx_eq->BAND2_GAIN.gain.fine_gain & 0x0f];
		arg->s32Argv[i++] = s8EqFineGainTable[codec_rx_eq->BAND3_GAIN.gain.fine_gain & 0x0f];
		arg->s32Argv[i++] = s8EqFineGainTable[codec_rx_eq->BAND4_GAIN.gain.fine_gain & 0x0f];
		arg->s32Argv[i++] = s8EqFineGainTable[codec_rx_eq->BAND5_GAIN.gain.fine_gain & 0x0f];
		arg->s32Argv[i++] = s8EqFineGainTable[codec_rx_eq->BAND6_GAIN.gain.fine_gain & 0x0f];
		arg->s32Argv[i++] = s8EqFineGainTable[codec_rx_eq->BAND7_GAIN.gain.fine_gain & 0x0f];
		arg->s32Argv[i++] = s8EqFineGainTable[codec_rx_eq->BAND8_GAIN.gain.fine_gain & 0x0f];
		arg->s32Argv[i++] = s8EqFineGainTable[codec_rx_eq->BAND9_GAIN.gain.fine_gain & 0x0f];
		arg->s32NumArg = i;
		break;
	case SA8900_CTL_BASS_CONFIG:
		if(sizeof(AUDIO_CODEC_BASS_CONFIG_S) > sizeof(caps.udata.data)) {
			OSWL_LOG_E("%s: AUDIO_CODEC_BASS_CONFIG_S size(%d) is bigger than caps data size(%d)\n", __func__, sizeof(AUDIO_CODEC_BASS_CONFIG_S), sizeof(caps.udata.data));
			return SA_FAILURE;
		}

		caps.main_type = AUDIO_TYPE_OUTPUT|AUDIO_TYPE_MIXER;
		caps.sub_type  = AUDIO_MIXER_BASS;
		ret = rt_device_control(device_handle, AUDIO_CTL_GETCAPS, &caps);
		if (ret < 0) {
			OSWL_LOG_E("%s: rt_device_control error(%d) for codec_rx_bass config\n", __func__, ret);
			return SA_FAILURE;
		}

		codec_rx_bass = (AUDIO_CODEC_BASS_CONFIG_S *)caps.udata.data;

		if((SA_U8)0xf == ((SA_U8)codec_rx_bass->BASS_CTRL.ctrl.BASS & 0xf)) {
			arg->s32Argv[0] = 0;
		} else {
			arg->s32Argv[0] = 1;
		}
		if((SA_U8)0x1 == ((SA_U8)codec_rx_bass->BASS_CTRL.ctrl.BB & 0x1)) {
			arg->s32Argv[1] = 1;
		} else {
			arg->s32Argv[1] = 0;
		}
		if((SA_U8)0x1 == ((SA_U8)codec_rx_bass->BASS_CTRL.ctrl.BC & 0x1)) {
			arg->s32Argv[2] = 1;
		} else {
			arg->s32Argv[2] = 0;
		}
		if(arg->s32Argv[1] == 0){
			arg->s32Argv[3] = s8BassIntensityTable0[codec_rx_bass->BASS_CTRL.ctrl.BASS & 0x0f];
		} else {
			arg->s32Argv[3] = s8BassIntensityTable1[codec_rx_bass->BASS_CTRL.ctrl.BASS & 0x0f];
		}
		
		arg->s32NumArg = 4;
		break;
	case SA8900_CTL_TREBLE_CONFIG:
		if(sizeof(AUDIO_CODEC_TREBLE_CONFIG_S) > sizeof(caps.udata.data)) {
			OSWL_LOG_E("%s: AUDIO_CODEC_TREBLE_CONFIG_S size(%d) is bigger than caps data size(%d)\n", __func__, sizeof(AUDIO_CODEC_TREBLE_CONFIG_S), sizeof(caps.udata.data));
			return SA_FAILURE;
		}

		caps.main_type = AUDIO_TYPE_OUTPUT|AUDIO_TYPE_MIXER;
		caps.sub_type  = AUDIO_MIXER_TREBLE;
		ret = rt_device_control(device_handle, AUDIO_CTL_GETCAPS, &caps);
		if (ret < 0) {
			OSWL_LOG_E("%s: rt_device_control error(%d) for codec_rx_treble config\n", __func__, ret);
			return SA_FAILURE;
		}

		codec_rx_treble = (AUDIO_CODEC_TREBLE_CONFIG_S *)caps.udata.data;

		if((SA_U8)0xf == ((SA_U8)codec_rx_treble->TREBLE_CTRL.ctrl.TRBL & 0xf)) {
			arg->s32Argv[0] = 0;
		} else {
			arg->s32Argv[0] = 1;
		}
		if((SA_U8)0x1 == ((SA_U8)codec_rx_treble->TREBLE_CTRL.ctrl.TC & 0x1)) {
			arg->s32Argv[1] = 1;
		} else {
			arg->s32Argv[1] = 0;
		}
		arg->s32Argv[2] = s8TrebleIntensityTable[codec_rx_treble->TREBLE_CTRL.ctrl.TRBL & 0x0f];
		arg->s32NumArg = 3;
		break;
	case SA8900_CTL_WINDNOISE_CONFIG:
		if(((sizeof(AUDIO_CODEC_FILTER_CONFIG_S)-1)/3 + 1) > ARRAY_SIZE(arg->s32Argv)) {
			OSWL_LOG_E("%s: argv size(%d) is less than AUDIO_CODEC_FILTER_CONFIG_S required(%d)\n", __func__, ARRAY_SIZE(arg->s32Argv), (sizeof(AUDIO_CODEC_FILTER_CONFIG_S)-1)/3 + 1);
			return SA_FAILURE;
		}

		if(sizeof(AUDIO_CODEC_FILTER_CONFIG_S) > sizeof(caps.udata.data)) {
			OSWL_LOG_E("%s: AUDIO_CODEC_FILTER_CONFIG_S size(%d) is bigger than caps data size(%d)\n", __func__, sizeof(AUDIO_CODEC_FILTER_CONFIG_S), sizeof(caps.udata.data));
			return SA_FAILURE;
		}

		caps.main_type = AUDIO_TYPE_INPUT|AUDIO_TYPE_MIXER;
		caps.sub_type  = AUDIO_MIXER_FILTER;
		ret = rt_device_control(device_handle, AUDIO_CTL_GETCAPS, &caps);
		if (ret < 0) {
			OSWL_LOG_E("%s: rt_device_control error(%d) for codec_tx_filter config\n", __func__, ret);
			return SA_FAILURE;
		}

		codec_tx_filter = (AUDIO_CODEC_FILTER_CONFIG_S *)caps.udata.data;

		OSWL_LOG_I("%s: codec_tx_filter enable status %d, %d, %d, %d\n", __func__, 
			(SA_U8)codec_tx_filter->R3F.ctrl.s8Filter1Enable&0x01, (SA_U8)codec_tx_filter->R3F.ctrl.s8Filter2Enable&0x01,
			(SA_U8)codec_tx_filter->R3F.ctrl.s8Filter3Enable&0x01, (SA_U8)codec_tx_filter->R3F.ctrl.s8Filter4Enable&0x01);
		if(((SA_U8)1 == ((SA_U8)codec_tx_filter->R3F.ctrl.s8Filter1Enable & 1)) && ((SA_U8)1 == ((SA_U8)codec_tx_filter->R3F.ctrl.s8Filter2Enable & 1)) &&
		   ((SA_U8)1 == ((SA_U8)codec_tx_filter->R3F.ctrl.s8Filter3Enable & 1)) && ((SA_U8)1 == ((SA_U8)codec_tx_filter->R3F.ctrl.s8Filter4Enable & 1))) {
			arg->s32Argv[0] = 1;
		} else {
			arg->s32Argv[0] = 0;
		}

		j = 1;
		pchar = &codec_tx_filter->R40;
		for(i=0;i<sizeof(AUDIO_CODEC_FILTER_CONFIG_S);i+=3) {
			arg->s32Argv[j++] = (pchar[i] & 0xff) | ((pchar[i+1] << 8) & 0xff00) | ((pchar[i+2] << 16) & 0xff0000);
		}
		arg->s32NumArg = (sizeof(AUDIO_CODEC_FILTER_CONFIG_S)-1)/3 + 1;
		break;
	case SA8900_CTL_HARD_REFERENCE_CONFIG:
		caps.main_type               = AUDIO_TYPE_INPUT|AUDIO_TYPE_MIXER;
		caps.sub_type                = AUDIO_MIXER_HARD_REF;
		ret = rt_device_control(device_handle, AUDIO_CTL_GETCAPS, &caps);
		if (ret < 0) {
			OSWL_LOG_E("%s: rt_device_control error for codec hard refence config!\n", __func__);
			return SA_FAILURE;
		}
		arg->s32NumArg = 3;
		arg->s32Argv[0] = caps.udata.data[0];
		arg->s32Argv[1] = caps.udata.data[1];
		arg->s32Argv[2] = caps.udata.data[2];
		OSWL_LOG_I("%s: hard reference status %d %d %d\n", __func__, arg->s32Argv[0], arg->s32Argv[1], arg->s32Argv[2]);
		break;
	case SA8900_CTL_AO_VOL:
		caps.main_type               = AUDIO_TYPE_OUTPUT|AUDIO_TYPE_MIXER;
		caps.sub_type                = AUDIO_MIXER_VOLUME;
		ret = rt_device_control(device_handle, AUDIO_CTL_GETCAPS, &caps);
		if (ret < 0) {
			OSWL_LOG_E("%s: rt_device_control error for AO volume config!\n", __func__);
			return SA_FAILURE;
		}
		arg->s32NumArg = 2;
		arg->s32Argv[1] = caps.udata.value;
		OSWL_LOG_I("%s: volume is  %d\n", __func__, arg->s32Argv[1]);
		break;
	default:
		OSWL_LOG_E("%s: unsupported command(%d) err!\n", __func__, cmd);
		return SA_FAILURE;
	}

	return SA_SUCCESS;
}

static SA_S32 adev_ctl_cset(const struct SA_AUDIO_CTL_HW_DEVICE_S *dev, SA_VOID *ctl_handle, SA_U32 cmd, SA_VOID *valArg)
{
	SA_S32 i = 0;
	SA_S32 j = 0;
	SA_S32 ret = 0;
	SA_S8 *pchar;
	ACODEC_CTL_VAL_INFO_S *arg;
	rt_device_t device_handle;
	struct rt_audio_caps caps = {0};
	AUDIO_CODEC_DAC_EQ_CONFIG_S *codec_rx_eq = NULL;
	AUDIO_CODEC_ADC_EQ_CONFIG_S *codec_tx_eq = NULL;
	AUDIO_CODEC_BASS_CONFIG_S *codec_rx_bass = NULL;
	AUDIO_CODEC_TREBLE_CONFIG_S *codec_rx_treble = NULL;
	AUDIO_CODEC_FILTER_CONFIG_S *codec_tx_filter = NULL;

	if(NULL == ctl_handle) {
		OSWL_LOG_E("%s: ctl handle is NULL\n", __func__);
		return SA_FAILURE;
	}
	if(NULL == valArg) {
		OSWL_LOG_E("%s: valArg is NULL\n", __func__);
		return SA_FAILURE;
	}

	device_handle = (rt_device_t)ctl_handle;
	arg = (ACODEC_CTL_VAL_INFO_S *)valArg;
	memset(&caps, 0, sizeof(caps));

	switch (cmd)
	{
	case SA8900_CTL_ADC_EQ_CONFIG:
		if(sizeof(AUDIO_CODEC_ADC_EQ_CONFIG_S) > sizeof(caps.udata.data)) {
			OSWL_LOG_E("%s: AUDIO_CODEC_ADC_EQ_CONFIG_S size(%d) is bigger than caps data size(%d)\n", __func__, sizeof(AUDIO_CODEC_ADC_EQ_CONFIG_S), sizeof(caps.udata.data));
			return SA_FAILURE;
		}
		for(i=1;i<11;i++) {
			ret = is_value_in_array(arg->s32Argv[i], s8EqPreGainTable, sizeof(s8EqPreGainTable));
			if(ret < 0) {
				OSWL_LOG_E("%s: ADC EQ pregain(%d) is not supported for band%d\n", __func__, arg->s32Argv[i], i);
				return SA_FAILURE;
			}
		}
		for(i=11;i<21;i++) {
			ret = is_value_in_array(arg->s32Argv[i], s8EqFineGainTable, sizeof(s8EqFineGainTable));
			if(ret < 0) {
				OSWL_LOG_E("%s: ADC EQ fine gain(%d) is not supported for band%d\n", __func__, arg->s32Argv[i], i-10);
				return SA_FAILURE;
			}
		}

		codec_tx_eq = (AUDIO_CODEC_ADC_EQ_CONFIG_S *)caps.udata.data;

		if(1 == arg->s32Argv[0]) {
			codec_tx_eq->BAND_CTRL.ctrl.L_BAND0_EQ_EN = 0;
			codec_tx_eq->BAND_CTRL.ctrl.L_EQ_EN = 1;
			codec_tx_eq->BAND_CTRL.ctrl.R_BAND0_EQ_EN = 0;
			codec_tx_eq->BAND_CTRL.ctrl.R_EQ_EN = 1;

			i = 1;		
			codec_tx_eq->BAND0_GAIN.gain.pre_gain = value_to_index(arg->s32Argv[i++], s8EqPreGainTable, sizeof(s8EqPreGainTable));
			codec_tx_eq->BAND1_GAIN.gain.pre_gain = value_to_index(arg->s32Argv[i++], s8EqPreGainTable, sizeof(s8EqPreGainTable));
			codec_tx_eq->BAND2_GAIN.gain.pre_gain = value_to_index(arg->s32Argv[i++], s8EqPreGainTable, sizeof(s8EqPreGainTable));
			codec_tx_eq->BAND3_GAIN.gain.pre_gain = value_to_index(arg->s32Argv[i++], s8EqPreGainTable, sizeof(s8EqPreGainTable));
			codec_tx_eq->BAND4_GAIN.gain.pre_gain = value_to_index(arg->s32Argv[i++], s8EqPreGainTable, sizeof(s8EqPreGainTable));
			codec_tx_eq->BAND5_GAIN.gain.pre_gain = value_to_index(arg->s32Argv[i++], s8EqPreGainTable, sizeof(s8EqPreGainTable));
			codec_tx_eq->BAND6_GAIN.gain.pre_gain = value_to_index(arg->s32Argv[i++], s8EqPreGainTable, sizeof(s8EqPreGainTable));
			codec_tx_eq->BAND7_GAIN.gain.pre_gain = value_to_index(arg->s32Argv[i++], s8EqPreGainTable, sizeof(s8EqPreGainTable));
			codec_tx_eq->BAND8_GAIN.gain.pre_gain = value_to_index(arg->s32Argv[i++], s8EqPreGainTable, sizeof(s8EqPreGainTable));
			codec_tx_eq->BAND9_GAIN.gain.pre_gain = value_to_index(arg->s32Argv[i++], s8EqPreGainTable, sizeof(s8EqPreGainTable));

			codec_tx_eq->BAND0_GAIN.gain.fine_gain = value_to_index(arg->s32Argv[i++], s8EqFineGainTable, sizeof(s8EqFineGainTable));
			codec_tx_eq->BAND1_GAIN.gain.fine_gain = value_to_index(arg->s32Argv[i++], s8EqFineGainTable, sizeof(s8EqFineGainTable));
			codec_tx_eq->BAND2_GAIN.gain.fine_gain = value_to_index(arg->s32Argv[i++], s8EqFineGainTable, sizeof(s8EqFineGainTable));
			codec_tx_eq->BAND3_GAIN.gain.fine_gain = value_to_index(arg->s32Argv[i++], s8EqFineGainTable, sizeof(s8EqFineGainTable));
			codec_tx_eq->BAND4_GAIN.gain.fine_gain = value_to_index(arg->s32Argv[i++], s8EqFineGainTable, sizeof(s8EqFineGainTable));
			codec_tx_eq->BAND5_GAIN.gain.fine_gain = value_to_index(arg->s32Argv[i++], s8EqFineGainTable, sizeof(s8EqFineGainTable));
			codec_tx_eq->BAND6_GAIN.gain.fine_gain = value_to_index(arg->s32Argv[i++], s8EqFineGainTable, sizeof(s8EqFineGainTable));
			codec_tx_eq->BAND7_GAIN.gain.fine_gain = value_to_index(arg->s32Argv[i++], s8EqFineGainTable, sizeof(s8EqFineGainTable));
			codec_tx_eq->BAND8_GAIN.gain.fine_gain = value_to_index(arg->s32Argv[i++], s8EqFineGainTable, sizeof(s8EqFineGainTable));
			codec_tx_eq->BAND9_GAIN.gain.fine_gain = value_to_index(arg->s32Argv[i++], s8EqFineGainTable, sizeof(s8EqFineGainTable));
		} else {
			codec_tx_eq->BAND_CTRL.ctrl.L_BAND0_EQ_EN = 0;
			codec_tx_eq->BAND_CTRL.ctrl.L_EQ_EN = 0;
			codec_tx_eq->BAND_CTRL.ctrl.R_BAND0_EQ_EN = 0;
			codec_tx_eq->BAND_CTRL.ctrl.R_EQ_EN = 0;
		}

		caps.main_type = AUDIO_TYPE_INPUT|AUDIO_TYPE_MIXER;
		caps.sub_type  = AUDIO_MIXER_EQUALIZER;
		ret = rt_device_control(device_handle, AUDIO_CTL_CONFIGURE, &caps);
		if (ret < 0) {
			OSWL_LOG_E("%s: rt_device_control error(%d) for codec_tx_eq config\n", __func__, ret);
			return SA_FAILURE;
		}
		break;
	case SA8900_CTL_DAC_EQ_CONFIG:
		if(sizeof(AUDIO_CODEC_DAC_EQ_CONFIG_S) > sizeof(caps.udata.data)) {
			OSWL_LOG_E("%s: AUDIO_CODEC_DAC_EQ_CONFIG_S size(%d) is bigger than caps data size(%d)\n", __func__, sizeof(AUDIO_CODEC_DAC_EQ_CONFIG_S), sizeof(caps.udata.data));
			return SA_FAILURE;
		}
		for(i=1;i<11;i++) {
			ret = is_value_in_array(arg->s32Argv[i], s8EqPreGainTable, sizeof(s8EqPreGainTable));
			if(ret < 0) {
				OSWL_LOG_E("%s: DAC EQ pregain(%d) is not supported for band%d\n", __func__, arg->s32Argv[i], i);
				return SA_FAILURE;
			}
		}
		for(i=11;i<21;i++) {
			ret = is_value_in_array(arg->s32Argv[i], s8EqFineGainTable, sizeof(s8EqFineGainTable));
			if(ret < 0) {
				OSWL_LOG_E("%s: DAC EQ fine gain(%d) is not supported for band%d\n", __func__, arg->s32Argv[i], i-10);
				return SA_FAILURE;
			}
		}

		codec_rx_eq = (AUDIO_CODEC_DAC_EQ_CONFIG_S *)caps.udata.data;

		if(1 == arg->s32Argv[0]) {
			codec_rx_eq->BAND_CTRL.ctrl.L_BAND0_EQ_EN = 0;
			codec_rx_eq->BAND_CTRL.ctrl.L_EQ_EN = 1;
			codec_rx_eq->BAND_CTRL.ctrl.R_BAND0_EQ_EN = 0;
			codec_rx_eq->BAND_CTRL.ctrl.R_EQ_EN = 1;

			i = 1;
			codec_rx_eq->BAND0_GAIN.gain.pre_gain = value_to_index(arg->s32Argv[i++], s8EqPreGainTable, sizeof(s8EqPreGainTable));
			codec_rx_eq->BAND1_GAIN.gain.pre_gain = value_to_index(arg->s32Argv[i++], s8EqPreGainTable, sizeof(s8EqPreGainTable));
			codec_rx_eq->BAND2_GAIN.gain.pre_gain = value_to_index(arg->s32Argv[i++], s8EqPreGainTable, sizeof(s8EqPreGainTable));
			codec_rx_eq->BAND3_GAIN.gain.pre_gain = value_to_index(arg->s32Argv[i++], s8EqPreGainTable, sizeof(s8EqPreGainTable));
			codec_rx_eq->BAND4_GAIN.gain.pre_gain = value_to_index(arg->s32Argv[i++], s8EqPreGainTable, sizeof(s8EqPreGainTable));
			codec_rx_eq->BAND5_GAIN.gain.pre_gain = value_to_index(arg->s32Argv[i++], s8EqPreGainTable, sizeof(s8EqPreGainTable));
			codec_rx_eq->BAND6_GAIN.gain.pre_gain = value_to_index(arg->s32Argv[i++], s8EqPreGainTable, sizeof(s8EqPreGainTable));
			codec_rx_eq->BAND7_GAIN.gain.pre_gain = value_to_index(arg->s32Argv[i++], s8EqPreGainTable, sizeof(s8EqPreGainTable));
			codec_rx_eq->BAND8_GAIN.gain.pre_gain = value_to_index(arg->s32Argv[i++], s8EqPreGainTable, sizeof(s8EqPreGainTable));
			codec_rx_eq->BAND9_GAIN.gain.pre_gain = value_to_index(arg->s32Argv[i++], s8EqPreGainTable, sizeof(s8EqPreGainTable));

			codec_rx_eq->BAND0_GAIN.gain.fine_gain = value_to_index(arg->s32Argv[i++], s8EqFineGainTable, sizeof(s8EqFineGainTable));
			codec_rx_eq->BAND1_GAIN.gain.fine_gain = value_to_index(arg->s32Argv[i++], s8EqFineGainTable, sizeof(s8EqFineGainTable));
			codec_rx_eq->BAND2_GAIN.gain.fine_gain = value_to_index(arg->s32Argv[i++], s8EqFineGainTable, sizeof(s8EqFineGainTable));
			codec_rx_eq->BAND3_GAIN.gain.fine_gain = value_to_index(arg->s32Argv[i++], s8EqFineGainTable, sizeof(s8EqFineGainTable));
			codec_rx_eq->BAND4_GAIN.gain.fine_gain = value_to_index(arg->s32Argv[i++], s8EqFineGainTable, sizeof(s8EqFineGainTable));
			codec_rx_eq->BAND5_GAIN.gain.fine_gain = value_to_index(arg->s32Argv[i++], s8EqFineGainTable, sizeof(s8EqFineGainTable));
			codec_rx_eq->BAND6_GAIN.gain.fine_gain = value_to_index(arg->s32Argv[i++], s8EqFineGainTable, sizeof(s8EqFineGainTable));
			codec_rx_eq->BAND7_GAIN.gain.fine_gain = value_to_index(arg->s32Argv[i++], s8EqFineGainTable, sizeof(s8EqFineGainTable));
			codec_rx_eq->BAND8_GAIN.gain.fine_gain = value_to_index(arg->s32Argv[i++], s8EqFineGainTable, sizeof(s8EqFineGainTable));
			codec_rx_eq->BAND9_GAIN.gain.fine_gain = value_to_index(arg->s32Argv[i++], s8EqFineGainTable, sizeof(s8EqFineGainTable));
		} else {
			codec_rx_eq->BAND_CTRL.ctrl.L_BAND0_EQ_EN = 0;
			codec_rx_eq->BAND_CTRL.ctrl.L_EQ_EN = 0;
			codec_rx_eq->BAND_CTRL.ctrl.R_BAND0_EQ_EN = 0;
			codec_rx_eq->BAND_CTRL.ctrl.R_EQ_EN = 0;
		}

		caps.main_type = AUDIO_TYPE_OUTPUT|AUDIO_TYPE_MIXER;
		caps.sub_type  = AUDIO_MIXER_EQUALIZER;
		ret = rt_device_control(device_handle, AUDIO_CTL_CONFIGURE, &caps);
		if (ret < 0) {
			OSWL_LOG_E("%s: rt_device_control error(%d) for codec_rx_eq config\n", __func__, ret);
			return SA_FAILURE;
		}
		break;
	case SA8900_CTL_BASS_CONFIG:
		if(sizeof(AUDIO_CODEC_BASS_CONFIG_S) > sizeof(caps.udata.data)) {
			OSWL_LOG_E("%s: AUDIO_CODEC_BASS_CONFIG_S size(%d) is bigger than caps data size(%d)\n", __func__, sizeof(AUDIO_CODEC_BASS_CONFIG_S), sizeof(caps.udata.data));
			return SA_FAILURE;
		}

		codec_rx_bass = (AUDIO_CODEC_BASS_CONFIG_S *)caps.udata.data;

		if(arg->s32Argv[1] == 0) {
			ret = is_value_in_array(arg->s32Argv[3], s8BassIntensityTable0, sizeof(s8BassIntensityTable0));
		} else {
			ret = is_value_in_array(arg->s32Argv[3], s8BassIntensityTable1, sizeof(s8BassIntensityTable1));
		}
		if(ret < 0) {
			OSWL_LOG_E("%s: DAC bass gain(%d) is not supported\n", __func__, arg->s32Argv[3]);
			return SA_FAILURE;
		}

		if(arg->s32Argv[0]) {
			if(arg->s32Argv[1] == 0) {
				codec_rx_bass->BASS_CTRL.ctrl.BASS = value_to_index(arg->s32Argv[3], s8BassIntensityTable0, sizeof(s8BassIntensityTable0));
			} else {
				codec_rx_bass->BASS_CTRL.ctrl.BASS = value_to_index(arg->s32Argv[3], s8BassIntensityTable1, sizeof(s8BassIntensityTable1));
			}
		} else {
			codec_rx_bass->BASS_CTRL.ctrl.BASS = 0xf;
		}
		codec_rx_bass->BASS_CTRL.ctrl.BB = arg->s32Argv[1];
		codec_rx_bass->BASS_CTRL.ctrl.BC = arg->s32Argv[2];

		caps.main_type = AUDIO_TYPE_OUTPUT|AUDIO_TYPE_MIXER;
		caps.sub_type  = AUDIO_MIXER_BASS;
		ret = rt_device_control(device_handle, AUDIO_CTL_CONFIGURE, &caps);
		if (ret < 0) {
			OSWL_LOG_E("%s: rt_device_control error(%d) for codec_rx_bass config\n", __func__, ret);
			return SA_FAILURE;
		}
		break;
	case SA8900_CTL_TREBLE_CONFIG:
		if(sizeof(AUDIO_CODEC_TREBLE_CONFIG_S) > sizeof(caps.udata.data)) {
			OSWL_LOG_E("%s: AUDIO_CODEC_TREBLE_CONFIG_S size(%d) is bigger than caps data size(%d)\n", __func__, sizeof(AUDIO_CODEC_TREBLE_CONFIG_S), sizeof(caps.udata.data));
			return SA_FAILURE;
		}
		ret = is_value_in_array(arg->s32Argv[2], s8TrebleIntensityTable, sizeof(s8TrebleIntensityTable));
		if(ret < 0) {
			OSWL_LOG_E("%s: DAC treble gain(%d) is not supported\n", __func__, arg->s32Argv[2]);
			return SA_FAILURE;
		}

		codec_rx_treble = (AUDIO_CODEC_TREBLE_CONFIG_S *)caps.udata.data;

		if(arg->s32Argv[0]) {
			codec_rx_treble->TREBLE_CTRL.ctrl.TRBL = value_to_index(arg->s32Argv[2], s8TrebleIntensityTable, sizeof(s8TrebleIntensityTable));
		} else {
			codec_rx_treble->TREBLE_CTRL.ctrl.TRBL = 0xf;
		}
		codec_rx_treble->TREBLE_CTRL.ctrl.TC = arg->s32Argv[1];

		caps.main_type = AUDIO_TYPE_OUTPUT|AUDIO_TYPE_MIXER;
		caps.sub_type  = AUDIO_MIXER_TREBLE;
		ret = rt_device_control(device_handle, AUDIO_CTL_CONFIGURE, &caps);
		if (ret < 0) {
			OSWL_LOG_E("%s: rt_device_control error(%d) for codec_rx_treble config\n", __func__, ret);
			return SA_FAILURE;
		}
		break;
	case SA8900_CTL_WINDNOISE_CONFIG:
		if(((arg->s32NumArg-1) * 3 + 1) != sizeof(AUDIO_CODEC_FILTER_CONFIG_S)) {
			OSWL_LOG_E("%s: numarg(%d) is not equal to the sizeof of AUDIO_CODEC_FILTER_CONFIG_S(%d)\n", __func__, ((arg->s32NumArg-1) * 3 + 1), sizeof(AUDIO_CODEC_FILTER_CONFIG_S));
			return SA_FAILURE;
		}
		if(sizeof(AUDIO_CODEC_FILTER_CONFIG_S) > sizeof(caps.udata.data)) {
			OSWL_LOG_E("%s: AUDIO_CODEC_FILTER_CONFIG_S size(%d) is bigger than caps data size(%d)\n", __func__, sizeof(AUDIO_CODEC_FILTER_CONFIG_S), sizeof(caps.udata.data));
			return SA_FAILURE;
		}

		codec_tx_filter = (AUDIO_CODEC_FILTER_CONFIG_S *)caps.udata.data;

		if(arg->s32Argv[0]) {
			codec_tx_filter->R3F.ctrl.s8Filter1Enable = 1;
			codec_tx_filter->R3F.ctrl.s8Filter2Enable = 1;
			codec_tx_filter->R3F.ctrl.s8Filter3Enable = 1;
			codec_tx_filter->R3F.ctrl.s8Filter4Enable = 1;
		} else {
			codec_tx_filter->R3F.ctrl.s8Filter1Enable = 0;
			codec_tx_filter->R3F.ctrl.s8Filter2Enable = 0;
			codec_tx_filter->R3F.ctrl.s8Filter3Enable = 0;
			codec_tx_filter->R3F.ctrl.s8Filter4Enable = 0;
		}

		j = 0;
		pchar = &codec_tx_filter->R40;
		for(i=1;i<arg->s32NumArg;i++) {
			pchar[j++] =  arg->s32Argv[i]&0xff;
			pchar[j++] = (arg->s32Argv[i] >> 8)&0xff;
			pchar[j++] = (arg->s32Argv[i] >> 16)&0xff;
		}

		caps.main_type = AUDIO_TYPE_INPUT|AUDIO_TYPE_MIXER;
		caps.sub_type  = AUDIO_MIXER_FILTER;
		ret = rt_device_control(device_handle, AUDIO_CTL_CONFIGURE, &caps);
		if (ret < 0) {
			OSWL_LOG_E("%s: rt_device_control error(%d) for codec_tx_filter config\n", __func__, ret);
			return SA_FAILURE;
		}
		break;
	case SA8900_CTL_HARD_REFERENCE_CONFIG:
		if(arg->s32NumArg != 3) {
			OSWL_LOG_E("%s: parameter of SA8900_CTL_HARD_REFERENCE_CONFIG must 3\n", __func__);
			return SA_FAILURE;
		}
		caps.main_type               = AUDIO_TYPE_INPUT|AUDIO_TYPE_MIXER;
		caps.sub_type                = AUDIO_MIXER_HARD_REF;
		caps.udata.data[0]           = arg->s32Argv[0];
		caps.udata.data[1]           = arg->s32Argv[1];
		caps.udata.data[2]           = arg->s32Argv[2];
		ret = rt_device_control(device_handle, AUDIO_CTL_CONFIGURE, &caps);
		if (ret < 0) {
			OSWL_LOG_E("%s: rt_device_control error for codec hard refence config!\n", __func__);
			return SA_FAILURE;
		}
		break;
	case SA8900_CTL_AO_VOL:
		if(arg->s32NumArg != 2) {
			OSWL_LOG_E("%s: parameter of SA8900_CTL_AO_VOL must 2\n", __func__);
			return SA_FAILURE;
		}
		if((arg->s32Argv[1] < 0) || (arg->s32Argv[1] > 255)) {
			OSWL_LOG_E("%s: volume(%d) should be in range [0, 255]\n", __func__);
			return SA_FAILURE;
		}

		caps.main_type               = AUDIO_TYPE_OUTPUT|AUDIO_TYPE_MIXER;
		caps.sub_type                = AUDIO_MIXER_VOLUME;
		caps.udata.value             = arg->s32Argv[1];
		ret = rt_device_control(device_handle, AUDIO_CTL_CONFIGURE, &caps);
		if (ret < 0) {
			OSWL_LOG_E("%s: rt_device_control error for AO volume config!\n", __func__);
			return SA_FAILURE;
		}
		break;
	default:
		OSWL_LOG_E("%s: unsupported command(%d) err!\n", __func__, cmd);
		return SA_FAILURE;
	}
	return SA_SUCCESS;
}

static SA_S32 adev_ctl_open(const struct SA_AUDIO_CTL_HW_DEVICE_S *dev, const AIO_CTL_ATTR_S *pAttr, SA_VOID **ctlHandle)
{
	rt_device_t handle;
	char devName[32];

	if((SA_NULL == pAttr)||(SA_NULL == dev)) {
		return -1; 
	}

	if((pAttr->s32SoundDevId < 0) || (pAttr->s32SoundDevId > 2)) {
		OSWL_LOG_E("err pAttr->s32SoundDevId!\n");
		return -1; 
	}

	switch (pAttr->s32SoundDevId) {
		case AUDIO_HW_SOUND_I2S0:
		snprintf(devName, sizeof(devName), "sound0");
		break;
		
		case AUDIO_HW_SOUND_I2S1:
		snprintf(devName, sizeof(devName), "sound1");
		break;

		case AUDIO_HW_SOUND_PDM:
		snprintf(devName, sizeof(devName), "pdm2");
		break;
				
		default:
		OSWL_LOG_E("no support sound_id\n");
		return -1;
	}

	handle = rt_device_find(devName);
	if (NULL == handle) {
		OSWL_LOG_E("%s: rt_device_find err!\n", __func__);
		return SA_FAILURE;
	}

	*ctlHandle = handle;

	return SA_SUCCESS;
}

static SA_S32 adev_ctl_close(const struct SA_AUDIO_CTL_HW_DEVICE_S *dev,SA_VOID *ctlHandle)
{
	return SA_SUCCESS;
}

SA_S32 oswl_ctl_device_open(const SA_CHAR *name, struct SA_AUDIO_CTL_HW_DEVICE_S **device)
{
	static GENERIC_AUDIO_CTL_HW_DEVICE_S *adev; 

	if (0 != strcmp(name, AUDIO_HW_INTERFACE)) {
		SA_ERR("oswl_audio_hw_device_open name error!\n");
		audioErrorCode = AUDIO_ERROR_CODE_DEVICE_ERROR;
		return -1;
	}

	if (0 != audioDeviceRefCount) {
		SA_ERR("oswl_audio_hw_device_open device exits\n");
		audioErrorCode = AUDIO_ERROR_CODE_DEVICE_ERROR;
		return -1;
	}

	adev = (GENERIC_AUDIO_CTL_HW_DEVICE_S *)oswl_malloc(sizeof(GENERIC_AUDIO_CTL_HW_DEVICE_S));
	if (SA_NULL == adev) {
		SA_ERR("oswl_audio_hw_device_open adev malloc error!\n");
		audioErrorCode = AUDIO_ERROR_CODE_NULL_POINTER;
		return -1;
	}

	adev->device.ctl_open = adev_ctl_open;
	adev->device.ctl_cset = adev_ctl_cset;
	adev->device.ctl_cget = adev_ctl_cget;
	adev->device.ctl_close = adev_ctl_close;

	*device = &adev->device;
	adev->isInit = SA_TRUE;
	audioDeviceRefCount++;

	return SA_SUCCESS;
}


SA_S32 oswl_ctl_device_close(struct SA_AUDIO_CTL_HW_DEVICE_S *device)
{

	struct SA_GENERIC_AUDIO_CTL_HW_DEVICE_S *adev = (struct SA_GENERIC_AUDIO_CTL_HW_DEVICE_S *)device;

	if (0 == audioDeviceRefCount) {
		SA_ERR("oswl_audio_hw_device_close audioDeviceRefCount error!\n");
		audioErrorCode = AUDIO_ERROR_CODE_DEVICE_ERROR;
		return -1;
	}

	oswl_free(adev);
	audioDeviceRefCount = 0;

	return SA_SUCCESS;
}