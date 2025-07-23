#include <zephyr/irq.h>
#include "osal.h"

//#define OSAL_IRQ_THREAD

struct osal_irq_context {
	unsigned int irq;
	unsigned int active;
	osal_irq_handler_t handler;
	osal_irq_handler_t thread_fn;
	void *dev;
	struct osal_list_head node;
	char name[16];
};

static OSAL_LIST_HEAD(osal_irq_list);
static K_MUTEX_DEFINE(osal_irq_mutex);

#ifdef OSAL_IRQ_THREAD
static K_SEM_DEFINE(osal_irq_sem, 0, 1);

static void osal_isr_thread(void)
{
	struct osal_irq_context *c, *p;

    while (1) {
		k_sem_take(&osal_irq_sem, K_FOREVER);

		k_mutex_lock(&osal_irq_mutex, K_FOREVER);
		osal_list_for_each_entry_safe(c, p, &osal_irq_list, node) {
			if (c->active == 1) {
				c->active = 2;
				break;
			}
		}
		k_mutex_unlock(&osal_irq_mutex);

		if (c->active == 2) {
			c->handler(c->irq, c->dev);
			c->active = 0;
		}
    }
}

static K_KERNEL_THREAD_DEFINE(osal_irq_task, 512, osal_isr_thread, NULL, NULL, NULL, 2, 0, 0);

#endif

void osal_isr_register(unsigned int irq, osal_irq_handler_t handler, osal_irq_handler_t thread_fn, const char *name, void *dev)
{
	struct osal_irq_context *c = (struct osal_irq_context *)osal_kmalloc(sizeof(struct osal_irq_context), osal_gfp_kernel);
	if (c) {
		memset(c, 0, sizeof(struct osal_irq_context));
		c->irq = irq;
		c->handler = handler;
		c->thread_fn = thread_fn;
		c->dev = dev;
		memcpy(c->name, name, 16);
		k_mutex_lock(&osal_irq_mutex, K_FOREVER);
		osal_list_add_tail(&c->node, &osal_irq_list);
		k_mutex_unlock(&osal_irq_mutex);
	}
}

static char *__osal_free_irq(unsigned int irq, void *dev)
{
	struct osal_irq_context *c, *p;
	int is_found = 0;
	irq_disable(irq);

	k_mutex_lock(&osal_irq_mutex, K_FOREVER);
    osal_list_for_each_entry_safe(c, p, &osal_irq_list, node) {
		if (c->irq == irq) {
			osal_list_del(&c->node);
			is_found = 1;
			break;
		}
	}
	k_mutex_unlock(&osal_irq_mutex);

	if (is_found) {
		osal_kfree(c);
		return c->name;
	}

	return NULL;
}
void osal_isr_handler(const void *param)
{
	struct osal_irq_context *c, *p;
	int is_found = 0;
	osal_printk("%s enter\n", __func__);
    osal_list_for_each_entry_safe(c, p, &osal_irq_list, node) {
		if (c->irq == (int)param) {
			is_found = 1;
			break;
		}
	}
	osal_printk("%s is_found=%d\n", __func__, is_found);
	if (is_found) {
		osal_printk("%s %s\n", __func__, c->name);
		if (c->handler)
			c->handler(c->irq, c->dev);
#ifdef OSAL_IRQ_THREAD
		if(c->thread_fn) {
			c->active = 1;
			k_sem_give(&osal_irq_sem);
		}
#endif
	}
}

#ifdef CONFIG_DYNAMIC_INTERRUPTS
__attribute ((visibility("default"))) 
int osal_request_irq(unsigned int irq, osal_irq_handler_t handler, osal_irq_handler_t thread_fn, const char *name,
                     void *dev)
{
	osal_isr_register(irq, handler, thread_fn, name, dev);
	irq_connect_dynamic(irq, 2, osal_isr_handler, irq, 0);
	irq_enable(irq);
	return 0;
}

__attribute ((visibility("default"))) 
void osal_free_irq(unsigned int irq, void *dev)
{
	__osal_free_irq(irq, dev);
	irq_disconnect_dynamic(irq, 2, osal_isr_handler, irq, 0);
}
#else

__attribute ((visibility("default"))) 
void osal_free_irq(unsigned int irq, void *dev)
{
	__osal_free_irq(irq, dev);
}
#endif

__attribute ((visibility("default"))) 
int osal_in_interrupt(void)
{
    return k_is_in_isr();
}

static void __work_func(struct k_work *item)
{
	osal_workqueue_t *wq = CONTAINER_OF(item, osal_workqueue_t, work);
		
	if (wq->workq_func)
		wq->workq_func(wq->param);
}
__attribute ((visibility("default"))) 
void osal_workq_init(osal_workqueue_t *wq)
{
    if (wq == NULL) {
        osal_printk("%s line[%d]- parameter invalid!\n", __FUNCTION__, __LINE__);
        return;
    }

	k_work_init(&wq->work, __work_func);
}
__attribute ((visibility("default"))) 
int osal_workq_schedule(osal_workqueue_t *wq)
{
    if (wq == NULL) {
        osal_printk("%s line[%d]- parameter invalid!\n", __FUNCTION__, __LINE__);
        return -EINVAL;
    }

	return k_work_submit(&wq->work);
}
__attribute ((visibility("default"))) 
int osal_workq_cancel(osal_workqueue_t *wq)
{
	int ret = 0;

    if (wq == NULL) {
        osal_printk("%s line[%d]- parameter invalid!\n", __FUNCTION__, __LINE__);
        return -EINVAL;
    }

	ret = k_work_cancel(&wq->work);

	return ret;
}
__attribute ((visibility("default"))) 
void osal_interrupt_mask(int irq)
{
	irq_disable(irq);
}
__attribute ((visibility("default"))) 
void osal_interrupt_umask(int irq)
{
	irq_enable(irq);
}
