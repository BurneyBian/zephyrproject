#include <rtthread.h>
#include <rthw.h>
#include <stdlib.h>
#include "osal.h"

int osal_get_random_value(void)
{
	int val;

	srand(rt_tick_get());
	val = rand();

	return val;
}

RTM_EXPORT(osal_get_random_value);