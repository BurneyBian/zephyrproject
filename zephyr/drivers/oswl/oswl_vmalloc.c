#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <rtthread.h>
#include "oswl.h"
#include <string.h>

#define OSWL_MALLOC_S1 8
#define OSWL_MALLOC_S2 8
#define OSWL_MALLOC_S (OSWL_MALLOC_S1 + OSWL_MALLOC_S2)
#define OSWL_MALLOC_E 8
#define OSWL_MALLOC_OFF (OSWL_MALLOC_S + OSWL_MALLOC_E)

#ifdef RT_USING_OSWL_HEAP
extern void *rt_oswl_malloc(unsigned long size);
extern void *rt_oswl_realloc(void *ptr, unsigned long newsize);
extern void *rt_oswl_calloc(unsigned long count, unsigned long size);
extern void rt_oswl_free(void *ptr);
extern void *rt_oswl_malloc_align(unsigned long size, unsigned long align);
extern void rt_oswl_free_align(void *ptr);

#define __oswl_malloc 		rt_oswl_malloc
#define __oswl_realloc 		rt_oswl_realloc
#define __oswl_calloc 		rt_oswl_calloc
#define __oswl_free 		rt_oswl_free
#define __oswl_malloc_align	rt_oswl_malloc_align
#define __oswl_free_align 	rt_oswl_free_align
#else
#define __oswl_malloc 		rt_malloc
#define __oswl_realloc 		rt_realloc
#define __oswl_calloc 		rt_calloc
#define __oswl_free 		rt_free
#define __oswl_malloc_align	rt_malloc_align
#define __oswl_free_align 	rt_free_align
#endif

void *oswl_malloc(unsigned long size)
{
	unsigned long *p_len;
	size = (size + sizeof(unsigned long) - 1)/sizeof(unsigned long) * sizeof(unsigned long);
	char *p = (char *)__oswl_malloc(size + OSWL_MALLOC_OFF);
	if (p) {
		rt_memset((p + OSWL_MALLOC_S + size), 0xaa, OSWL_MALLOC_E);
		rt_memset((p + OSWL_MALLOC_S1), 0xaa, OSWL_MALLOC_S2);
		p_len = (unsigned long *)p;
		*p_len = (unsigned long)(p + OSWL_MALLOC_S + size);
		p += OSWL_MALLOC_S;
	}

	return p;
}
RTM_EXPORT(oswl_malloc);

void oswl_free(const void *addr)
{
	unsigned long *p_len;
	unsigned int *p_right, *p_magic;

	if (!addr)
		return;

	p_len = (unsigned long *)((unsigned long )addr - OSWL_MALLOC_S);
	p_magic = (unsigned int *)((unsigned long )addr - OSWL_MALLOC_S2);

	if (*p_magic != 0xaaaaaaaa)
	{
		OSWL_LOG_E("ERROR:find data left overflow [0x%lx]\n", (unsigned long )addr);
		__oswl_free((void *)((unsigned long )addr - OSWL_MALLOC_S));
		return;
	}

	p_right = (unsigned int *)*p_len;
	if (*p_right != 0xaaaaaaaa)
		OSWL_LOG_E("ERROR:find data right overflow [0x%lx]\n", (unsigned long )addr);

    __oswl_free((void *)((unsigned long )addr - OSWL_MALLOC_S));
}
RTM_EXPORT(oswl_free);

void *oswl_calloc(int count, unsigned long size)
{
   char *p;

	size = size * count;
	p = (char *)oswl_malloc(size);

	if (p)
		rt_memset(p, 0, size);

   return p;
}
RTM_EXPORT(oswl_calloc);

void *oswl_realloc(void* addr, unsigned long size)
{
    char *p = (char *)oswl_malloc(size);
	if (addr && p) {
		rt_memcpy(p, (char *)addr, size);
	}

	if (addr)
		oswl_free(addr);

	return p;
}
RTM_EXPORT(oswl_realloc);

#ifdef OSWL_VM_DEBUG
void *oswl_malloc_debug(int module, unsigned long size)
{
	int fd;
	char strr[32];
	void *p = __oswl_malloc(size);
	strr[0] = 'a';
	strr[1] = ':';
	strr[31] = 0;
	
	rt_sprintf(&strr[2],"%02x\n", module);
	strr[4] = ':';
	rt_sprintf(&strr[5],"%lx\n", (unsigned long)p);
	strr[21] = ':';
	rt_sprintf(&strr[22],"%08x\n", (unsigned int)size);
	//OSWL_LOG_I("%s %d\n", strr, strlen(strr));
	fd = open("/proc/m_uinfo",O_WRONLY);
	write(fd, strr, 31);
	close(fd);

    return p;
}

void *oswl_malloc_debug_ext(int module, char *mod_name, unsigned long size)
{
	int fd;
	char strr[64];
	void *p = __oswl_malloc(size);
	strr[0] = 'a';
	strr[1] = ':';
	strr[63] = 0;

	rt_sprintf(&strr[2],"%02x\n", module);
	strr[4] = ':';
	rt_sprintf(&strr[5],"%lx\n", (unsigned long)p);
	strr[21] = ':';
	rt_sprintf(&strr[22],"%08x\n", (unsigned int)size);
	rt_strncpy(strr+32, mod_name, 32);
	//OSWL_LOG_I("%s %d\n", strr, strlen(strr));
	fd = open("/proc/m_uinfo",O_WRONLY);
	write(fd, strr, 64);
	close(fd);

    return p;
}

void oswl_free_debug(int module, const void *addr)
{
	int fd;
	char ftrr[24];

	__oswl_free((void *)addr);

	ftrr[0] = 'f';
	ftrr[1] = ':';
	ftrr[21] = 0;
	
	rt_sprintf(&ftrr[2],"%02x\n", module);
	ftrr[4] = ':';
	rt_sprintf(&ftrr[5],"%lx\n", (unsigned long)addr);
	//OSWL_LOG_I("%s %d\n", ftrr, strlen(ftrr));
	fd = open("/proc/m_uinfo",O_WRONLY);
	write(fd, ftrr, 22);
	close(fd);
}

void *oswl_calloc_debug(int module, int count, unsigned long size)
{
	int fd;
	char strr[32];
	void *p = __oswl_calloc(count, size);
	strr[0] = 'a';
	strr[1] = ':';
	strr[31] = 0;
	
	rt_sprintf(&strr[2],"%02x\n", module);
	strr[4] = ':';
	rt_sprintf(&strr[5],"%lx\n", (unsigned long)p);
	strr[21] = ':';
	rt_sprintf(&strr[22],"%08x\n", (unsigned int)(count*size));

	fd = open("/proc/m_uinfo",O_WRONLY);
	write(fd, strr, 31);
	close(fd);

    return p;
}
#else

void *oswl_malloc_debug(int module, unsigned long size)
{
	return __oswl_malloc(size);
}

void *oswl_malloc_debug_ext(int module, char *mod_name ,unsigned long size)
{
	return __oswl_malloc(size);
}
void oswl_free_debug(int module, const void *addr)
{
	__oswl_free((void *)addr);
}

void *oswl_calloc_debug(int module, int count, unsigned long size)
{
	return __oswl_calloc(count, size);
}
#endif