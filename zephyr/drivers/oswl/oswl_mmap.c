#include <rtthread.h>
#include <rthw.h>
#include "oswl.h"
#ifdef BSP_USING_A55
#include "mmu.h"
#endif

void *oswl_mmap(unsigned long phy_addr, unsigned long size, int *v)
{
	*v = 1;
	return (void *)acme_phyaddr_need_remap(phy_addr, 1);
}

void *oswl_mmap_cache(unsigned long phy_addr, unsigned long size, int *v)
{
	*v = 2;
	if (phy_addr & 0x00000fff)
		OSWL_LOG_W("WARNNING:[%s] addr[%lx] Not align\n", __func__, phy_addr);

	return (void *)acme_phyaddr_need_remap(phy_addr, 1);
}

void oswl_munmap(unsigned long virt_addr, unsigned long phy_addr, unsigned long size, int v)
{}

RTM_EXPORT(oswl_mmap);
RTM_EXPORT(oswl_mmap_cache);
RTM_EXPORT(oswl_munmap);