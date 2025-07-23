#include <rthw.h>
#include "rtconfig.h"
#include <rtdevice.h>
#include <rtthread.h>
#include "osal.h"
#include "sa_dmac_if.h"
#include "sa_hal_axi_dmac.h"

static rt_sem_t g_dmacpySem[18];

unsigned long osal_dmacpy_init(unsigned long dev)
{
    return dma_request_channel(!!dev);
}

void osal_dmacpy_deinit(int fd)
{
    if(fd < 0)
        return;
    dma_free_channel(fd);
}

static void osal_dmacpy_callback(uint8_t channel)
{
    if(g_dmacpySem[channel])
        rt_sem_release(g_dmacpySem[channel]);
}

int osal_dmacpy_start(int fd, unsigned long sar, unsigned long dar, unsigned int len)
{
    int ret;
    if(fd < 0){
        osal_printk_err("dmacpy not init\n");
        return -1;
    }
    g_dmacpySem[fd] = rt_sem_create("dma_sem", 0, RT_IPC_FLAG_PRIO);
    ret = dma_async_memcpy(fd, sar, dar, len, osal_dmacpy_callback);
    ret = rt_sem_take(g_dmacpySem[fd], RT_TICK_PER_SECOND*2);
    if(ret != 0){
        osal_printk_err("oswl_dmacpy overtime(%d)\n", ret);
        dma_terminate_all(fd);
    }
    rt_sem_delete(g_dmacpySem[fd]);
    g_dmacpySem[fd] = 0;
    return ret;
}
#ifdef BSP_USING_DMAC
#include "sa_dmac_if.h"
struct dma_chan {
	unsigned long base;
	unsigned int channel;
	unsigned int is_memmaster;
	unsigned int is_devmaster;
	unsigned int len;
	struct dma_slave_config config;
	struct dma_async_tx_descriptor *desc;
};

struct dma_async_tx_descriptor  {
	unsigned char channel;
	unsigned long buf;
	size_t len;
	size_t period_len;
	unsigned long flags;
	struct dma_slave_config *config;
};

#ifdef RT_USING_OFW
static struct dma_chan *ofw_get_dma_no_lock(struct rt_ofw_node *np, int index, const char *name)
{
	struct rt_ofw_node *dma_ofw_np = RT_NULL;
    struct dma_chan *dma_channel = RT_NULL;
    struct rt_ofw_cell_args dma_args;
	rt_uint64_t thread_id;
	rt_uint64_t out_address, out_size;
	unsigned char chan;

    if (!rt_ofw_parse_phandle_cells(np, "dmas", "#dma-cells", index, &dma_args)) {
        int count;
        dma_ofw_np = dma_args.data;
        count = dma_args.args_count;

		if ((count == 2) || (count == 3))
			dma_channel = rt_malloc(sizeof(struct dma_chan));

        if (dma_channel) {
			chan = dma_args.args[0];
			dma_channel->is_memmaster = dma_args.args[1];
			if (count == 3)
				dma_channel->is_devmaster = dma_args.args[2];
        }

		if (!rt_ofw_get_address(dma_ofw_np, 0, &out_address, &out_size)) {
			dma_channel->base = out_address;
		}
		dma_channel->channel = (unsigned char)(chan + (dma_channel->base == 0x40000000) ? DMAC_AXI_MAX_CHAN : 0);
    }

    return dma_channel;
}

osal_dma_chan osal_dma_request_chan(osal_device dev, const char *name)
{
	struct rt_platform_device *pdev = (struct rt_platform_device *)dev;
	struct rt_ofw_node *np = pdev->parent.ofw_node;
	const char *dev_name = np->name;
	int index = 0;

	//osal_printk("%s dev_name=%s\n", __func__, dev_name);

    if (np)
    {
		if (name) 
			index = rt_ofw_prop_index_of_string(np, "dma-names", name);

		if (index >= 0)
		{
			return (osal_dma_chan)ofw_get_dma_no_lock(np, index, name);
		}	
    }

	return RT_NULL;
}
RTM_EXPORT(osal_dma_request_chan);
#endif

void osal_set_dma_slave_config(struct osal_dma_slave_config* config,
	enum osal_dma_transfer_direction direction,
	unsigned long src_addr,
	unsigned long dst_addr,
	enum osal_dma_slave_buswidth src_addr_width, 
	enum osal_dma_slave_buswidth dst_addr_width,
	unsigned int src_maxburst,
	unsigned int dst_maxburst,
	unsigned int src_port_window_size,
    unsigned int  dst_port_window_size,
	rt_bool_t device_fc, 
	unsigned int slave_id)
{
	struct dma_slave_config *dma_config = (struct dma_slave_config *)config;

	if (direction == OSAL_DMA_MEM_TO_MEM)
		dma_config->direction = DMA_MEM_TO_MEM;
	else if (direction == OSAL_DMA_MEM_TO_DEV)
		dma_config->direction = DMA_MEM_TO_DEV;
	else if (direction == OSAL_DMA_DEV_TO_MEM)
		dma_config->direction = DMA_DEV_TO_MEM;
	else if (direction == OSAL_DMA_DEV_TO_DEV)
		dma_config->direction = DMA_DEV_TO_DEV;
	else if (direction == OSAL_DMA_TRANS_NONE)
		dma_config->direction = DMA_TRANS_NONE;

	dma_config->src_addr = src_addr;
	dma_config->dst_addr = dst_addr;
	dma_config->src_maxburst = src_maxburst;
	dma_config->dst_maxburst = dst_maxburst;

	if (src_addr_width == OSAL_DMA_SLAVE_BUSWIDTH_1_BYTE )
		dma_config->src_addr_width = DMA_TR_WIDTH_8;
	else if (src_addr_width == OSAL_DMA_SLAVE_BUSWIDTH_2_BYTES)
		dma_config->src_addr_width = DMA_TR_WIDTH_16;
	else if (src_addr_width == OSAL_DMA_SLAVE_BUSWIDTH_4_BYTES)
		dma_config->src_addr_width = DMA_TR_WIDTH_32;
	else if (src_addr_width == OSAL_DMA_SLAVE_BUSWIDTH_8_BYTES)
		dma_config->src_addr_width = DMA_TR_WIDTH_64;
	else if (src_addr_width == OSAL_DMA_SLAVE_BUSWIDTH_16_BYTES)
		dma_config->src_addr_width = DMA_TR_WIDTH_128;
	else if (src_addr_width == OSAL_DMA_SLAVE_BUSWIDTH_4_BYTES)
		dma_config->src_addr_width = DMA_AHB_MAX_TR_WIDTH;


	if (dst_addr_width == OSAL_DMA_SLAVE_BUSWIDTH_1_BYTE )
		dma_config->dst_addr_width = DMA_TR_WIDTH_8;
	else if (dst_addr_width == OSAL_DMA_SLAVE_BUSWIDTH_2_BYTES)
		dma_config->dst_addr_width = DMA_TR_WIDTH_16;
	else if (dst_addr_width == OSAL_DMA_SLAVE_BUSWIDTH_4_BYTES)
		dma_config->dst_addr_width = DMA_TR_WIDTH_32;
	else if (dst_addr_width == OSAL_DMA_SLAVE_BUSWIDTH_8_BYTES)
		dma_config->dst_addr_width = DMA_TR_WIDTH_64;
	else if (dst_addr_width == OSAL_DMA_SLAVE_BUSWIDTH_16_BYTES)
		dma_config->dst_addr_width = DMA_TR_WIDTH_128;
	else if (dst_addr_width == OSAL_DMA_SLAVE_BUSWIDTH_4_BYTES)
		dma_config->dst_addr_width = DMA_AHB_MAX_TR_WIDTH;
}
RTM_EXPORT(osal_set_dma_slave_config);

int osal_dmaengine_slave_config(osal_dma_chan chan, struct osal_dma_slave_config *config)
{
	struct dma_chan *dma_channel = (struct dma_chan *)chan;
	struct dma_slave_config *dma_config = (struct dma_slave_config *)config;
	//dma_channel->config = (struct dma_slave_config *)config;
	memcpy(&dma_channel->config, config, sizeof(struct dma_slave_config));


	return 0;	
}
RTM_EXPORT(osal_dmaengine_slave_config);

osal_dma_async_tx_descriptor osal_dmaengine_prep_slave_single(
        osal_dma_chan *chan, 
		unsigned long buf, 
		size_t len,
        enum osal_dma_transfer_direction dir, 
		unsigned long flags)
{
	struct dma_async_tx_descriptor *desc = RT_NULL;
    struct dma_chan *dma_channel = (struct dma_chan *)chan;
	struct dma_slave_config *dma_config = (struct dma_slave_config *)&dma_channel->config;

	dma_channel->len = len;
	desc = rt_malloc(sizeof(struct dma_async_tx_descriptor));
	if (desc) {
		//desc->channel =  (unsigned char)(dma_channel->channel + (dma_channel->base == 0x40000000) ? DMAC_AXI_MAX_CHAN : 0);
		desc->channel = dma_channel->channel;
		desc->len = len;
		desc->flags = flags;
		desc->period_len = 0;
		desc->config = dma_config;
		desc->buf = buf;
		dma_channel->desc = desc;
		
		if (dir == OSAL_DMA_MEM_TO_MEM)
			dma_config->direction = DMA_MEM_TO_MEM;
		else if (dir == OSAL_DMA_MEM_TO_DEV) {
			dma_config->direction = DMA_MEM_TO_DEV;
			dma_config->src_addr = buf;
		} else if (dir == OSAL_DMA_DEV_TO_MEM) {
			dma_config->direction = DMA_DEV_TO_MEM;
			dma_config->dst_addr = buf;
		} else if (dir == OSAL_DMA_DEV_TO_DEV)
			dma_config->direction = DMA_DEV_TO_DEV;
		else if (dir == OSAL_DMA_TRANS_NONE)
			dma_config->direction = DMA_TRANS_NONE;
	}

	return (osal_dma_async_tx_descriptor)desc;
}
RTM_EXPORT(osal_dmaengine_prep_slave_single);

osal_dma_async_tx_descriptor  osal_dmaengine_prep_dma_cyclic(
                osal_dma_chan chan, unsigned long buf_addr, size_t buf_len,
                size_t period_len, enum osal_dma_transfer_direction dir, unsigned long flags)
{
	struct dma_async_tx_descriptor *desc = RT_NULL;
    struct dma_chan *dma_channel = (struct dma_chan *)chan;
	struct dma_slave_config *dma_config = (struct dma_slave_config *)&dma_channel->config;

	dma_channel->len = buf_len;
	desc = rt_malloc(sizeof(struct dma_async_tx_descriptor));
	if (desc) {
		//desc->channel =  (unsigned char)(dma_channel->channel + (dma_channel->base == 0x40000000) ? DMAC_AXI_MAX_CHAN : 0);
		desc->channel = dma_channel->channel;
		desc->len = buf_len;
		desc->flags = flags;
		desc->config = dma_config;
		desc->period_len = period_len;
		desc->buf = buf_addr;
		dma_channel->desc = desc;

		if (dir == OSAL_DMA_MEM_TO_MEM)
			dma_config->direction = DMA_MEM_TO_MEM;
		else if (dir == OSAL_DMA_MEM_TO_DEV) {
			dma_config->direction = DMA_MEM_TO_DEV;
			dma_config->src_addr = buf_addr;
		} else if (dir == OSAL_DMA_DEV_TO_MEM) {
			dma_config->direction = DMA_DEV_TO_MEM;
			dma_config->dst_addr = buf_addr;
		} else if (dir == OSAL_DMA_DEV_TO_DEV)
			dma_config->direction = DMA_DEV_TO_DEV;
		else if (dir == OSAL_DMA_TRANS_NONE)
			dma_config->direction = DMA_TRANS_NONE;

	}

	return (osal_dma_async_tx_descriptor)desc;	
}
RTM_EXPORT(osal_dmaengine_prep_dma_cyclic);

struct dma_callback {
	void *callback_param;
	osal_dma_cb dma_cb;
	int cyclic;
};

static void dmacpy_cb(unsigned char channel, void *param)
{
	struct dma_callback *dma_param = (struct dma_callback *)param;

	if (dma_param->dma_cb)
		dma_param->dma_cb(dma_param->callback_param);

	if (!dma_param->cyclic)
		rt_free(dma_param);
}

void osal_set_dma_async_tx_descriptor(osal_dma_async_tx_descriptor des,
	void (*dma_cb)(void*), void *callback_param)
{
	int ret;
	struct dma_callback *dma_param = (struct dma_callback *)rt_malloc(sizeof(struct dma_callback));
	struct dma_async_tx_descriptor *desc = (struct dma_async_tx_descriptor *)des;

	dma_slave_config(desc->channel, desc->config);

	if (!desc->period_len)
		ret = dma_single_tran(desc->channel, desc->config->dst_addr, desc->config->src_addr, desc->len);
	else
		ret = dma_cyclic_tran(desc->channel, desc->buf, desc->len, desc->period_len);

	if (ret)
		return ret;

	dma_param->dma_cb = dma_cb;
	dma_param->callback_param = callback_param;
	dma_param->cyclic = desc->period_len ? 1 : 0;
	dma_register_callback_with_param(desc->channel, dmacpy_cb, dma_param);

	return 0;
}
RTM_EXPORT(osal_set_dma_async_tx_descriptor);

enum osal_dma_status osal_dmaengine_tx_status(osal_dma_chan chan, osal_dma_cookie_t cookie,struct osal_dma_tx_state *state)
{
	enum dma_channel_state dma_state;
	unsigned int count, len;
	struct dma_chan *dma_channel = (struct dma_chan *)chan;
	struct dma_async_tx_descriptor *desc = (struct dma_async_tx_descriptor *)dma_channel->desc;
	//unsigned char channel = (unsigned char)(dma_channel->channel + (dma_channel->base == 0x40000000) ? DMAC_AXI_MAX_CHAN : 0);
	unsigned char channel = desc->channel;

	dma_state = dma_get_tx_status(channel, &count, &len);

    state->last = 0;
    state->used = 0;
    //state->residue = dma_channel->len - len;
	state->residue = count*len;
    state->in_flight_bytes = 0;
		
	if (dma_state == DMA_CHANNEL_STATE_BUSY)
		return OSAL_DMA_IN_PROGRESS;
	else if (dma_state == DMA_CHANNEL_STATE_COMPLETE)
		return OSAL_DMA_COMPLETE;
	else if (dma_state == DMA_CHANNEL_STATE_CLOSED)
		return OSAL_DMA_ERROR;
	else
		return OSAL_DMA_PAUSED;
}
RTM_EXPORT(osal_dmaengine_tx_status);

osal_dma_cookie_t osal_dmaengine_submit(osal_dma_async_tx_descriptor des)
{
	struct dma_async_tx_descriptor *desc = (struct dma_async_tx_descriptor *)des;

	return dma_start_transfer(desc->channel);
}
RTM_EXPORT(osal_dmaengine_submit);

void osal_dma_async_issue_pending(osal_dma_chan chan)
{
	
}
RTM_EXPORT(osal_dma_async_issue_pending);

void *osal_dma_alloc_coherent_new(osal_dma_chan chan,
	size_t size,
	unsigned long *dma_handle,
	unsigned int osal_gfp_flag)
{
	return osal_dma_alloc_coherent(RT_NULL, size, dma_handle);
}
RTM_EXPORT(osal_dma_alloc_coherent_new);

void osal_dma_sync_single_for_device(osal_dma_chan chan, unsigned long addr,
	size_t size,
	enum osal_dma_data_direction dir)
{
	if (dir == OSAL_DMA_FROM_DEVICE)
		acme_invalid_dcache_area(addr, size);
	else if(dir == OSAL_DMA_TO_DEVICE)
		acme_cpuc_flush_dcache_area(addr, size);
}
RTM_EXPORT(osal_dma_sync_single_for_device);

osal_dma_chan osal_dma_cyclic_request_chan(int dev)
{
    struct dma_chan *dma_channel = RT_NULL;

	dma_channel = rt_malloc(sizeof(struct dma_chan));
	if (dma_channel) {
		dma_channel->channel = dma_request_channel(dev);
	}
	//osal_printk("dma_channel->channel=%d\n",dma_channel->channel);

    return (osal_dma_chan)dma_channel;
}
RTM_EXPORT(osal_dma_cyclic_request_chan);

void osal_dma_release_channel(osal_dma_chan chan)
{
	struct dma_chan *dma_channel = (struct dma_chan *)chan;

	if (dma_channel && dma_channel->desc) {
		dma_free_channel(dma_channel->desc->channel);

		rt_free(dma_channel->desc);
		rt_free(dma_channel);
	}
}
//RTM_EXPORT(osal_dma_release_channel);

void osal_dma_free_coherent_new(osal_device dev, size_t size, void *cpu_addr, dma_addr_t dma_handle)
{
	osal_dma_free_coherent(RT_NULL, size, cpu_addr, dma_handle);
}
int osal_dmaengine_terminate_async(osal_dma_chan chan)
{
	struct dma_chan *dma_channel = (struct dma_chan *)chan;
	int ret;

	if (dma_channel && dma_channel->desc) {
		ret = dma_terminate_all(dma_channel->desc->channel);
		//osal_printk_err("%s ret=%d\n", __func__ , ret);
		return ret;
	}

	return -1;
}

RTM_EXPORT(osal_dma_release_channel);
RTM_EXPORT(osal_dma_free_coherent_new);
RTM_EXPORT(osal_dmaengine_terminate_async);

#endif
