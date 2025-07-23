
/*
 * Copyright (c) 2016 Open-RnD Sp. z o.o.
 * Copyright (c) 2016 BayLibre, SAS
 * Copyright (c) 2017 Linaro Limited.
 * Copyright (c) 2017 RnDity Sp. z o.o.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef ZEPHYR_INCLUDE_DRIVERS_CLOCK_CONTROL_SA6920_CLOCK_CONTROL_H_
#define ZEPHYR_INCLUDE_DRIVERS_CLOCK_CONTROL_SA6920_CLOCK_CONTROL_H_

#include <zephyr/drivers/clock_control.h>
#include <zephyr/dt-bindings/clock/sa6920_clock.h>


#define SA6920_CLOCK_CONTROLLER DEVICE_DT_GET(DT_NODELABEL(crg))
#endif 