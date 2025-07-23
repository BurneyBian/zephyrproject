#include "osal.h"

__attribute ((visibility("default"))) 
void *osal_vmalloc(unsigned long size)
{
	unsigned long *p_len;
	char *p = (char *)k_aligned_alloc(8, size + 16);
	if (p) {
		memset((p + 8 + size), 0xaa, 8);
		p_len = (unsigned long *)p;
		*p_len = (unsigned long)(p + 8 + size);
		p += 8;
	}

	return p;
}

__attribute ((visibility("default"))) 
void osal_vfree(const void *addr)
{
	unsigned long *p_len;
	unsigned int *p;

	if (!addr)
		return;

	p_len = (unsigned long *)((unsigned long )addr - 8);
	p = (unsigned int *)*p_len;
	if (*p != 0xaaaaaaaa)
		osal_printk("ERROR:find data overflow [0x%lx]\n", (unsigned long )p);

    k_free((void *)((unsigned long )addr - 8));
}
__attribute ((visibility("default"))) 
void *osal_kmalloc(unsigned long size, unsigned int osal_gfp_flag)
{
    return osal_vmalloc(size);
}
__attribute ((visibility("default"))) 
void *osal_kzalloc(unsigned long size, unsigned int osal_gfp_flag)
{
	char *p;

	p = (char *)osal_vmalloc(size);
	if (p)
		memset(p, 0, size);

   return p;
}
__attribute ((visibility("default"))) 
void osal_kfree(const void *addr)
{
    osal_vfree(addr);
}
__attribute ((visibility("default"))) 
void *osal_kmalloc_align(unsigned long size, unsigned int osal_gfp_flag, unsigned int align)
{
	return k_aligned_alloc(align, size);
}
__attribute ((visibility("default"))) 
void osal_kfree_align(void *addr)
{
    k_free(addr);
}

//*********************************************************//
#if 0

#else
__attribute ((visibility("default"))) 
void *osal_kmalloc_debug(int module, unsigned long size, unsigned int osal_gfp_flag)
{
    return k_malloc(size);
}
__attribute ((visibility("default"))) 
void *osal_kmalloc_debug_ext(int module, char *mod_name, unsigned long size, unsigned int osal_gfp_flag)
{
    return k_malloc(size);
}
__attribute ((visibility("default"))) 
void osal_kfree_debug(int module, const void *addr)
{
    k_free((void *)addr);
}
#endif
__attribute ((visibility("default"))) 
void *osal_vmalloc_debug(int module, unsigned long size)
{
    return osal_kmalloc_debug(module, size, 0);
}
__attribute ((visibility("default"))) 
void *osal_vmalloc_debug_ext(int module, char *mod_name,unsigned long size)
{
    return osal_kmalloc_debug_ext(module, mod_name, size, 0);
}
__attribute ((visibility("default"))) 
void osal_vfree_debug(int module, const void *addr)
{
    osal_kfree_debug(module, (void *)addr);
}
