#include "osal.h"
#include <zephyr/cache.h>
__attribute ((visibility("default"))) 
void osal_cpuc_flush_dcache_area(void *addr, int size)
{
    sys_cache_data_flush_range(addr, size);
}
__attribute ((visibility("default"))) 
void osal_flush_dcache_area(void *kvirt, unsigned long phys_addr, unsigned long length)
{
	sys_cache_data_flush_range(kvirt, length);
}
__attribute ((visibility("default"))) 
void osal_invalid_dcache_area(void *kvirt, unsigned long length)
{
	sys_cache_data_invd_range(kvirt, length);
}
__attribute ((visibility("default"))) 
int osal_flush_dcache_all(void)
{
	return sys_cache_data_flush_all();
}

