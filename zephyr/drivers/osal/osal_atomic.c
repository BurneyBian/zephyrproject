#include "osal.h"
#include <zephyr/sys/atomic.h>

__attribute ((visibility("default"))) 
int osal_atomic_init(osal_atomic_t *atomic)
{
	atomic_t *__atomic = osal_kmalloc(sizeof(atomic_t), osal_gfp_kernel);
	if (__atomic) {
		*__atomic = ATOMIC_INIT(0);
		atomic->atomic = __atomic;
		return 0;
	} else {
		return -EINVAL;
	}
}

__attribute ((visibility("default"))) 
void osal_atomic_destory(osal_atomic_t *atomic)
{
	osal_kfree(atomic->atomic);
}

__attribute ((visibility("default"))) 
int osal_atomic_read(osal_atomic_t *atomic)
{
	return atomic_get(atomic->atomic);
	
}

__attribute ((visibility("default"))) 
void osal_atomic_set(osal_atomic_t *atomic, int i)
{
    atomic_set(atomic->atomic, i);
}

__attribute ((visibility("default"))) 
int osal_atomic_inc_return(osal_atomic_t *atomic)
{
	return atomic_inc(atomic->atomic);
}

__attribute ((visibility("default"))) 
int osal_atomic_dec_return(osal_atomic_t *atomic)
{
	return atomic_dec(atomic->atomic);
}

