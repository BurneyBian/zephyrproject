#!/bin/sh

source /opt/zephyrproject/v4.1.0/.venv/bin/activate
export ZEPHYR_TOOLCHAIN_VARIANT=cross-compile
export CROSS_COMPILE=/opt/zephyrproject/v4.1.0/arm-zephyr-eabi/bin/arm-zephyr-eabi-
west zephyr-export
