#include "osal.h"

__attribute__((weak)) unsigned long osal_mmz_alloc(char *name, unsigned int size, unsigned int align)
{
	osal_printk("%s Not support!\n", __FUNCTION__);
	return 0;
}
__attribute__((weak)) void osal_mmz_free(unsigned long paddr)
{
	osal_printk("%s Not support!\n", __FUNCTION__);
}
__attribute__((weak)) void *osal_mmz_remap(unsigned long paddr, unsigned int size, unsigned char cached)
{
	osal_printk("%s Not support!\n", __FUNCTION__);
	return NULL;
}
__attribute__((weak)) void osal_mmz_unmap(void *vaddr)
{
	osal_printk("%s Not support!\n", __FUNCTION__);
}
__attribute__((weak)) int osal_print_get_level(void)
{
	osal_printk("%s Not support!\n", __FUNCTION__);
	return 0;
}
__attribute__((weak)) int osal_print_get_module_state(int idx)
{
	osal_printk("%s Not support!\n", __FUNCTION__);
	return 1;
}
__attribute__((weak)) int osal_print_out(const char *fmt, ...)
{
	osal_printk("%s Not support!\n", __FUNCTION__);
	return 0;
}
__attribute__((weak)) int osal_printk_get_level(void)
{
	return 5;
}
__attribute__((weak)) osal_proc_entry_t *osal_create_proc_entry(const char *name, osal_proc_entry_t *parent)
{
	osal_printk("%s Not support!\n", __FUNCTION__);
	return NULL;
}
__attribute__((weak)) osal_proc_entry_t *osal_proc_mkdir(const char *name, osal_proc_entry_t *parent)
{
	osal_printk("%s Not support!\n", __FUNCTION__);
	return NULL;
}
__attribute__((weak)) void osal_remove_proc_entry(const char *name, osal_proc_entry_t *parent)
{
	osal_printk("%s Not support!\n", __FUNCTION__);
}
__attribute__((weak)) int osal_seq_printf(osal_proc_entry_t *entry, const char *fmt, ...)
{
	osal_printk("%s Not support!\n", __FUNCTION__);
	return 0;
}
__attribute__((weak)) void osal_workq_init(osal_workqueue_t *wq)
{
	osal_printk("%s Not support!\n", __FUNCTION__);
}
__attribute__((weak)) int osal_workq_schedule(osal_workqueue_t *wq)
{
	osal_printk("%s Not support!\n", __FUNCTION__);
	return 0;
}
__attribute__((weak)) int osal_workq_cancel(osal_workqueue_t *wq)
{
	osal_printk("%s Not support!\n", __FUNCTION__);
	return 0;
}
__attribute__((weak)) int osal_platform_driver_register_new(struct osal_platform_driver *drv){return 0;}
__attribute__((weak)) void osal_platform_driver_unregister_new(struct osal_platform_driver *drv){}
__attribute__((weak)) struct osal_resource *osal_platform_get_resource_byname_new(struct osal_platform_device *dev, unsigned int type,
                                        const char *name){return NULL;}
__attribute__((weak)) struct osal_resource *osal_platform_get_resource_new(struct osal_platform_device *dev, unsigned int type,
                                 unsigned int num){return NULL;}
__attribute__((weak)) int osal_platform_get_irq_new(struct osal_platform_device *dev, unsigned int num){return 0;}
__attribute__((weak)) int osal_platform_get_irq_byname_new(struct osal_platform_device *dev, const char *name){return 0;}
__attribute__((weak)) void *osal_platform_ioremap_resource_new(struct osal_platform_device *dev, struct osal_resource *osal_resource){return NULL;}
__attribute__((weak)) void osal_platform_set_drvdata(struct osal_platform_device *dev, void *data){}
__attribute__((weak)) void *osal_platform_get_drvdata(const struct osal_platform_device *dev){return NULL;}

__attribute__((weak)) osal_pinctrl osal_pinctrl_get(struct osal_platform_device *dev){return 0;}
__attribute__((weak)) osal_pinctrl_state osal_pinctrl_lookup_state(osal_pinctrl p, const char *name){return 0;}
__attribute__((weak)) int osal_pinctrl_select_state(osal_pinctrl p, osal_pinctrl_state ps){return 0;}
__attribute__((weak)) osal_clk osal_clk_get(struct osal_platform_device *dev, const char *name){return 0;}
__attribute__((weak)) int osal_clk_prepare_enable(osal_clk clk){return 0;}
__attribute__((weak)) void osal_clk_disable_unprepare(osal_clk clk){}
__attribute__((weak)) int osal_clk_set_rate(osal_clk clk, unsigned long rate){return 0;}
__attribute__((weak)) unsigned long osal_clk_get_rate(osal_clk clk){return 0;}
__attribute__((weak)) int osal_clk_set_parent(osal_clk clk, osal_clk parent){return 0;}
__attribute__((weak)) void osal_dev_set_drvdata(struct osal_platform_device *dev, void *data){}
__attribute__((weak)) void *osal_dev_get_drvdata(const struct osal_platform_device *dev){return NULL;}
__attribute__((weak)) osal_device osal_get_device(struct osal_platform_device *dev){return NULL;}
__attribute__((weak)) osal_device_node osal_of_find_node_by_name(osal_device_node from,const char *name){return NULL;}
__attribute__((weak)) osal_device_node osal_of_get_node(struct osal_platform_device* drv){return NULL;}
__attribute__((weak)) osal_device_node osal_of_find_compatible_node(osal_device_node from, const char *type, const char *compat){return NULL;}
__attribute__((weak)) void osal_of_node_put(osal_device_node node){}
__attribute__((weak)) int osal_of_irq_get(osal_device_node dev, int index){return 0;}
__attribute__((weak)) osal_clk osal_of_clk_get(osal_device_node np, int index){return 0;}
__attribute__((weak)) int osal_of_property_read_u32_array(osal_device_node node,const char *propname,unsigned int *out_values, size_t sz){return 0;}
__attribute__((weak)) int osal_of_property_read_u32(osal_device_node node,const char *propname,unsigned int *out_value){return 0;}
__attribute__((weak)) osal_reset_control osal_devm_reset_control_get_optional_exclusive(struct osal_platform_device *dev, const char *id){return NULL;}
__attribute__((weak)) int osal_reset_control_deassert(osal_reset_control rstc){return 0;}
__attribute__((weak)) int osal_reset_control_reset(osal_reset_control rstc){return 0;}
__attribute__((weak)) unsigned long osal_get_pid(void){return 0;}
__attribute__((weak)) unsigned long osal_get_tgid(void){return osal_get_pid();}
__attribute__((weak)) void osal_cpumask_clear(struct osal_cpumask* dstp){}
__attribute__((weak)) void osal_cpumask_set_cpu(unsigned int cpu, struct osal_cpumask *dstp){}
__attribute__((weak)) int osal_irq_set_affinity_hint(unsigned int irq, const struct osal_cpumask *m){return 0;}
__attribute__((weak)) osal_dma_chan osal_dma_request_chan(osal_device dev, const char *name){return NULL;}
__attribute__((weak)) void *osal_dma_alloc_coherent_new(osal_dma_chan chan, size_t size,unsigned long *dma_handle, unsigned int osal_gfp_flag){return NULL;}
__attribute__((weak)) void osal_set_dma_slave_config(struct osal_dma_slave_config* config,enum osal_dma_transfer_direction direction,unsigned long src_addr,unsigned long dst_addr,enum osal_dma_slave_buswidth src_addr_width,
                               enum osal_dma_slave_buswidth dst_addr_width,unsigned int src_maxburst,unsigned int dst_maxburst,unsigned int src_port_window_size,
                               unsigned int dst_port_window_size,int device_fc, unsigned int slave_id){}
__attribute__((weak)) int osal_dmaengine_slave_config(osal_dma_chan chan,struct osal_dma_slave_config *config){return 0;}
__attribute__((weak)) osal_dma_async_tx_descriptor osal_dmaengine_prep_slave_single(
        osal_dma_chan *chan, unsigned long buf, size_t len,
        enum osal_dma_transfer_direction dir, unsigned long flags){return NULL;}
__attribute__((weak)) void osal_set_dma_async_tx_descriptor(osal_dma_async_tx_descriptor des,void (*dma_cb)(void*),void *callback_param){}
__attribute__((weak)) osal_dma_cookie_t osal_dmaengine_submit(osal_dma_async_tx_descriptor desc){return 0;}
__attribute__((weak)) void osal_dma_async_issue_pending(osal_dma_chan chan){}
__attribute__((weak)) void osal_dma_sync_single_for_device(osal_dma_chan chan, unsigned long addr,size_t size, enum osal_dma_data_direction dir){}
__attribute__((weak)) int osal_get_blk_wq_item_count(void){return 0;}
__attribute__((weak)) void osal_get_blk_wq_item(char* buf,int len){}
__attribute__((weak)) void osal_clear_blk_wq_list(void){}
__attribute__((weak)) int osal_get_blk_task_item_count(void){return 0;}
__attribute__((weak)) void osal_get_blk_task_item(char* buf,int len){}
__attribute__((weak)) void osal_clear_blk_task_list(void){}
__attribute__((weak)) void osal_dwc_dma_irq_timer_enable(unsigned int interval) {}
__attribute__((weak)) void osal_dwc_dma_irq_timer_disable(void) {}
__attribute__((weak)) void osal_axi_dma_irq_timer_enable(unsigned int interval) {}
__attribute__((weak)) void osal_axi_dma_irq_timer_disable(void) {}
__attribute__((weak)) int  osal_isprint(int c){return 0;}
__attribute__((weak)) int osal_sa6920_clk_set_isp_rate(unsigned long rate){return 0;}
__attribute__((weak)) unsigned long osal_sa6920_clk_get_isp_rate(void){return 0;}


__attribute__((weak)) unsigned long osal_dmacpy_init(unsigned long dev){return 0;}
__attribute__((weak)) void osal_dmacpy_deinit(int channel){}
__attribute__((weak)) int osal_dmacpy_start(int fd, unsigned long sar, unsigned long dar, unsigned int len){return 0;}
