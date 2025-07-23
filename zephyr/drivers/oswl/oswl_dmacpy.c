#include <rthw.h>
#include "rtconfig.h"
#include <rtdevice.h>
#include <rtthread.h>

#ifdef BSP_USING_DMAC
#include "sa_dmac_if.h"

extern unsigned long osal_dmacpy_init(unsigned long dev);
extern void osal_dmacpy_deinit(int fd);
extern int osal_dmacpy_start(int fd, unsigned long sar, unsigned long dar, unsigned int len);

int oswl_dmacpy_init(unsigned long dev)
{        
    return osal_dmacpy_init(dev);
}

void oswl_dmacpy_deinit(int fd)
{
    osal_dmacpy_deinit(fd);
}

int oswl_dmacpy_start(int fd, unsigned long sar, unsigned long dst, unsigned int len)
{ 
    return osal_dmacpy_start(fd, sar, dst, len);
}
#endif