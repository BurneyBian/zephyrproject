#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>

#include "osal.h"

#include <drivers/ofw.h>

struct reset_control {
	//unsigned long base;
	unsigned int offset;
	unsigned char bit;
};

static struct reset_control *ofw_get_reset_no_lock(struct rt_ofw_node *np, int index, const char *name)
{
	//struct rt_ofw_node *rst_np = RT_NULL;
    struct reset_control *rstc = RT_NULL;
    struct rt_ofw_cell_args reset_args;

    if (!rt_ofw_parse_phandle_cells(np, "resets", "#reset-cells", index, &reset_args))
    {
        int count;
        struct rt_ofw_node *reset_ofw_np = reset_args.data;

        count = reset_args.args_count;
		//因为6920的reset设备全部定义在了clock设备上，又#reset-cells定义为2，故无法区分每个设备的起始地址
		//rst_np = (struct rt_ofw_node *)reset_args->data;

		if (count == 2)
			rstc = rt_malloc(sizeof(struct reset_control));//内存泄露？

        if (rstc)
        {
            rstc->offset = reset_args.args[0];
			rstc->bit = reset_args.args[1];
        }
    }

    return rstc;
}

osal_reset_control osal_devm_reset_control_get_optional_exclusive(struct osal_platform_device *dev, const char *id)
{
	struct rt_platform_device *pdev = (struct rt_platform_device *)dev->private_data;
	struct rt_ofw_node *np = pdev->parent.ofw_node;
	const char *dev_name = np->name;
	int index = 0;

    if (np)
    {
		if (id) 
			index = rt_ofw_prop_index_of_string(np, "reset-names", id);

		if (index >= 0)
		{
			return (osal_reset_control)ofw_get_reset_no_lock(np, index, id);
		}	
    }

	return RT_NULL;
}
RTM_EXPORT(osal_devm_reset_control_get_optional_exclusive);

int osal_reset_control_deassert(osal_reset_control rstc)
{
	unsigned int val;
	unsigned long base;// = rstc->base;
	unsigned int offset = ((struct reset_control *)rstc)->offset;
	unsigned char bit = ((struct reset_control *)rstc)->bit;
	//osal_printk("%s:[%d %d]\n", __func__, offset, bit);
	if (offset == 0xC)
		base = (unsigned long)0x48070000;
	else if (offset == 0x20)
		base = (unsigned long)0x48890000;
	else
		base = (unsigned long)0x50000000;

	val = HWREG32(base + offset);
	val |= (1U << bit);
	HWREG32(base + offset) = val;

	return 0;
}
RTM_EXPORT(osal_reset_control_deassert);

int osal_reset_control_reset(osal_reset_control rstc)
{
	unsigned int val;
	unsigned long base;// = rstc->base;
	unsigned int offset = ((struct reset_control *)rstc)->offset;
	unsigned char bit = ((struct reset_control *)rstc)->bit;
	//osal_printk("%s:[%d %d]\n", __func__, offset, bit);
	if (offset == 0xC)
		base = (unsigned long)0x48070000;
	else if (offset == 0x20)
		base = (unsigned long)0x48890000;
	else
		base = (unsigned long)0x50000000;

	val = HWREG32(base + offset);
	val |= (1U << bit);
	HWREG32(base + offset) = val;

	rt_thread_mdelay(1);

	val = HWREG32(base + offset);
	val &= ~(1U << bit);
	HWREG32(base + offset) = val;

	return 0;
}
RTM_EXPORT(osal_reset_control_reset);
