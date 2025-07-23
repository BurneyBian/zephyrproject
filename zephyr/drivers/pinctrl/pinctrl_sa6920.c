/*
 * Copyright (c) 2016 Open-RnD Sp. z o.o.
 * Copyright (c) 2021 Linaro Limited
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/init.h>
#include <zephyr/drivers/pinctrl.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(pinctrl, CONFIG_PINCTRL_LOG_LEVEL);

/** Helper to extract IO port number from SA6920_PINMUX() encoded value */
#define SA6920_DT_PINMUX_PORT(__pin) \
	((__pin) >> 16)

/** Helper to extract IO pin number from SA6920_PINMUX() encoded value */
#define SA6920_DT_PINMUX_LINE(__pin) \
	(((__pin) >> 8) & 0xff)

/** Helper to extract IO pin func from SA6920_PINMUX() encoded value */
#define SA6920_DT_PINMUX_FUNC(__pin) \
	(((__pin) & 0x3) << 7)


/** Utility macro that expands to the PORT port address if it exists */
#define SA_PORT_ADDR_OR_NONE(nodelabel)					\
	IF_ENABLED(DT_NODE_EXISTS(DT_NODELABEL(nodelabel)),			\
		   (DT_REG_ADDR(DT_NODELABEL(nodelabel)),))

static const uint32_t sa6920_port_addrs[] = {
	SA_PORT_ADDR_OR_NONE(gpioa)
	SA_PORT_ADDR_OR_NONE(gpiob)
	SA_PORT_ADDR_OR_NONE(gpioc)
	SA_PORT_ADDR_OR_NONE(gpiod)
	SA_PORT_ADDR_OR_NONE(gpioe)
	SA_PORT_ADDR_OR_NONE(gpiof)
    SA_PORT_ADDR_OR_NONE(gpioh)
};

static int SA6920_pin_configure(uint32_t mux, uint32_t pin_cgf)
{
    uint32_t port =  SA6920_DT_PINMUX_PORT(mux);
    uint32_t line =  SA6920_DT_PINMUX_LINE(mux);
    uint32_t pin_func =  SA6920_DT_PINMUX_FUNC(mux);
    uintptr_t base_reg = sa6920_port_addrs[port];
    uint32_t val;

    val = sys_read32(base_reg + line *4);
    val &= ~(0x1FC0);//bit2-8  pull-up[2:3],driver-strength[4-6],func[7-8]

    val |= pin_func;
    val |= pin_cgf;

    sys_write32(val, base_reg + line *4);

	return 0;
}

int pinctrl_configure_pins(const pinctrl_soc_pin_t *pins, uint8_t pin_cnt,
			   uintptr_t reg)
{
	uint32_t mux ,pin_cgf = 0;
	int ret = 0;

    ARG_UNUSED(reg);

	for (uint8_t i = 0U; i < pin_cnt; i++) {
		mux = pins[i].pinmux;
        pin_cgf = pins[i].pincfg;
		ret = SA6920_pin_configure(mux, pin_cgf);
		if (ret < 0) {
			return ret;
		}
	}
	return 0;
}
