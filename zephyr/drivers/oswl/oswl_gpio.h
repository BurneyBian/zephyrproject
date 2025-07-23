#ifndef __OSWL_GPIO_H__
#define __OSWL_GPIO_H__

#ifdef SA6920_SOC
#define A_GPIO(pin)			(pin)
#define D_GPIO(pin)			(pin + 0x8)
#define B_GPIO(pin)			(pin + 0x19)
#define E_GPIO(pin)			(pin + 0x1B)
#define C_GPIO(pin)			(pin + 0x29)
#define F_GPIO(pin)			(pin + 0x2F)
#define H_GPIO(pin)			(pin + 0x4A)
#endif

int oswl_gpio_set_dir(unsigned int gpio, unsigned int out_flag);

int oswl_gpio_set_value(unsigned int gpio, unsigned int value);

int oswl_gpio_get_value(unsigned int gpio, unsigned int *value);

int oswl_gpio_get(unsigned int gpio);

void oswl_gpio_release(unsigned int gpio);
#endif