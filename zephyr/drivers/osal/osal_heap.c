/*
 * Copyright (c) 2022-2025, acme system software Team
 *
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-6-30     wth       the first version
 */
#include <zephyr/kernel.h>
#include "osal.h"
#include "osal_list.h"

OSAL_LIST_HEAD(osal_list);

struct osal_zone_node {
    struct osal_list_head node;
	char name[32];
	unsigned long addr;
	unsigned long len;
	void *osal_heap;
};

void sdk_osal_heap_init(char *name, void *begin_addr, void *end_addr)
{
	struct osal_zone_node *osal_zone;
	struct k_heap *osal_heap;
    unsigned long begin_align = OSAL_ALIGN((unsigned long)begin_addr, 4096);
    unsigned long end_align   = OSAL_ALIGN_DOWN((unsigned long)end_addr, 4096);

	osal_zone = (struct osal_zone_node *)k_malloc(sizeof(struct osal_zone_node));
	if (!osal_zone)
		return;

	osal_heap = (struct k_heap *)k_malloc(sizeof(struct k_heap));
	if (!osal_heap) {
		k_free(osal_zone);
		return;
	}
 
	osal_zone->addr = (unsigned long)begin_addr;
	osal_zone->len = ((unsigned long)end_addr - (unsigned long)begin_addr);
	osal_zone->osal_heap = osal_heap;
	strcpy(osal_zone->name, name);
	osal_list_add_tail(&osal_zone->node, &osal_list);

	k_heap_init(osal_heap, begin_addr, end_align - begin_align);
}

static void *find_osal_heap(char *name)
{
	struct osal_zone_node *osal_zone, *osal_zone_tmp = NULL;
	void *osal_heap = NULL;
	int is_found = 0;

	osal_list_for_each_entry(osal_zone, &osal_list, node){
		if (!osal_zone_tmp)
			osal_zone_tmp = osal_zone;
		if (!strcmp(osal_zone->name, name))
		{
			is_found = 1;
			break;
		}
	}

	if (is_found)
	{
		osal_heap = osal_zone->osal_heap;
	} else {
		osal_heap = osal_zone_tmp->osal_heap;
		osal_printk_war("WARNING: your zone's name it not match any zone!, pls check!\n");
	}

	return osal_heap;
}

void *sdk_osal_malloc(char *name, int size)
{
    unsigned int level;
    void *ptr;
	struct k_heap *osal_heap;

    level = irq_lock();

	osal_heap = (struct k_heap *)find_osal_heap(name);
    ptr = k_heap_alloc(osal_heap, size, K_FOREVER);

    irq_unlock(level);

    if(0== ptr )
    {
        osal_printk_err("osal malloc fail,len=%d\n",size);
    }

    return ptr;
}

void *sdk_osal_realloc(char *name, void *rmem, int newsize)
{
    unsigned int level;
    void *ptr;
	struct k_heap *osal_heap;

    level = irq_lock();

	osal_heap = (struct k_heap *)find_osal_heap(name);
    ptr = k_heap_realloc(osal_heap, rmem, newsize, K_FOREVER);

    irq_unlock(level);

    if(0== ptr )
    {
        osal_printk_err("osal malloc fail,len=%d\n", newsize);
    }

    return ptr;
}

void sdk_osal_free(char *name, void *rmem)
{
    unsigned int level;
	struct k_heap *osal_heap;

    if (rmem == NULL) return;

    level = irq_lock();
	osal_heap = (struct k_heap *)find_osal_heap(name);
    k_heap_free(osal_heap, rmem);
    irq_unlock(level);
}

void *sdk_osal_malloc_align(char *name, int size, int align)
{
    unsigned int level;
    void *ptr;
	struct k_heap *osal_heap;

    level = irq_lock();

	osal_heap = (struct k_heap *)find_osal_heap(name);
    ptr = k_heap_aligned_alloc(osal_heap, align, size, K_FOREVER);

    irq_unlock(level);

    return ptr;
}

void sdk_osal_free_align(char *name, void *ptr)
{
    sdk_osal_free(name, ptr);
}
