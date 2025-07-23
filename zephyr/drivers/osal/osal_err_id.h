#ifndef OSAL_ERR_ID_H
#define OSAL_ERR_ID_H
#include "osal_module_id.h"

#define OSAL_KMID_ERR(x, y) ((x & 0xff)<< 8 | (y & 0x7f))

//error_code

/*16bit 高8位是模块，低七位有效errid   */
#define OSAL_MOD_VO_d_error1 OSAL_KMID_ERR(OSAL_MOD_VO_d, 1)
#define OSAL_MOD_VO_d_error2 OSAL_KMID_ERR(OSAL_MOD_VO_d, 2)
//todo: add error_code here!

#endif
