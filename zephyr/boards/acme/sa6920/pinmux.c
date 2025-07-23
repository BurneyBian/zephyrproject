/*
 * Copyright (c) 2016 Linaro Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/device.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <soc.h>
#include <zephyr/sys/sys_io.h>

static int _pinmux_init(void)
{
    //TODO:
	//i2c0-pad
	// sys_write32(0x3,0x480800b0);
	// sys_write32(0x3,0x480800b4);
	return 0;
}

SYS_INIT(_pinmux_init, PRE_KERNEL_1,
	 CONFIG_KERNEL_INIT_PRIORITY_DEFAULT);
