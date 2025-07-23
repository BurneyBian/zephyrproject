/*
 * Copyright (c) 2025 Advanced Micro Devices, Inc. (AMD)
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __SDHC_H__
#define __SDHC_H__

/* Bit map for status register */
#define SDHC_INTR_CC_MASK	BIT(0)
#define SDHC_INTR_TC_MASK	BIT(1)
#define SDHC_INTR_ERR_MASK	BIT(15)
#define SDHC_NORM_INTR_ALL	0xFFFFU

/* Bit map for error register */
#define SDHC_ERROR_INTR_ALL	0xF3FFU

/* Bit map for ADMA2 attribute */
#define SDHC_DESC_VALID	BIT(0)
#define SDHC_DESC_END	BIT(1)
#define SDHC_DESC_TRAN	BIT(5)

/* Bit map and constant values for ADMA2 Configuration */
#define SDHC_ADMA2_64		0x18U
#define SDHC_DESC_MAX_LENGTH	65536U

/* Bit map for present stage register */
#define SDHC_PSR_INHIBIT_DAT_MASK	BIT(1)
#define SDHC_INTR_BRR_MASK		BIT(5)
#define SDHC_PSR_CARD_INSRT_MASK	BIT(16)
#define SDHC_CARD_BUSY		0x1F00000U

/* Bit map for transfer mode register */
#define SDHC_TM_DMA_EN_MASK		BIT(0)
#define SDHC_TM_BLK_CNT_EN_MASK		BIT(1)
#define SDHC_TM_AUTO_CMD12_EN_MASK		BIT(2)
#define SDHC_TM_DAT_DIR_SEL_MASK		BIT(4)
#define SDHC_TM_MUL_SIN_BLK_SEL_MASK	BIT(5)

/* Bit map for host control1 register */
#define SDHC_DAT_WIDTH4_MASK	BIT(1)
#define SDHC_HS_SPEED_MODE_EN_MASK	BIT(2)
#define SDHC_DAT_WIDTH8_MASK	BIT(5)

/* Bit map for power control register */
#define SDHC_PC_BUS_PWR_MASK	BIT(0)
#define SDHC_PC_EMMC_HW_RST_MASK	BIT(4)
#define SDHC_PC_BUS_VSEL_3V3	0x0EU
#define SDHC_PC_BUS_VSEL_3V0	0x0CU

/* Bit map for host control2 register */
#define SDHC_HC2_1V8_EN_MASK	BIT(3)
#define SDHC_HC2_EXEC_TNG_MASK	BIT(6)
#define SDHC_HC2_SAMP_CLK_SEL_MASK	BIT(7)
#define SDHC_UHS_SPEED_MODE_SDR12	0U
#define SDHC_UHS_SPEED_MODE_SDR50	2U
#define SDHC_UHS_SPEED_MODE_SDR104	3U
#define SDHC_UHS_SPEED_MODE_DDR50	4U
#define SDHC_UHS_SPEED_MODE_DDR200	5U
#define SDHC_HC2_UHS_MODE		7U

/* Bit map to read host capabilities register */
#define SDHC_1P8_VOL_SUPPORT		26U
#define SDHC_3P0_VOL_SUPPORT		25U
#define SDHC_3P3_VOL_SUPPORT		24U
#define SDHC_3P0_CURRENT_SUPPORT_SHIFT	8U
#define SDHC_1P8_CURRENT_SUPPORT_SHIFT	16U
#define SDHC_CURRENT_BYTE			0xFFU
#define SDHC_SDMA_SUPPORT			22U
#define SDHC_HIGH_SPEED_SUPPORT		21U
#define SDHC_ADMA2_SUPPORT			19U
#define SDHC_MAX_BLK_LEN_SHIFT		16U
#define SDHC_MAX_BLK_LEN			3U
#define SDHC_DDR50_SUPPORT			34U
#define SDHC_SDR104_SUPPORT		33U
#define SDHC_SDR50_SUPPORT			32U
#define SDHC_SLOT_TYPE_SHIFT		30U
#define SDHC_SLOT_TYPE_GET			3U
#define SDHC_8BIT_SUPPORT			18U
#define SDHC_4BIT_SUPPORT			18U
#define SDHC_SDR400_SUPPORT		63U

/* Bit map for tap delay register */
#define SDHC_ITAPCHGWIN	BIT(9)
#define SDHC_ITAPDLYENA	BIT(8)

/* Bit map for phy1 register */
#define SDHC_PHYREG1_ITAP_DLY_SHIFT	0x1U
#define SDHC_PHYREG1_ITAP_EN_MASK		BIT(0)
#define SDHC_PHYREG1_STROBE_SEL_SHIFT	16U
#define SDHC_PHYREG1_ITAP_CHGWIN_MASK	BIT(6)
#define SDHC_PHYREG1_OTAP_EN_MASK		BIT(8)
#define SDHC_PHYREG1_OTAP_DLY_SHIFT	0xCU
#define SDHC_PHYREG1_ITAP_DLY		0x3EU
#define SDHC_PHY_STRB_SEL_SIG		0x0077U
#define SDHC_PHYREG1_OTAP_DLY		0xF000U
#define SDHC_PHYREG1_STROBE_SEL		0xFF0000U

/* Bit map for phy2 register */
#define SDHC_PHYREG2_DLL_EN_MASK		BIT(0)
#define SDHC_PHYREG2_DLL_RDY_MASK		BIT(1)
#define SDHC_PHYREG2_FREQ_SEL_SHIFT	BIT(2)
#define SDHC_PHYREG2_TRIM_ICP_SHIFT	BIT(3)
#define SDHC_PHYREG2_DLYTX_SEL_MASK	BIT(16)
#define SDHC_PHYREG2_DLYRX_SEL_MASK	BIT(17)
#define SDHC_PHYREG2_TRIM_ICP_DEF_VAL	0x8U
#define SDHC_PHYREG2_FREQ_SEL		0x70U
#define SDHC_PHYREG2_TRIM_ICP		0xF00U

/* Bit map for software register */
#define SDHC_SWRST_ALL_MASK	BIT(0)

/* Bit map for response types */
#define RESP_NONE	SDHC_CMD_RESP_NONE
#define RESP_R1B	SDHC_CMD_RESP_L48_BSY_CHK | \
	SDHC_CMD_CRC_CHK_EN_MASK | \
	SDHC_CMD_INX_CHK_EN_MASK
#define RESP_R1	SDHC_CMD_RESP_L48_MASK | \
	SDHC_CMD_CRC_CHK_EN_MASK | \
	SDHC_CMD_INX_CHK_EN_MASK
#define RESP_R2	SDHC_CMD_RESP_L136_MASK | \
	SDHC_CMD_CRC_CHK_EN_MASK
#define RESP_R3	SDHC_CMD_RESP_L48_MASK
#define RESP_R6	SDHC_CMD_RESP_L48_BSY_CHK | \
	SDHC_CMD_CRC_CHK_EN_MASK | \
	SDHC_CMD_INX_CHK_EN_MASK
#define SDHC_CMD_RESP_NONE			0x0U
#define SDHC_CMD_RESP_L136_MASK		BIT(0)
#define SDHC_CMD_RESP_L48_MASK		BIT(1)
#define SDHC_CMD_RESP_L48_BSY_CHK		0x3U
#define SDHC_CMD_CRC_CHK_EN_MASK		BIT(3)
#define SDHC_CMD_INX_CHK_EN_MASK		BIT(4)
#define SDHC_CMD_RESP_INVAL		0xFFU
#define SDHC_OPCODE_SHIFT			0x8U
#define SDHC_RESP				0xFU

/* Bit map to update response type */
#define SDHC_CRC_LEFT_SHIFT	0x8U
#define SDHC_CRC_RIGHT_SHIFT	0x18U

/* Bit map for clock configuration */
#define SDHC_CC_DIV_SHIFT			0x8U
#define SDHC_CC_EXT_MAX_DIV_CNT		0x7FEU
#define SDHC_CC_SDCLK_FREQ_SEL		0xFFU
#define SDHC_CC_SDCLK_FREQ_SEL_EXT		0x3U
#define SDHC_CC_EXT_DIV_SHIFT		0x6U
#define SDHC_CLOCK_CNT_SHIFT		0x1U

/* Bit map for enable clock */
#define SDHC_CC_INT_CLK_EN_MASK		BIT(0)
#define SDHC_CC_INT_CLK_STABLE_MASK	BIT(1)
#define SDHC_CC_SD_CLK_EN_MASK		BIT(2)

/* Tuning command parameters */
#define SDHC_TUNING_CMD_BLKCOUNT	0x1U
#define SDHC_MAX_TUNING_COUNT	0X28U
#define SDHC_TUNING_CMD_BLKSIZE	0x40U
#define SDHC_BLK_SIZE_512		0x200U

/* Constant tap delay values and mask */
#define SDHC_SD_OTAP_DEFAULT_PHASES	{60, 0, 48, 0, 48, 72, 90, 36, 60, 90, 0}
#define SDHC_SD_ITAP_DEFAULT_PHASES	{132, 0, 132, 0, 132, 0, 0, 162, 90, 0, 0}
#define SDHC_EMMC_OTAP_DEFAULT_PHASES	{113, 0, 0, 0, 0, 0, 0, 0, 113, 79, 45}
#define SDHC_EMMC_ITAP_DEFAULT_PHASES	{0, 0, 0, 0, 0, 0, 0, 0, 39, 0, 0}
#define SDHC_TIMING_MMC_HS			0U
#define SDHC_CLK_PHASES			2U
#define SDHC_ITAP				0
#define SDHC_OTAP				1
#define SDHC_MAX_CLK_PHASE			360U
#define SDHC_SD_200HZ_MAX_OTAP		8U
#define SDHC_SD_50HZ_MAX_OTAP		30U
#define SDHC_SD_100HZ_MAX_OTAP		15U
#define SDHC_SD_200HZ_MAX_ITAP		30U
#define SDHC_SD_50HZ_MAX_ITAP		120U
#define SDHC_SD_100HZ_MAX_ITAP		60U
#define SDHC_EMMC_200HZ_MAX_OTAP		32U
#define SDHC_EMMC_50HZ_MAX_OTAP		16U
#define SDHC_EMMC_50HZ_MAX_ITAP		32U

/* Constant dll clock frequency select */
#define SDHC_FREQSEL_200M_170M	0x0U
#define SDHC_FREQSEL_170M_140M	0x1U
#define SDHC_FREQSEL_140M_110M	0x2U
#define SDHC_FREQSEL_110M_80M	0x3U
#define SDHC_FREQSEL_80M_50M	0x4U
#define SDHC_200_FREQ		200U
#define SDHC_170_FREQ		170U
#define SDHC_140_FREQ		140U
#define SDHC_110_FREQ		110U
#define SDHC_80_FREQ		80U

#define SDHC_KHZ_TO_MHZ		1000000U

#define SDHC_DAT_PRESENT_SEL_MASK	BIT(5)

#define SDHC_TXFR_INTR_EN_MASK	0x8023U
#define SDHC_DAT_LINE_TIMEOUT	0xEU

#define SDHC_SD_SLOT		0x0U
#define SDHC_EMMC_SLOT		0X1U


#define BITS_PER_LONG 32
#define GENMASK(h, l)        (((~0UL) << (l)) & (~0UL >> (BITS_PER_LONG - 1 - (h))))
/*
 * Controller registers
 */
#define SDHCI_DMA_ADDRESS   0x00
#define SDHCI_ARGUMENT2     SDHCI_DMA_ADDRESS
#define SDHCI_32BIT_BLK_CNT SDHCI_DMA_ADDRESS

#define SDHCI_BLOCK_SIZE             0x04
#define SDHCI_MAKE_BLKSZ(dma, blksz) (((dma & 0x7) << 12) | (blksz & 0xFFF))

#define SDHCI_BLOCK_COUNT 0x06

#define SDHCI_ARGUMENT 0x08

#define SDHCI_TRANSFER_MODE   0x0C
#define SDHCI_TRNS_DMA        0x01
#define SDHCI_TRNS_BLK_CNT_EN 0x02
#define SDHCI_TRNS_AUTO_CMD12 0x04
#define SDHCI_TRNS_AUTO_CMD23 0x08
#define SDHCI_TRNS_AUTO_SEL   0x0C
#define SDHCI_TRNS_READ       0x10
#define SDHCI_TRNS_MULTI      0x20

#define SDHCI_COMMAND       0x0E
#define SDHCI_CMD_RESP_MASK 0x03
#define SDHCI_CMD_CRC       0x08
#define SDHCI_CMD_INDEX     0x10
#define SDHCI_CMD_DATA      0x20
#define SDHCI_CMD_ABORTCMD  0xC0

#define SDHCI_CMD_RESP_NONE       0x00
#define SDHCI_CMD_RESP_LONG       0x01
#define SDHCI_CMD_RESP_SHORT      0x02
#define SDHCI_CMD_RESP_SHOBUSY 0x03

#define SDHCI_MAKE_CMD(c, f) (((c & 0xff) << 8) | (f & 0xff))
#define SDHCI_GET_CMD(c)     ((c >> 8) & 0x3f)

#define SDHCI_RESPONSE 0x10

#define SDHCI_BUFFER 0x20

#define SDHCI_PRESENT_STATE   0x24
#define SDHCI_CMD_INHIBIT     0x00000001
#define SDHCI_DATA_INHIBIT    0x00000002
#define SDHCI_DOING_WRITE     0x00000100
#define SDHCI_DOING_READ      0x00000200
#define SDHCI_SPACE_AVAILABLE 0x00000400
#define SDHCI_DATA_AVAILABLE  0x00000800
#define SDHCI_CARD_PRESENT    0x00010000
#define SDHCI_CARD_PRES_SHIFT 16
#define SDHCI_CD_STABLE       0x00020000
#define SDHCI_CD_LVL          0x00040000
#define SDHCI_CD_LVL_SHIFT    18
#define SDHCI_WRITE_PROTECT   0x00080000
#define SDHCI_DATA_LVL_MASK   0x00F00000
#define SDHCI_DATA_LVL_SHIFT  20
#define SDHCI_DATA_0_LVL_MASK 0x00100000
#define SDHCI_CMD_LVL         0x01000000

#define SDHCI_HOST_CONTROL    0x28
#define SDHCI_CTRL_LED        0x01
#define SDHCI_CTRL_4BITBUS    0x02
#define SDHCI_CTRL_HISPD      0x04
#define SDHCI_CTRL_DMA_MASK   0x18
#define SDHCI_CTRL_SDMA       0x00
#define SDHCI_CTRL_ADMA1      0x08
#define SDHCI_CTRL_ADMA32     0x10
#define SDHCI_CTRL_ADMA64     0x18
#define SDHCI_CTRL_ADMA3      0x18
#define SDHCI_CTRL_8BITBUS    0x20
#define SDHCI_CTRL_CDTEST_INS 0x40
#define SDHCI_CTRL_CDTEST_EN  0x80

#define SDHCI_POWER_CONTROL 0x29
#define SDHCI_POWER_ON      0x01
#define SDHCI_POWER_180     0x0A
#define SDHCI_POWER_300     0x0C
#define SDHCI_POWER_330     0x0E
/*
 * VDD2 - UHS2 or PCIe/NVMe
 * VDD2 power on/off and voltage select
 */
#define SDHCI_VDD2_POWER_ON  0x10
#define SDHCI_VDD2_POWER_120 0x80
#define SDHCI_VDD2_POWER_180 0xA0

#define SDHCI_BLOCK_GAP_CONTROL 0x2A

#define SDHCI_WAKE_UP_CONTROL 0x2B
#define SDHCI_WAKE_ON_INT     0x01
#define SDHCI_WAKE_ON_INSERT  0x02
#define SDHCI_WAKE_ON_REMOVE  0x04

#define SDHCI_CLOCK_CONTROL    0x2C
#define SDHCI_DIVIDER_SHIFT    8
#define SDHCI_DIVIDER_HI_SHIFT 6
#define SDHCI_DIV_MASK         0xFF
#define SDHCI_DIV_MASK_LEN     8
#define SDHCI_DIV_HI_MASK      0x300
#define SDHCI_PROG_CLOCK_MODE  0x0020
#define SDHCI_CLOCK_CARD_EN    0x0004
#define SDHCI_CLOCK_PLL_EN     0x0008
#define SDHCI_CLOCK_INT_STABLE 0x0002
#define SDHCI_CLOCK_INT_EN     0x0001

#define SDHCI_TIMEOUT_CONTROL 0x2E

#define SDHCI_SOFTWARE_RESET 0x2F
#define SDHCI_RESET_ALL      0x01
#define SDHCI_RESET_CMD      0x02
#define SDHCI_RESET_DATA     0x04

#define SDHCI_INT_STATUS       0x30
#define SDHCI_ERR_INT_STATUS       0x32
#define SDHCI_INT_ENABLE       0x34
#define SDHCI_SIGNAL_ENABLE    0x38
#define SDHCI_INT_RESPONSE     0x00000001
#define SDHCI_INT_DATA_END     0x00000002
#define SDHCI_INT_BLK_GAP      0x00000004
#define SDHCI_INT_DMA_END      0x00000008
#define SDHCI_INT_SPACE_AVAIL  0x00000010
#define SDHCI_INT_DATA_AVAIL   0x00000020
#define SDHCI_INT_CARD_INSERT  0x00000040
#define SDHCI_INT_CARD_REMOVE  0x00000080
#define SDHCI_INT_CARD_INT     0x00000100
#define SDHCI_INT_RETUNE       0x00001000
#define SDHCI_INT_CQE          0x00004000
#define SDHCI_INT_ERROR        0x00008000
#define SDHCI_INT_TIMEOUT      0x00010000
#define SDHCI_INT_CRC          0x00020000
#define SDHCI_INT_END_BIT      0x00040000
#define SDHCI_INT_INDEX        0x00080000
#define SDHCI_INT_DATA_TIMEOUT 0x00100000
#define SDHCI_INT_DATA_CRC     0x00200000
#define SDHCI_INT_DATA_END_BIT 0x00400000
#define SDHCI_INT_BUS_POWER    0x00800000
#define SDHCI_INT_AUTO_CMD_ERR 0x01000000
#define SDHCI_INT_ADMA_ERROR   0x02000000

#define SDHCI_INT_NORMAL_MASK 0x00007FFF
#define SDHCI_INT_ERROR_MASK  0xFFFF8000

#define SDHCI_INT_CMD_MASK  (SDHCI_INT_RESPONSE | SDHCI_INT_TIMEOUT | SDHCI_INT_CRC | SDHCI_INT_END_BIT | SDHCI_INT_INDEX | SDHCI_INT_AUTO_CMD_ERR)
#define SDHCI_INT_DATA_MASK (SDHCI_INT_DATA_END | SDHCI_INT_DMA_END | SDHCI_INT_DATA_AVAIL | SDHCI_INT_SPACE_AVAIL | SDHCI_INT_DATA_TIMEOUT | SDHCI_INT_DATA_CRC | SDHCI_INT_DATA_END_BIT | SDHCI_INT_ADMA_ERROR | SDHCI_INT_BLK_GAP)
#define SDHCI_INT_ALL_MASK  ((unsigned int)-1)

#define SDHCI_CQE_INT_ERR_MASK ( \
    SDHCI_INT_ADMA_ERROR | SDHCI_INT_BUS_POWER | SDHCI_INT_DATA_END_BIT | SDHCI_INT_DATA_CRC | SDHCI_INT_DATA_TIMEOUT | SDHCI_INT_INDEX | SDHCI_INT_END_BIT | SDHCI_INT_CRC | SDHCI_INT_TIMEOUT)

#define SDHCI_CQE_INT_MASK (SDHCI_CQE_INT_ERR_MASK | SDHCI_INT_CQE)

#define SDHCI_AUTO_CMD_STATUS  0x3C
#define SDHCI_AUTO_CMD_TIMEOUT 0x00000002
#define SDHCI_AUTO_CMD_CRC     0x00000004
#define SDHCI_AUTO_CMD_END_BIT 0x00000008
#define SDHCI_AUTO_CMD_INDEX   0x00000010

#define SDHCI_HOST_CONTROL2          0x3E
#define SDHCI_CTRL_UHS_MASK          0x0007
#define SDHCI_CTRL_UHS_SDR12         0x0000
#define SDHCI_CTRL_UHS_SDR25         0x0001
#define SDHCI_CTRL_UHS_SDR50         0x0002
#define SDHCI_CTRL_UHS_SDR104        0x0003
#define SDHCI_CTRL_UHS_DDR50         0x0004
#define SDHCI_CTRL_HS400             0x0005 /* Non-standard */
#define SDHCI_CTRL_VDD_180           0x0008
#define SDHCI_CTRL_DRV_TYPE_MASK     0x0030
#define SDHCI_CTRL_DRV_TYPE_B        0x0000
#define SDHCI_CTRL_DRV_TYPE_A        0x0010
#define SDHCI_CTRL_DRV_TYPE_C        0x0020
#define SDHCI_CTRL_DRV_TYPE_D        0x0030
#define SDHCI_CTRL_EXEC_TUNING       0x0040
#define SDHCI_CTRL_TUNED_CLK         0x0080
#define SDHCI_CMD23_ENABLE           0x0800
#define SDHCI_CTRL_V4_MODE           0x1000
#define SDHCI_CTRL_64BIT_ADDR        0x2000
#define SDHCI_CTRL_PRESET_VAL_ENABLE 0x8000

#define SDHCI_CAPABILITIES       0x40
#define SDHCI_TIMEOUT_CLK_MASK   GENMASK(5, 0)
#define SDHCI_TIMEOUT_CLK_SHIFT  0
#define SDHCI_TIMEOUT_CLK_UNIT   0x00000080
#define SDHCI_CLOCK_BASE_MASK    GENMASK(13, 8)
#define SDHCI_CLOCK_BASE_SHIFT   8
#define SDHCI_CLOCK_V3_BASE_MASK GENMASK(15, 8)
#define SDHCI_MAX_BLOCK_MASK     0x00030000
#define SDHCI_MAX_BLOCK_SHIFT    16
#define SDHCI_CAN_DO_8BIT        0x00040000
#define SDHCI_CAN_DO_ADMA2       0x00080000
#define SDHCI_CAN_DO_ADMA1       0x00100000
#define SDHCI_CAN_DO_HISPD       0x00200000
#define SDHCI_CAN_DO_SDMA        0x00400000
#define SDHCI_CAN_DO_SUSPEND     0x00800000
#define SDHCI_CAN_VDD_330        0x01000000
#define SDHCI_CAN_VDD_300        0x02000000
#define SDHCI_CAN_VDD_180        0x04000000
#define SDHCI_CAN_64BIT_V4       0x08000000
#define SDHCI_CAN_64BIT          0x10000000

#define SDHCI_CAPABILITIES_1            0x44
#define SDHCI_SUPPOSDR50             0x00000001
#define SDHCI_SUPPOSDR104            0x00000002
#define SDHCI_SUPPODDR50             0x00000004
#define SDHCI_DRIVER_TYPE_A             0x00000010
#define SDHCI_DRIVER_TYPE_C             0x00000020
#define SDHCI_DRIVER_TYPE_D             0x00000040
#define SDHCI_RETUNING_TIMER_COUNT_MASK GENMASK(11, 8)
#define SDHCI_USE_SDR50_TUNING          0x00002000
#define SDHCI_RETUNING_MODE_MASK        GENMASK(15, 14)
#define SDHCI_CLOCK_MUL_MASK            GENMASK(23, 16)
#define SDHCI_CAN_DO_ADMA3              0x08000000
#define SDHCI_SUPPOHS400             0x80000000 /* Non-standard */

#define SDHCI_MAX_CURRENT            0x48
#define SDHCI_MAX_CURRENT_LIMIT      GENMASK(7, 0)
#define SDHCI_MAX_CURRENT_330_MASK   GENMASK(7, 0)
#define SDHCI_MAX_CURRENT_300_MASK   GENMASK(15, 8)
#define SDHCI_MAX_CURRENT_180_MASK   GENMASK(23, 16)
#define SDHCI_MAX_CURRENT_MULTIPLIER 4

/* 4C-4F reserved for more max current */

#define SDHCI_SET_ACMD12_ERROR 0x50
#define SDHCI_SET_INT_ERROR    0x52

#define SDHCI_ADMA_ERROR 0x54

/* 55-57 reserved */

#define SDHCI_ADMA_ADDRESS    0x58
#define SDHCI_ADMA_ADDRESS_HI 0x5C

/* 60-FB reserved */

#define SDHCI_PRESET_FOR_HIGH_SPEED 0x64
#define SDHCI_PRESET_FOR_SDR12      0x66
#define SDHCI_PRESET_FOR_SDR25      0x68
#define SDHCI_PRESET_FOR_SDR50      0x6A
#define SDHCI_PRESET_FOR_SDR104     0x6C
#define SDHCI_PRESET_FOR_DDR50      0x6E
#define SDHCI_PRESET_FOR_HS400      0x74 /* Non-standard */
#define SDHCI_PRESET_DRV_MASK       GENMASK(15, 14)
#define BIT(nr)                     ((1) << (nr))

#define SDHCI_PRESET_CLKGEN_SEL      BIT(10)
#define SDHCI_PRESET_SDCLK_FREQ_MASK GENMASK(9, 0)

#define SDHCI_SLOT_INT_STATUS 0xFC

#define SDHCI_HOST_VERSION     0xFE
#define SDHCI_VENDOR_VER_MASK  0xFF00
#define SDHCI_VENDOR_VER_SHIFT 8
#define SDHCI_SPEC_VER_MASK    0x00FF
#define SDHCI_SPEC_VER_SHIFT   0
#define SDHCI_SPEC_100         0
#define SDHCI_SPEC_200         1
#define SDHCI_SPEC_300         2
#define SDHCI_SPEC_400         3
#define SDHCI_SPEC_410         4
#define SDHCI_SPEC_420         5

/*
 * End of controller registers.
 */

struct reg_base {
	volatile uint32_t sdma_sysaddr;  /**< SDMA System Address */
	volatile uint16_t block_size;    /**< Block Size */
	volatile uint16_t block_count;   /**< Block Count */
	volatile uint32_t argument;      /**< Argument */
	volatile uint16_t transfer_mode; /**< Transfer Mode */
	volatile uint16_t cmd;           /**< Command */

	volatile uint32_t resp_0;               /**< Response Register 0 */
	volatile uint32_t resp_1;               /**< Response Register 1 */
	volatile uint32_t resp_2;               /**< Response Register 2 */
	volatile uint32_t resp_3;               /**< Response Register 3 */
	volatile uint32_t data_port;            /**< Buffer Data Port */
	volatile uint32_t present_state;        /**< Present State */
	volatile uint8_t host_ctrl1;            /**< Host Control 1 */
	volatile uint8_t power_ctrl;            /**< Power Control */
	volatile uint8_t block_gap_ctrl;        /**< Block Gap Control */
	volatile uint8_t wake_up_ctrl;          /**< Wakeup Control */
	volatile uint16_t clock_ctrl;           /**< Clock Control */
	volatile uint8_t timeout_ctrl;          /**< Timeout Control */
	volatile uint8_t sw_reset;              /**< Software Reset */
	volatile uint16_t normal_int_stat;      /**< Normal Interrupt Status */
	volatile uint16_t err_int_stat;         /**< Error Interrupt Status */
	volatile uint16_t normal_int_stat_en;   /**< Normal Interrupt Status Enable */
	volatile uint16_t err_int_stat_en;      /**< Error Interrupt Status Enable */
	volatile uint16_t normal_int_signal_en; /**< Normal Interrupt Signal Enable */
	volatile uint16_t err_int_signal_en;    /**< Error Interrupt Signal Enable */
	volatile uint16_t auto_cmd_err_stat;    /**< Auto CMD Error Status */
	volatile uint16_t host_ctrl2;           /**< Host Control 2 */
	volatile uint64_t capabilities;         /**< Capabilities */

	volatile uint64_t max_current_cap;        /**< Max Current Capabilities */
	volatile uint16_t force_err_autocmd_stat; /**< Force Event for Auto CMD Error Status*/
	volatile uint16_t force_err_int_stat;     /**< Force Event for Error Interrupt Status */
	volatile uint8_t adma_err_stat;           /**< ADMA Error Status */
	volatile uint8_t reserved0[3];
	volatile uint64_t adma_sys_addr; /**< ADMA System Address */
	volatile uint16_t preset_val_0;   /**< Preset Value 0 */
	volatile uint16_t preset_val_1;   /**< Preset Value 1 */
	volatile uint16_t preset_val_2;   /**< Preset Value 2 */
	volatile uint16_t preset_val_3;   /**< Preset Value 3 */
	volatile uint16_t preset_val_4;   /**< Preset Value 4 */
	volatile uint16_t preset_val_5;   /**< Preset Value 5 */
	volatile uint16_t preset_val_6;   /**< Preset Value 6 */
	volatile uint16_t preset_val_7;   /**< Preset Value 7 */
	volatile uint32_t boot_timeout;   /**< Boot Timeout */
	volatile uint16_t reserved1[58];
	volatile uint32_t reserved2[5];
	volatile uint16_t slot_intr_stat;     /**< Slot Interrupt Status */
	volatile uint16_t host_cntrl_version; /**< Host Controller Version */
	volatile uint32_t reserved4[64];
	volatile uint32_t cq_ver;            /**< Command Queue Version */
	volatile uint32_t cq_cap;            /**< Command Queue Capabilities */
	volatile uint32_t cq_cfg;            /**< Command Queue Configuration */
	volatile uint32_t cq_ctrl;           /**< Command Queue Control */
	volatile uint32_t cq_intr_stat;      /**< Command Queue Interrupt Status */
	volatile uint32_t cq_intr_stat_en;   /**< Command Queue Interrupt Status Enable */
	volatile uint32_t cq_intr_sig_en;    /**< Command Queue Interrupt Signal Enable */
	volatile uint32_t cq_intr_coalesc;   /**< Command Queue Interrupt Coalescing */
	volatile uint32_t cq_tdlba;          /**< Command Queue Task Desc List Base Addr */
	volatile uint32_t cq_tdlba_upr;      /**< Command Queue Task Desc List Base Addr Upr */
	volatile uint32_t cq_task_db;        /**< Command Queue Task DoorBell */
	volatile uint32_t cq_task_db_notify; /**< Command Queue Task DoorBell Notify */
	volatile uint32_t cq_dev_qstat;      /**< Command Queue Device queue status */
	volatile uint32_t cq_dev_pend_task;  /**< Command Queue Device pending tasks */
	volatile uint32_t cq_task_clr;       /**< Command Queue Task Clr */
	volatile uint32_t reserved6;
	volatile uint32_t cq_ssc1;  /**< Command Queue Send Status Configuration 1 */
	volatile uint32_t cq_ssc2;  /**< Command Queue Send Status Configuration 2 */
	volatile uint32_t cq_crdct; /**< Command response for direct command */
	volatile uint32_t reserved7;
	volatile uint32_t cq_rmem;  /**< Command response mode error mask */
	volatile uint32_t cq_terri; /**< Command Queue Task Error Information */
	volatile uint32_t cq_cri;   /**< Command Queue Command response index */
	volatile uint32_t cq_cra;   /**< Command Queue Command response argument */
	volatile uint32_t cq_cerri; /**< Command Queue Command Error Information */
	volatile uint32_t reserved8[3];
	volatile uint32_t phy_ctrl1; /**< Configuring phyctrl */
	volatile uint32_t phy_ctrl2; /**< Configuring phyctrl and DLL */
	volatile uint32_t bist_ctrl; /**< BIST test control */
	volatile uint32_t bist_stat; /**< BIST test status */
	volatile uint32_t hs200_tap; /**< HS200 Tap Delay Select */
	volatile uint32_t reserved3[15261];
	volatile uint32_t itap_dly; /**< Input Tap Delay Select */
	volatile uint32_t otap_dly; /**< Output Tap Delay Select */
} __packed;
#endif /* __SDHC_H__ */