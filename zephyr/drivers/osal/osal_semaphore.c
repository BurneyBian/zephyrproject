#include <zephyr/kernel.h>
#include "osal.h"
__attribute ((visibility("default"))) 
int osal_sema_init(osal_semaphore_t *sem, int val)
{
    if (sem == NULL) {
        osal_printk("%s - parameter invalid!\n", __FUNCTION__);
        return -EINVAL;
    }

	return k_sem_init(&sem->sem, val, K_SEM_MAX_LIMIT); //1?
}
__attribute ((visibility("default"))) 
int osal_down(osal_semaphore_t *sem)
{
    if (sem == NULL) {
        osal_printk("%s - parameter invalid!\n", __FUNCTION__);
        return -EINVAL;
    }
	return k_sem_take(&sem->sem, K_FOREVER);
}
__attribute ((visibility("default"))) 
int osal_down_interruptible(osal_semaphore_t *sem)
{
	return osal_down(sem);
}
__attribute ((visibility("default"))) 
int osal_down_trylock(osal_semaphore_t *sem)
{
    if (sem == NULL) {
        osal_printk("%s - parameter invalid!\n", __FUNCTION__);
        return -EINVAL;
    }
	return k_sem_take(&sem->sem, K_NO_WAIT);
}
__attribute ((visibility("default"))) 
void osal_up(osal_semaphore_t *sem)
{
    if (sem == NULL) {
        osal_printk("%s - parameter invalid!\n", __FUNCTION__);
        return;
    }

	k_sem_give(&sem->sem);	
}
__attribute ((visibility("default"))) 
void osal_sema_destory(osal_semaphore_t *sem)
{}

