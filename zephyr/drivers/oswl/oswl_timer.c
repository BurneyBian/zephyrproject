#include <rtthread.h>
#include <rthw.h>
#include "oswl.h"

void oswl_udelay(unsigned int usecs)
{
	rt_hw_us_delay(usecs);
}

void oswl_mdelay(unsigned int msecs)
{
    rt_thread_mdelay(msecs);
}

RTM_EXPORT(oswl_mdelay);
