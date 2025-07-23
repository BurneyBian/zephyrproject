#include <zephyr/kernel.h>
#include "osal.h"
__attribute ((visibility("default"))) 
int osal_mutex_init(osal_mutex_t *mutex)
{
    if (mutex == NULL) {
        osal_printk_err("%s - parameter invalid!\n", __FUNCTION__);
        return -EINVAL;
    }

	k_mutex_init(&mutex->mutex);
    return 0;
}
__attribute ((visibility("default"))) 
int osal_mutex_lock(osal_mutex_t *mutex)
{
    if (mutex == NULL) {
        osal_printk_err("%s - parameter invalid!\n", __FUNCTION__);
        return -EINVAL;
    }
	return k_mutex_lock(&mutex->mutex, K_FOREVER);
}
__attribute ((visibility("default"))) 
int osal_mutex_trylock(osal_mutex_t *mutex)
{
    if (mutex == NULL) {
        osal_printk_err("%s - parameter invalid!\n", __FUNCTION__);
        return -EINVAL;
    }
	return k_mutex_lock(&mutex->mutex, K_NO_WAIT);
}
__attribute ((visibility("default"))) 
int osal_mutex_lock_interruptible(osal_mutex_t *mutex)
{
   return osal_mutex_lock(mutex);
}
__attribute ((visibility("default"))) 
void osal_mutex_unlock(osal_mutex_t *mutex)
{
    if (mutex == NULL) {
        osal_printk_err("%s - parameter invalid!\n", __FUNCTION__);
        return;
    }

	k_mutex_unlock(&mutex->mutex);
}
__attribute ((visibility("default"))) 
void osal_mutex_destory(osal_mutex_t *mutex)
{}
