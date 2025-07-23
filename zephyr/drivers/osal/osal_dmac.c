#include <rthw.h>
#include "rtconfig.h"
#include <rtdevice.h>
#include <rtthread.h>

#include "sa_dmac_if.h"
#include "sa_oswl_mmz.h"
#include "osal.h"

struct dmacpy_info {
    int chan;
	enum dma_transfer_direction dir;
	void *cb_param;
	void (*cb)(void *);

	void *priv_data;
	void *data_sg;

	unsigned char src_addr_width;
	unsigned char src_maxburst;
	unsigned char dst_addr_width;
	unsigned char dst_maxburst;

	rt_sem_t sem;
};

unsigned long osal_dmacpy_init(unsigned long dev)
{
	int chan;
	struct dmacpy_info *info;

    chan = dma_request_channel(!!dev);
    if(!chan)
        return 0;

    info = rt_malloc(sizeof(struct dmacpy_info));
    if(!info) {
		dma_free_channel(chan);
        osal_printk_err("Err:%s malloc fail\n", __func__);
        return RT_NULL;
    } 

    rt_memset(info, 0, sizeof(struct dmacpy_info));
	info->chan = chan;

	return (unsigned long)(info);
}

unsigned long osal_dmacpy_slave_sg_init(unsigned long dev)
{
	return osal_dmacpy_init(dev);
}
RTM_EXPORT(osal_dmacpy_slave_sg_init);

unsigned long osal_dmacpy_slave_single_init(unsigned long dev)
{
	return osal_dmacpy_init(dev);
}
RTM_EXPORT(osal_dmacpy_slave_single_init);

unsigned long osal_dmacpy_cyclic_init(unsigned long dev)
{
	return osal_dmacpy_init(dev);
}
RTM_EXPORT(osal_dmacpy_cyclic_init);

void osal_dmacpy_deinit(unsigned long channel)
{
    struct dmacpy_info *info;

    info = (struct dmacpy_info *)channel;
    if(info) {
        if(info->chan) {
            dma_free_channel(info->chan);
        }
		if (info->priv_data)
			rt_free(info->priv_data);

        rt_free(info);
    }
}
RTM_EXPORT(osal_dmacpy_deinit);

static void dma_memcpy_cb(SA_U8 channel, void *param)
{
	struct dmacpy_info *info = (struct dmacpy_info *)param;

	if (param)
		rt_sem_release(info->sem);
}

int osal_dmacpy_start(unsigned long channel, unsigned long src, unsigned long dst, size_t size)
{
	int ret;
    struct dmacpy_info *info = (struct dmacpy_info *)channel;
	struct dma_slave_config config;  

	rt_memset(&config, 0, sizeof(struct dma_slave_config));

	info->sem = rt_sem_create("dmacpy_sem", 0, RT_IPC_FLAG_PRIO);

	config.direction = DMA_MEM_TO_MEM;
	config.src_addr = src;
	config.dst_addr = dst;

	dma_slave_config(info->chan, &config);

	ret = dma_async_memcpy_with_param(info->chan, (unsigned long)src,
		(unsigned long)dst, size, dma_memcpy_cb, info);

	if (!ret)
		rt_sem_take(info->sem, RT_WAITING_FOREVER);

	rt_sem_delete(info->sem);

	return ret;
}
RTM_EXPORT(osal_dmacpy_start);

struct osal_dmacpy_info * osal_dmacpy_info_init(unsigned long channel, osal_dma_cb cb, void *param)
{
	struct osal_dmacpy_info *osal_info = NULL;
    struct dmacpy_info *info;

    info = (struct dmacpy_info *)channel;

	osal_info = rt_malloc(sizeof(struct osal_dmacpy_info));
	if (osal_info && info) {
		osal_info->channel = channel;
		info->cb = cb;
		info->cb_param = param;
		info->priv_data = osal_info;
	}

	return osal_info;
}
RTM_EXPORT(osal_dmacpy_info_init);

static void dmacpy_cb(SA_U8 channel, void *param)
{
	struct dmacpy_info *info = (struct dmacpy_info *)param;

	if (info && info->cb)
		info->cb(info->cb_param);
}

int osal_dmacpy_slave_single_start(struct osal_dmacpy_info *osal_info)
{
	int ret = -1;
    struct dmacpy_info *info;
	struct dma_slave_config config;  

	rt_memset(&config, 0, sizeof(struct dma_slave_config));

    info = (struct dmacpy_info *)osal_info->channel;

	config.direction = osal_info->to_dev ? DMA_MEM_TO_DEV : DMA_DEV_TO_MEM;
	config.src_addr_width = osal_info->src_addr_width;
	config.dst_addr_width = osal_info->dst_addr_width;
	config.src_maxburst = osal_info->src_maxburst;
	config.dst_maxburst = osal_info->dst_maxburst;
	if (config.direction == DMA_MEM_TO_DEV)
		config.src_addr = osal_info->src;
	else
		config.dst_addr = osal_info->dst;

	dma_slave_config(info->chan, &config);
	
    ret = dma_single_tran(info->chan, osal_info->dst, osal_info->src, osal_info->size);
	if (ret)
		return ret;

	dma_register_callback_with_param(info->chan, dmacpy_cb, info);
	return dma_start_transfer(info->chan);
}
RTM_EXPORT(osal_dmacpy_slave_single_start);

int osal_dmacpy_slave_sg_start(struct osal_dmacpy_info *osal_info)
{
	osal_printk_err("Error: [%s] RTT Not support.\n", __func__);
	return -ENOTSUP;
}
RTM_EXPORT(osal_dmacpy_slave_sg_start);

int osal_dmacpy_cyclic_start(struct osal_dmacpy_info *osal_info)
{
	int ret = -1;
    struct dmacpy_info *info;
	struct dma_slave_config config;  

	rt_memset(&config, 0, sizeof(struct dma_slave_config));

    info = (struct dmacpy_info *)osal_info->channel;

	config.direction = osal_info->to_dev ? DMA_MEM_TO_DEV : DMA_DEV_TO_MEM;
	config.src_addr_width = osal_info->src_addr_width;
	config.dst_addr_width = osal_info->dst_addr_width;
	config.src_maxburst = osal_info->src_maxburst;
	config.dst_maxburst = osal_info->dst_maxburst;
	if (config.direction == DMA_MEM_TO_DEV)
		config.src_addr = osal_info->src;
	else
		config.dst_addr = osal_info->dst;

	dma_slave_config(info->chan, &config);

    ret = dma_cyclic_tran(info->chan, osal_info->buf_addr, osal_info->size, osal_info->period_len);
	if (ret)
		return ret;

	dma_register_callback_with_param(info->chan, dmacpy_cb, info);

	return dma_start_transfer(info->chan);
}
RTM_EXPORT(osal_dmacpy_cyclic_start);

int osal_dmacpy_channel_stop(struct osal_dmacpy_info *osal_info)
{
	int ret = -EINVAL;
    struct dmacpy_info *info = (struct dmacpy_info *)osal_info->channel;

	if (info)
		ret = dma_terminate_all(info->chan);

	return ret;
}
RTM_EXPORT(osal_dmacpy_channel_stop);
