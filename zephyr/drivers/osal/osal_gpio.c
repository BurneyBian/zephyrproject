#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

#include <rtthread.h>
#include <rtdevice.h>
#include <stdlib.h>
#include <string.h>
#include "rtconfig.h"
#include <rthw.h>

#ifdef RT_USING_PIN
//#include <pin.h>
int osal_gpio_request(unsigned int gpio, const char *label)
{
	return 0;
}

void osal_gpio_free(unsigned int gpio)
{
}

int osal_gpio_direction_input(unsigned int gpio)
{
	rt_pin_mode(gpio, PIN_MODE_INPUT);
	return 0;
}

int osal_gpio_direction_output(unsigned int gpio, int value)
{
	rt_pin_mode(gpio, PIN_MODE_OUTPUT);
	rt_pin_write(gpio, value);
	return 0;
}

int osal_gpio_get_value(unsigned int gpio)
{
	return rt_pin_read(gpio);
}

void osal_gpio_set_value(unsigned int gpio, int value)
{
	rt_pin_write(gpio, value);
}

#elif defined(BSP_USING_PIN)
#include "sa_hal_gpio.h"

int osal_gpio_request(unsigned int gpio, const char *label)
{
	return 0;
}

void osal_gpio_free(unsigned int gpio)
{

}

int osal_gpio_direction_input(unsigned int gpio)
{
	return gpio_direction_input(gpio);
}

int osal_gpio_direction_output(unsigned int gpio, int value)
{
    int ret = gpio_direction_output(gpio, value);
	if (!ret) {
		gpio_set_value(gpio, value);
		return 0;
	}

	return ret;
}

int osal_gpio_get_value(unsigned int gpio)
{
	return gpio_get_value(gpio);
}

void osal_gpio_set_value(unsigned int gpio, int value)
{
	gpio_set_value(gpio, value);
}

#else

int osal_gpio_request(unsigned int gpio, const char *label)
{
	osal_printk("%s Warning:Not support\n", __FUNCTION__);
	return 0;
}

void osal_gpio_free(unsigned int gpio)
{
	osal_printk("%s Warning:Not support\n", __FUNCTION__);
}

int osal_gpio_direction_input(unsigned int gpio)
{
	osal_printk("%s Warning:Not support\n", __FUNCTION__);
	return 0;
}

int osal_gpio_direction_output(unsigned int gpio, int value)
{
	osal_printk("%s Warning:Not support\n", __FUNCTION__);
	return 0;
}

int osal_gpio_get_value(unsigned int gpio)
{
	osal_printk("%s Warning:Not support\n", __FUNCTION__);
	return 0;
}

void osal_gpio_set_value(unsigned int gpio, int value)
{
	osal_printk("%s Warning:Not support\n", __FUNCTION__);
}
#endif

RTM_EXPORT(osal_gpio_request);
RTM_EXPORT(osal_gpio_free);
RTM_EXPORT(osal_gpio_direction_input);
RTM_EXPORT(osal_gpio_direction_output);
RTM_EXPORT(osal_gpio_get_value);
RTM_EXPORT(osal_gpio_set_value);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

