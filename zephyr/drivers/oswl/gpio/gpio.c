#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <rtthread.h>
#include <rtdevice.h>
#include "oswl.h"
#include "oswl_gpio.h"
 /****************************************************************
 * Constants
 ****************************************************************/
 
#define SYSFS_GPIO_DIR "/dev/gpio"
#define MAX_BUF 64
 
/****************************************************************
 * gpio_set_dir
 ****************************************************************/
int oswl_gpio_set_dir(unsigned int gpio, unsigned int out_flag)
{
	char buf[MAX_BUF];
	unsigned int  portpin;
 
	if (gpio > 94) {
		OSWL_LOG_E("gpio:%d/set-dir failed! gpio num (0~94)\n", gpio);
		return -1;
	}

	snprintf(buf, sizeof(buf), "P%d", gpio);
	portpin = rt_pin_get(buf);
	if (out_flag)
	{
		rt_pin_mode(portpin, PIN_MODE_OUTPUT);
	}
	else
	{
		rt_pin_mode(portpin, PIN_MODE_INPUT);
	}

	return 0;
}

/****************************************************************
 * gpio_set_value
 ****************************************************************/
int oswl_gpio_set_value(unsigned int gpio, unsigned int value)
{
	char buf[MAX_BUF];
	unsigned int  portpin;

	if (gpio > 94) {
		OSWL_LOG_E("gpio:%d/set-value failed! gpio num (0~94)\n", gpio);
		return -1;
	}

	snprintf(buf, sizeof(buf), "P%d", gpio);
	portpin = rt_pin_get(buf);
	rt_pin_mode(portpin, PIN_MODE_OUTPUT);
	if (value)
	{
		rt_pin_write(portpin, PIN_HIGH);
	}
	else
	{
		rt_pin_write(portpin, PIN_LOW);
	}
 
	return 0;
}
 
/****************************************************************
 * gpio_get_value
 ****************************************************************/
int oswl_gpio_get_value(unsigned int gpio, unsigned int *value)
{
	char buf[MAX_BUF];
	unsigned int  portpin;

	if (gpio > 94) {
		OSWL_LOG_E("gpio:%d/get-value failed! gpio num (0~87)\n", gpio);
		return -1;
	}

	snprintf(buf, sizeof(buf), "P%d", gpio);
	portpin = rt_pin_get(buf);

	*value = rt_pin_read(portpin);

	return 0;
}

int oswl_gpio_get(unsigned int gpio)
{
	return 0;
}

void oswl_gpio_release(unsigned int gpio)
{
    
}

RTM_EXPORT(oswl_gpio_set_dir);
RTM_EXPORT(oswl_gpio_set_value);
RTM_EXPORT(oswl_gpio_get_value);
RTM_EXPORT(oswl_gpio_get);
RTM_EXPORT(oswl_gpio_release);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
