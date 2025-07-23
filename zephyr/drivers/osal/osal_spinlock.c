#include <zephyr/spinlock.h>
#include "osal.h"
__attribute ((visibility("default"))) 
int osal_spin_lock_init(osal_spinlock_t *lock)
{
	if (lock == NULL) {
		osal_printk("%s - parameter invalid!\n", __FUNCTION__);
		return -EINVAL;
    }
    return 0;
}
__attribute ((visibility("default"))) 
void osal_spin_lock(osal_spinlock_t *lock)
{
	if (lock)
		lock->key = k_spin_lock(&lock->lock);
	else
		osal_printk("%s - parameter invalid!\n", __FUNCTION__);
}
__attribute ((visibility("default"))) 
int osal_spin_trylock(osal_spinlock_t *lock)
{
	if (lock)
		return k_spin_trylock(&lock->lock, &lock->key);
	else
		osal_printk("%s - parameter invalid!\n", __FUNCTION__);
	return -EINVAL;
}
__attribute ((visibility("default"))) 
void osal_spin_unlock(osal_spinlock_t *lock)
{
	if (lock)
		k_spin_unlock(&lock->lock, lock->key);
	else
		osal_printk("%s - parameter invalid!\n", __FUNCTION__);
}
__attribute ((visibility("default"))) 
void osal_spin_lock_irqsave(osal_spinlock_t *lock, unsigned long *flags)
{
	osal_spin_lock(lock);
}
__attribute ((visibility("default"))) 
void osal_spin_unlock_irqrestore(osal_spinlock_t *lock, unsigned long *flags)
{
	osal_spin_unlock(lock);
}
__attribute ((visibility("default"))) 
void osal_spin_lock_destory(osal_spinlock_t *lock)
{}
