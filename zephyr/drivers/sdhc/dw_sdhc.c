/*
 * Copyright (c) 2025 Advanced Micro Devices, Inc. (AMD)
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT snps_designware_sdhc

#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sdhc.h>
#include <zephyr/sd/sd_spec.h>
#include <zephyr/sd/sd.h>
#include <zephyr/sys/util.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/clock_control.h>
#include "dw_sdhc.h"
#include <stdint.h>

#include <zephyr/cache.h>
#include <zephyr/arch/cpu.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/kernel.h>

#include <soc.h>

LOG_MODULE_REGISTER(dw_sdhc, CONFIG_SD_LOG_LEVEL);

#define CHECK_BITS(b) ((uint64_t)1 << (b))

#define SDHC_SLOT_TYPE(dev) \
	((((struct sd_data *)dev->data)->props.host_caps.slot_type != 0) ? \
	 SDHC_EMMC_SLOT : SDHC_SD_SLOT)

#define SDHC_GET_HOST_PROP_BIT(cap, b) ((uint8_t)((cap & (CHECK_BITS(b))) >> b))

/**
 * @brief ADMA2 descriptor table structure.
 */
typedef struct {
	/**< Attributes of descriptor */
	uint16_t attribute;
	/**< Length of current dma transfer max 64kb */
	uint16_t length;
	/**< source/destination address for current dma transfer */
	uint64_t address;
} __packed adma2_descriptor;

/**
 * @brief Holds device private data.
 */
struct sd_data {
	DEVICE_MMIO_RAM;
	/**< Current I/O settings of SDHC */
	struct sdhc_io host_io;
	/**< Supported properties of SDHC */
	struct sdhc_host_props props;
	/**< SDHC IRQ events */
	struct k_event irq_event;
	/**< Used to identify HC internal phy register */
	bool has_phy;
	/**< transfer mode and data direction */
	uint16_t transfermode;
	/**< Maximum input clock supported by HC */
	uint32_t maxclock;
	/**< ADMA descriptor table */
	adma2_descriptor adma2_descrtbl[MAX(1, CONFIG_HOST_ADMA2_DESC_SIZE)];
};

/**
 * @brief Holds SDHC configuration data.
 */
struct sdhci_config {
	/* MMIO mapping information for SDHC register base address */
	DEVICE_MMIO_ROM;
	/**< Pointer to the device structure representing the clock bus */
	const struct device *clock_dev;
	/**< Callback to the device interrupt configuration api */
	void (*irq_config_func)(const struct device *dev);
	/**< Card detection pin available or not */
	bool broken_cd;
	/**< Support hs200 mode. */
	bool hs200_mode;
	/**< Support hs400 mode */
	bool hs400_mode;
	/**< delay given to card to power up or down fully */
	uint16_t powerdelay;
	uint16_t clkid;
	uintptr_t base_addr;
};

/**
 * @brief
 * polled wait for selected number of 32 bit events
 */
static int8_t sdhc_waitl_events(const void *base, int32_t timeout_ms, uint32_t events,
		uint32_t value)
{
	int8_t ret = -EAGAIN;

	for (uint32_t retry = 0; retry < timeout_ms; retry++) {
		if ((*((volatile uint32_t *)base) & events) == value) {
			ret = 0;
			break;
		}
		k_msleep(1);
	}

	return ret;
}

/**
 * @brief
 * polled wait for selected number of 8 bit events
 */
static int8_t sdhc_waitb_events(const void *base, int32_t timeout_ms, uint32_t events,
		uint32_t value)
{
	int8_t ret = -EAGAIN;

	for (uint32_t retry = 0; retry < timeout_ms; retry++) {
		if ((*((volatile uint8_t *)base) & events) == value) {
			ret = 0;
			break;
		}
		k_msleep(1);
	}

	return ret;
}

/**
 * @brief
 * Polled wait for any one of the given event
 */
static int8_t sdhc_wait_for_events(const void *base, int32_t timeout_ms,
		uint32_t events)
{
	int8_t ret = -EAGAIN;

	for (uint32_t retry = 0; retry < timeout_ms; retry++) {
		if ((*((volatile uint32_t *)base) & events) != 0U) {
			ret = 0;
			break;
		}
		k_msleep(1);
	}

	return ret;
}

/**
 * @brief
 * Check card is detected by host
 */
static int sdhci_card_detect(const struct device *dev)
{
	const volatile struct reg_base *reg = (struct reg_base *)DEVICE_MMIO_GET(dev);
	const struct sdhci_config *config = dev->config;
	uintptr_t base_addr = reg;

	if ((sys_read32(base_addr + SDHCI_PRESENT_STATE) & SDHC_PSR_CARD_INSRT_MASK) != 0U) {
		return 1;
	}

	/* In case of polling always treat card is detected */
	if (config->broken_cd == true) {
		return 1;
	}

	return 0;
}

/**
 * @brief
 * Clear the controller status registers
 */
static void sdhci_clear_intr(volatile struct reg_base *reg)
{
	uintptr_t base_addr = reg;
	sys_write32(0xffffffff,base_addr + SDHCI_INT_STATUS);
}

/**
 * @brief
 * Setup ADMA2 discriptor table for data transfer
 */
static int sdhci_setup_adma(const struct device *dev, const struct sdhc_data *data)
{
	volatile struct reg_base *reg = (struct reg_base *)DEVICE_MMIO_GET(dev);
	struct sd_data *dev_data = dev->data;
	uint32_t adma_table;
	uint32_t descnum;
	const uint8_t *buff = data->data;
	int ret = 0;
	uintptr_t base_addr = reg;

	if ((data->block_size * data->blocks) < SDHC_DESC_MAX_LENGTH) {
		adma_table = 1U;
	} else {
		adma_table = ((data->block_size * data->blocks) /
				SDHC_DESC_MAX_LENGTH);
		if (((data->block_size * data->blocks) % SDHC_DESC_MAX_LENGTH) != 0U) {
			adma_table += 1U;
		}
	}

	if (adma_table > CONFIG_HOST_ADMA2_DESC_SIZE) {
		LOG_ERR("Descriptor size is too big");
		return -ENOTSUP;
	}

	for (descnum = 0U; descnum < (adma_table - 1U); descnum++) {
		dev_data->adma2_descrtbl[descnum].address =
			((uintptr_t)buff + (descnum * SDHC_DESC_MAX_LENGTH));
		dev_data->adma2_descrtbl[descnum].attribute =
			SDHC_DESC_TRAN | SDHC_DESC_VALID;
		dev_data->adma2_descrtbl[descnum].length = 0U;
	}

	dev_data->adma2_descrtbl[adma_table - 1U].address =
		((uintptr_t)buff + (descnum * SDHC_DESC_MAX_LENGTH));
	dev_data->adma2_descrtbl[adma_table - 1U].attribute = SDHC_DESC_TRAN |
		SDHC_DESC_END | SDHC_DESC_VALID;
	dev_data->adma2_descrtbl[adma_table - 1U].length =
		((data->blocks * data->block_size) - (descnum * SDHC_DESC_MAX_LENGTH));

	//reg->adma_sys_addr = ((uintptr_t)&(dev_data->adma2_descrtbl[0]) & ~(uintptr_t)0x0);
	sys_write32(((uintptr_t)&(dev_data->adma2_descrtbl[0]) & ~(uintptr_t)0x0),base_addr + SDHCI_ADMA_ADDRESS);

	//arch_dcache_flush_range(dev_data->adma2_descrtbl,64);

	return ret;
}

/**
 * @brief
 * Frame the command
 */
static uint16_t sdhc_cmd_frame(struct sdhc_command *cmd, bool data, uint8_t slottype)
{
	uint16_t command = (cmd->opcode << SDHC_OPCODE_SHIFT);

	switch (cmd->response_type & SDHC_RESP) {
	case SD_RSP_TYPE_NONE:
		command |= RESP_NONE;
		break;

	case SD_RSP_TYPE_R1:
		command |= RESP_R1;
		break;

	case SD_RSP_TYPE_R1b:
		command |= RESP_R1B;
		break;

	case SD_RSP_TYPE_R2:
		command |= RESP_R2;
		break;

	case SD_RSP_TYPE_R3:
		command |= RESP_R3;
		break;

	case SD_RSP_TYPE_R6:
		command |= RESP_R6;
		break;

	case SD_RSP_TYPE_R7:
		/* As per spec, EMMC does not support R7 */
		if (slottype == SDHC_EMMC_SLOT) {
			return SDHC_CMD_RESP_INVAL;
		}
		command |= RESP_R1;
		break;

	default:
		LOG_DBG("Invalid response type");
		return SDHC_CMD_RESP_INVAL;
	}

	/* EMMC does not support APP command */
	if ((cmd->opcode == SD_APP_CMD) && (slottype == SDHC_EMMC_SLOT)) {
		LOG_DBG("Invalid response type");
		return SDHC_CMD_RESP_INVAL;
	}

	if (data) {
		command |= SDHC_DAT_PRESENT_SEL_MASK;
	}

	return command;
}

/**
 * @brief
 * Check command response is success or failed also clears status registers
 */
static int8_t sdhc_cmd_response(const struct device *dev, struct sdhc_command *cmd)
{
	const struct sdhci_config *config = dev->config;
	volatile struct reg_base *reg = (struct reg_base *)DEVICE_MMIO_GET(dev);
	struct sd_data *dev_data = dev->data;
	uint32_t mask;
	uint32_t events;
	int8_t ret;
	k_timeout_t timeout;
	uintptr_t base_addr = reg;

	mask = SDHC_INTR_ERR_MASK | SDHC_INTR_CC_MASK;
	if ((cmd->opcode == SD_SEND_TUNING_BLOCK) || (cmd->opcode == MMC_SEND_TUNING_BLOCK)) {
		mask |= SDHC_INTR_BRR_MASK;
	}

	if (config->irq_config_func == NULL) {

		ret = sdhc_wait_for_events((void *)&reg->normal_int_stat,
				cmd->timeout_ms, mask);
		if (ret != 0) {
			LOG_ERR("No response from card");
			return ret;
		}

		//if ((reg->normal_int_stat & SDHC_INTR_ERR_MASK) != 0U) {
		if ((sys_read32(base_addr + SDHCI_INT_STATUS) & SDHC_INTR_ERR_MASK) != 0U) {
			LOG_ERR("Error response from card");
			reg->err_int_stat = SDHC_ERROR_INTR_ALL;
			sys_write16(SDHC_ERROR_INTR_ALL, reg + SDHCI_ERR_INT_STATUS);
			return -EINVAL;
		}
		reg->normal_int_stat = SDHC_INTR_CC_MASK;
		sys_write32(reg->normal_int_stat, base_addr + SDHCI_INT_STATUS);
	} else {

		timeout = K_MSEC(cmd->timeout_ms);

		events = k_event_wait(&dev_data->irq_event, mask, false, timeout);
		if ((events & SDHC_INTR_ERR_MASK) != 0U) {
			LOG_ERR("Error response from card");
			ret = -EINVAL;
		} else if ((events & SDHC_INTR_CC_MASK) ||
				(events & SDHC_INTR_BRR_MASK)) {
			ret = 0;
		} else {
			LOG_ERR("No response from card");
			ret = -EAGAIN;
		}
	}

	return ret;
}

/**
 * @brief
 * Update response member of command structure which is used by subsystem
 */
static void sdhc_update_response(const volatile struct reg_base *reg,
		struct sdhc_command *cmd)
{
	uintptr_t base_addr = reg;
	if (cmd->response_type == SD_RSP_TYPE_NONE) {
		return;
	}

	if (cmd->response_type == SD_RSP_TYPE_R2) {
		cmd->response[0] = sys_read32(base_addr + SDHCI_RESPONSE);//reg->resp_0;
		cmd->response[1] = sys_read32(base_addr + SDHCI_RESPONSE + 4);//reg->resp_1;
		cmd->response[2] = sys_read32(base_addr + SDHCI_RESPONSE + 8);//reg->resp_2;
		cmd->response[3] = sys_read32(base_addr + SDHCI_RESPONSE + 0xc);//reg->resp_3;

		/* CRC is striped from the response performing shifting to update response */
		for (uint8_t i = 3; i != 0; i--) {
			cmd->response[i] <<= SDHC_CRC_LEFT_SHIFT;
			cmd->response[i] |= cmd->response[i-1] >> SDHC_CRC_RIGHT_SHIFT;
		}
		cmd->response[0] <<= SDHC_CRC_LEFT_SHIFT;
	} else {
		cmd->response[0] = sys_read32(base_addr + SDHCI_RESPONSE);//reg->resp_0;
	}

}

/**
 * @brief
 * Setup and send the command and also check for response
 */
static int8_t sdhci_cmd(const struct device *dev, struct sdhc_command *cmd, bool data)
{
	const struct sdhci_config *config = dev->config;
	volatile struct reg_base *reg = (struct reg_base *)DEVICE_MMIO_GET(dev);
	struct sd_data *dev_data = dev->data;
	uint16_t command;
	uint8_t slottype = SDHC_SLOT_TYPE(dev);
	uintptr_t base_addr = reg;
	int8_t ret;

	
	//reg->timeout_ctrl = SDHC_DAT_LINE_TIMEOUT;
	sys_write16(SDHC_DAT_LINE_TIMEOUT, base_addr + SDHCI_TIMEOUT_CONTROL);
	
	sdhci_clear_intr(reg);

	/* Frame command */
	command = sdhc_cmd_frame(cmd, data, slottype);

	if (command == SDHC_CMD_RESP_INVAL) {
		return -EINVAL;
	}

	if ((cmd->opcode != SD_SEND_TUNING_BLOCK) && (cmd->opcode != MMC_SEND_TUNING_BLOCK)) {
		if (((sys_read32(base_addr + SDHCI_PRESENT_STATE) & SDHC_PSR_INHIBIT_DAT_MASK) != 0U) &&
				((command & SDHC_DAT_PRESENT_SEL_MASK) != 0U)) {
			LOG_ERR("Card data lines busy");
			return -EBUSY;
		}
	}

	if (config->irq_config_func != NULL) {
		k_event_clear(&dev_data->irq_event, SDHC_TXFR_INTR_EN_MASK);
	}

	sys_write32(cmd->arg,base_addr + SDHCI_ARGUMENT);
	sys_write16(dev_data->transfermode,base_addr + SDHCI_TRANSFER_MODE);
	
	sys_write16(command,base_addr + SDHCI_COMMAND);

	/* Check for response */
	ret = sdhc_cmd_response(dev, cmd);
	if (ret != 0) {
		return ret;
	}
	sdhc_update_response(reg, cmd);

	return 0;
}

/**
 * @brief
 * Check for data transfer completion
 */
static int8_t sdhci_xfr(const struct device *dev, struct sdhc_data *data)
{
	const struct sdhci_config *config = dev->config;
	volatile struct reg_base *reg = (struct reg_base *)DEVICE_MMIO_GET(dev);
	struct sd_data *dev_data = dev->data;
	uint32_t events;
	uint32_t mask;
	int8_t ret;
	k_timeout_t timeout;
	
	mask = SDHC_INTR_ERR_MASK | SDHC_INTR_TC_MASK;
	if (config->irq_config_func == NULL) {
		ret = sdhc_wait_for_events((void *)&reg->normal_int_stat,
				data->timeout_ms, mask);
		if (ret != 0) {
			LOG_ERR("Data transfer timeout");
			return ret;
		}

		if ((reg->normal_int_stat & SDHC_INTR_ERR_MASK) != 0U) {
			reg->err_int_stat = SDHC_ERROR_INTR_ALL;
			LOG_ERR("Error at data transfer");
			return -EINVAL;
		}

		reg->normal_int_stat = SDHC_INTR_TC_MASK;
	} else {
		timeout = K_MSEC(data->timeout_ms);

		events = k_event_wait(&dev_data->irq_event, mask, false, timeout);

		if ((events & SDHC_INTR_ERR_MASK) != 0U) {
			LOG_ERR("Error at data transfer");
			ret = -EINVAL;
		} else if ((events & SDHC_INTR_TC_MASK) != 0U) {
			ret = 0;
		} else {
			LOG_ERR("Data transfer timeout");
			ret = -EAGAIN;
		}
	}
	return ret;
}

/**
 * @brief
 * Performs data and command transfer and check for transfer complete
 */
static int8_t sdhci_transfer(const struct device *dev, struct sdhc_command *cmd,
		struct sdhc_data *data)
{
	volatile struct reg_base *reg = (struct reg_base *)DEVICE_MMIO_GET(dev);
	uintptr_t base_addr = reg;
	int8_t ret = -EINVAL;

	/* Check command line is in use */
	//if ((reg->present_state & 1U) != 0U) {
	if ((sys_read32(base_addr + SDHCI_PRESENT_STATE) & 1U) != 0U) {
		LOG_ERR("Command lines are busy");
		return -EBUSY;
	}

	if (data != NULL) {
		sys_write16(data->block_size & 0xfff, base_addr + SDHCI_BLOCK_SIZE);
		sys_write16(data->blocks & 0xffff, base_addr + SDHCI_BLOCK_COUNT);

		/* Setup ADMA2 if data is present */
		ret = sdhci_setup_adma(dev, data);
		if (ret != 0) {
			return ret;
		}

		/* Send command and check for command complete */
		ret = sdhci_cmd(dev, cmd, true);
		if (ret != 0) {
			return ret;
		}

		/* Check for data transfer complete */
		ret = sdhci_xfr(dev, data);
		if (ret != 0) {
			return ret;
		}
	} else {
		/* Send command and check for command complete */
		ret = sdhci_cmd(dev, cmd, false);
		return ret;
	}

	return ret;
}

/**
 * @brief
 * Configure transfer mode and transfer command and data
 */
static int sdhci_request(const struct device *dev, struct sdhc_command *cmd,
		struct sdhc_data *data)
{
	struct sd_data *dev_data = dev->data;
	int ret;
	const volatile struct reg_base *reg = (struct reg_base *)DEVICE_MMIO_GET(dev);
	uintptr_t base_addr = reg;
	//LOG_INF("zcq sdhci_request cmd %d",cmd->opcode);

	if (dev_data->transfermode == 0U) {
		dev_data->transfermode = SDHC_TM_DMA_EN_MASK |
			SDHC_TM_BLK_CNT_EN_MASK |
			SDHC_TM_DAT_DIR_SEL_MASK;
	}

	switch (cmd->opcode) {
	case SD_READ_MULTIPLE_BLOCK:
		dev_data->transfermode |= SDHC_TM_AUTO_CMD12_EN_MASK |
			SDHC_TM_MUL_SIN_BLK_SEL_MASK;
		ret = sdhci_transfer(dev, cmd, data);
		break;

	case SD_WRITE_MULTIPLE_BLOCK:
		dev_data->transfermode |= SDHC_TM_AUTO_CMD12_EN_MASK |
			SDHC_TM_MUL_SIN_BLK_SEL_MASK;
		dev_data->transfermode &= ~SDHC_TM_DAT_DIR_SEL_MASK;
		ret = sdhci_transfer(dev, cmd, data);
		break;

	case SD_WRITE_SINGLE_BLOCK:
		dev_data->transfermode &= ~SDHC_TM_DAT_DIR_SEL_MASK;
		ret = sdhci_transfer(dev, cmd, data);
		break;

	default:
		ret = sdhci_transfer(dev, cmd, data);
	}
	dev_data->transfermode = 0;

	return ret;
}

/**
 * @brief
 * Populate sdhc_host_props structure with all sd host controller property
 */
static int sdhci_host_props(const struct device *dev, struct sdhc_host_props *props)
{
	const struct sdhci_config *config = dev->config;
	const volatile struct reg_base *reg = (struct reg_base *)DEVICE_MMIO_GET(dev);
	struct sd_data *dev_data = dev->data;
	const uint64_t cap = reg->capabilities;
	const uint64_t current = reg->max_current_cap;

	props->f_max = SD_CLOCK_208MHZ;
	props->f_min = SDMMC_CLOCK_400KHZ;

	props->power_delay = config->powerdelay;

	props->host_caps.vol_180_support = SDHC_GET_HOST_PROP_BIT(cap,
			SDHC_1P8_VOL_SUPPORT);
	props->host_caps.vol_300_support = SDHC_GET_HOST_PROP_BIT(cap,
			SDHC_3P0_VOL_SUPPORT);
	props->host_caps.vol_330_support = SDHC_GET_HOST_PROP_BIT(cap,
			SDHC_3P3_VOL_SUPPORT);
	props->max_current_330 = (uint8_t)(current & SDHC_CURRENT_BYTE);
	props->max_current_300 = (uint8_t)((current >> SDHC_3P0_CURRENT_SUPPORT_SHIFT) &
			SDHC_CURRENT_BYTE);
	props->max_current_180 = (uint8_t)((current >> SDHC_1P8_CURRENT_SUPPORT_SHIFT) &
			SDHC_CURRENT_BYTE);
	props->host_caps.sdma_support = SDHC_GET_HOST_PROP_BIT(cap, SDHC_SDMA_SUPPORT);
	props->host_caps.high_spd_support = SDHC_GET_HOST_PROP_BIT(cap,
			SDHC_HIGH_SPEED_SUPPORT);
	props->host_caps.adma_2_support = SDHC_GET_HOST_PROP_BIT(cap,
			SDHC_ADMA2_SUPPORT);
	props->host_caps.max_blk_len = (uint8_t)((cap >> SDHC_MAX_BLK_LEN_SHIFT) &
			SDHC_MAX_BLK_LEN);
	props->host_caps.ddr50_support = SDHC_GET_HOST_PROP_BIT(cap, SDHC_DDR50_SUPPORT);
	props->host_caps.sdr104_support = SDHC_GET_HOST_PROP_BIT(cap,
			SDHC_SDR104_SUPPORT);
	props->host_caps.sdr50_support = SDHC_GET_HOST_PROP_BIT(cap, SDHC_SDR50_SUPPORT);
	props->host_caps.slot_type = (uint8_t)((cap >> SDHC_SLOT_TYPE_SHIFT) &
			SDHC_SLOT_TYPE_GET);
	props->host_caps.bus_8_bit_support = SDHC_GET_HOST_PROP_BIT(cap,
			SDHC_8BIT_SUPPORT);
	props->host_caps.bus_4_bit_support = SDHC_GET_HOST_PROP_BIT(cap,
			SDHC_4BIT_SUPPORT);

	if ((cap & CHECK_BITS(SDHC_SDR400_SUPPORT)) != 0U) {
		props->host_caps.hs400_support = (uint8_t)config->hs400_mode;
		dev_data->has_phy = true;
	}
	props->host_caps.hs200_support = (uint8_t)config->hs200_mode;

	dev_data->props = *props;

	return 0;
}

/**
 * @brief
 * Calculate clock value based on the selected speed
 */
static uint16_t sdhci_cal_clock(uint32_t maxclock, enum sdhc_clock_speed speed)
{
	uint16_t divcnt;
	uint16_t divisor = 0U, clockval = 0U;

	if (maxclock <= speed) {
		divisor = 0U;
	} else {
		for (divcnt = 2U; divcnt <= SDHC_CC_EXT_MAX_DIV_CNT; divcnt += 2U) {
			if ((maxclock / divcnt) <= speed) {
				divisor = divcnt >> SDHC_CLOCK_CNT_SHIFT;
				break;
			}
		}
	}

	clockval |= (divisor & SDHC_CC_SDCLK_FREQ_SEL) << SDHC_CC_DIV_SHIFT;
	clockval |= ((divisor >> SDHC_CC_DIV_SHIFT) & SDHC_CC_SDCLK_FREQ_SEL_EXT) <<
		SDHC_CC_EXT_DIV_SHIFT;

	return clockval;
}


/**
 * @brief
 * Set clock and wait for clock to be stable
 */
static int sdhci_set_clock(const struct device *dev, enum sdhc_clock_speed speed)
{
	const struct sdhci_config *config = dev->config;
	volatile struct reg_base *reg = (struct reg_base *)DEVICE_MMIO_GET(dev);
	uintptr_t base_addr = reg; 
	struct sd_data *dev_data = dev->data;
	int ret;
	uint16_t value;

	/* Disable clock */
	//reg->clock_ctrl = 0;
	if (speed == 0U) {
		return 0;
	}

	/* Get input clock rate */
	ret = clock_control_get_rate(config->clock_dev,(clock_control_subsys_t)&config->clkid, &dev_data->maxclock);
	if (ret != 0) {
		LOG_ERR("Failed to get clock\n");
		return ret;
	}

	/* Calculate clock */
	value = sdhci_cal_clock(dev_data->maxclock, speed);
	value |= SDHC_CC_INT_CLK_EN_MASK;
	value |= sys_read16(base_addr + SDHCI_CLOCK_CONTROL);

	/* Enable div clock */
	value |= SDHC_CC_SD_CLK_EN_MASK | SDHC_CC_INT_CLK_EN_MASK | SDHC_CC_INT_CLK_STABLE_MASK;
	
	sys_write16(value,base_addr + SDHCI_CLOCK_CONTROL);

	// /* Wait max 150ms for internal clock to be stable */
	ret = sdhc_waitb_events((void *)(base_addr + SDHCI_CLOCK_CONTROL), 150,
			SDHC_CC_INT_CLK_STABLE_MASK, SDHC_CC_INT_CLK_STABLE_MASK);
	if (ret != 0) {
		return ret;
	}

	return ret;
}

/**
 * @brief
 * Set bus width on the controller
 */
static int8_t sdhci_set_buswidth(volatile struct reg_base *reg,
		enum sdhc_bus_width width)
{
	uintptr_t base_addr = reg; 
	uint8_t ctrl = 0;

	ctrl = sys_read8(base_addr + SDHCI_HOST_CONTROL);
	switch (width) {
	case SDHC_BUS_WIDTH1BIT:
		ctrl &= ~SDHC_DAT_WIDTH8_MASK;
		
		ctrl &= ~SDHC_DAT_WIDTH4_MASK;
		break;

	case SDHC_BUS_WIDTH4BIT:
		ctrl &= ~SDHC_DAT_WIDTH8_MASK;
		ctrl |= SDHC_DAT_WIDTH4_MASK;
		break;

	case SDHC_BUS_WIDTH8BIT:
		ctrl |= SDHC_DAT_WIDTH8_MASK;
		break;

	default:
		return -EINVAL;
	}
	sys_write8(ctrl ,base_addr + SDHCI_HOST_CONTROL);
	return 0;
}

/**
 * @brief
 * Enable or disable power
 */
static void sdhci_set_power(const struct device *dev, enum sdhc_power power)
{
	volatile struct reg_base *reg = (struct reg_base *)DEVICE_MMIO_GET(dev);
	uintptr_t base_addr = reg; 
	
	reg->power_ctrl = sys_read8(base_addr + SDHCI_POWER_CONTROL);

	if (SDHC_SLOT_TYPE(dev) == SDHC_EMMC_SLOT) {
		if (power == SDHC_POWER_ON) {
			reg->power_ctrl &= ~SDHC_PC_EMMC_HW_RST_MASK;
			reg->power_ctrl |= SDHC_PC_BUS_PWR_MASK;
		} else {
			reg->power_ctrl |= SDHC_PC_EMMC_HW_RST_MASK;
			reg->power_ctrl &= ~SDHC_PC_BUS_PWR_MASK;
		}
	} else {
		if (power == SDHC_POWER_ON) {
			reg->power_ctrl |= SDHC_PC_BUS_PWR_MASK;
		} else {
			reg->power_ctrl &= ~SDHC_PC_BUS_PWR_MASK;
		}
	}
	sys_write8(reg->power_ctrl ,base_addr + SDHCI_POWER_CONTROL);
}

/**
 * @brief
 * Set voltage level and signalling voltage
 */
static int8_t sdhci_set_voltage(volatile struct reg_base *reg, enum sd_voltage voltage)
{
	uintptr_t base_addr = reg; 

	reg->host_ctrl2 = sys_read16(base_addr + SDHCI_HOST_CONTROL2);

	switch (voltage) {
	case SD_VOL_3_3_V:
		reg->power_ctrl = SDHC_PC_BUS_VSEL_3V3;
		reg->host_ctrl2 &= ~SDHC_HC2_1V8_EN_MASK;
		break;

	case SD_VOL_3_0_V:
		reg->power_ctrl = SDHC_PC_BUS_VSEL_3V0;
		reg->host_ctrl2 &= ~SDHC_HC2_1V8_EN_MASK;
		break;

	case SD_VOL_1_8_V:
		reg->host_ctrl2 |= SDHC_HC2_1V8_EN_MASK;
		break;

	default:
		return -EINVAL;
	}
	sys_write16(reg->host_ctrl2,base_addr + SDHCI_HOST_CONTROL2);
	sys_write8(reg->power_ctrl, base_addr + SDHCI_POWER_CONTROL);
	return 0;
}

/**
 * @brief
 * Set otap delay based on selected speed mode for SD 3.0
 */
static void sdhci_config_sd_otap_delay(const struct device *dev,
		enum sdhc_timing_mode timing)
{
	volatile struct reg_base *reg = (struct reg_base *)DEVICE_MMIO_GET(dev);
	uint32_t def_degrees[SDHC_TIMING_HS400 + 1] = SDHC_SD_OTAP_DEFAULT_PHASES;
	uint32_t degrees = 0, otapdly = 0;
	uint8_t tap_max = 0;

	switch (timing) {
	case SDHC_TIMING_SDR104:
	case SDHC_TIMING_HS200:
		tap_max = SDHC_SD_200HZ_MAX_OTAP;
		break;

	case SDHC_TIMING_DDR50:
	case SDHC_TIMING_SDR25:
	case SDHC_TIMING_HS:
		tap_max = SDHC_SD_50HZ_MAX_OTAP;
		break;

	case SDHC_TIMING_SDR50:
		tap_max = SDHC_SD_100HZ_MAX_OTAP;
		break;

	default:
		return;
	}

	if ((timing == SDHC_TIMING_HS) && (SDHC_SLOT_TYPE(dev) == SDHC_EMMC_SLOT)) {
		degrees = def_degrees[SDHC_TIMING_MMC_HS];
	} else {
		degrees = def_degrees[timing];
	}

	otapdly = (degrees * tap_max) / SDHC_MAX_CLK_PHASE;

	/* Set the clock phase */
	reg->otap_dly = otapdly;
}

/**
 * @brief
 * Set itap delay based on selected speed mode for SD 3.0
 */
static void sdhci_config_sd_itap_delay(const struct device *dev,
		enum sdhc_timing_mode timing)
{
	volatile struct reg_base *reg = (struct reg_base *)DEVICE_MMIO_GET(dev);
	uint32_t def_degrees[SDHC_TIMING_HS400 + 1] = SDHC_SD_ITAP_DEFAULT_PHASES;
	uint32_t degrees = 0, itapdly = 0;
	uint8_t tap_max = 0;

	switch (timing) {
	case SDHC_TIMING_SDR104:
	case SDHC_TIMING_HS200:
		tap_max = SDHC_SD_200HZ_MAX_ITAP;
		break;

	case SDHC_TIMING_DDR50:
	case SDHC_TIMING_SDR25:
	case SDHC_TIMING_HS:
		tap_max = SDHC_SD_50HZ_MAX_ITAP;
		break;

	case SDHC_TIMING_SDR50:
		tap_max = SDHC_SD_100HZ_MAX_ITAP;
		break;

	default:
		return;
	}

	if ((timing == SDHC_TIMING_HS) && (SDHC_SLOT_TYPE(dev) == SDHC_EMMC_SLOT)) {
		degrees = def_degrees[SDHC_TIMING_MMC_HS];
	} else {
		degrees = def_degrees[timing];
	}

	itapdly = (degrees * tap_max) / SDHC_MAX_CLK_PHASE;

	/* Set the clock phase */
	if (itapdly != 0U) {
		reg->itap_dly = SDHC_ITAPCHGWIN;
		reg->itap_dly |= SDHC_ITAPDLYENA;
		reg->itap_dly |= itapdly;
		reg->itap_dly &= ~SDHC_ITAPCHGWIN;
	}
}

// /**
//  * @brief
//  * Set otap delay based on selected speed mode for EMMC 5.1
//  */
// static void sdhci_config_emmc_otap_delay(const struct device *dev,
// 		enum sdhc_timing_mode timing)
// {
// 	volatile struct reg_base *reg = (struct reg_base *)DEVICE_MMIO_GET(dev);
// 	uint32_t def_degrees[SDHC_TIMING_HS400 + 1] = SDHC_EMMC_OTAP_DEFAULT_PHASES;
// 	uint32_t degrees = 0, otapdly = 0;
// 	uint8_t tap_max = 0;

// 	uintptr_t base_addr = reg; 
// 	switch (timing) {
// 	case SDHC_TIMING_HS400:
// 	case SDHC_TIMING_HS200:
// 		tap_max = SDHC_EMMC_200HZ_MAX_OTAP;
// 		break;
// 	case SDHC_TIMING_HS:
// 		tap_max = SDHC_EMMC_50HZ_MAX_OTAP;
// 		break;
// 	default:
// 		return;
// 	}

// 	if (timing == SDHC_TIMING_HS) {
// 		degrees = def_degrees[SDHC_TIMING_MMC_HS];
// 	} else {
// 		degrees = def_degrees[timing];
// 	}

// 	otapdly = (degrees * tap_max) / SDHC_MAX_CLK_PHASE;

// 	/* Set the clock phase */
// 	if (otapdly != 0U) {
// 		reg->phy_ctrl1 |= SDHC_PHYREG1_OTAP_EN_MASK;
// 		reg->phy_ctrl1 &= ~SDHC_PHYREG1_OTAP_DLY;
// 		reg->phy_ctrl1 |= otapdly << SDHC_PHYREG1_OTAP_DLY_SHIFT;
// 	}
// }

// /**
//  * @brief
//  * Set itap delay based on selected speed mode for EMMC 5.1
//  */
// static void sdhci_config_emmc_itap_delay(const struct device *dev,
// 		enum sdhc_timing_mode timing)
// {
// 	volatile struct reg_base *reg = (struct reg_base *)DEVICE_MMIO_GET(dev);
// 	uint32_t def_degrees[SDHC_TIMING_HS400 + 1] = SDHC_EMMC_ITAP_DEFAULT_PHASES;
// 	uint32_t degrees = 0, itapdly = 0;
// 	uint8_t tap_max = 0;

// 	/* Select max tap based on speed mode */
// 	switch (timing) {
// 	case SDHC_TIMING_HS400:
// 	case SDHC_TIMING_HS200:
// 		/* Strobe select tap point for strb90 and strb180 */
// 		reg->phy_ctrl1 &= ~SDHC_PHYREG1_STROBE_SEL;
// 		if (timing == SDHC_TIMING_HS400) {
// 			reg->phy_ctrl1 |=
// 				(SDHC_PHY_STRB_SEL_SIG) << SDHC_PHYREG1_STROBE_SEL_SHIFT;
// 		}
// 		break;
// 	case SDHC_TIMING_HS:
// 		tap_max = SDHC_EMMC_50HZ_MAX_ITAP;
// 		break;
// 	default:
// 		return;
// 	}

// 	/* default clock phase based on speed mode */
// 	if (timing == SDHC_TIMING_HS) {
// 		degrees = def_degrees[SDHC_TIMING_MMC_HS];
// 	} else {
// 		degrees = def_degrees[timing];
// 	}

// 	itapdly = (degrees * tap_max) / SDHC_MAX_CLK_PHASE;

// 	/* Set the clock phase */
// 	if (itapdly != 0U) {
// 		reg->phy_ctrl1 |= SDHC_PHYREG1_ITAP_CHGWIN_MASK;
// 		reg->phy_ctrl1 |= SDHC_PHYREG1_ITAP_EN_MASK;
// 		reg->phy_ctrl1 &= ~SDHC_PHYREG1_ITAP_DLY;
// 		reg->phy_ctrl1 |= itapdly << SDHC_PHYREG1_ITAP_DLY_SHIFT;
// 		reg->phy_ctrl1 &= ~SDHC_PHYREG1_ITAP_CHGWIN_MASK;
// 	}
// }

/**
 * @brief
 * Set speed mode and config tap delay
 */
static int8_t sdhcI_set_timing(const struct device *dev, enum sdhc_timing_mode timing)
{
	volatile struct reg_base *reg = (struct reg_base *)DEVICE_MMIO_GET(dev);
	const struct sd_data *dev_data = dev->data;
	uint16_t mode = 0;
	uint8_t ctrl1 = 0;
	uintptr_t base_addr = reg; 

	ctrl1 = sys_read8(base_addr + SDHCI_HOST_CONTROL);

	switch (timing) {
	case SDHC_TIMING_LEGACY:
		ctrl1 &= ~SDHC_HS_SPEED_MODE_EN_MASK;
		sys_write8(ctrl1 , base_addr + SDHCI_HOST_CONTROL);
		break;

	case SDHC_TIMING_SDR25:
	case SDHC_TIMING_HS:
		ctrl1 |= SDHC_HS_SPEED_MODE_EN_MASK;
		sys_write8(ctrl1 , base_addr + SDHCI_HOST_CONTROL);
		break;

	case SDHC_TIMING_SDR12:
		mode = SDHC_UHS_SPEED_MODE_SDR12;
		break;

	case SDHC_TIMING_SDR50:
		mode = SDHC_UHS_SPEED_MODE_SDR50;
		break;

	case SDHC_TIMING_HS200:
	case SDHC_TIMING_SDR104:
		mode = SDHC_UHS_SPEED_MODE_SDR104;
		break;

	case SDHC_TIMING_DDR50:
	case SDHC_TIMING_DDR52:
		mode = SDHC_UHS_SPEED_MODE_DDR50;
		break;

	case SDHC_TIMING_HS400:
		mode = SDHC_UHS_SPEED_MODE_DDR200;
		break;

	default:
		return -EINVAL;
	}

	/* Select one of UHS mode */
	// if (timing > SDHC_TIMING_HS) {
	// 	reg->host_ctrl2 &= ~SDHC_HC2_UHS_MODE;
	// 	reg->host_ctrl2 |= mode;
	// }

	/* clock phase delays are different for SD 3.0 and EMMC 5.1 */
	// if (dev_data->has_phy == true) {
	// 	sdhci_config_emmc_otap_delay(dev, timing);
	// 	sdhci_config_emmc_itap_delay(dev, timing);
	// } else {
	// 	sdhci_config_sd_otap_delay(dev, timing);
	// 	sdhci_config_sd_itap_delay(dev, timing);
	// }

	return 0;
}

/**
 * @brief
 * Set voltage, power, clock, timing, bus width on host controller
 */
static int sdhci_set_io(const struct device *dev, struct sdhc_io *ios)
{
	int ret;
	struct sd_data *dev_data = dev->data;
	struct sdhc_io *host_io = (struct sdhc_io *)&dev_data->host_io;
	volatile struct reg_base *reg = (struct reg_base *)DEVICE_MMIO_GET(dev);

	/* Check given clock is valid */
	if ((ios->clock != 0) && ((ios->clock > dev_data->props.f_max) ||
				(ios->clock < dev_data->props.f_min))) {
		LOG_ERR("Invalid clock value");
		return -EINVAL;
	}

	/* Set power on or off */
	if (ios->power_mode != host_io->power_mode) {
		sdhci_set_power(dev, ios->power_mode);
		host_io->power_mode = ios->power_mode;
	}

	/* Set voltage level */
	if (ios->signal_voltage != host_io->signal_voltage) {
		ret = sdhci_set_voltage(reg, ios->signal_voltage);
		if (ret != 0) {
			LOG_ERR("Failed to set voltage level");
			return ret;
		}
		host_io->signal_voltage = ios->signal_voltage;
	}

	/* Set speed mode */
	if (ios->timing != host_io->timing) {
		ret = sdhcI_set_timing(dev, ios->timing);
		if (ret != 0) {
			LOG_ERR("Failed to set speed mode");
			return ret;
		}
		host_io->timing = ios->timing;
	}

	/* Set clock */
	if (ios->clock != host_io->clock) {
		ret = sdhci_set_clock(dev, ios->clock);
		if (ret != 0) {
			LOG_ERR("Failed to set clock");
			return ret;
		}
		host_io->clock = ios->clock;
	}

	/* Set bus width */
	if (ios->bus_width != host_io->bus_width) {
		ret = sdhci_set_buswidth(reg, ios->bus_width);
		if (ret != 0) {
			LOG_ERR("Failed to set bus width");
			return ret;
		}
		host_io->bus_width = ios->bus_width;
	}

	return 0;
}

/**
 * @brief
 * Perform reset and enable status registers
 */
static int sdhci_host_reset(const struct device *dev)
{
	const struct sdhci_config *config = dev->config;
	volatile struct reg_base *reg = (struct reg_base *)DEVICE_MMIO_GET(dev);
	int ret;
	uintptr_t base_addr = reg;

	/* Perform software reset  */
	reg->sw_reset = SDHC_SWRST_ALL_MASK;
	sys_write8(reg->sw_reset, base_addr + SDHCI_SOFTWARE_RESET);
	/* Wait max 100ms for software reset to complete */
	ret = sdhc_waitb_events((void *)(base_addr + SDHCI_SOFTWARE_RESET), 100,
			SDHC_SWRST_ALL_MASK, 0);
	if (ret != 0) {
		LOG_ERR("Device is busy");
		return -EBUSY;
	}

	sys_write32(4, base_addr + 0x534);
	

	/* Enable status reg and configure interrupt */
	reg->normal_int_stat_en = SDHC_NORM_INTR_ALL;
	reg->err_int_stat_en = SDHC_ERROR_INTR_ALL;
	sys_write32(reg->normal_int_stat_en | (reg->normal_int_stat_en << 16), base_addr + SDHCI_INT_STATUS);
	reg->err_int_signal_en = 0;
	sys_write32(0, base_addr + SDHCI_SIGNAL_ENABLE);

	if (config->irq_config_func == NULL) {
		reg->normal_int_signal_en = 0;
		sys_write32(0, base_addr + SDHCI_INT_ENABLE);
	} else {
		/*
		 * Enable command complete, transfer complete, read buffer ready and error status
		 * interrupt
		 */
		reg->normal_int_signal_en = SDHC_TXFR_INTR_EN_MASK;
		sys_write32(0xffffffff, base_addr + SDHCI_INT_ENABLE);
		//sys_write32(0xffffffff, base_addr + SDHCI_SIGNAL_ENABLE);
	}

	/* Data line time out interval */
	reg->timeout_ctrl = SDHC_DAT_LINE_TIMEOUT;
	sys_write16(reg->timeout_ctrl, base_addr + SDHCI_TIMEOUT_CONTROL);

	/* Select ADMA2 */
	reg->host_ctrl1 = SDHC_ADMA2_64;
	sys_write32(reg->host_ctrl1, base_addr + SDHCI_HOST_CONTROL);

	reg->block_size = SDHC_BLK_SIZE_512;
	sys_write32(SDHCI_MAKE_BLKSZ(7, reg->block_size), base_addr + SDHCI_BLOCK_SIZE);

	sdhci_clear_intr(reg);

	sys_write16(0x1000, base_addr +  SDHCI_HOST_CONTROL2);//EMMC NEED

	sys_write32(0xffffffff, base_addr + SDHCI_SIGNAL_ENABLE);
	return ret;
}

/**
 * @brief
 * Check for card busy
 */
static int sdhci_card_busy(const struct device *dev)
{
	int8_t ret;
	volatile struct reg_base *reg = (struct reg_base *)DEVICE_MMIO_GET(dev);
	uintptr_t base_addr = reg; 

	/* Wait max 2ms for card to send next command */
	ret = sdhc_waitl_events((void *)(base_addr + SDHCI_PRESENT_STATE), 2,
			sdhci_card_busy, 0);
	if (ret != 0) {
		return 0;
	}

	return 1;
}

/**
 * @brief
 * Enable tuning clock
 */
static int sdhci_card_tuning(const struct device *dev)
{
	struct sd_data *dev_data = dev->data;
	const struct sdhc_io *io = &dev_data->host_io;
	volatile struct reg_base *reg = (struct reg_base *)DEVICE_MMIO_GET(dev);
	struct sdhc_command cmd = {0};
	uint8_t blksize;
	uint8_t count;
	int ret;

	if ((io->timing == SDHC_TIMING_HS200) || (io->timing == SDHC_TIMING_HS400)) {
		cmd.opcode = MMC_SEND_TUNING_BLOCK;
	} else {
		cmd.opcode = SD_SEND_TUNING_BLOCK;
	}

	cmd.response_type = SD_RSP_TYPE_R1;
	cmd.timeout_ms = CONFIG_SD_CMD_TIMEOUT;

	blksize = SDHC_TUNING_CMD_BLKSIZE;
	if (io->bus_width == SDHC_BUS_WIDTH8BIT) {
		blksize = SDHC_TUNING_CMD_BLKSIZE * 2;
	}

	dev_data->transfermode = SDHC_TM_DAT_DIR_SEL_MASK;
	reg->block_size = blksize;
	reg->block_count = SDHC_TUNING_CMD_BLKCOUNT;

	/* Execute tuning */
	reg->host_ctrl2 |= SDHC_HC2_EXEC_TNG_MASK;

	for (count = 0; count < SDHC_MAX_TUNING_COUNT; count++) {
		ret = sdhci_cmd(dev, &cmd, true);
		if (ret != 0) {
			return ret;
		}
		if ((reg->host_ctrl2 & SDHC_HC2_EXEC_TNG_MASK) == 0U) {
			break;
		}
	}

	/* Check tuning completed successfully */
	if ((reg->host_ctrl2 & SDHC_HC2_SAMP_CLK_SEL_MASK) == 0U) {
		return -EINVAL;
	}

	dev_data->transfermode = 0;

	return 0;
}

/**
 * @brief
 * Perform early system init for SDHC
 */
static int sdhc_init(const struct device *dev)
{
	const struct sdhci_config *config = dev->config;
	struct sd_data *dev_data = dev->data;
	volatile struct reg_base *reg = (struct reg_base *)DEVICE_MMIO_GET(dev); 
	uintptr_t base_addr = reg; 

	DEVICE_MMIO_MAP(dev, K_MEM_CACHE_NONE);

// sys_write32(0x5, 0x500100d0);
// sys_write32(0xd, 0x500100d4);
// sys_write32(0xd, 0x500100d8);
// sys_write32(0xd, 0x500100dc);
// sys_write32(0xd, 0x500100e0);
// sys_write32(0xd, 0x500100e4);

// 	sys_write32(0x0, 0x50000044);
// 	k_msleep(1);
// 	sys_write32(0xffff, 0x50000044);

	if (device_is_ready(config->clock_dev) == 0) {
		LOG_ERR("Clock control device not ready");
		return -ENODEV;
	}
	sdhci_host_reset(dev);

	if (config->irq_config_func != NULL) {

		k_event_init(&dev_data->irq_event);

		sys_write32(0xfffffeff,base_addr + 0x34);
		sys_write32(0xfffffeff,base_addr + 0x38);
		config->irq_config_func(dev);
	}
	
	return 0;
}

static DEVICE_API(sdhc, sdhc_api) = {
	.reset = sdhci_host_reset,
	.request = sdhci_request,
	.set_io = sdhci_set_io,
	.get_card_present = sdhci_card_detect,
	.execute_tuning = sdhci_card_tuning,
	.card_busy = sdhci_card_busy,
	.get_host_props = sdhci_host_props,
};

#define SDHC_INTR_CONFIG(n)                                                                  \
	static void sdhc_irq_handler##n(const struct device *dev)                            \
	{                                                                                         \
		volatile struct reg_base *reg = (struct reg_base *)DEVICE_MMIO_GET(dev);          \
		struct sd_data *dev_data = dev->data; 									\
		uintptr_t base_addr = reg; 								\
		uint32_t status = sys_read32(base_addr + SDHCI_INT_STATUS);				\
		if ((status & SDHC_INTR_CC_MASK) != 0U) {                      \
			sys_write16(reg->normal_int_stat,base_addr + SDHCI_INT_STATUS);                          \
			k_event_post(&dev_data->irq_event, SDHC_INTR_CC_MASK);               \
		}                                                                                 \
		if ((status & SDHC_INTR_BRR_MASK) != 0U) {                     \
			sys_write16(SDHC_INTR_BRR_MASK,base_addr + SDHCI_INT_STATUS);                        \
			k_event_post(&dev_data->irq_event, SDHC_INTR_BRR_MASK);              \
		}                                                                                 \
		if ((status & SDHC_INTR_TC_MASK) != 0U) {                      \
			sys_write16(SDHC_INTR_TC_MASK,base_addr + SDHCI_INT_STATUS);                          \
			k_event_post(&dev_data->irq_event, SDHC_INTR_TC_MASK);               \
		}                                                                                 \
		if ((status & SDHC_INTR_ERR_MASK) != 0U) {                     \
			sys_write16(SDHC_INTR_ERR_MASK,base_addr + SDHCI_INT_STATUS);                       \
			sys_write16(SDHC_ERROR_INTR_ALL,base_addr + SDHCI_ERR_INT_STATUS);                                \
			k_event_post(&dev_data->irq_event, SDHC_INTR_ERR_MASK);              \
		}                                                                                 \
	}                                                                                         \
	static void sdhci_config_intr##n(const struct device *dev)                            \
	{             						\
		IRQ_CONNECT(DT_INST_IRQN(n), DT_INST_IRQ(n, priority),                            \
				sdhc_irq_handler##n, DEVICE_DT_INST_GET(n), 0);                  \
		irq_enable(DT_INST_IRQN(n));                                                      \
	}

#define SDHC_INTR_FUNC_REG(n) .irq_config_func = sdhci_config_intr##n,

#define SDHC_INTR_CONFIG_NULL
#define SDHC_INTR_FUNC_REG_NULL .irq_config_func = NULL,

#define SDHC_INTR_CONFIG_API(n) COND_CODE_1(DT_INST_NODE_HAS_PROP(n, interrupts),            \
		(SDHC_INTR_CONFIG(n)), (SDHC_INTR_CONFIG_NULL))

#define SDHC_INTR_FUNC_REG_API(n) COND_CODE_1(DT_INST_NODE_HAS_PROP(n, interrupts),          \
		(SDHC_INTR_FUNC_REG(n)), (SDHC_INTR_FUNC_REG_NULL))

#define SDHC_INIT(n)                                                                         \
	SDHC_INTR_CONFIG_API(n)                                                              \
	const static struct sdhci_config sdhc_inst_##n = {                               \
		DEVICE_MMIO_ROM_INIT(DT_DRV_INST(n)),                                             \
		.clock_dev = DEVICE_DT_GET(DT_INST_CLOCKS_CTLR(n)),                               \
		.clkid = DT_INST_CLOCKS_CELL(n,id),                               \
		.base_addr = DT_INST_REG_ADDR(n),									\
		SDHC_INTR_FUNC_REG_API(n)                                                    \
		.broken_cd = DT_INST_PROP_OR(n, broken_cd, 0),                                    \
		.powerdelay = DT_INST_PROP_OR(n, power_delay_ms, 0),                              \
		.hs200_mode = DT_INST_PROP_OR(n, mmc_hs200_1_8v, 0),                              \
		.hs400_mode = DT_INST_PROP_OR(n, mmc_hs400_1_8v, 0),                              \
	};                                                                                        \
	static struct sd_data data##n;                                                            \
	                                                                                          \
	DEVICE_DT_INST_DEFINE(n, sdhc_init, NULL, &data##n,                                  \
			&sdhc_inst_##n, POST_KERNEL,                                         \
			CONFIG_KERNEL_INIT_PRIORITY_DEVICE, &sdhc_api);

DT_INST_FOREACH_STATUS_OKAY(SDHC_INIT)