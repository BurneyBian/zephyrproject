#include "osal.h"
#include "./devfs/devfs.h"

int osal_registerdevice(osal_dev_t *osal_dev)
{
    if (!osal_dev || !osal_dev->fops) {
        osal_printk_err("%s - parameter invalid!\n", __func__);
        return -EINVAL;
    }

    osal_device_register(osal_dev);

#ifdef RT_USING_PM
	osal_pm_device_register(osal_dev);
#endif

	return 0;
}

void osal_deregisterdevice(osal_dev_t *pdev)
{
    if (pdev == NULL) {
        osal_printk_err("%s - parameter invalid!\n", __func__);
        return;
    }
#ifdef RT_USING_PM
	osal_pm_device_unregister(pdev);
#endif
    osal_device_unregister(pdev);

}

osal_dev_t *osal_createdev(const char *name)
{
    osal_dev_t *pdev = NULL;

    if (name == NULL) {
        osal_printk_err("%s - parameter invalid!\n", __func__);
        return NULL;
    }

    pdev = (osal_dev_t *)osal_kmalloc(sizeof(osal_dev_t), osal_gfp_kernel);
    if (pdev == NULL) {
        osal_printk_err("%s - osal_kmalloc error!\n", __func__);
        return NULL;
    }

    memset(pdev, 0, sizeof(osal_dev_t));
    strcpy(pdev->name, name);

    return pdev;
}

int osal_destroydev(osal_dev_t *osal_dev)
{
    if (osal_dev == NULL) {
        osal_printk_err("%s - parameter invalid!\n", __func__);
        return -EINVAL;
    }

    osal_kfree(osal_dev);

    return 0;
}

void osal_poll_wait(osal_poll_t *table, osal_wait_t *wait)
{
	if (table == NULL) {
        osal_printk_err("%s - table invalid!\n", __func__);
        return;
    }

    if (wait == NULL) {
        osal_printk_err("%s - wait invalid!\n", __func__);
        return;
    }
	//rt_poll_add((rt_wqueue_t *)(wait->wait), table->poll_table);
}

#define OSAL_PAGE_SIZE 4096

extern void *osal_vm_dma_debug_alloc(void *addr, char *name, int len);
extern void osal_vm_dma_debug_free(void *addr, int len);

void *osal_dma_alloc_coherent(osal_dev_t *dev, int size, unsigned long *dma_handle)
{
	return NULL;
}

void osal_dma_free_coherent(osal_dev_t *dev, int size, void *cpu_addr, unsigned long dma_handle)
{
}

void osal_dma_map_single(osal_dev_t *dev, void *ptr, int size, int dir_to_dev)
{
	osal_cpuc_flush_dcache_area(ptr, size);
}

void osal_dma_unmap_single(osal_dev_t *dev, void *ptr, int size, int dir_to_dev)
{
	if (!dir_to_dev) { //invalid cache
		osal_invalid_dcache_area(ptr, size);
	}
}

void osal_poll_requested_events(osal_poll_t *poll, unsigned int *event)
{
    //*event = poll ? ((rt_pollreq_t *)(poll->poll_table))->_key : ~(unsigned int)0;
}

