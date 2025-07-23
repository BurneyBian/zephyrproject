#ifndef OSWL_M_ID_H
#define OSWL_M_ID_H

/*8bit表示 模块，一个只对应一个LAPI*/
#define SUB_MID_NUM(x, y) ((x & 0x0f)<< 4 | (y & 0x0f))

//main module 0~15
#define OSWL_MOD_SYS		0  /*没有LAPI的都放到  这里 */
#define OSWL_MOD_VO			1
#define OSWL_MOD_ISP		2
#define OSWL_MOD_ISP_LITE	3
#define OSWL_MOD_NPU		4 //anne vnne npu
#define OSWL_MOD_VPU		5
#define OSWL_MOD_AUD		6
#define OSWL_MOD_VPSS		7
#define OSWL_MOD_VI			8


//each sub module 0~15
#define OSWL_MOD_ISP_BE SUB_MID_NUM(OSWL_MOD_ISP, 0)
#define OSWL_MOD_ISP_FE_0 SUB_MID_NUM(OSWL_MOD_ISP, 1)
#define OSWL_MOD_ISP_FE_1 SUB_MID_NUM(OSWL_MOD_ISP, 2)
#define OSWL_MOD_ISP_FE_2 SUB_MID_NUM(OSWL_MOD_ISP, 3)
#define OSWL_MOD_ISP_FE_3 SUB_MID_NUM(OSWL_MOD_ISP, 4)
#define OSWL_MOD_ISP_PE SUB_MID_NUM(OSWL_MOD_ISP, 5)

#define OSWL_MOD_VI_0 SUB_MID_NUM(OSWL_MOD_VI, 0)
#define OSWL_MOD_VI_1 SUB_MID_NUM(OSWL_MOD_VI, 1)


#define OSWL_MOD_VPU_JPEG_ENC SUB_MID_NUM(OSWL_MOD_VPU, 0)
#define OSWL_MOD_VPU_JPEG_DEC SUB_MID_NUM(OSWL_MOD_VPU, 1)
#define OSWL_MOD_VPU_H264_H265 SUB_MID_NUM(OSWL_MOD_VPU, 2)

#define OSWL_MOD_VO_a SUB_MID_NUM(OSWL_MOD_VO, 0)
#define OSWL_MOD_VO_b SUB_MID_NUM(OSWL_MOD_VO, 1)
#define OSWL_MOD_VO_c SUB_MID_NUM(OSWL_MOD_VO, 2)
#define OSWL_MOD_VO_d SUB_MID_NUM(OSWL_MOD_VO, 3)

#endif
