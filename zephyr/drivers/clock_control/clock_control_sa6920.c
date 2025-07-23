/*
 * Copyright (c) 2023 Gerson Fernando Budke <nandojve@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT sa6920_crg

#include <stdint.h>

#include <zephyr/arch/cpu.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/clock_control.h>
#include <zephyr/drivers/clock_control/sa6920_clock_control.h>
#include <soc.h>

#include <zephyr/logging/log.h>
#include <zephyr/shell/shell.h>
LOG_MODULE_REGISTER(clock_control, CONFIG_CLOCK_CONTROL_LOG_LEVEL);


/* Driver config */
struct sa6920_crg_config {
	uintptr_t always_base;
	uintptr_t filter_base;
    uintptr_t shutdown_base;
    uintptr_t pmu_addr;
};

static const struct sa6920_crg_config *devcfg;

#define CLK_BIT(n)			(1 << n)
#define CLK_RATE_24M			(24000000)

#define always_readl(off)  (sys_readl(devcfg->always_base + off))
#define always_writel(val, off)  (sys_writel(val,devcfg->always_base + off))
#define filter_readl(off)  (sys_readl(devcfg->filter_base + off))
#define filter_writel(val, off)  (sys_writel(val,devcfg->filter_base + off))
#define shutdown_readl(off)  (sys_readl(devcfg->shutdown_base + off))
#define shutdown_writel(val, off)  (sys_writel(val,devcfg->shutdown_base + off))


/* alwayson crg */
#define ALWAYSON_SEL_CONFIG			(0x0)
#define ALWAYSON_CLKGT_CONFIG		(0x8) /* ALWAYSON clk gating config register */
#define ALWAYSON_APB_CLK_DIV		(0x1C) 

#define FILTER_SEL_CONFIG           (0x0)
    #define PLL1_CTRL_CLK_SW        CLK_BIT(0)
    #define PLL1_CLK_DIV2           0x2
    #define PLL1_CLK_DIV4           0x4
    #define PLL1_CLK_DIV6           0x6
#define FILTER_VIN_CONFIG			(0x4)
    #define FILTER_VIN_SNR_MCLK_MUX_SHIFT (0x6)
    #define FILTER_VIN_SNR_MCLK_XTAL_DIV_SHIFT (0x7)
    #define FILTER_VIN_SNR_MCLK_XTAL_DIV_WIDTH (0x3)
    #define FILTER_VIN_SNR_MCLK_PLL_DIV_SHIFT (10)
    #define FILTER_VIN_SNR_MCLK_PLL_DIV_WIDTH (0x6)
#define FILTER_VPU_CONFIG			(0x8)
#define FILTER_VNNE_CONFIG			(0xC)
#define FILTER_PLL1_CONFIG0         (0x10)
#define FILTER_PLL1_STATUS          (0x18)
#define FILTER_CLKGT_CONFIG			(0x1C)
#define FILTER_PIXEL_CONFIG			(0x28)
#define FILTER_BUS_CLK_DIV			(0x2C)
#define SMS_CLKGT_CONFIG			(0x30)
#define FILTER_CLKGT_CONFIG1		(0x38)
#define FILTER_APB_PCLK_DIV			(0x3C)


#define SA6920_SHUTDOWN_SEL_CONFIG  (0x0)
#define PERI_CLK_SW             (0)
    #define PERI_CLK_SW_MASK	(0x1)
#define CPU_CLK_SW              (1)
    #define CPU_CLK_SW_MASK	    (0x7)
#define DDR_CLK_SW              (4)
    #define DDR_CLK_SW_MASK	    (0x7)
#define BUS_CLK_SW              (7)
    #define BUS_CLK_SW_MASK	    (0x7)
#define NPU_CLK_SW              (10)
    #define NPU_CLK_SW_MASK	    (0x7)
#define VPU_CLK_SW              (13)
    #define VPU_CLK_SW_MASK	    (0x7)
#define ISP_CLK_SW              (16)
    #define ISP_CLK_SW_MASK	    (0x1)
#define PLL1_CLK_SELECT	        (0x5)
#define PLL2_CLK_SELECT	        (0x3)
#define PLL3_CLK_SELECT	        (0x1)
#define XTAL24M_CLK_SELECT	    (0x0)
#define CLK_DIV(n) 	(n)

#define SHUTDOWN_PERI_DIV_CLKGT		    (0x4)
#define SHUTDOWN_CPU_DIV_CONFIG         (0x8)
    #define A55_SCLK_DIV_CLKGT          CLK_BIT(0)
    #define CPU_PCLK_DIV_CLKGT          CLK_BIT(1)
    #define CPU_PLL_CLK_DIV             (0x2)
    #define CPU_PLL_CLK_DIV_MSAK        (0xF)
#define SHUTDOWN_DDR_DIV_CONFIG         (0xC)
    #define DDR_DIV_CLKGT               CLK_BIT(0)
    #define DDR_PLL_CLK_DIV             (0x1)
    #define DDR_PLL_CLK_DIV_MSAK        (0xF)
#define SHUTDOWN_MAINBUS_DIV_CONFIG     (0x10)
    #define BUS_ACLK_DIV_CLKGT          CLK_BIT(0)
    #define SRAMC_DIV_CLKGT             CLK_BIT(1)
    #define DPHY_DIV_CLKGT              CLK_BIT(3)
    #define APB_DIV_CLKGT               CLK_BIT(4)
    #define AHB_DIV_CLKGT               CLK_BIT(5)
    #define BUS_PLL_CLK_DIV             (0x6)
    #define BUS_PLL_CLK_DIV_MSAK        (0xF)
#define SHUTDOWN_NPU_DIV_CONFIG		    (0x14)
#define SHUTDOWN_VPU_DIV_CLKGT		    (0x18)
#define SHUTDOWN_ISP_DIV_CLKGT		    (0x1C)
#define SHUTDOWN_CLKGT_CONFIG0          (0x20)
    #define SHUT_PADC_GATE_CLK_ON       CLK_BIT(0)
    #define SHUT_GPIO_GATE_CLK_ON       CLK_BIT(3)
    #define USB_GATE_CLK_ON             CLK_BIT(7)
#define SHUTDOWN_CLKGT_CONFIG1          (0x28)
    #define A55_GATE_CLK_ON             CLK_BIT(0)
    #define CPU_GATE_CLK_ON             (CLK_BIT(1) | CLK_BIT(2) | CLK_BIT(3) | CLK_BIT(4))
#define SHUTDOWN_CLKGT_CONFIG2          (0x30)
    #define DDRC_GATE_CLK_ON            (CLK_BIT(0) | CLK_BIT(5) | CLK_BIT(6) | CLK_BIT(7) | CLK_BIT(8))
    #define DDRPHY_GATE_CLK_ON          (CLK_BIT(1) | CLK_BIT(2) | CLK_BIT(3) | CLK_BIT(4))
#define SHUTDOWN_CLKGT_CONFIG3          (0x38)
    #define SHUT_SRAMC_GATE_CLK_ON      (CLK_BIT(1) | CLK_BIT(3))
    #define SHUT_DMAC_GATE_CLK_ON       CLK_BIT(2)
#define SHUTDOWN_I2S1_DIV			    (0x40)
    #define SFC_MCLK_DIV_SHIFT      (24)
    #define SFC_MCLK_DIV_MASK       (0x1f)
    #define PDM_MCLK_DIV_SHIFT      (14)
    #define PDM_MCLK_DIV_WIDTH      (8)
    #define PDM_SCLK_DIV_SHIFT      (6)
    #define PDM_SCLK_DIV_WIDTH      (8)
#define SHUTDOWN_CLKGT_CONFIG4          (0x44)
    #define MMC0_GATE_CLK_ON            (CLK_BIT(0) | CLK_BIT(1) | CLK_BIT(2) | CLK_BIT(15))
    #define MMC1_GATE_CLK_ON            (CLK_BIT(4) | CLK_BIT(5) | CLK_BIT(6) | CLK_BIT(16))
    #define SFC_GATE_CLK_ON             (CLK_BIT(12) | CLK_BIT(13))
#define SHUTDOWN_CLKGT_CONFIG5		(0x4C) 
#define SHUTDOWN_PLL2_CONFIG0       (0x54)
#define SHUTDOWN_PLL2_STATUS        (0x5C)
#define SHUTDOWN_VPU_DIV			(0x60)
#define SHUTDOWN_VPU_NPU_CLKGT		(0x64)
#define SHUTDOWN_PLL3_CONFIG0       (0x6C)
#define PLL_RESETB                  1
#define SHUTDOWN_PLL3_STATUS        (0x74)
    #define SA6920_PLLLOCK_SHIFT        (0x0)
    #define SA6920_PLL_RESETB           CLK_BIT(0)
#define SHUTDOWN_ISP_CLK_SEL		(0x78)
#define SHUTDOWN_ISP_ACLK_DIV		(0x7C)
#define SHUTDOWN_ISP_BPCLK_CONFIG	(0x80)
    #define ISP_BPCLK_CYC_L_SHIFT   (0x2)
    #define ISP_BPCLK_CYC_L_WIDTH   (0x4)
    #define ISP_BPCLK_CYC_H_SHIFT   (0x6)
    #define ISP_BPCLK_CYC_H_WIDTH   (0x4)
    #define ISP_BPCLK_SW              (17)
    #define ISP_BPCLK_SW_MASK	    (0x3)
    #define ISP_BPCLK_PLL1_CLK_SELECT	        (0x2)
    #define ISP_BPCLK_PLL2_CLK_SELECT	        (0x1)
    #define ISP_BPCLK_PLL3_CLK_SELECT	        (0x0)

#define SHUTDOWN_VIN_ISP_CONFIG		(0x84)
    #define SHUTDOWN_VIN_SNR_MCLK_MUX_SHIFT (0x5)
    #define SHUTDOWN_VIN_SNR_MCLK_XTAL_DIV_SHIFT (0x6)
    #define SHUTDOWN_VIN_SNR_MCLK_XTAL_DIV_WIDTH (0x3)
    #define SHUTDOWN_VIN_SNR_MCLK_PLL_DIV_SHIFT (0x9)
    #define SHUTDOWN_VIN_SNR_MCLK_PLL_DIV_WIDTH (0x6)
#define SHUTDOWN_CLKGT_CONFIG7		(0x88) 
#define SHUTDOWN_LZ4_DIV			(0x90)
#define SHUTDOWN_VOUT_DIV			(0x94)
#define SHUTDOWN_SRAMC_CONFIG		(0x98)
#define SHUTDOWN_MSHC_DIV           (0x9C)
#define SHUTDOWN_USB_ASST_DIV		(0xA0)
#define SHUTDOWN_PDM_DIV			(0xA4)
    #define PDM_CLK_DIV_SHIFT       (0)
    #define PDM_CLK_DIV_WIDTH       (8)
#define SHUTDOWN_SPI_DIV			(0xA8)
#define SHUTDOWN_APB_CONFIG         (0xAC)
#define SHUTDOWN_AHB_CONFIG         (0xB0)
#define SHUTDOWN_SPI0_DIV			(0xB4)

#define SA6920_MDIV_MASK	(0x7ff)
#define SA6920_PDIV_MASK	(0x7f)
#define SA6920_SDIV_MASK	(0xf)
#define SA6920_MDIV_SHIFT	(9)
#define SA6920_PDIV_SHIFT	(2)
#define SA6920_SDIV_SHIFT	(20)

struct acme_pll_rate_table {
	unsigned int rate;
	unsigned int pdiv;
	unsigned int mdiv;
	unsigned int sdiv;
};

#define PLL_SA6920_RATE(_fin, _rate, _m, _p, _s)			\
	{							\
		.rate	=	(_rate),				\
		.mdiv	=	(_m),				\
		.pdiv	=	(_p),				\
		.sdiv	=	(_s),				\
	}

struct SysClockCfg {
	//uint32_t  pll1_rate;
    uint32_t  pll2_rate;
    uint32_t  pll3_rate;
    unsigned char  peri_clk_sw;
    unsigned char  cpu_clk_sw;
    unsigned char  ddr_clk_sw;
    unsigned char  bus_clk_sw;
    unsigned char  npu_clk_sw;
    unsigned char  vpu_clk_sw;
    unsigned char  isp_clk_sw;
};

struct Clock {
	//uint32_t  pll1_rate;
    unsigned int   clock_id;
    uint32_t  gate_reg;
    unsigned char  gate_shift;
    uint32_t  div_reg;
    unsigned char  div_shift;
    unsigned char  div_width;
    uint32_t  div_reg2;
    unsigned char  div_shift2;
    unsigned char  div_width2;
    unsigned char  div_flags;
};

/* Bit mask of specific length */
#define CLK_DIV_MASK(width)	((1 << (width)) - 1)
/* Division rounded to closest integer */
#define CLK_ROUND_DIV(a, b)       ((a + (b / 2)) / b)
/* Division rounded to integer greater than actual value */
#define CLK_CEIL_DIV(a, b)        ((a + b - 1) / b)

/* Limits the value within min and max */
#define CLK_LIMIT_VALUE(val, min, max) \
	do { \
		if (val < min) { \
			val = min; \
		} \
		if (val > max) { \
			val = max; \
		} \
	} while (0)

#define COMPOSITE(_id, gr, gs)	\
	{							\
		.clock_id		= _id,				\
		.gate_reg	    = gr,			\
		.gate_shift	    = gs,			\
		.div_reg		= 0,				\
		.div_shift		= 0,				\
		.div_width		= 0,				\
        .div_reg2		= 0,				\
		.div_shift2		= 0,				\
		.div_width2		= 0,				\
        .div_flags	    = 0,				\
	}
#define COMPOSITE_1(_id, gr, gs, dr, ds, dw)	\
	{							\
		.clock_id		= _id,				\
		.gate_reg	    = gr,			\
		.gate_shift	    = gs,			\
		.div_reg		= dr,				\
		.div_shift		= ds,				\
		.div_width		= dw,				\
        .div_reg2		= 0,				\
		.div_shift2		= 0,				\
		.div_width2		= 0,				\
        .div_flags	    = 1,				\
	}
#define COMPOSITE_2(_id, gr, gs, dr, ds, dw, dr2, ds2, dw2)	\
	{							\
		.clock_id		= _id,				\
		.gate_reg	    = gr,			\
		.gate_shift	    = gs,			\
		.div_reg		= dr,				\
		.div_shift		= ds,				\
		.div_width		= dw,				\
        .div_reg2		= dr2,				\
		.div_shift2		= ds2,				\
		.div_width2		= dw2,				\
        .div_flags	    = 2,				\
	}
#define COMPOSITE_DIV(_id, dr, ds, dw)	\
	{							\
		.clock_id		= _id,				\
		.gate_reg	    = 0,			\
		.gate_shift	    = 0,			\
		.div_reg		= dr,				\
		.div_shift		= ds,				\
		.div_width		= dw,				\
        .div_reg2		= 0,				\
		.div_shift2		= 0,				\
		.div_width2		= 0,				\
        .div_flags	    = 1,				\
	}
#define COMPOSITE_DIV2(_id, dr, ds, dw, dr2, ds2, dw2)	\
	{							\
		.clock_id		= _id,				\
		.gate_reg	    = 0,			\
		.gate_shift	    = 0,			\
		.div_reg		= dr,				\
		.div_shift		= ds,				\
		.div_width		= dw,				\
        .div_reg2		= dr2,				\
		.div_shift2		= ds2,				\
		.div_width2		= dw2,				\
        .div_flags	    = 2,				\
	}



/* 1<=p<=63,64<=m<=1023,0<=s<=6 */
static const struct acme_pll_rate_table sa6920_pll_rate_table[] = {
	PLL_SA6920_RATE(CLK_RATE_24M, 1188000000U,  0xc6,  4, 0),
    /* pll2 */
    PLL_SA6920_RATE(CLK_RATE_24M, 1024000000U,  0x200, 3, 2),
    PLL_SA6920_RATE(CLK_RATE_24M, 1125000000U,  0x177, 4, 1),
    PLL_SA6920_RATE(CLK_RATE_24M, 1152000000U,  0x179, 4, 1),
    PLL_SA6920_RATE(CLK_RATE_24M, 1536000000U,  0x100, 2, 1),
    PLL_SA6920_RATE(CLK_RATE_24M, 1880000000U,  0x1D6, 3, 1),
    PLL_SA6920_RATE(CLK_RATE_24M, 2048000000U,  0x200, 3, 1),
    PLL_SA6920_RATE(CLK_RATE_24M, 2250000000U,  0x177, 4, 0),
//    PLL_SA6920_RATE(CLK_RATE_24M, 2250000000U,  0x178, 2, 1),
    PLL_SA6920_RATE(CLK_RATE_24M, 2304000000U,  0x180, 2, 1),
    PLL_SA6920_RATE(CLK_RATE_24M, 3072000000U,  0x180, 3, 0),
    /* pll3 */
    PLL_SA6920_RATE(CLK_RATE_24M, 1200000000U,  0x12C, 3, 1),
    PLL_SA6920_RATE(CLK_RATE_24M, 1320000000U,  0xDC, 2, 1),
    PLL_SA6920_RATE(CLK_RATE_24M, 1380000000U,  0x159, 3, 1),
    PLL_SA6920_RATE(CLK_RATE_24M, 1600000000U,  0x190, 3, 1),
    PLL_SA6920_RATE(CLK_RATE_24M, 1880000000U,  0x1d6, 3, 1),
    PLL_SA6920_RATE(CLK_RATE_24M, 2100000000U,  0x20D, 3, 1),
    PLL_SA6920_RATE(CLK_RATE_24M, 2400000000U,  0x12C, 3, 0),
    PLL_SA6920_RATE(CLK_RATE_24M, 3072000000U,  0x300, 3, 3),
    PLL_SA6920_RATE(CLK_RATE_24M, 3450000000U,  0x23f, 4, 0),
};


/**********************************************
 *	@brief	 sa6920_clk_pll_recalc_rate
 *  @param
 *	@return   void
 **********************************************/
uint32_t sa6920_clk_pll_recalc_rate(uint32_t pll_reg)
{
	uint32_t mdiv, pdiv, sdiv, pll_con;
	uint64_t fvco = CLK_RATE_24M;

	pll_con = sys_read32(pll_reg);

	mdiv = (pll_con >> SA6920_MDIV_SHIFT) & SA6920_MDIV_MASK;
	pdiv = (pll_con >> SA6920_PDIV_SHIFT) & SA6920_PDIV_MASK;
	sdiv = (pll_con >> SA6920_SDIV_SHIFT) & SA6920_SDIV_MASK;

	fvco *= mdiv;
	fvco = (uint32_t)(fvco / (pdiv * CLK_BIT(sdiv)));

	return (uint32_t)fvco;
}

static bool sa6920_mp_change(const struct acme_pll_rate_table *rate, uint32_t pll_con)
{
	uint32_t ou_mdiv, ou_pdiv;

	ou_mdiv = (pll_con >> SA6920_MDIV_SHIFT) & SA6920_MDIV_MASK;
	ou_pdiv = (pll_con >> SA6920_PDIV_SHIFT) & SA6920_PDIV_MASK;

	return (rate->mdiv != ou_mdiv || rate->pdiv != ou_pdiv);
}

/**********************************************
 *	@brief	 sa6920_clk_pll_set_rate
 *  @param
 *	@return   int8_t
 **********************************************/
static int8_t sa6920_clk_pll_set_rate(uint32_t pll_reg,
    uint32_t lock_reg, uint32_t rate)
{
    uint32_t pll_con, i, retry = 2;
    uint32_t ou_rate;

    ou_rate = sa6920_clk_pll_recalc_rate(pll_reg);
    if(ou_rate == rate) {
        /* Wait until the PLL is locked.timeout=150us */
        for(i = 0; i < 150; i++) {
            pll_con = sys_read32(lock_reg);
            if(pll_con & (0x1 << SA6920_PLLLOCK_SHIFT))
                break;
            k_usleep(1);
        }
        if(i == 150) {
            LOG_INF("lock_offs=0x%x rate=%u\n\r",lock_reg, rate);
            return -1;
        }
        return 0;
    }

    for (i = 0; i < ARRAY_SIZE(sa6920_pll_rate_table); i++) {
		if (rate == sa6920_pll_rate_table[i].rate) {
			break;
        }
	}
    if (i == ARRAY_SIZE(sa6920_pll_rate_table)) {
		LOG_ERR("Invalid rate : %u for pll clk\n",rate);
		return -1;
    }
    pll_con = sys_read32(pll_reg);
    if (!(sa6920_mp_change(&sa6920_pll_rate_table[i], pll_con))) {
		/* If only s change, change just s value only*/
		pll_con &= ~(SA6920_SDIV_MASK << SA6920_SDIV_SHIFT);
		pll_con |= sa6920_pll_rate_table[i].sdiv << SA6920_SDIV_SHIFT;
		sys_write32(pll_con, pll_reg);
		return 0;
	}

lock_timeout:
    /* set resetb low */
    pll_con &= ~SA6920_PLL_RESETB;
	sys_write32(pll_con, pll_reg);
    /* Change PLL PMS values */
	pll_con &= ~((SA6920_MDIV_MASK << SA6920_MDIV_SHIFT) |
			(SA6920_PDIV_MASK << SA6920_PDIV_SHIFT) |
			(SA6920_SDIV_MASK << SA6920_SDIV_SHIFT));
	pll_con |= (sa6920_pll_rate_table[i].mdiv << SA6920_MDIV_SHIFT) |
			(sa6920_pll_rate_table[i].pdiv << SA6920_PDIV_SHIFT) |
			(sa6920_pll_rate_table[i].sdiv << SA6920_SDIV_SHIFT);
    pll_con &= ~SA6920_PLL_RESETB;//keep resetb low
	sys_write32(pll_con, pll_reg);

    /* set resetb high */
    pll_con |= SA6920_PLL_RESETB;
    sys_write32(pll_con, pll_reg);

	/* Wait until the PLL is locked if it is enabled.timeout=150us */
    for(i = 0; i < 150; i++) {
        pll_con = sys_read32(lock_reg);
        if(pll_con & (0x1 << SA6920_PLLLOCK_SHIFT))
            break;
        k_usleep(1);
    }
    if(i == 150) {
        LOG_INF ("Timeout lock_offs=0x%x rate=%u\n\r", lock_reg, rate);
        retry--;
        if(!retry) {
            goto lock_timeout;
        }
        else {
            return -1;
        }
    }
	return 0;
}

/**********************************************
 *	@brief	 sa6920 clock set pll
 *  @param	  uint32_t idx
 *	@return   void
 **********************************************/
uint32_t sa6920_clk_get_pll_rate(uint16_t clockidx)
{
    uint32_t pll_reg;
    switch(clockidx)
    {
    case MODULE_CLK_PLL1:
    {
        pll_reg = devcfg->filter_base + FILTER_PLL1_CONFIG0;
        break;
    }
    case MODULE_CLK_PLL2:
    {
        pll_reg = devcfg->shutdown_base + SHUTDOWN_PLL2_CONFIG0;
        break;
    }
    case MODULE_CLK_PLL3:
    {
        pll_reg = devcfg->shutdown_base + SHUTDOWN_PLL3_CONFIG0;
        break;
    }
    default:
        LOG_ERR("sa6920_clk_get_pll_rate idx=%d is mismatch!\n\r",clockidx);
        return 0;
    }

    return sa6920_clk_pll_recalc_rate(pll_reg);
}

/**********************************************
 *	@brief	 sa6920_pll_clk_sw
 *  @param	  pll_base:base register;
 *	@return   void
 **********************************************/
void sa6920_pll_clk_sw(uint32_t pll_reg, uint8_t clk_sw, uint8_t clk_sw_val)
{
    uint32_t val;

    val = sys_read32(pll_reg);
    if(pll_reg == devcfg->filter_base)
    {
        val &= (~(0x1 << PLL1_CTRL_CLK_SW));
        val |= (clk_sw_val << PLL1_CTRL_CLK_SW);
    }
    else
    {
        switch(clk_sw)
        {
        case  PERI_CLK_SW:
        {
            val &= (~(PERI_CLK_SW_MASK << PERI_CLK_SW));
            val |= (clk_sw_val << PERI_CLK_SW);
            break;
        }
        case  CPU_CLK_SW:
        {
            val &= (~(CPU_CLK_SW_MASK << CPU_CLK_SW));
            val |= (clk_sw_val << CPU_CLK_SW);
            break;
        }
        case  DDR_CLK_SW:
        {
            val &= (~(DDR_CLK_SW_MASK << DDR_CLK_SW));
            val |= (clk_sw_val << DDR_CLK_SW);
            break;
        }
        case  BUS_CLK_SW:
        {
            val &= (~(BUS_CLK_SW_MASK << BUS_CLK_SW));
            val |= (clk_sw_val << BUS_CLK_SW);
            break;
        }
        case  NPU_CLK_SW:
        {
            val &= (~(NPU_CLK_SW_MASK << NPU_CLK_SW));
            val |= (clk_sw_val << NPU_CLK_SW);
            break;
        }
        case  VPU_CLK_SW:
        {
            val &= (~(VPU_CLK_SW_MASK << VPU_CLK_SW));
            val |= (clk_sw_val << VPU_CLK_SW);
            break;
        }
        case  ISP_CLK_SW:
        {
            val &= (~(ISP_CLK_SW_MASK << ISP_CLK_SW));
            val |= (clk_sw_val << ISP_CLK_SW);
            break;
        }
        case  ISP_BPCLK_SW:
        {
            val &= (~(ISP_BPCLK_SW_MASK << 0));
            val |= ((clk_sw_val >> 1) << 0);
            break;
        }

        default:
            LOG_INF ("sa6920_pll_clk_sw clk_sw=0x%x is mismatch!\n\r",clk_sw);
            return;
        }
    }
    sys_write32(val, devcfg->shutdown_base + pll_reg);
}

/**********************************************
 *	@brief	 sa6920 clock set pll
 *  @param	  uint32_t idx
 *	@return   void
 **********************************************/
int32_t sa6920_clk_set_pll_rate(uint16_t clockidx, uint32_t rate)
{
    uint32_t val;
    switch(clockidx)
    {
    case MODULE_CLK_PLL1:
    sa6920_clk_pll_set_rate(devcfg->filter_base + FILTER_PLL1_CONFIG0, devcfg->filter_base + FILTER_PLL1_STATUS, rate);
    break;

    case MODULE_CLK_PLL2:
    /* set sel_config = 24M before change pll value */
    #define DDR_CLK_SW              (4)
    #define DDR_CLK_SW_MASK	    (0x7)

    //sys_write32(SA6920_SHUTDOWN_SEL_CONFIG, 0);
    val = sys_read32(devcfg->shutdown_base + SA6920_SHUTDOWN_SEL_CONFIG);
    val = val & (0x7<<4);

    sys_write32(val, devcfg->shutdown_base + SA6920_SHUTDOWN_SEL_CONFIG);


  //  LOG_INF ("sa6920_clk_set_rate pll2 = %u\n\r",rate);
    sa6920_clk_pll_set_rate(devcfg->shutdown_base + SHUTDOWN_PLL2_CONFIG0, devcfg->shutdown_base + SHUTDOWN_PLL2_STATUS, rate);
    break;

    case MODULE_CLK_PLL3:
    /* set sel_config = 24M before change pll value */
    sa6920_pll_clk_sw(devcfg->shutdown_base + SA6920_SHUTDOWN_SEL_CONFIG,
                DDR_CLK_SW, XTAL24M_CLK_SELECT);//switch to 24M first
    sa6920_clk_pll_set_rate(devcfg->shutdown_base + SHUTDOWN_PLL3_CONFIG0, devcfg->shutdown_base + SHUTDOWN_PLL3_STATUS, rate);
    break;

    default:
        LOG_INF ("sa6920_clk_set_pll_rate idx=0x%x is mismatch!\n\r",clockidx);
        return -1;
    }
    return 0;
}

/**********************************************
 *	@brief	 sa6920_clk_div_set
 *  @param	  ctrl:0: get; 1: set
 *	@return   void
 **********************************************/
void sa6920_clk_div_set(uint32_t idx, uint8_t div)
{
    uint32_t val;

    switch(idx)
    {
    case  SHUTDOWN_MSHC0_BCLK:
    case  SHUTDOWN_MSHC1_BCLK:
    {
        sys_write32(div, devcfg->shutdown_base + SHUTDOWN_MSHC_DIV);
        break;
    }
    case  SHUTDOWN_SFC_MCLK:
    {
        val = sys_read32(devcfg->shutdown_base + SHUTDOWN_I2S1_DIV);
        val &= (~(SFC_MCLK_DIV_MASK << SFC_MCLK_DIV_SHIFT));
        val |= (div << SFC_MCLK_DIV_SHIFT);
        sys_write32(val, devcfg->shutdown_base + SHUTDOWN_I2S1_DIV);
 
        break;
    }
    case  SHUTDOWN_A55_SCLK:
    {
        if (div == 0)
            break;
        val = sys_read32(devcfg->shutdown_base + SHUTDOWN_CPU_DIV_CONFIG);
        val &= (~(CPU_PLL_CLK_DIV_MSAK << CPU_PLL_CLK_DIV));
        val |= (div << CPU_PLL_CLK_DIV);
        sys_write32(val, devcfg->shutdown_base + SHUTDOWN_CPU_DIV_CONFIG);
        break;
    }
    case  SHUTDOWN_DDRC_CLK:
    {
        val = sys_read32(devcfg->shutdown_base + SHUTDOWN_DDR_DIV_CONFIG);
        val &= (~(DDR_PLL_CLK_DIV_MSAK << DDR_PLL_CLK_DIV));
        val |= (div << DDR_PLL_CLK_DIV);
        sys_write32(val, devcfg->shutdown_base + SHUTDOWN_DDR_DIV_CONFIG);
        break;
    }
    case  SHUTDOWN_BUS_ACLK:
    {
        val = sys_read32(devcfg->shutdown_base + SHUTDOWN_MAINBUS_DIV_CONFIG);
        val &= (~(BUS_PLL_CLK_DIV_MSAK << BUS_PLL_CLK_DIV));
        val |= (div << BUS_PLL_CLK_DIV);
        sys_write32(val, devcfg->shutdown_base + SHUTDOWN_MAINBUS_DIV_CONFIG);
        break;
    }
    default:
        LOG_INF ("sa6920_clk_div_set idx=0x%x is mismatch!\n\r",idx);
        break;
    }
}

/**********************************************
 *	@brief	 sa6920 a55 clock set
 *  @param	  uint32_t idx
 *	@return   void
 **********************************************/
void sa6920_clk_info_set(uint32_t idx, uint8_t clk_sel, uint8_t div)
{
    /* pll1 clk_sel=0x5; pll2 clk_sel=0x3; pll3 clk_sel=0x1 */
    if(clk_sel != 0x1 && clk_sel != 0x3 && clk_sel != 0x5) {
        LOG_INF ("sa6920_clk_info_set idx=0x%x clk_sel=%d err!\n\r",idx,clk_sel);
        return;
    }

    //bug fix for 6920
    unsigned int pll3_value = sys_read32(devcfg->shutdown_base + SHUTDOWN_PLL3_CONFIG0);
    if( !(pll3_value & SA6920_PLL_RESETB))
    {
        sys_write32(pll3_value | SA6920_PLL_RESETB, devcfg->shutdown_base + SHUTDOWN_PLL3_CONFIG0); // enable pll3
    }

    switch (idx)
    {
        case  SHUTDOWN_A55_SCLK:
        {
            sa6920_pll_clk_sw(devcfg->shutdown_base + SA6920_SHUTDOWN_SEL_CONFIG,
                CPU_CLK_SW, XTAL24M_CLK_SELECT);//switch to 24M first
            sa6920_clk_div_set(idx, div);
            sa6920_pll_clk_sw(devcfg->shutdown_base + SA6920_SHUTDOWN_SEL_CONFIG,
                CPU_CLK_SW, clk_sel);
        }
        break;
        case  SHUTDOWN_DDRC_CLK:
        {
            sa6920_pll_clk_sw(devcfg->shutdown_base + SA6920_SHUTDOWN_SEL_CONFIG,
                DDR_CLK_SW, XTAL24M_CLK_SELECT);//switch to 24M first
            sa6920_clk_div_set(idx, div);
            sa6920_pll_clk_sw(devcfg->shutdown_base + SA6920_SHUTDOWN_SEL_CONFIG,
                DDR_CLK_SW, clk_sel);
        }
        break;
        case  SHUTDOWN_BUS_ACLK:
        {
            sa6920_pll_clk_sw(devcfg->shutdown_base + SA6920_SHUTDOWN_SEL_CONFIG,
                BUS_CLK_SW, XTAL24M_CLK_SELECT);//switch to 24M first
            sa6920_clk_div_set(idx, div);
            sys_write32(0x4, devcfg->shutdown_base + SHUTDOWN_AHB_CONFIG);
            sys_write32(0x4, devcfg->shutdown_base + SHUTDOWN_APB_CONFIG);
            sa6920_pll_clk_sw(devcfg->shutdown_base + SA6920_SHUTDOWN_SEL_CONFIG,
                BUS_CLK_SW, clk_sel);
        }
        break;
        case  SHUTDOWN_NPU_CLK:
        {
            sa6920_pll_clk_sw(devcfg->shutdown_base + SA6920_SHUTDOWN_SEL_CONFIG,
                NPU_CLK_SW, XTAL24M_CLK_SELECT);//switch to 24M first
            sa6920_pll_clk_sw(devcfg->shutdown_base + SA6920_SHUTDOWN_SEL_CONFIG,
                NPU_CLK_SW, clk_sel);
        }
        break;
        case  SHUTDOWN_VENC_CLK:
        {
            sa6920_pll_clk_sw(devcfg->shutdown_base + SA6920_SHUTDOWN_SEL_CONFIG,
                VPU_CLK_SW, XTAL24M_CLK_SELECT);//switch to 24M first
            sa6920_pll_clk_sw(devcfg->shutdown_base + SA6920_SHUTDOWN_SEL_CONFIG,
                VPU_CLK_SW, clk_sel);
        }
        break;
        case  SHUTDOWN_VIN_MCLK:
        {
            sa6920_pll_clk_sw(devcfg->shutdown_base + SA6920_SHUTDOWN_SEL_CONFIG,
                ISP_CLK_SW, XTAL24M_CLK_SELECT);//switch to 24M first
            sa6920_pll_clk_sw(devcfg->shutdown_base + SA6920_SHUTDOWN_SEL_CONFIG,
                ISP_CLK_SW, clk_sel);
        }
        break;
        case  SHUTDOWN_ISP_ACLK:
        {
            sa6920_pll_clk_sw(devcfg->shutdown_base + SHUTDOWN_ISP_BPCLK_CONFIG,
                ISP_BPCLK_SW, clk_sel);
        }
        break;
        default:
            LOG_INF ("sa6920_clk_info_set idx=0x%x is mismatch!\n\r",idx);
            break;
    }

    if( !(pll3_value & SA6920_PLL_RESETB))
    {
        sys_write32(pll3_value, devcfg->shutdown_base + SHUTDOWN_PLL3_CONFIG0);
    }
}

/**********************************************
 *	@brief	  sa6920_clk_div_get
 *  @param	  ctrl:0: get; 1: set
 *	@return
 **********************************************/
uint8_t sa6920_clk_div_get(uint32_t idx)
{
    uint32_t val = 0;

    switch(idx)
    {
    case  SHUTDOWN_A55_SCLK:
    {
        val = sys_read32(devcfg->shutdown_base + SHUTDOWN_CPU_DIV_CONFIG);
        val = (val >> CPU_PLL_CLK_DIV) & CPU_PLL_CLK_DIV_MSAK;
        break;
    }
    case  SHUTDOWN_DDRC_CLK:
    {
        val = sys_read32(devcfg->shutdown_base + SHUTDOWN_DDR_DIV_CONFIG);
        val = (val >> DDR_PLL_CLK_DIV) & DDR_PLL_CLK_DIV_MSAK;
        break;
    }
    case  SHUTDOWN_BUS_ACLK:
    {
        val = sys_read32(devcfg->shutdown_base + SHUTDOWN_MAINBUS_DIV_CONFIG);
        val = (val >> BUS_PLL_CLK_DIV) & BUS_PLL_CLK_DIV_MSAK;
        break;
    }
    default:
        LOG_INF ("sa6920_clk_div_get idx=0x%x err!\n\r",idx);
        break;
    }
    return val;
}

static void sa6920_clk_filter_pll_switch(bool enable)
{
    uint32_t val;
    if(enable)
    {
        sys_write32(0x3, devcfg->filter_base + FILTER_BUS_CLK_DIV);
        sys_write32(0x4, devcfg->filter_base + FILTER_APB_PCLK_DIV);
        sys_write32(0x8, devcfg->filter_base + FILTER_BUS_CLK_DIV);

        val = sys_read32(devcfg->filter_base + FILTER_SEL_CONFIG);
        val |= PLL1_CTRL_CLK_SW;
        sys_write32(val, devcfg->filter_base + FILTER_SEL_CONFIG);
    }
    else
    {
        val = sys_read32(devcfg->filter_base + FILTER_SEL_CONFIG);
        val &= ~PLL1_CTRL_CLK_SW;
        sys_write32(val, devcfg->filter_base + FILTER_SEL_CONFIG);
        sys_write32(0x1, devcfg->filter_base + FILTER_BUS_CLK_DIV);
        sys_write32(0x1, devcfg->filter_base + FILTER_APB_PCLK_DIV);
    }

    return;
}

const struct Clock sa6920_composite_clock[] = {
    /* alwayson */

    COMPOSITE(ALWAYSON_I2C_MCLK,  ALWAYSON_CLKGT_CONFIG, 5),
    COMPOSITE(ALWAYSON_I2S0_MCLK,  ALWAYSON_CLKGT_CONFIG, 12),
    COMPOSITE_1(ALWAYSON_I2S_MCLKOUT,  ALWAYSON_CLKGT_CONFIG, 16, ALWAYSON_SEL_CONFIG, 2, 2),
    COMPOSITE(ALWAYSON_I2S_PCLK,  ALWAYSON_CLKGT_CONFIG, 23),

    /* filter */
    COMPOSITE_1(FILTER_SRAMC2_MCLK,  FILTER_CLKGT_CONFIG, 0, FILTER_BUS_CLK_DIV, 0, 4),
    COMPOSITE_1(FILTER_JPEG_ACLK,    FILTER_CLKGT_CONFIG, 1, FILTER_BUS_CLK_DIV, 0, 4),
    COMPOSITE_1(FILTER_ISPLITE_ACLK, FILTER_CLKGT_CONFIG, 2, FILTER_BUS_CLK_DIV, 0, 4),
    COMPOSITE_1(FILTER_VNNE_MCLK,    FILTER_CLKGT_CONFIG, 3, FILTER_VNNE_CONFIG, 0, 4),
    COMPOSITE_1(FILTER_VNNE_ACLK,    FILTER_CLKGT_CONFIG, 4, FILTER_BUS_CLK_DIV, 0, 4),
    COMPOSITE_DIV(FILTER_BUS_ACLK, FILTER_BUS_CLK_DIV, 0, 4),
    COMPOSITE_DIV2(FILTER_APB_PCLK,FILTER_BUS_CLK_DIV, 0, 4, FILTER_APB_PCLK_DIV, 0, 4),
    COMPOSITE_1(FILTER_VIN_ACLK,    FILTER_CLKGT_CONFIG, 5, FILTER_BUS_CLK_DIV, 0, 4),
    COMPOSITE_1(FILTER_VIN_MCLK,    FILTER_CLKGT_CONFIG, 6, FILTER_VIN_CONFIG,  0, 6),
    COMPOSITE_1(FILTER_PIXEL_MCLK,   FILTER_CLKGT_CONFIG, 7, FILTER_PIXEL_CONFIG, 0, 8),
    COMPOSITE(FILTER_VIN_SNR_MCLK,  FILTER_CLKGT_CONFIG, 8),
    COMPOSITE(FILTER_CNT_MCLK,       FILTER_CLKGT_CONFIG, 9),
    COMPOSITE_1(FILTER_JPEG_MCLK,    FILTER_CLKGT_CONFIG, 10, FILTER_VPU_CONFIG, 0, 8),
    COMPOSITE(FILTER_ADC_MAIN_CLK,   FILTER_CLKGT_CONFIG, 15),
    COMPOSITE(FILTER_VIN_DVP_PCLK,  FILTER_CLKGT_CONFIG, 16),
    COMPOSITE(FILTER_OTP_MCLK,       FILTER_CLKGT_CONFIG, 21),

    /* shutdown */
    COMPOSITE_1(SHUTDOWN_SPI0_MCLK, SHUTDOWN_CLKGT_CONFIG0, 4, SHUTDOWN_SPI0_DIV, 0, 7),
    COMPOSITE_1(SHUTDOWN_SPI1_MCLK, SHUTDOWN_CLKGT_CONFIG0, 5, SHUTDOWN_SPI_DIV, 0, 7),
    COMPOSITE_1(SHUTDOWN_SPI2_MCLK, SHUTDOWN_CLKGT_CONFIG0, 6, SHUTDOWN_SPI_DIV, 0, 7),
    COMPOSITE(SHUTDOWN_USB_HCLK, SHUTDOWN_CLKGT_CONFIG0, 7),
    COMPOSITE_1(SHUTDOWN_UART2_MCLK, SHUTDOWN_CLKGT_CONFIG0, 16, SHUTDOWN_SPI_DIV, 0, 7),
    COMPOSITE_1(SHUTDOWN_UART3_MCLK, SHUTDOWN_CLKGT_CONFIG0, 17, SHUTDOWN_SPI_DIV, 0, 7),
    COMPOSITE(SHUTDOWN_I2S1_SLV_CLK, SHUTDOWN_CLKGT_CONFIG0, 20),
    COMPOSITE_2(SHUTDOWN_PDM_MCLK, SHUTDOWN_CLKGT_CONFIG0, 22, 
        SHUTDOWN_I2S1_DIV, 14, 8, SHUTDOWN_PDM_DIV, 0, 8),
    COMPOSITE_2(SHUTDOWN_PDM_SCLK, SHUTDOWN_CLKGT_CONFIG0, 22, 
        SHUTDOWN_I2S1_DIV, 6, 8, SHUTDOWN_PDM_DIV, 0, 8),
    COMPOSITE_1(SHUTDOWN_I2S1_MCLK, SHUTDOWN_CLKGT_CONFIG0, 26, SHUTDOWN_PDM_DIV, 0, 8),

    COMPOSITE_1(SHUTDOWN_BUS_ACLK, SHUTDOWN_CLKGT_CONFIG3, 5, 
        SHUTDOWN_MAINBUS_DIV_CONFIG, 6, 4),
    COMPOSITE_1(SHUTDOWN_DDRC_CLK, SHUTDOWN_CLKGT_CONFIG2, 0, SHUTDOWN_DDR_DIV_CONFIG, 1, 4),
    COMPOSITE_1(SHUTDOWN_A55_SCLK, SHUTDOWN_CLKGT_CONFIG1, 0, SHUTDOWN_CPU_DIV_CONFIG, 2, 4),
    COMPOSITE_1(SHUTDOWN_SRAMC_MCLK, SHUTDOWN_CLKGT_CONFIG3, 1, SHUTDOWN_SRAMC_CONFIG, 0, 4),
    COMPOSITE(SHUTDOWN_DMAC_CORE_CLK, SHUTDOWN_CLKGT_CONFIG3, 2),
    COMPOSITE_1(SHUTDOWN_MSHC0_BCLK, SHUTDOWN_CLKGT_CONFIG4, 1, SHUTDOWN_MSHC_DIV, 0, 5 ),
    COMPOSITE_1(SHUTDOWN_MSHC1_BCLK, SHUTDOWN_CLKGT_CONFIG4, 5, SHUTDOWN_MSHC_DIV, 0, 5 ),
    COMPOSITE_1(SHUTDOWN_SFC_MCLK, SHUTDOWN_CLKGT_CONFIG4, 12, SHUTDOWN_I2S1_DIV, 24, 5 ),
    COMPOSITE(SHUTDOWN_WTD_MCLK, SHUTDOWN_CLKGT_CONFIG5, 0),
    COMPOSITE_1(SHUTDOWN_NPU_CLK, SHUTDOWN_VPU_NPU_CLKGT, 0, SHUTDOWN_NPU_DIV_CONFIG, 1, 4),
    COMPOSITE_1(SHUTDOWN_VENC_CLK, SHUTDOWN_VPU_NPU_CLKGT, 1, SHUTDOWN_VPU_DIV, 0, 6),
    COMPOSITE_1(SHUTDOWN_JDEC_CLK, SHUTDOWN_VPU_NPU_CLKGT, 2, SHUTDOWN_VPU_DIV, 6, 6),
    COMPOSITE_1(SHUTDOWN_VIN_MCLK, SHUTDOWN_CLKGT_CONFIG7, 0, SHUTDOWN_VIN_ISP_CONFIG, 0, 4),
    COMPOSITE(SHUTDOWN_VIN_SNR_MCLK, SHUTDOWN_CLKGT_CONFIG7, 3),
    COMPOSITE(SHUTDOWN_ISP_ACLK, SHUTDOWN_CLKGT_CONFIG7, 4),
    COMPOSITE_1(SHUTDOWN_VOUT1_PIXEL_CLK, SHUTDOWN_CLKGT_CONFIG7, 6, SHUTDOWN_VOUT_DIV, 0, 5),
    COMPOSITE(SHUTDOWN_VIN_CNT_CLK, SHUTDOWN_CLKGT_CONFIG7, 9),
    COMPOSITE(SHUTDOWN_FE_MCLK, SHUTDOWN_CLKGT_CONFIG7, 10),
    COMPOSITE(SHUTDOWN_BE_MCLK, SHUTDOWN_CLKGT_CONFIG7, 11),
    COMPOSITE(SHUTDOWN_PE_MCLK, SHUTDOWN_CLKGT_CONFIG7, 12),
};

static uint32_t sa6920_clk_get_filter_rate(uint32_t div1,uint32_t div2)
{
    uint32_t val;
    uint32_t clk;

    val = sys_read32(devcfg->filter_base + FILTER_SEL_CONFIG);
    if(val & PLL1_CTRL_CLK_SW)
        /* pll2 */
        clk = sa6920_clk_pll_recalc_rate(devcfg->filter_base + FILTER_PLL1_CONFIG0);
    else 
        clk = CLK_RATE_24M;
    
    return ((clk / div2) / div1);
}

static uint32_t sa6920_clk_get_filter_vin_rate(void)
{
    uint32_t val,div;
    uint32_t clk;

    val = sys_read32(devcfg->filter_base + FILTER_VIN_CONFIG);
    if((val >> FILTER_VIN_SNR_MCLK_MUX_SHIFT) & 0x1) {
        /* pll1 */
        clk = sa6920_clk_pll_recalc_rate(devcfg->filter_base + FILTER_PLL1_CONFIG0);
        div = (val >> FILTER_VIN_SNR_MCLK_PLL_DIV_SHIFT) 
            & CLK_DIV_MASK(FILTER_VIN_SNR_MCLK_PLL_DIV_WIDTH);
    }
    else {
        clk = CLK_RATE_24M;
        div = (val >> FILTER_VIN_SNR_MCLK_XTAL_DIV_SHIFT) 
            & CLK_DIV_MASK(FILTER_VIN_SNR_MCLK_XTAL_DIV_WIDTH);
    }
    return (clk / div);
}

static uint32_t sa6920_clk_get_shutdown_peri_rate(uint32_t div1,uint32_t div2)
{
    uint32_t val;
    uint32_t clk;

    val = sys_read32(devcfg->shutdown_base + SA6920_SHUTDOWN_SEL_CONFIG);
    if(val & PERI_CLK_SW_MASK)
        /* pll2 */
        clk = sa6920_clk_pll_recalc_rate(devcfg->shutdown_base + SHUTDOWN_PLL2_CONFIG0);
    else 
        clk = CLK_RATE_24M; 
    return ((clk / div2) / div1);
}

static uint32_t sa6920_clk_get_a55_rate(void)
{
    uint32_t val, div;
    uint32_t clk;

    val = sys_read32(devcfg->shutdown_base + SA6920_SHUTDOWN_SEL_CONFIG);

    if(!((val >> CPU_CLK_SW) & 0x1))
        return  CLK_RATE_24M;
    else if((val >> CPU_CLK_SW) & 0x2)
        clk =  sa6920_clk_pll_recalc_rate(devcfg->shutdown_base + SHUTDOWN_PLL2_CONFIG0);
    else if((val >> CPU_CLK_SW) & 0x4)
        clk = (sa6920_clk_pll_recalc_rate(devcfg->filter_base + FILTER_PLL1_CONFIG0));
    else
        clk = sa6920_clk_pll_recalc_rate(devcfg->shutdown_base + SHUTDOWN_PLL3_CONFIG0);
    
    div = sa6920_clk_div_get(SHUTDOWN_A55_SCLK);

    return clk / div;
}

static uint32_t sa6920_clk_get_ddrc_rate(void)
{
    uint32_t val, div;
    uint32_t clk;

    val = sys_read32(devcfg->shutdown_base + SA6920_SHUTDOWN_SEL_CONFIG);
    if(!((val >> DDR_CLK_SW) & 0x1))
        return  CLK_RATE_24M;
    else if((val >> DDR_CLK_SW) & 0x2)
        clk =  sa6920_clk_pll_recalc_rate(devcfg->shutdown_base + SHUTDOWN_PLL2_CONFIG0);
    else if((val >> DDR_CLK_SW) & 0x4)
        clk = (sa6920_clk_pll_recalc_rate(devcfg->filter_base + FILTER_PLL1_CONFIG0));
    else
        clk = sa6920_clk_pll_recalc_rate(devcfg->shutdown_base + SHUTDOWN_PLL3_CONFIG0);
    div = sa6920_clk_div_get(SHUTDOWN_DDRC_CLK); 
    return clk / div;
}

static uint32_t sa6920_clk_get_nic_rate(void)
{
    uint32_t val, div;
    uint32_t clk;

    val = sys_read32(devcfg->shutdown_base + SA6920_SHUTDOWN_SEL_CONFIG);
    if(!((val >> BUS_CLK_SW) & 0x1))
        return  CLK_RATE_24M;
    else if((val >> BUS_CLK_SW) & 0x2)
        clk =  sa6920_clk_pll_recalc_rate(devcfg->shutdown_base + SHUTDOWN_PLL2_CONFIG0);
    else if((val >> BUS_CLK_SW) & 0x4)
        clk = (sa6920_clk_pll_recalc_rate(devcfg->filter_base + FILTER_PLL1_CONFIG0));
    else
        clk = sa6920_clk_pll_recalc_rate(devcfg->shutdown_base + SHUTDOWN_PLL3_CONFIG0);
    div = sa6920_clk_div_get(SHUTDOWN_BUS_ACLK);
    return clk / div; 
}

static uint32_t sa6920_clk_get_npu_rate(uint32_t div)
{
    uint32_t val;
    uint32_t clk;

    val = sys_read32(devcfg->shutdown_base + SA6920_SHUTDOWN_SEL_CONFIG);
    if(!((val >> NPU_CLK_SW) & 0x1))
        return  CLK_RATE_24M;
    else if((val >> NPU_CLK_SW) & 0x2)
        clk =  sa6920_clk_pll_recalc_rate(devcfg->shutdown_base + SHUTDOWN_PLL2_CONFIG0);
    else if((val >> NPU_CLK_SW) & 0x4)
        clk = (sa6920_clk_pll_recalc_rate(devcfg->filter_base + FILTER_PLL1_CONFIG0));
    else
        clk = sa6920_clk_pll_recalc_rate(devcfg->shutdown_base + SHUTDOWN_PLL3_CONFIG0);
    return (clk / div);
}

static uint32_t sa6920_clk_get_vpu_rate(uint32_t div)
{
    uint32_t val;
    uint32_t clk;

    val = sys_read32(devcfg->shutdown_base + SA6920_SHUTDOWN_SEL_CONFIG);
    if(!((val >> VPU_CLK_SW) & 0x1))
        clk = CLK_RATE_24M;
    else if((val >> VPU_CLK_SW) & 0x2)
        clk =  sa6920_clk_pll_recalc_rate(devcfg->shutdown_base + SHUTDOWN_PLL2_CONFIG0);
    else if((val >> VPU_CLK_SW) & 0x4)
        clk = sa6920_clk_pll_recalc_rate(devcfg->filter_base + FILTER_PLL1_CONFIG0);
    else
        clk = sa6920_clk_pll_recalc_rate(devcfg->shutdown_base + SHUTDOWN_PLL3_CONFIG0);
    return (clk / div);
}

static uint32_t sa6920_clk_get_shutdown_vin_rate(uint16_t clockidx, uint32_t div)
{
    uint32_t val;
    uint32_t clk;

    if(clockidx == SHUTDOWN_VIN_SNR_MCLK)
    {
        val = sys_read32(devcfg->shutdown_base + SHUTDOWN_VIN_ISP_CONFIG);
        if((val >> SHUTDOWN_VIN_SNR_MCLK_MUX_SHIFT) & 0x1) {
            /* pll1 */
            clk = sa6920_clk_pll_recalc_rate(devcfg->filter_base + FILTER_PLL1_CONFIG0);
            div = (sys_read32(devcfg->shutdown_base + SHUTDOWN_VIN_ISP_CONFIG) >> SHUTDOWN_VIN_SNR_MCLK_PLL_DIV_SHIFT) 
                & CLK_DIV_MASK(SHUTDOWN_VIN_SNR_MCLK_PLL_DIV_WIDTH);
        }
        else {
            clk = CLK_RATE_24M;
            div = (sys_read32(devcfg->shutdown_base + SHUTDOWN_VIN_ISP_CONFIG) >> SHUTDOWN_VIN_SNR_MCLK_XTAL_DIV_SHIFT) 
                & CLK_DIV_MASK(SHUTDOWN_VIN_SNR_MCLK_XTAL_DIV_WIDTH);
        }
    }
    else
    {
        val = sys_read32(devcfg->shutdown_base + SA6920_SHUTDOWN_SEL_CONFIG);
        if((val >> ISP_CLK_SW) & 0x1)
            /* pll1 */
            clk = sa6920_clk_pll_recalc_rate(devcfg->filter_base + FILTER_PLL1_CONFIG0);
        else 
            clk = CLK_RATE_24M;
        if(clockidx == SHUTDOWN_DSI2_IPI_CLK)
            div = div / 4;
        if(clockidx == SHUTDOWN_DSI2_SYS_CLK)
            div = 4;
    }
    return (clk / div);
}

uint32_t sa6920_clk_get_isp_rate(void)
{
    uint32_t val, div;
    uint32_t clk;

    val = sys_read32(devcfg->shutdown_base + SHUTDOWN_ISP_BPCLK_CONFIG);
    val = val & 0x3;
    if (val  == 0)
        clk = sa6920_clk_pll_recalc_rate(devcfg->shutdown_base + SHUTDOWN_PLL3_CONFIG0);
    else if(val == 0x2)
        clk = (sa6920_clk_pll_recalc_rate(devcfg->filter_base + FILTER_PLL1_CONFIG0));
    else 
        clk =  sa6920_clk_pll_recalc_rate(devcfg->shutdown_base + SHUTDOWN_PLL2_CONFIG0);
    val = sys_read32(devcfg->shutdown_base + SHUTDOWN_ISP_BPCLK_CONFIG);
    div = ((val >> ISP_BPCLK_CYC_L_SHIFT) & CLK_DIV_MASK(ISP_BPCLK_CYC_L_WIDTH))
        + ((val >> ISP_BPCLK_CYC_H_SHIFT) & CLK_DIV_MASK(ISP_BPCLK_CYC_H_WIDTH)) + 2;
    return (clk / div);

}

uint32_t sa_get_star_clk(void)
{
    uint32_t val,div;

    val = sys_read32(devcfg->pmu_addr); 
    if(val & 0x1)
    {
        val = sys_read32(devcfg->filter_base + FILTER_SEL_CONFIG);
        /* source pll_clk = pll1_clk / div  */
        div = (val >> 1) & 0x7;
        return (sa6920_clk_pll_recalc_rate(devcfg->filter_base + FILTER_PLL1_CONFIG0) / div);
    }
    else
        return CLK_RATE_24M;
}

int32_t sa_set_star_clk(uint32_t rate)
{
    uint32_t val;
    if(rate == CLK_RATE_24M)
    {
        sys_write32(0, devcfg->pmu_addr);
    }
    if(rate == 594000000)
    {
        val = sys_read32(devcfg->filter_base + FILTER_SEL_CONFIG);
	val &= ~(0x0E);
        val |= (0x2 << 1);
        sys_write32(val, devcfg->filter_base + FILTER_SEL_CONFIG);
        sys_write32(1, devcfg->pmu_addr);
    }
    if(rate == 297000000)
    {
        val = sys_read32(devcfg->filter_base + FILTER_SEL_CONFIG);
	    val &= ~(0x0E);
        val |= (0x4 << 1);
        sys_write32(val, devcfg->filter_base + FILTER_SEL_CONFIG);
        sys_write32(1, devcfg->pmu_addr);
    }
    if(rate == 198000000)
    {
        val = sys_read32(devcfg->filter_base + FILTER_SEL_CONFIG);
	    val &= ~(0x0E);
        val |= (0x6 << 1);
        sys_write32(val, devcfg->filter_base + FILTER_SEL_CONFIG);
        sys_write32(1, devcfg->pmu_addr);
    }
    return 0;
}

static void sa6920_clk_round_div(uint32_t parent_clk,
            uint32_t rate,const struct Clock *comp_clock,uintptr_t base_addr)
{
    uint32_t val, div;

    div =  (uint8_t)CLK_CEIL_DIV(parent_clk, rate);
    CLK_LIMIT_VALUE(div, 1, CLK_DIV_MASK(comp_clock->div_width));

    val = sys_read32(base_addr + comp_clock->div_reg);
    val &= ~(CLK_DIV_MASK(comp_clock->div_width) << comp_clock->div_shift);
    val |= div << comp_clock->div_shift;

    sys_write32(val, base_addr + comp_clock->div_reg);
}


static int32_t sa6920_clk_set_filter_rate(uint32_t rate, const struct Clock *comp_clock)
{
    uint32_t val;
    uint32_t parent_clk;

    if(rate <= CLK_RATE_24M) {
        // select xtal_clk
        sa6920_pll_clk_sw(devcfg->filter_base + FILTER_SEL_CONFIG, PLL1_CTRL_CLK_SW, 0x0);
    }
    else {
        val = sys_read32(devcfg->filter_base + FILTER_SEL_CONFIG);
        if(val & PLL1_CTRL_CLK_SW)
            /* pll2 */
            parent_clk = sa6920_clk_pll_recalc_rate(devcfg->filter_base + FILTER_PLL1_CONFIG0);
        else 
            parent_clk = CLK_RATE_24M;

        if(parent_clk < rate)
            return 0;
        sa6920_clk_round_div(parent_clk, rate, comp_clock, devcfg->filter_base);
    }
    return 0;
}

static int32_t sa6920_clk_set_filter_vin_rate(uint32_t rate)
{
    uint32_t val,div;
    uint32_t parent_clk;

    val = sys_read32(devcfg->filter_base + FILTER_VIN_CONFIG);
    if(rate <= CLK_RATE_24M) {//select parent xtal24M
        val &= (~(1 << FILTER_VIN_SNR_MCLK_MUX_SHIFT));
        parent_clk = CLK_RATE_24M;
        div =  (uint8_t)CLK_CEIL_DIV(parent_clk, rate);
        CLK_LIMIT_VALUE(div, 1, CLK_DIV_MASK(FILTER_VIN_SNR_MCLK_XTAL_DIV_WIDTH));
        val &= ~(CLK_DIV_MASK(FILTER_VIN_SNR_MCLK_XTAL_DIV_WIDTH) << FILTER_VIN_SNR_MCLK_XTAL_DIV_SHIFT);
        val |= div << FILTER_VIN_SNR_MCLK_XTAL_DIV_SHIFT;
    }
    else {//select parent pll1
        val |= (1 << FILTER_VIN_SNR_MCLK_MUX_SHIFT);
        parent_clk = sa6920_clk_pll_recalc_rate(FILTER_PLL1_CONFIG0);
        div =  (uint8_t)CLK_CEIL_DIV(parent_clk, rate);
        CLK_LIMIT_VALUE(div, 1, CLK_DIV_MASK(FILTER_VIN_SNR_MCLK_PLL_DIV_WIDTH));
        val &= ~(CLK_DIV_MASK(FILTER_VIN_SNR_MCLK_PLL_DIV_WIDTH) << FILTER_VIN_SNR_MCLK_PLL_DIV_SHIFT);
        val |= div << FILTER_VIN_SNR_MCLK_PLL_DIV_SHIFT;
    }
    sys_write32(val, devcfg->filter_base + FILTER_VIN_CONFIG);
    return 0;
}

static int32_t sa6920_clk_set_shutdown_peri_rate(uint32_t rate, const struct Clock *comp_clock)
{
    uint32_t val;
    uint32_t parent_clk;
 
    val = sys_read32(devcfg->shutdown_base + SA6920_SHUTDOWN_SEL_CONFIG);
    if(val & PERI_CLK_SW_MASK)
        /* pll2 */
        parent_clk = sa6920_clk_pll_recalc_rate(devcfg->shutdown_base + SHUTDOWN_PLL2_CONFIG0);
    else 
        parent_clk = CLK_RATE_24M;

    if(parent_clk < rate)
        return 0;
    sa6920_clk_round_div(parent_clk, rate, comp_clock,devcfg->shutdown_base);
    
    return 0;
}


static int32_t sa6920_clk_set_isp_rate(uint32_t rate)
{
    uint32_t val, div, div_h, div_l;
    uint32_t parent_clk;

    val = sys_read32(devcfg->shutdown_base + SHUTDOWN_ISP_BPCLK_CONFIG);
    val = val & 0x3;
    if (val  == 0)
        parent_clk = sa6920_clk_pll_recalc_rate(devcfg->shutdown_base + SHUTDOWN_PLL3_CONFIG0);
    else if(val == 0x2)
        parent_clk = (sa6920_clk_pll_recalc_rate(devcfg->filter_base + FILTER_PLL1_CONFIG0));
    else 
        parent_clk =  sa6920_clk_pll_recalc_rate(devcfg->shutdown_base + SHUTDOWN_PLL2_CONFIG0);
    
    //div =  (uint8_t)CLK_ROUND_DIV(parent_clk, rate);
    //CLK_LIMIT_VALUE(div, 1, CLK_DIV_MASK(ISP_BPCLK_CYC_H_WIDTH));
    div =  (uint8_t)CLK_CEIL_DIV(parent_clk, rate);

    div_h = 0;
    div_l = div - 2;    
    val = sys_read32(devcfg->shutdown_base + SHUTDOWN_ISP_BPCLK_CONFIG);
    val &= ~(CLK_DIV_MASK(ISP_BPCLK_CYC_L_WIDTH) << ISP_BPCLK_CYC_L_SHIFT);
    val |= div_l << ISP_BPCLK_CYC_L_SHIFT;
    val &= ~(CLK_DIV_MASK(ISP_BPCLK_CYC_H_WIDTH) << ISP_BPCLK_CYC_H_SHIFT);
    val |= div_h << ISP_BPCLK_CYC_H_SHIFT;

    sys_write32(val, devcfg->shutdown_base + SHUTDOWN_ISP_BPCLK_CONFIG);
    return 0;

}

static int32_t sa6920_clk_set_pdm_rate(uint32_t rate, const struct Clock *comp_clock)
{
    uint32_t parent_clk;
    uint32_t div1 = 1,i;

    for(i = 0; i < ARRAY_SIZE(sa6920_composite_clock); i++) {
        if(SHUTDOWN_I2S1_MCLK == sa6920_composite_clock[i].clock_id){
			break;
        }
	}
    div1 = sys_read32(devcfg->shutdown_base + sa6920_composite_clock[i].div_reg);
        div1 = (div1 >> sa6920_composite_clock[i].div_shift) & 
            CLK_DIV_MASK(sa6920_composite_clock[i].div_width);

    parent_clk = sa6920_clk_get_shutdown_peri_rate(div1, 1);
    if(parent_clk < rate)
        return 0;

    sa6920_clk_round_div(parent_clk, rate, comp_clock,devcfg->shutdown_base);

    return 0;
}

static int32_t sa6920_clk_set_sys_rate(uint32_t rate, 
       const struct Clock *comp_clock, uint8_t clk_sw)
{
    uint32_t val;
    uint32_t parent_clk;

    if(rate == CLK_RATE_24M) {
        // select xtal_clk
        sa6920_pll_clk_sw(devcfg->shutdown_base + SA6920_SHUTDOWN_SEL_CONFIG, clk_sw, 0x0);
    }
    else {
        val = sys_read32(devcfg->shutdown_base + SA6920_SHUTDOWN_SEL_CONFIG);
        if(!((val >> clk_sw) & 0x1))
            parent_clk = CLK_RATE_24M;
        else if((val >> clk_sw) & 0x2)
            parent_clk =  sa6920_clk_pll_recalc_rate(devcfg->shutdown_base + SHUTDOWN_PLL2_CONFIG0);
        else if((val >> clk_sw) & 0x4)
            parent_clk = (sa6920_clk_pll_recalc_rate(devcfg->filter_base + FILTER_PLL1_CONFIG0));
        else
            parent_clk = sa6920_clk_pll_recalc_rate(devcfg->shutdown_base + SHUTDOWN_PLL3_CONFIG0);
        if(parent_clk < rate)
            return 0;
        sa6920_clk_round_div(parent_clk, rate, comp_clock, devcfg->shutdown_base);
    }
    return 0;
}

static int32_t sa6920_clk_set_shutdown_vin_rate(uint16_t clockidx, uint32_t rate,
                const struct Clock *comp_clock)
{
    uint32_t val,div;
    uint32_t parent_clk;

    if(clockidx == SHUTDOWN_VIN_SNR_MCLK) {
        val = sys_read32(devcfg->shutdown_base + SHUTDOWN_VIN_ISP_CONFIG);
        if(rate <= CLK_RATE_24M) {//select parent xtal24M
            val &= (~(1 << SHUTDOWN_VIN_SNR_MCLK_MUX_SHIFT));
            parent_clk = CLK_RATE_24M;
            div =  (uint8_t)CLK_CEIL_DIV(parent_clk, rate);
            CLK_LIMIT_VALUE(div, 1, CLK_DIV_MASK(SHUTDOWN_VIN_SNR_MCLK_XTAL_DIV_WIDTH));
            val &= ~(CLK_DIV_MASK(SHUTDOWN_VIN_SNR_MCLK_XTAL_DIV_WIDTH) << SHUTDOWN_VIN_SNR_MCLK_XTAL_DIV_SHIFT);
            val |= div << SHUTDOWN_VIN_SNR_MCLK_XTAL_DIV_SHIFT;
        }
        else { //select parent pll1
            val |= (1 << SHUTDOWN_VIN_SNR_MCLK_MUX_SHIFT);
            parent_clk = sa6920_clk_pll_recalc_rate(devcfg->shutdown_base + FILTER_PLL1_CONFIG0);
            div =  (uint8_t)CLK_CEIL_DIV(parent_clk, rate);
            CLK_LIMIT_VALUE(div, 1, CLK_DIV_MASK(SHUTDOWN_VIN_SNR_MCLK_PLL_DIV_WIDTH));
            val &= ~(CLK_DIV_MASK(SHUTDOWN_VIN_SNR_MCLK_PLL_DIV_WIDTH) << SHUTDOWN_VIN_SNR_MCLK_PLL_DIV_SHIFT);
            val |= div << SHUTDOWN_VIN_SNR_MCLK_PLL_DIV_SHIFT;
        }
        sys_write32(val, devcfg->shutdown_base + SHUTDOWN_VIN_ISP_CONFIG);
    }
    else {
        if(rate <= CLK_RATE_24M) {//select parent xtal24M
            parent_clk = CLK_RATE_24M;
            if(clockidx == SHUTDOWN_DSI2_IPI_CLK)
                rate = rate / 4;
            sa6920_clk_round_div(parent_clk, rate, comp_clock, devcfg->shutdown_base);
        }
        else {//select parent pll1
            parent_clk = sa6920_clk_pll_recalc_rate(devcfg->shutdown_base + FILTER_PLL1_CONFIG0);
            if(clockidx == SHUTDOWN_DSI2_IPI_CLK)
                rate = rate / 4;
            sa6920_clk_round_div(parent_clk, rate, comp_clock, devcfg->shutdown_base);
        }
    }
    return 0;
}


// void hal_module_crg_reset(module_reset_reg rst_idx)
// {
//     unsigned int bit = rst_idx&0xff;
// 	uint32_t reg = (rst_idx&0xFFFF0000) +((rst_idx>>8)&0xff);
	
// 	/* Deassert PORESET */
// 	_IOWBF32(reg,bit,1,0x01);
	
// 	rt_hw_us_delay(1);
	
// 	_IOWBF32(reg,bit,1,0x00);
// }

// void hal_module_crg_reset_assert(module_reset_reg rst_idx)
// {
// 	unsigned int bit = rst_idx&0xff;
// 	uint32_t reg = (rst_idx&0xFFFF0000) +((rst_idx>>8)&0xff);

// 	/* assert PORESET */
// 	_IOWBF32(reg,bit,1,0x01);
// }
// void hal_module_crg_reset_deassert(module_reset_reg rst_idx)
// {
// 	unsigned int bit = rst_idx&0xff;
// 	uint32_t reg = (rst_idx&0xFFFF0000) +((rst_idx>>8)&0xff);
	
// 	/* Deassert PORESET */
// 	_IOWBF32(reg,bit,1,0x00);

// }


static int sa6920_clock_control_on(const struct device *dev,
				      clock_control_subsys_t sys)
{
	ARG_UNUSED(dev);

    uint16_t clockidx =  *(uint16_t *)sys;
    uint32_t i, val;
    uintptr_t base_addr;

    for(i = 0; i < ARRAY_SIZE(sa6920_composite_clock); i++) {
        if(clockidx == sa6920_composite_clock[i].clock_id){
			break;
        }
	}
    if (i == ARRAY_SIZE(sa6920_composite_clock)) {
		LOG_ERR("Invalid rate : %d for gate enable clk\n",clockidx);
		return -1;
    }

    if(clockidx < FILTER_ADC_MAIN_CLK)
        base_addr = devcfg->always_base;
    else if (clockidx < SHUTDOWN_SFC_MCLK)
        base_addr = devcfg->filter_base;
    else
        base_addr = devcfg->shutdown_base;

    val = sys_read32(base_addr + sa6920_composite_clock[i].gate_reg);
    val |= 1 << sa6920_composite_clock[i].gate_shift;

    sys_write32(val, base_addr + sa6920_composite_clock[i].gate_reg);
	
    return 0;
}

static int sa6920_clock_control_off(const struct device *dev,
				       clock_control_subsys_t sys)
{
	ARG_UNUSED(dev);

    uint16_t clockidx =  *(uint16_t *)sys;
    uint32_t i, val;
    uintptr_t base_addr;

    for(i = 0;i < ARRAY_SIZE(sa6920_composite_clock);i++) {
        if(clockidx == sa6920_composite_clock[i].clock_id){
			break;
        }
	}
    if (i == ARRAY_SIZE(sa6920_composite_clock)) {
		LOG_ERR(" Invalid rate : %d for gate disable clk\n",clockidx);
		return -1;
    }

    if(clockidx < FILTER_ADC_MAIN_CLK)
        base_addr = devcfg->always_base;
    else if (clockidx < SHUTDOWN_SFC_MCLK)
        base_addr = devcfg->filter_base;
    else
        base_addr = devcfg->shutdown_base;

    val = sys_read32(base_addr + sa6920_composite_clock[i].gate_reg);
    val &= ~(1 << sa6920_composite_clock[i].gate_shift);

    sys_write32(val, base_addr + sa6920_composite_clock[i].gate_reg);

	return 0;
}


static int sa6920_clk_get_rate(uint32_t clockidx)
{
	
    uint32_t div1 = 1,div2 = 1;
    uint32_t i;
    uintptr_t base_addr;

    if(clockidx == MODULE_CLK_PLL1 || clockidx == MODULE_CLK_PLL2 || clockidx == MODULE_CLK_PLL3)
    {
        return sa6920_clk_get_pll_rate(clockidx);
    }
    if(clockidx == ALWAYSON_STAR_CLK)
    {
        return sa_get_star_clk();
    }
    /* find clockid in composite table  */
    for(i = 0; i < ARRAY_SIZE(sa6920_composite_clock); i++) {
        if(clockidx == sa6920_composite_clock[i].clock_id){
			break;
        }
	}
    if (i == ARRAY_SIZE(sa6920_composite_clock)) {
		LOG_ERR("warning : clockidx %d for get clock\n",clockidx);
        return 0;
    }

    if(clockidx < FILTER_ADC_MAIN_CLK)
        base_addr = devcfg->always_base;
    else if (clockidx < SHUTDOWN_SFC_MCLK)
        base_addr = devcfg->filter_base;
    else
        base_addr = devcfg->shutdown_base;

    /* get clock div val */
    if(sa6920_composite_clock[i].div_flags != 0) {
        div1 = sys_read32(base_addr + sa6920_composite_clock[i].div_reg);
        div1 = (div1 >> sa6920_composite_clock[i].div_shift) & 
            CLK_DIV_MASK(sa6920_composite_clock[i].div_width);
        if(sa6920_composite_clock[i].div_flags == 2) {
            div2 = sys_read32(base_addr + sa6920_composite_clock[i].div_reg2);
            div2 = (div2 >> sa6920_composite_clock[i].div_shift2) & 
            CLK_DIV_MASK(sa6920_composite_clock[i].div_width2);
        }
    }
    else {
        div1 = 1;
    }
  //  LOG_ERR("clockidx %d div1=%d div2=%d\n",clockidx,div1,div2);
    switch(clockidx) 
    {  
    case  ALWAYSON_I2S_MCLKOUT:
    {
        return (CLK_RATE_24M / div1);
        break;
    }
    case  FILTER_SRAMC2_MCLK:
    case  FILTER_JPEG_ACLK:
    case  FILTER_ISPLITE_ACLK:
    case  FILTER_VNNE_MCLK:
    case  FILTER_VNNE_ACLK:
    case  FILTER_VIN_ACLK:
    case  FILTER_VIN_MCLK:
    case  FILTER_PIXEL_MCLK:
    case  FILTER_JPEG_MCLK:
    case  FILTER_BUS_ACLK:
    case  FILTER_APB_PCLK:
    {
        return sa6920_clk_get_filter_rate(div1, div2);
        break;
    }
    case  FILTER_VIN_SNR_MCLK:
    {
        return sa6920_clk_get_filter_vin_rate();
        break;
    }
    case  SHUTDOWN_A55_SCLK:
    {
        return sa6920_clk_get_a55_rate();
        break;
    }
    case  SHUTDOWN_DDRC_CLK:
    {
        return sa6920_clk_get_ddrc_rate();
        break;
    }
    case  SHUTDOWN_BUS_ACLK://bus_aclk
    {
       return sa6920_clk_get_nic_rate();
        break;
    }
    case  SHUTDOWN_DMAC_CORE_CLK:
    case  SHUTDOWN_MSHC0_BCLK:
    case  SHUTDOWN_MSHC1_BCLK:
    case  SHUTDOWN_SFC_MCLK:
    case  SHUTDOWN_USB_HCLK:
    case  SHUTDOWN_I2S1_MCLK:
    case  SHUTDOWN_SPI0_MCLK:
    case  SHUTDOWN_SPI1_MCLK:
    case  SHUTDOWN_SPI2_MCLK:
    case  SHUTDOWN_UART2_MCLK:
    case  SHUTDOWN_UART3_MCLK:
    case  SHUTDOWN_PDM_MCLK:
    case  SHUTDOWN_PDM_SCLK:
    case  SHUTDOWN_SRAMC_MCLK:
    {
        return sa6920_clk_get_shutdown_peri_rate(div1, div2);
        break;
    }
    case SHUTDOWN_NPU_CLK:
    {
        return sa6920_clk_get_npu_rate(div1);
        break;
    }

    case SHUTDOWN_JDEC_CLK:
    case SHUTDOWN_VENC_CLK:
    {
        return sa6920_clk_get_vpu_rate(div1);
        break;
    }

    case SHUTDOWN_VIN_MCLK:
    case SHUTDOWN_DSI2_SYS_CLK:
    case SHUTDOWN_VOUT1_PIXEL_CLK:
    case SHUTDOWN_DSI2_IPI_CLK:
    case SHUTDOWN_VIN_SNR_MCLK:
    {
       return sa6920_clk_get_shutdown_vin_rate(clockidx, div1);
        break;
    }

    case SHUTDOWN_ISP_ACLK:
    {
        return sa6920_clk_get_isp_rate();
        break;
    }
    case SHUTDOWN_I2S1_SLV_CLK:
    {
        return CLK_RATE_24M;
        break;
    }

    default:
        LOG_ERR("sa6920_clk_get_rate idx=%d err!\n\r",clockidx);
        return 0;
    }

	return 0;
}
static int sa6920_clock_control_get_rate(const struct device *dev,
					    clock_control_subsys_t sys,
					    uint32_t *rate)
{
	ARG_UNUSED(dev);

    uint16_t clockidx =  *(uint16_t *)sys;

    uint32_t div1 = 1,div2 = 1;
    uint32_t i;
    uintptr_t base_addr;

    if(clockidx == MODULE_CLK_PLL1 || clockidx == MODULE_CLK_PLL2 || clockidx == MODULE_CLK_PLL3)
    {
        return sa6920_clk_get_pll_rate(clockidx);
    }
    if(clockidx == ALWAYSON_STAR_CLK)
    {
        return sa_get_star_clk();
    }
    /* find clockid in composite table  */
    for(i = 0; i < ARRAY_SIZE(sa6920_composite_clock); i++) {
        if(clockidx == sa6920_composite_clock[i].clock_id){
			break;
        }
	}
    if (i == ARRAY_SIZE(sa6920_composite_clock)) {
		LOG_ERR("warning : clockidx %d for get clock\n",clockidx);
        return 0;
    }

    if(clockidx < FILTER_ADC_MAIN_CLK)
        base_addr = devcfg->always_base;
    else if (clockidx < SHUTDOWN_SFC_MCLK)
        base_addr = devcfg->filter_base;
    else
        base_addr = devcfg->shutdown_base;

    /* get clock div val */
    if(sa6920_composite_clock[i].div_flags != 0) {
        div1 = sys_read32(base_addr + sa6920_composite_clock[i].div_reg);
        div1 = (div1 >> sa6920_composite_clock[i].div_shift) & 
            CLK_DIV_MASK(sa6920_composite_clock[i].div_width);
        if(sa6920_composite_clock[i].div_flags == 2) {
            div2 = sys_read32(base_addr + sa6920_composite_clock[i].div_reg2);
            div2 = (div2 >> sa6920_composite_clock[i].div_shift2) & 
            CLK_DIV_MASK(sa6920_composite_clock[i].div_width2);
        }
    }
    else {
        div1 = 1;
    }
  //  LOG_ERR("clockidx %d div1=%d div2=%d\n",clockidx,div1,div2);
    switch(clockidx) 
    {  
    case  ALWAYSON_I2S_MCLKOUT:
    {
        *rate = (CLK_RATE_24M / div1);
        break;
    }
    case  FILTER_SRAMC2_MCLK:
    case  FILTER_JPEG_ACLK:
    case  FILTER_ISPLITE_ACLK:
    case  FILTER_VNNE_MCLK:
    case  FILTER_VNNE_ACLK:
    case  FILTER_VIN_ACLK:
    case  FILTER_VIN_MCLK:
    case  FILTER_PIXEL_MCLK:
    case  FILTER_JPEG_MCLK:
    case  FILTER_BUS_ACLK:
    case  FILTER_APB_PCLK:
    {
        *rate = sa6920_clk_get_filter_rate(div1, div2);
        break;
    }
    case  FILTER_VIN_SNR_MCLK:
    {
        *rate = sa6920_clk_get_filter_vin_rate();
        break;
    }
    case  SHUTDOWN_A55_SCLK:
    {
        *rate = sa6920_clk_get_a55_rate();
        break;
    }
    case  SHUTDOWN_DDRC_CLK:
    {
        *rate = sa6920_clk_get_ddrc_rate();
        break;
    }
    case  SHUTDOWN_BUS_ACLK://bus_aclk
    {
        *rate = sa6920_clk_get_nic_rate();
        break;
    }
    case  SHUTDOWN_DMAC_CORE_CLK:
    case  SHUTDOWN_MSHC0_BCLK:
    case  SHUTDOWN_MSHC1_BCLK:
    case  SHUTDOWN_SFC_MCLK:
    case  SHUTDOWN_USB_HCLK:
    case  SHUTDOWN_I2S1_MCLK:
    case  SHUTDOWN_SPI0_MCLK:
    case  SHUTDOWN_SPI1_MCLK:
    case  SHUTDOWN_SPI2_MCLK:
    case  SHUTDOWN_UART2_MCLK:
    case  SHUTDOWN_UART3_MCLK:
    case  SHUTDOWN_PDM_MCLK:
    case  SHUTDOWN_PDM_SCLK:
    case  SHUTDOWN_SRAMC_MCLK:
    {
        *rate = sa6920_clk_get_shutdown_peri_rate(div1, div2);
        break;
    }
    case SHUTDOWN_NPU_CLK:
    {
        *rate = sa6920_clk_get_npu_rate(div1);
        break;
    }

    case SHUTDOWN_JDEC_CLK:
    case SHUTDOWN_VENC_CLK:
    {
        *rate = sa6920_clk_get_vpu_rate(div1);
        break;
    }

    case SHUTDOWN_VIN_MCLK:
    case SHUTDOWN_DSI2_SYS_CLK:
    case SHUTDOWN_VOUT1_PIXEL_CLK:
    case SHUTDOWN_DSI2_IPI_CLK:
    case SHUTDOWN_VIN_SNR_MCLK:
    {
        *rate = sa6920_clk_get_shutdown_vin_rate(clockidx, div1);
        break;
    }

    case SHUTDOWN_ISP_ACLK:
    {
        *rate = sa6920_clk_get_isp_rate();
        break;
    }
    case SHUTDOWN_I2S1_SLV_CLK:
    {
        *rate = CLK_RATE_24M;
        break;
    }

    default:
        LOG_ERR("sa6920_clk_get_rate idx=%d err!\n\r",clockidx);
        return 0;
    }

	return 0;
}

static int sa6920_clock_control_set_rate(const struct device *dev,
					    clock_control_subsys_t sys,
					    clock_control_subsys_rate_t clock_rate)
{
	ARG_UNUSED(dev);

    uint32_t rate = (uintptr_t)clock_rate;

    uint16_t clockidx =  *(uint16_t *)sys;
    uint32_t i;
 
    if(clockidx == ALWAYSON_STAR_CLK)
    {
        sa_set_star_clk(rate);
        return 0;
    }
    /* find clockid in composite table  */
    for(i = 0; i < ARRAY_SIZE(sa6920_composite_clock); i++) {
        if(clockidx == sa6920_composite_clock[i].clock_id){
			break;
        }
	}
    if (i == ARRAY_SIZE(sa6920_composite_clock)) {
		LOG_ERR("warning : clockidx %d for set clock\n",clockidx);
        return 0;
    }
    switch(clockidx) 
    { 
    case  ALWAYSON_I2S_MCLKOUT:
    {
        sa6920_clk_round_div(CLK_RATE_24M, rate, &sa6920_composite_clock[i],devcfg->always_base);
        break;
    }
    case  FILTER_SRAMC2_MCLK:
    case  FILTER_JPEG_ACLK:
    case  FILTER_ISPLITE_ACLK:
    case  FILTER_VNNE_MCLK:
    case  FILTER_VNNE_ACLK:
    case  FILTER_VIN_ACLK:
    case  FILTER_VIN_MCLK:
    case  FILTER_PIXEL_MCLK:
    case  FILTER_JPEG_MCLK:
    {
        sa6920_clk_set_filter_rate(rate, &sa6920_composite_clock[i]);
        break;
    } 
    case FILTER_VIN_SNR_MCLK:
    {
        sa6920_clk_set_filter_vin_rate(rate);
        break;
    } 
    case  SHUTDOWN_A55_SCLK:
    {
        sa6920_clk_set_sys_rate(rate, &sa6920_composite_clock[i], CPU_CLK_SW);
        break;
    }
    case  SHUTDOWN_DDRC_CLK:
    {
        sa6920_clk_set_sys_rate(rate, &sa6920_composite_clock[i], DDR_CLK_SW);
        break;
    }
    case  SHUTDOWN_BUS_ACLK:
    {
        sa6920_clk_set_sys_rate(rate, &sa6920_composite_clock[i], BUS_CLK_SW);
        break;
    }
    case  SHUTDOWN_NPU_CLK:
    {
        sa6920_clk_set_sys_rate(rate, &sa6920_composite_clock[i], NPU_CLK_SW);
        break;
    }
    case  SHUTDOWN_JDEC_CLK:
    case  SHUTDOWN_VENC_CLK:
    {
        sa6920_clk_set_sys_rate(rate, &sa6920_composite_clock[i], VPU_CLK_SW);
        break;
    }
    case  SHUTDOWN_MSHC0_BCLK:
    case  SHUTDOWN_MSHC1_BCLK:
    case  SHUTDOWN_SFC_MCLK:
    case  SHUTDOWN_USB_HCLK:
    case  SHUTDOWN_I2S1_MCLK:
    case  SHUTDOWN_SPI0_MCLK:
    case  SHUTDOWN_SPI1_MCLK:
    case  SHUTDOWN_SPI2_MCLK:
    case  SHUTDOWN_UART2_MCLK:
    case  SHUTDOWN_UART3_MCLK:
    {
        sa6920_clk_set_shutdown_peri_rate(rate, &sa6920_composite_clock[i]);
        break;
    }

    case  SHUTDOWN_PDM_MCLK:
    case  SHUTDOWN_PDM_SCLK:
    {
        sa6920_clk_set_pdm_rate(rate, &sa6920_composite_clock[i]);
        break;
    }
    case SHUTDOWN_VIN_MCLK:
    case SHUTDOWN_VOUT1_PIXEL_CLK:
    case SHUTDOWN_DSI2_IPI_CLK:
    case SHUTDOWN_VIN_SNR_MCLK:
    {
        sa6920_clk_set_shutdown_vin_rate(clockidx, rate, &sa6920_composite_clock[i]);
        break;
    }
    case SHUTDOWN_ISP_ACLK:
    {
        sa6920_clk_set_isp_rate(rate);
        break;
    }

    default:
        LOG_ERR("sa6920_clk_set_rate idx=%d is mismatch!\n\r",clockidx);
        return 0;
    }
    return 0;

}

static int sa6920_clock_control_init(const struct device *dev)
{
    devcfg = dev->config;
    
    sa6920_clk_filter_pll_switch(1);

    return 0;
}

static enum clock_control_status
sa6920_clock_control_get_status(const struct device *dev,
				   clock_control_subsys_t sys)
{
	ARG_UNUSED(dev);

    uint16_t clockidx =  *(uint16_t *)sys;

    uint32_t i, val;
    uintptr_t base_addr;

    for(i = 0; i < ARRAY_SIZE(sa6920_composite_clock); i++) {
        if(clockidx == sa6920_composite_clock[i].clock_id){
			break;
        }
	}
    if (i == ARRAY_SIZE(sa6920_composite_clock)) {
		LOG_ERR(" Invalid rate : %d for gate enable clk\n",clockidx);
		return -1;
    }

    if(clockidx < FILTER_ADC_MAIN_CLK)
        base_addr = devcfg->always_base;
    else if (clockidx < SHUTDOWN_SFC_MCLK)
        base_addr = devcfg->filter_base;
    else
        base_addr = devcfg->shutdown_base;

    val = sys_read32(base_addr + sa6920_composite_clock[i].gate_reg);
    
    if ((val >> sa6920_composite_clock[i].gate_shift) & 0x1)
        return CLOCK_CONTROL_STATUS_ON;
    else
        return CLOCK_CONTROL_STATUS_OFF;
}


static int dump_clk(const struct shell *sh, size_t argc, char **argv)
{
	ARG_UNUSED(sh);

    LOG_INF("clk:%d:PLL1-%u\n", MODULE_CLK_PLL1, sa6920_clk_get_rate(MODULE_CLK_PLL1));
    LOG_INF("clk:%d:PLL2-%u\n", MODULE_CLK_PLL2, sa6920_clk_get_rate(MODULE_CLK_PLL2));
    LOG_INF("clk:%d:PLL3-%u\n", MODULE_CLK_PLL3, sa6920_clk_get_rate(MODULE_CLK_PLL3));

    LOG_INF("clk:%d:STAR-%u\n", ALWAYSON_STAR_CLK, sa6920_clk_get_rate(ALWAYSON_STAR_CLK));
    LOG_INF("clk:%d:A55-%u\n", SHUTDOWN_A55_SCLK, sa6920_clk_get_rate(SHUTDOWN_A55_SCLK));
    LOG_INF("clk:%d:ALWAYSON_I2S_MCLKOUT-%u\n", ALWAYSON_I2S_MCLKOUT, sa6920_clk_get_rate(ALWAYSON_I2S_MCLKOUT));
    LOG_INF("clk:%d:FILTER_SRAMC2_MCLK-%u\n", FILTER_SRAMC2_MCLK, sa6920_clk_get_rate(FILTER_SRAMC2_MCLK));
    LOG_INF("clk:%d:FILTER_BUS_ACLK-%u\n", FILTER_BUS_ACLK, sa6920_clk_get_rate(FILTER_BUS_ACLK));
    LOG_INF("clk:%d:FILTER_VNNE_MCLK-%u\n", FILTER_VNNE_MCLK, sa6920_clk_get_rate(FILTER_VNNE_MCLK));
    LOG_INF("clk:%d:FILTER_VIN_MCLK-%u\n", FILTER_VIN_MCLK, sa6920_clk_get_rate(FILTER_VIN_MCLK));
    LOG_INF("clk:%d:FILTER_PIXEL_MCLK-%u\n", FILTER_PIXEL_MCLK, sa6920_clk_get_rate(FILTER_PIXEL_MCLK));
    LOG_INF("clk:%d:FILTER_JPEG_MCLK-%u\n", FILTER_JPEG_MCLK, sa6920_clk_get_rate(FILTER_JPEG_MCLK));
    LOG_INF("clk:%d:FILTER_VIN_SNR_MCLK-%u\n", FILTER_VIN_SNR_MCLK, sa6920_clk_get_rate(FILTER_VIN_SNR_MCLK));
    
    LOG_INF("clk:%d:SHUTDOWN_DMAC_CORE_CLK-%u\n", SHUTDOWN_DMAC_CORE_CLK, sa6920_clk_get_rate(SHUTDOWN_DMAC_CORE_CLK));
    LOG_INF("clk:%d:SHUTDOWN_DDRC_CLK-%u\n", SHUTDOWN_DDRC_CLK, sa6920_clk_get_rate(SHUTDOWN_DDRC_CLK));
    LOG_INF("clk:%d:SHUTDOWN_BUS_ACLK-%u\n", SHUTDOWN_BUS_ACLK, sa6920_clk_get_rate(SHUTDOWN_BUS_ACLK));
    LOG_INF("clk:%d:SHUTDOWN_MSHC0_BCLK-%u\n", SHUTDOWN_MSHC0_BCLK, sa6920_clk_get_rate(SHUTDOWN_MSHC0_BCLK));
    LOG_INF("clk:%d:SHUTDOWN_MSHC1_BCLK-%u\n", SHUTDOWN_MSHC1_BCLK, sa6920_clk_get_rate(SHUTDOWN_MSHC1_BCLK));
    LOG_INF("clk:%d:SHUTDOWN_SFC_MCLK-%u\n", SHUTDOWN_SFC_MCLK, sa6920_clk_get_rate(SHUTDOWN_SFC_MCLK));
    LOG_INF("clk:%d:SHUTDOWN_USB_HCLK-%u\n", SHUTDOWN_USB_HCLK, sa6920_clk_get_rate(SHUTDOWN_USB_HCLK));
    LOG_INF("clk:%d:SHUTDOWN_I2S1_MCLK-%u\n", SHUTDOWN_I2S1_MCLK, sa6920_clk_get_rate(SHUTDOWN_I2S1_MCLK));
    LOG_INF("clk:%d:SHUTDOWN_SPI0_MCLK-%u\n", SHUTDOWN_SPI0_MCLK, sa6920_clk_get_rate(SHUTDOWN_SPI0_MCLK));
    LOG_INF("clk:%d:SHUTDOWN_PDM_MCLK-%u\n", SHUTDOWN_PDM_MCLK, sa6920_clk_get_rate(SHUTDOWN_PDM_MCLK));
    LOG_INF("clk:%d:SHUTDOWN_PDM_SCLK-%u\n", SHUTDOWN_PDM_SCLK, sa6920_clk_get_rate(SHUTDOWN_PDM_SCLK));
    LOG_INF("clk:%d:SHUTDOWN_SRAMC_MCLK-%u\n", SHUTDOWN_SRAMC_MCLK, sa6920_clk_get_rate(SHUTDOWN_SRAMC_MCLK));
    LOG_INF("clk:%d:SHUTDOWN_NPU_CLK-%u\n", SHUTDOWN_NPU_CLK, sa6920_clk_get_rate(SHUTDOWN_NPU_CLK));
    LOG_INF("clk:%d:SHUTDOWN_JDEC_CLK-%u\n", SHUTDOWN_JDEC_CLK, sa6920_clk_get_rate(SHUTDOWN_JDEC_CLK));
    LOG_INF("clk:%d:SHUTDOWN_VENC_CLK-%u\n", SHUTDOWN_VENC_CLK, sa6920_clk_get_rate(SHUTDOWN_VENC_CLK));
    LOG_INF("clk:%d:SHUTDOWN_VIN_MCLK-%u\n", SHUTDOWN_VIN_MCLK, sa6920_clk_get_rate(SHUTDOWN_VIN_MCLK));
    LOG_INF("clk:%d:SHUTDOWN_VIN_SNR_MCLK-%u\n", SHUTDOWN_VIN_SNR_MCLK, sa6920_clk_get_rate(SHUTDOWN_VIN_SNR_MCLK));
    LOG_INF("clk:%d:SHUTDOWN_ISP_ACLK-%u\n",SHUTDOWN_ISP_ACLK, sa6920_clk_get_rate(SHUTDOWN_ISP_ACLK));
    LOG_INF("clk:%d:SHUTDOWN_I2S1_SLV_CLK-%u\n", SHUTDOWN_I2S1_SLV_CLK, sa6920_clk_get_rate(SHUTDOWN_I2S1_SLV_CLK));

	return 0;
}

SHELL_CMD_REGISTER(dumpclk, NULL, "dump_clk", dump_clk);


static DEVICE_API(clock_control, sa6920_clock_control_api) = {
	.on = sa6920_clock_control_on,
	.off = sa6920_clock_control_off,
	.get_rate = sa6920_clock_control_get_rate,
    .set_rate = sa6920_clock_control_set_rate,
	.get_status = sa6920_clock_control_get_status,
};

const struct sa6920_crg_config sa6920_crg_config_t = {
	.always_base = DT_INST_REG_ADDR_BY_IDX(0, 0),
	.filter_base = DT_INST_REG_ADDR_BY_IDX(0, 1),
	.shutdown_base = DT_INST_REG_ADDR_BY_IDX(0, 2),
    .pmu_addr = DT_INST_REG_ADDR_BY_IDX(0, 3),
};

DEVICE_DT_INST_DEFINE(0,
		    sa6920_clock_control_init,
		    NULL,
		    NULL, &sa6920_crg_config_t,
		    PRE_KERNEL_1,
		    CONFIG_CLOCK_CONTROL_INIT_PRIORITY,
		    &sa6920_clock_control_api);