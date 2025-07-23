#include "osal.h"


__attribute ((visibility("default"))) 
unsigned long osal_msecs_to_jiffies(const unsigned int m)
{
    return (unsigned long)k_ms_to_ticks_floor32(m);
}

__attribute ((visibility("default"))) 
int osal_wait_init(osal_wait_t *wait)
{
    struct k_sem *wq = NULL;

    if (wait == NULL) {
        osal_printk("%s - parameter invalid!\n", __FUNCTION__);
        return -EINVAL;
    }

    wq = (struct k_sem *)k_malloc(sizeof(struct k_sem));
    if (wq == NULL) {
        osal_printk("%s - malloc error!\n", __FUNCTION__);
        return -1;
    }

    k_sem_init(wq, 0, 1);
    wait->wait = wq;

    return 0;
}
__attribute ((visibility("default"))) 
int osal_wait_uninterruptible(osal_wait_t *wait, osal_wait_cond_func_t func, void *param)
{
    struct k_sem *wq = NULL;
	int condition = 0;

	if (!(wait && wait->wait)) {
        osal_printk("%s - parameter invalid!\n", __FUNCTION__);
        return -EINVAL;
    }

    wq = (struct k_sem *)(wait->wait);

    if (func) {
        condition = func(param);
    }

	if (!condition)
		k_sem_take(wq, K_FOREVER);

    return 0;
}
__attribute ((visibility("default"))) 
int osal_wait_interruptible(osal_wait_t *wait, osal_wait_cond_func_t func, void *param)
{
    return osal_wait_uninterruptible(wait, func, param);
}
__attribute ((visibility("default"))) 
int osal_wait_timeout_uninterruptible(osal_wait_t *wait, osal_wait_cond_func_t func, void *param, int timeout)
{
    struct k_sem *wq = NULL;
	int condition = 0;
	int ret = timeout;
	unsigned int end;

	if (!(wait && wait->wait)) {
        osal_printk("%s - parameter invalid!\n", __FUNCTION__);
        return -EINVAL;
    }

    wq = (struct k_sem *)(wait->wait);

    if (func) {
        condition = func(param);
    }

	if (!condition)
	{
		end = osal_get_tickcount() + timeout;
		ret = k_sem_take(wq, K_MSEC(timeout));

		if (ret != 0)
			ret = -ERESTARTSYS;
		else {
			if (timeout > 0)
				ret = end - osal_get_tickcount();
			else
				ret = 0;
		}
	}

    return ret;
}
__attribute ((visibility("default"))) 
int osal_wait_timeout_interruptible(osal_wait_t *wait, osal_wait_cond_func_t func, void *param, int timeout)
{
    return osal_wait_timeout_uninterruptible(wait, func, param, timeout);
}
__attribute ((visibility("default"))) 
void osal_wakeup(osal_wait_t *wait)
{
	if (wait && wait->wait)
		k_sem_give((struct k_sem *)(wait->wait));
}
__attribute ((visibility("default"))) 
void osal_wait_destory(osal_wait_t *wait)
{
	if (wait && wait->wait) {
		k_free(wait->wait);
		wait->wait = NULL;
	}
}

