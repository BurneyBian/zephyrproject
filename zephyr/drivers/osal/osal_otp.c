#include <rtthread.h>
#include <rthw.h>

#include "osal.h"

#define OTP_RD_SYSTEM_KEY_BLOCK2 2

//#define OTP_DEBUG
#ifdef OTP_DEBUG
static int chip_info_show(chip_info_t *info)
{
    osal_printk("****************%s****************\n", "chip info");
    osal_printk("chip no: %04x\n", *(uint16_t*)(info->no));
    osal_printk("chip package: %c\n", info->package);
    osal_printk("chip date: %d-%d-%d %d:%d:%d\n", info->date.year + 2000, info->date.month, info->date.day,
                                            info->date.hour, info->date.minute, info->date.second);
    osal_printk("chip lot id: %02x%02x%02x%02x%02x%02x%02x%02x\n", info->lot_id[0], info->lot_id[1], info->lot_id[2], info->lot_id[3],
                                info->lot_id[4], info->lot_id[5], info->lot_id[6], info->lot_id[7]);
    osal_printk("chip wafer: %d\n", info->wafer);
    osal_printk("chip wafer x: %d\n", info->wafer_x);
    osal_printk("chip wafer y: %d\n", info->wafer_y);
    osal_printk("CP Good die flag: %d\n", info->flag.cp_good_die_flag);
    osal_printk("FT Good die flag: %d\n", info->flag.ft_good_die_flag);

    return 0;
}
#endif

#ifdef SA6920_SOC
#define BIT_32(nr)			((1) << (nr))
#define OTP_BASE              (0x48840000UL)

#define OTP_RESET             (0x3F)
#define OTP_WORK_MODE_REG     (0x0)
#define OTP_OPSTART_ACTIVITY  (0x69205551)
#define OTP_OP_START_REG      (0x8)
#define OTP_OP_FINISH_BIT     BIT_32(0)
#define OTP_RD_FINISH         (OTP_OP_FINISH_BIT)
#define OTP_KEY_BLK_OFFSET    (0x4)
#define OTP_KEY_BLK_BASE_REG  (0x10)
#define OTP_MAX_COUNT         (1000000)
#define OTP_BLK_REG_NUMS      (9)
#define OTP_OP_STA_REG        (0xC)
#define OTP_BUSY              BIT_32(1)
#define OTP_OPSTART_STANBY    (0x6920AAA1)
#define OTP_OP_MODE_BIT       BIT_32(5)

static rt_bool_t otp_flag_check(unsigned int flag)
{
	unsigned int val;
	rt_bool_t state = RT_FALSE;
	unsigned int count = OTP_MAX_COUNT;

	while(count--){
		val = osal_readl(OTP_BASE + OTP_OP_STA_REG) & (flag | OTP_BUSY); 
		if(val == flag){
            state = RT_TRUE;
            break;
        }
		rt_hw_us_delay(1);
	}

    return state;
}

static rt_bool_t otp_activty_to_standby(void)
{
	unsigned int val;
	rt_bool_t state = RT_TRUE;
	unsigned int count = OTP_MAX_COUNT;

	val = osal_readl(OTP_BASE + OTP_OP_STA_REG) & OTP_OP_MODE_BIT;
	if (val !=  OTP_OP_MODE_BIT) {    
		val = osal_readl(OTP_BASE + OTP_OP_STA_REG) & OTP_BUSY;
		while (count--) {
			if (val != OTP_BUSY) {
				osal_writel(OTP_RESET, OTP_BASE + OTP_WORK_MODE_REG);
				osal_writel(OTP_OPSTART_STANBY, OTP_BASE + OTP_OP_START_REG);
				if (otp_flag_check(OTP_RD_FINISH)) {
					state = RT_TRUE;
				} else {
					osal_printk_err("otp stanby fail\n");
					state = RT_FALSE; 
				}
				break;
			}
			rt_hw_us_delay(1);
		}

		if (count < 1) {
			state = RT_FALSE;
		}
	}

   return state;
}

static rt_bool_t otp_blk_read(unsigned char blk, unsigned int *buf)
{
	unsigned int i;
	unsigned int val;
	unsigned int reg;

	if (otp_activty_to_standby() == RT_FALSE){
		osal_printk_err("otp blk = %d stanby read fail!\n",blk);
		return RT_FALSE;
	}

	osal_writel(OTP_RESET, OTP_BASE + OTP_WORK_MODE_REG);
	osal_writel(OTP_OPSTART_ACTIVITY, OTP_BASE + OTP_OP_START_REG);

	if (!otp_flag_check(OTP_OP_FINISH_BIT))
	{
      osal_printk_err("otp finish fail\n");
	}

	osal_writel(blk, OTP_BASE + OTP_WORK_MODE_REG);
	osal_writel(OTP_OPSTART_ACTIVITY, OTP_BASE + OTP_OP_START_REG);

	if (otp_flag_check(OTP_RD_FINISH)) {
		for (i = 0; i < OTP_BLK_REG_NUMS; i++) {
			reg = (i * OTP_KEY_BLK_OFFSET) + OTP_KEY_BLK_BASE_REG;
			val = osal_readl(OTP_BASE + reg);
			*buf++ = val;
		}
	} else {
		osal_printk_err("otp read blk=%d fail\n",blk);
		return RT_FALSE;
	}

	return RT_TRUE;
}
#else
static rt_bool_t otp_blk_read(uint8_t blk,uint32_t *buf)
{
	return RT_FALSE;
}
#endif

int osal_get_soc_info(chip_info_t *info)
{
	uint32_t buf[9]  = {0};

	if (RT_TRUE == otp_blk_read(2, buf)) {
		rt_memcpy(info, buf, sizeof(chip_info_t));
#ifdef OTP_DEBUG
		chip_info_show(info);
#endif
	}

	return 0;
}
RTM_EXPORT(osal_get_soc_info);
