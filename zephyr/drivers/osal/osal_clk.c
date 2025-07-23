#include "osal.h"

osal_clk osal_clk_get(struct osal_platform_device *dev, const char *name)
{
	osal_printk("%s Warning:Not support\n", __FUNCTION__);
	return 0;
}

int osal_clk_prepare_enable(osal_clk clk)
{
	osal_printk("%s Warning:Not support\n", __FUNCTION__);
	return 0;
}

void osal_clk_disable_unprepare(osal_clk clk)
{
	osal_printk("%s Warning:Not support\n", __FUNCTION__);
	return 0;
}

int osal_clk_set_rate(osal_clk clk,unsigned long rate)
{
	osal_printk("%s Warning:Not support\n", __FUNCTION__);
	return 0;
}

unsigned long osal_clk_get_rate(osal_clk clk)
{
	osal_printk("%s Warning:Not support\n", __FUNCTION__);
	return 0;
}

int osal_clk_set_parent(osal_clk clk, osal_clk parent)
{
	osal_printk("%s Warning:Not support\n", __FUNCTION__);
	return 0;
}


int osal_soc_clk_set_isp_rate(unsigned long rate)
{
	osal_printk("%s Warning:Not support\n", __FUNCTION__);
	return 0;
}

unsigned long osal_soc_clk_get_isp_rate(void)
{
	osal_printk("%s Warning:Not support\n", __FUNCTION__);
	return 1;
}
