#include "osal.h"

__attribute ((visibility("default"))) 
void *osal_ioremap(unsigned long phys_addr, unsigned int size)
{
	//acme_set_noncacheable_addr(phys_addr, size);

	return (void *)phys_addr;//acme_phyaddr_need_remap(phys_addr, 0);
}

__attribute ((visibility("default"))) 
void *osal_ioremap_nocache(unsigned long phys_addr, unsigned int size)
{
	return (void *)phys_addr;//osal_ioremap(phys_addr, size);
}

__attribute ((visibility("default"))) 
void *osal_ioremap_cached(unsigned long phys_addr, unsigned int size)
{
	//acme_set_cacheable_addr(phys_addr, size);

	return (void *)phys_addr;//acme_phyaddr_need_remap(phys_addr, 1);
}

__attribute ((visibility("default"))) 
void *osal_ioremap_wc(unsigned long phys_addr, unsigned int size)
{
    return (void *)phys_addr;//osal_ioremap(phys_addr, size);
}

__attribute ((visibility("default"))) 
void osal_iounmap(void *addr)
{}

__attribute ((visibility("default"))) 
unsigned long osal_copy_from_user(void *to, const void *from, unsigned long n)
{
    memcpy(to, from, n);
	return 0;
}

__attribute ((visibility("default"))) 
unsigned long osal_copy_to_user(void *to, const void *from, unsigned long n)
{
    memcpy(to, from, n);
	return 0;
}

__attribute ((visibility("default"))) 
int osal_access_ok(const void *addr, unsigned long size)
{
    return 1;
}

__attribute ((visibility("default"))) 
unsigned long osal_virt_to_phys(void *va)
{
	return (unsigned long)(va);
}

__attribute ((visibility("default"))) 
void *osal_phys_to_virt(unsigned long pa)
{
	return (void *)(pa);
}

