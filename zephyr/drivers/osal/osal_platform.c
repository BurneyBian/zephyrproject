#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>

#include "osal.h"

void osal_platform_set_drvdata(struct osal_platform_device *dev, void *data)
{
	dev->data = data;
}
RTM_EXPORT(osal_platform_set_drvdata);

void *osal_platform_get_drvdata(const struct osal_platform_device *dev)
{
	return dev->data;
}
RTM_EXPORT(osal_platform_get_drvdata);

void osal_dev_set_drvdata(struct osal_platform_device *dev, void *data)
{
	if (dev) {
		osal_platform_set_drvdata(dev, data);
	}
}
RTM_EXPORT(osal_dev_set_drvdata);

void *osal_dev_get_drvdata(const struct osal_platform_device *dev)
{
	if (dev) {
		return osal_platform_get_drvdata(dev);
	}
	return RT_NULL;
}
RTM_EXPORT(osal_dev_get_drvdata);


osal_device osal_get_device(struct osal_platform_device *dev)
{
	struct rt_platform_device *plat_dev;
	plat_dev = (struct rt_platform_device *)dev->private_data;

	return (osal_device)plat_dev;
}
RTM_EXPORT(osal_get_device);

static rt_err_t osal_platform_probe(struct rt_platform_device *plat_dev)
{
	struct osal_platform_driver *drv;
	struct osal_platform_device *dev;
	struct rt_platform_driver *plat_drv;
	struct osal_resource *osal_rc;
	struct rt_device *rt_dev = &plat_dev->parent;
	plat_drv = rt_container_of(rt_dev->drv, struct rt_platform_driver, parent);

	dev = (struct osal_platform_device *)rt_malloc(sizeof(struct osal_platform_device));
	osal_rc = (struct osal_resource *)rt_malloc(sizeof(struct osal_resource));
	drv = (struct osal_platform_driver *)plat_drv->data;

	if (dev && drv)
	{
		dev->private_data = plat_dev;
		dev->rc = osal_rc;
		plat_dev->priv = dev;
		return drv->probe(dev);
	}
}

static rt_err_t osal_platform_remove(struct rt_platform_device *plat_dev)
{
	int ret = -1;
	struct osal_platform_driver *drv;
	struct osal_platform_device *dev;
	struct rt_platform_driver *plat_drv;
	struct rt_device *rt_dev = &plat_dev->parent;

	plat_drv = rt_container_of(rt_dev->drv, struct rt_platform_driver, parent);
	drv = (struct osal_platform_driver *)plat_drv->data;
	dev = (struct osal_platform_device *)plat_dev->priv;

	if (dev && drv)
	{
		ret = drv->remove(dev);
		rt_free(dev->rc);
		rt_free(dev);
	}

	return ret;
}

static rt_err_t osal_platform_shutdown(struct rt_platform_device *plat_dev)
{
	struct osal_platform_driver *drv;
	struct osal_platform_device *dev;
	struct rt_platform_driver *plat_drv;
	struct rt_device *rt_dev = &plat_dev->parent;

	plat_drv = rt_container_of(rt_dev->drv, struct rt_platform_driver, parent);
	drv = (struct osal_platform_driver *)plat_drv->data;
	dev = (struct osal_platform_device *)plat_dev->priv;

	if (dev && drv)
	{
		drv->shutdown(dev);
	}

	return 0;
}

int osal_platform_driver_register_new(struct osal_platform_driver *drv)
{
	struct rt_platform_driver *plat_drv;
	struct rt_ofw_node_id *ids = rt_malloc(sizeof(struct rt_ofw_node_id));

	plat_drv = rt_malloc(sizeof(struct rt_platform_driver));

	drv->data = plat_drv;
	plat_drv->data = drv;

	plat_drv->name = drv->name;
	plat_drv->probe = osal_platform_probe;
	plat_drv->remove = osal_platform_remove;
	plat_drv->shutdown = osal_platform_shutdown;
#ifdef RT_USING_OFW
    plat_drv->ids = (const struct rt_ofw_node_id *)drv->osal_of_match_table;
#endif

	rt_platform_driver_register(plat_drv);

	return 0;
}
RTM_EXPORT(osal_platform_driver_register_new);

void osal_platform_driver_unregister_new(struct osal_platform_driver *drv)
{
	rt_free(drv->data);
}
RTM_EXPORT(osal_platform_driver_unregister_new);

struct osal_resource *osal_platform_get_resource_byname(struct osal_platform_device *dev, unsigned int type,
                                        const char *name)
{
	struct rt_platform_device *plat_dev = (struct rt_platform_device *)dev->private_data;
	rt_uint64_t out_address, out_size;
		
    if ((type == OSAL_IORESOURCE_MEM)||(type == OSAL_IORESOURCE_REG))
	{
		rt_dm_dev_get_address_by_name(&plat_dev->parent, name, &out_address, &out_size);
		dev->rc->start = out_address;
		dev->rc->end = out_address + out_size - 1;
	}

	return dev->rc;
}
RTM_EXPORT(osal_platform_get_resource_byname);

struct osal_resource *osal_platform_get_resource_new(struct osal_platform_device *dev, unsigned int type,
                                 unsigned int num)
{
	struct rt_platform_device *plat_dev = (struct rt_platform_device *)dev->private_data;
	rt_uint64_t out_address, out_size;
		
    if ((type == OSAL_IORESOURCE_MEM)||(type == OSAL_IORESOURCE_REG))
	{
		rt_dm_dev_get_address(&plat_dev->parent, num, &out_address, &out_size);
		dev->rc->start = out_address;
		dev->rc->end = out_address + out_size - 1;
	}

	return dev->rc;
}
RTM_EXPORT(osal_platform_get_resource_new);

int osal_platform_get_irq_new(struct osal_platform_device *dev, unsigned int num)
{
	struct rt_platform_device *pdev = (struct rt_platform_device *)dev->private_data;

	return rt_dm_dev_get_irq(&pdev->parent, num);
}
RTM_EXPORT(osal_platform_get_irq_new);

int osal_platform_get_irq_byname_new(struct osal_platform_device *dev, const char *name)
{
	struct rt_platform_device *pdev = (struct rt_platform_device *)dev->private_data;

	return rt_dm_dev_get_irq_by_name(&pdev->parent, name);
}
RTM_EXPORT(osal_platform_get_irq_byname_new);

void *osal_platform_ioremap_resource_new(struct osal_platform_device *dev, struct osal_resource *osal_resource)
{
	return (void *)osal_resource->start;
}
RTM_EXPORT(osal_platform_ioremap_resource_new);

//------------------------ pin & pinctrl ----------------------------//
#ifdef RT_USING_PINCTRL
osal_pinctrl osal_pinctrl_get(struct osal_platform_device *dev)
{
	struct rt_platform_device *plat_dev = (struct rt_platform_device *)dev->private_data;

	return (osal_pinctrl)(&plat_dev->parent);
}
	
osal_pinctrl_state osal_pinctrl_lookup_state(osal_pinctrl p, const char *name)
{
	unsigned int ret;

	ret = rt_pin_ctrl_confs_lookup((struct rt_device *)p, name);//实际是index
	ret |= 0xff000000;

	return (osal_pinctrl_state)ret;
}

int osal_pinctrl_select_state(osal_pinctrl p, osal_pinctrl_state ps)
{
	unsigned int ret = (unsigned int)ps;
	ret &= 0x00ffffff;
	return rt_pin_ctrl_confs_apply((struct rt_device *)p, (int)ret);
}
#else
osal_pinctrl osal_pinctrl_get(struct osal_platform_device *dev)
{
	return (osal_pinctrl)(0xa5a5a5a5);
}
	
osal_pinctrl_state osal_pinctrl_lookup_state(osal_pinctrl p, const char *name)
{
	return (osal_pinctrl_state)rt_pinctrl_info_pinctrl_find(name);
}

int osal_pinctrl_select_state(osal_pinctrl p, osal_pinctrl_state ps)
{
	return rt_pinctrl_info_config_by_info((struct pinctrl_info *)ps);
}
#endif
RTM_EXPORT(osal_pinctrl_get);
RTM_EXPORT(osal_pinctrl_lookup_state);
RTM_EXPORT(osal_pinctrl_select_state);
//--------------------------of-----------------------------------//
#ifdef RT_USING_OFW
#include <drivers/ofw.h>

osal_device_node osal_of_get_node(struct osal_platform_device* drv)
{
    struct rt_platform_device *plat_dev = (struct rt_platform_device *)drv->private_data;

    if (plat_dev)
        return (osal_device_node)plat_dev->parent.ofw_node;
    else
		return RT_NULL;
}
RTM_EXPORT(osal_of_get_node);

osal_device_node osal_of_find_node_by_name(osal_device_node from,
        const char *name)
{
    return (osal_device_node)rt_ofw_find_node_by_name((struct rt_ofw_node *)from, name);
}

RTM_EXPORT(osal_of_find_node_by_name);

void osal_of_node_put(osal_device_node node)
{
    rt_ofw_node_put((struct rt_ofw_node*)node);
}
RTM_EXPORT(osal_of_node_put);

int osal_of_property_read_u32(osal_device_node node, const char *propname, unsigned int *out_value)
{
   return rt_ofw_prop_read_u32((struct rt_ofw_node*)node, propname, (rt_uint32_t *)out_value);
}
RTM_EXPORT(osal_of_property_read_u32);

int osal_of_property_read_u32_array(osal_device_node node,const char *propname, unsigned int *out_values, size_t sz)
{
	int ret;
	ret = rt_ofw_prop_read_u32_array_index((struct rt_ofw_node*)node, propname, 0, sz, (rt_uint32_t *)out_values);

	if (ret >= 0)
		return 0;
	else
		return ret;
}
RTM_EXPORT(osal_of_property_read_u32_array);

osal_device_node osal_of_find_compatible_node(osal_device_node from, const char *type, const char *compat)
{
	//struct rt_ofw_node *np = rt_ofw_find_node_by_type((struct rt_ofw_node *)from, type);
	if (from)
		return (osal_device_node)rt_ofw_find_node_by_compatible((struct rt_ofw_node*)from, compat);
	else
		return RT_NULL;
}
RTM_EXPORT(osal_of_find_compatible_node);

int osal_of_device_is_compatible(const osal_device_node from, const char *name)
{
	return rt_ofw_node_is_compatible((struct rt_ofw_node*)from, name);
}
RTM_EXPORT(osal_of_device_is_compatible);

int osal_of_irq_get(osal_device_node dev, int index)
{
#ifdef RT_USING_PIC
    return rt_ofw_get_irq((struct rt_ofw_node*)dev, index);
#else
	osal_printk("WARNING: %s - Not support!\n", __FUNCTION__);
	return 0;
#endif
}
RTM_EXPORT(osal_of_irq_get);

osal_clk osal_of_clk_get(osal_device_node np, int index)
{
	osal_printk("WARNING: %s - Not support!\n", __FUNCTION__);
    return RT_NULL;
}
RTM_EXPORT(osal_of_clk_get);

#endif

unsigned long osal_get_pid(void)
{
#ifdef RT_USING_SMART
	rt_thread_t thd = rt_thread_self();

	return thd->tid;
#else
	osal_printk("WARNING: %s - Not support!\n", __FUNCTION__);
	return 0;
#endif
}
RTM_EXPORT(osal_get_pid);

unsigned long osal_get_tgid(void)
{
	return osal_get_pid();
}
RTM_EXPORT(osal_get_tgid);

void osal_cpumask_clear(struct osal_cpumask* dstp)
{
	dstp->bits = 0;
}
RTM_EXPORT(osal_cpumask_clear);

void osal_cpumask_set_cpu(unsigned int cpu, struct osal_cpumask *dstp)
{
	dstp->bits |= (1 << cpu);
}
RTM_EXPORT(osal_cpumask_set_cpu);

rt_weak rt_err_t rt_hw_interrupt_set_target_cpus(unsigned int irq, unsigned long cpu_mask)
{
	osal_printk("WARNING: %s - Not support!\n", __FUNCTION__);
	return -ENOTSUP;
}

int  osal_irq_set_affinity_hint(unsigned int irq, const struct osal_cpumask *m)
{
	return rt_hw_interrupt_set_target_cpus(irq, m->bits);
}
RTM_EXPORT(osal_irq_set_affinity_hint);


void *osal_devm_platform_ioremap_resource(struct osal_platform_device *dev, unsigned int index)
{
	struct rt_platform_device *plat_dev = (struct rt_platform_device *)dev->private_data;
	rt_uint64_t out_address, out_size;
		
	rt_dm_dev_get_address(&plat_dev->parent, index, &out_address, &out_size);

	return (void *)out_address;
}

int osal_devm_request_irq(struct osal_platform_device *dev, unsigned int irq, osal_irq_handler_t handler, unsigned long irqflags, const char *devname, void *dev_id)
{
	return osal_request_irq(irq, handler, RT_NULL, devname, dev_id);
}

void osal_devm_free_irq(struct osal_platform_device *dev, unsigned int irq, void *dev_id)
{
	osal_free_irq(irq, dev_id);
}

RTM_EXPORT(osal_devm_platform_ioremap_resource);
RTM_EXPORT(osal_devm_request_irq);
RTM_EXPORT(osal_devm_free_irq);
