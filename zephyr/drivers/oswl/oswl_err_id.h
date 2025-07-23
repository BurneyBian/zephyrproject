#ifndef OSWL_ERR_ID_H
#define OSWL_ERR_ID_H
#include "oswl_module_id.h"

#define OSWL_UMID_ERR(x, y) ((x & 0xff)<< 8 | (y & 0x7f) | 0x80)


//error_code


/*16bit 高8位是模块，低七位有效errid   */
#define OSWL_MOD_VI_0_error1 OSWL_UMID_ERR(OSWL_MOD_VI_0, 1)

#define OSWL_MOD_VI_0_error2 OSWL_UMID_ERR(OSWL_MOD_VI_0, 2)
//todo: add error_code here!

#endif