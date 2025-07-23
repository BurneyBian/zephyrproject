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

/**
 * @brief PIN configuration bitfield
 *
 * Pin configuration is coded with the following
 * fields
 *    Alternate Functions [ 0 : 3 ]
 *    GPIO Mode           [ 7 : 8 ]
 *    GPIO Output type    [ 0 ]
 *    GPIO Speed          [ 4 : 5  : 6 ]
 *    GPIO PUPD config    [ 2 : 3 ]
 *
 */

/* GPIO Mode */
#define SA6920_MODER_INPUT_MODE		(0x0 << SA6920_MODER_SHIFT)
#define SA6920_MODER_OUTPUT_MODE		(0x1 << SA6920_MODER_SHIFT)
#define SA6920_MODER_ALT_MODE		(0x2 << SA6920_MODER_SHIFT)
#define SA6920_MODER_ANALOG_MODE		(0x3 << SA6920_MODER_SHIFT)
#define SA6920_MODER_MASK	 	0x3
#define SA6920_MODER_SHIFT		2

/* GPIO Output type */
#define SA6920_OTYPER_PUSH_PULL		(0x1 << SA6920_PULL_SHIFT)
#define SA6920_OTYPER_OPEN_DRAIN		(0x1 << SA6920_OTYPER_SHIFT)
#define SA6920_OTYPER_MASK		0x3
#define SA6920_OTYPER_SHIFT		1
#define SA6920_PULL_SHIFT		2

/* driver strengt */
#define SA6920_DRIVER_STRENGTH_0    (0x0 << SA6920_OSPEEDR_SHIFT)
#define SA6920_DRIVER_STRENGTH_1 	(0x1 << SA6920_OSPEEDR_SHIFT)
#define SA6920_DRIVER_STRENGTH_2 	(0x2 << SA6920_OSPEEDR_SHIFT)
#define SA6920_DRIVER_STRENGTH_3 	(0x3 << SA6920_OSPEEDR_SHIFT)
#define SA6920_DRIVER_STRENGTH_4 	(0x4 << SA6920_OSPEEDR_SHIFT)
#define SA6920_DRIVER_STRENGTH_5	(0x5 << SA6920_OSPEEDR_SHIFT)
#define SA6920_OSPEEDR_MASK		0x7
#define SA6920_OSPEEDR_SHIFT	4

/* GPIO High impedance/Pull-up/pull-down */
#define SA6920_PUPDR_NO_PULL		(0x2 << SA6920_PUPDR_SHIFT)
#define SA6920_PUPDR_PULL_UP		(0x3 << SA6920_PUPDR_SHIFT)
#define SA6920_PUPDR_PULL_DOWN		(0x1 << SA6920_PUPDR_SHIFT)
#define SA6920_PUPDR_MASK		0x3
#define SA6920_PUPDR_SHIFT		2

/* GPIO plain output value */
#define SA6920_ODR_0			(0x0 << SA6920_ODR_SHIFT)
#define SA6920_ODR_1			(0x1 << SA6920_ODR_SHIFT)
#define SA6920_ODR_MASK			0x1
#define SA6920_ODR_SHIFT			11

#endif /* ZEPHYR_INCLUDE_DRIVERS_PINCTRL_PINCTRL_SOC_ACME_COMMON_H_ */

