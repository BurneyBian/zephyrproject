/* SPDX-License-Identifier: (GPL-2.0+ OR BSD-3-Clause) */
/*
 * Copyright (C) Micro BT Electronics Technology 2024 - All Rights Reserved
 * Author: renhaibo <renhaibo@microbt.com> for Micro BT Electronics Technology.
 */


#ifndef ZEPHYR_INCLUDE_DRIVERS_PINCTRL_PINCTRL_SOC_ACME_COMMON_H_
#define ZEPHYR_INCLUDE_DRIVERS_PINCTRL_PINCTRL_SOC_ACME_COMMON_H_

/*  define PIN modes */
#define GPIO	0x3
#define AF0	    0x0
#define AF1	    0x1
#define AF2	    0x2
#define AF4	    0x4
#define AF5	    0x5
#define AF6	    0x6
#define AF7	    0x7

/* define Pins number*/
#define PIN_NO(port, line)	((((port) - 'A') << 8) + (line))

#define ACME_PINMUX(port, line, mode) (((PIN_NO(port, line)) << 8) | (mode))

#endif /* ZEPHYR_INCLUDE_DRIVERS_PINCTRL_PINCTRL_SOC_ACME_COMMON_H_ */

