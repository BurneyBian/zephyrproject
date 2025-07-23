#include <string.h>
#include <zephyr/posix/unistd.h>
#include <zephyr/shell/shell.h>
#include "mmz_userdev_cmd.h"
#include "mmz_internal.h"
#include "osal.h"

struct mmz_zone_node {
    struct osal_list_head node;
	sa_mmz_t mmz_heap;
};

static long long sa_max_malloc_size = 0x40000000UL;
OSAL_LIST_HEAD(g_mmz_list);
extern void mmz_heap_cb(char *name, unsigned long phys_addr, unsigned long size);

static void sort_mmz_node(sa_mmz_t *mmz, sa_mmb_free_t *p)
{
    sa_mmb_free_t *node;

    osal_list_for_each_entry(node, &mmz->mmb_free_list, entry) {
    	//osal_printk_inf("====[%d] %lx %lx | %lx %lx\n", __LINE__,node->start, node->length, p->start, p->length);
		if (p->length <= node->length) {
			break;
		}
	}

	osal_list_add(&p->entry, node->entry.prev);
	//osal_printk_inf("====[%d] %lx %lx | %lx %lx\n", __LINE__,node->start, node->length, p->start, p->length);
}

static int do_mmb_alloc_bf(sa_mmb_t *mmb, sa_mmb_free_t *p)
{
	osal_list_add(&mmb->list, &mmb->zone->mmb_list);
	
	if (p->length > mmb->length) {
		p->length -= mmb->length;
		p->start += mmb->length;
		sort_mmz_node(mmb->zone, p);
	} else {
		k_free(p);
	}

	return 0;
}

static  sa_mmb_free_t *find_fixed_region_bf(sa_mmz_t *mmz,
                                       unsigned long size,
                                       unsigned long align) 
{
    unsigned long len = 0;
    sa_mmb_free_t *p, *tmp;

    align = mmz_grain_align(align);
    if (align == 0) {
        align = MMZ_GRAIN;
    }
    len = mmz_grain_align(size);

    osal_list_for_each_entry_safe(p, tmp, &mmz->mmb_free_list, entry) {
		if (p->length >= len) {
			osal_list_del(&p->entry);
			return p;
		}
	}

	return NULL;
}

static sa_mmb_t *__mmb_alloc(const char *name,
                              unsigned long size,
                              unsigned long align,
                              sa_mmz_t *mmz)
{
    sa_mmb_t *mmb = NULL;
    unsigned long fixed_start = 0;
    unsigned long fixed_len = ~1;
    sa_mmz_t *fixed_mmz = NULL;
	sa_mmb_free_t *p = NULL;

    if ((size == 0) || (size > sa_max_malloc_size)) {
        return NULL;
    }
    if (align == 0) {
        align = MMZ_GRAIN;
    }

    size = mmz_grain_align(size);

	p = find_fixed_region_bf(mmz, size, align);
    if (p && (fixed_len > p->length) && p->start) {
        fixed_start = p->start;
        fixed_mmz = mmz;
    }

    if (fixed_mmz == NULL) {
        return NULL;
    }

    mmb = (sa_mmb_t *)k_malloc(sizeof(sa_mmb_t));
    if (mmb == NULL) {
        return NULL;
    }

    memset(mmb, 0, sizeof(sa_mmb_t));
    mmb->zone = fixed_mmz;
    mmb->phys_addr = fixed_start;
    mmb->length = size;

    if (name != NULL) {
        strncpy(mmb->name, name, SA_MMB_NAME_LEN);
    } else {
        strncpy(mmb->name, "<null>", SA_MMB_NAME_LEN);
    }

    if (do_mmb_alloc_bf(mmb, p)) {
        k_free(mmb);
        mmb = NULL;
    }

    return mmb;
}

static void __mmb_free_bf(sa_mmb_t *mmb)
{
	sa_mmb_free_t *left_node = NULL, *right_node = NULL;
	sa_mmb_free_t *node, *p;
	sa_mmz_t *mmz = mmb->zone;

    osal_list_for_each_entry(node, &mmz->mmb_free_list, entry) {
    	//osal_printk_inf("====[%d] %lx %lx | %lx %lx\n", __LINE__,node->start, node->length, mmb->phys_addr, mmb->length);
		if (mmb->phys_addr == (node->start + node->length)) {
			left_node = node;
		} else if ((mmb->phys_addr + mmb->length) == node->start) {
			right_node = node;
		} else if ((mmb->phys_addr + mmb->length) < node->start) {
			break;
		}
	}

	if (left_node && right_node) { //左右合并 
		left_node->length += (mmb->length + right_node->length);
		osal_list_del(&right_node->entry);
		k_free(right_node);
		osal_list_del(&left_node->entry);
		sort_mmz_node(mmz, left_node);
	}
	else if (left_node) { //左合并
		left_node->length += mmb->length;
		osal_list_del(&left_node->entry);
		sort_mmz_node(mmz, left_node);
	}
	else if (right_node) { //右合并
		right_node->length += mmb->length;
		right_node->start = mmb->phys_addr;
		osal_list_del(&right_node->entry);
		sort_mmz_node(mmz, right_node);
	}
	else { //如果不能合并 
		p = (sa_mmb_free_t *)k_malloc(sizeof(sa_mmb_free_t));
		p->start = mmb->phys_addr;
		p->length = mmb->length;
		sort_mmz_node(mmz, p);
	}

    osal_list_del(&mmb->list);
    k_free(mmb);
}

__attribute ((visibility("default"))) 
void rt_mmz_heap_init_bf(char *name, void *begin_addr, void *end_addr)
{
	struct mmz_zone_node *mmz_zone;
	sa_mmb_free_t *mmb_free_node;
    unsigned long begin_align = OSAL_ALIGN((unsigned long)begin_addr, 4096);
    unsigned long end_align   = OSAL_ALIGN_DOWN((unsigned long)end_addr, 4096);
	osal_printk("[%s] enter\n", __func__);
    if (end_align <= begin_align) {
		osal_printk("[%s] start[%lx] >= end[%lx]\n", __func__, (unsigned long)begin_align, (unsigned long)end_align);
		return;
	}

	//osal_printk("[%s] 1\n", __func__);
	mmz_zone = (struct mmz_zone_node *)k_malloc(sizeof(struct mmz_zone_node));
	if (!mmz_zone) {
		osal_printk("[%s] malloc failed\n", __func__);
		return;
	}
	
	//osal_printk("[%s] 2\n", __func__);
	osal_list_add_tail(&mmz_zone->node, &g_mmz_list);
	memset(&mmz_zone->mmz_heap, 0, sizeof(sa_mmz_t));
	//osal_printk("[%s] 3\n", __func__);
	mmz_zone->mmz_heap.phys_start = (unsigned long)begin_addr;
	mmz_zone->mmz_heap.nbytes = (unsigned long)end_addr - (unsigned long)begin_addr;
	//mmz_zone->mmz_heap.order = is_remap;
	strcpy(mmz_zone->mmz_heap.name, name);
	OSAL_INIT_LIST_HEAD(&mmz_zone->mmz_heap.mmb_list);
	//osal_printk("[%s] 4\n", __func__);
	osal_printk("%s [%s]%08lx\n", __func__, mmz_zone->mmz_heap.name, mmz_zone->mmz_heap.phys_start);
	mmz_heap_cb(name, (unsigned long)begin_addr, (unsigned long)end_addr - (unsigned long)begin_addr);
	//osal_printk("[%s] 5\n", __func__);
	OSAL_INIT_LIST_HEAD(&mmz_zone->mmz_heap.mmb_free_list);
	mmb_free_node = (sa_mmb_free_t *)k_malloc(sizeof(sa_mmb_free_t)); 
	//osal_printk("[%s] 6\n", __func__);
	mmb_free_node->start = mmz_zone->mmz_heap.phys_start;
	mmb_free_node->length = mmz_zone->mmz_heap.nbytes;
	osal_list_add_tail(&mmb_free_node->entry, &mmz_zone->mmz_heap.mmb_free_list);
	osal_printk("[%s] exit\n", __func__);
}

static sa_mmz_t *find_mmz_heap(char *name)
{
	struct mmz_zone_node *mmz_zone, *mmz_zone_tmp = NULL;
	sa_mmz_t *mmz_heap;
	int is_found = 0;

	osal_list_for_each_entry(mmz_zone, &g_mmz_list, node){
		if (!mmz_zone_tmp)
			mmz_zone_tmp = mmz_zone;
		//osal_printk_inf("%s [%s][%s]\n", __func__, mmz_zone->mmz_heap.name, name);
		if (!strcmp(mmz_zone->mmz_heap.name, name))
		{
			is_found = 1;
			break;
		}
	}

	if (is_found)
	{
		mmz_heap = &mmz_zone->mmz_heap;
	} else {
		mmz_heap = &mmz_zone_tmp->mmz_heap;
		osal_printk_war("WARNING: your zone's name it not match any zone!, pls check!\n");
	}

	return mmz_heap;
}

sa_mmb_t *rt_mmz_malloc_bf(char *mmz_name, char *mmb_name, int size)
{
	sa_mmz_t *mmz_heap;
	sa_mmb_t *mmb;

	mmz_heap = find_mmz_heap(mmz_name);

    mmb = __mmb_alloc(mmb_name, size, MMZ_GRAIN, mmz_heap);

    if(NULL == mmb )
    {
        osal_printk_err("mmz malloc fail,len=%d\n",size);
    }

    return mmb;
}

void rt_mmz_free_bf(sa_mmb_t *mmb)
{
	if (mmb->phys_addr == 0)
		return;

    __mmb_free_bf(mmb);
}

int mmz_stat(const struct shell *shell, size_t argc, char **argv)
{
	sa_mmb_free_t *node;
	unsigned int total = 0;
	sa_mmz_t *mmz = NULL;

	if (argc == 2)
		mmz = find_mmz_heap(argv[1]);
	if (!mmz)
		mmz = find_mmz_heap("sram");
	if (!mmz)
	{
		osal_printk_war("**************Not find mmz zone**************\n");
		return 0;
	}

	osal_printk_inf("[%s]range: [%lx]----[%lx]\n", mmz->name, mmz->phys_start, mmz->phys_start + mmz->nbytes);
    osal_list_for_each_entry(node, &mmz->mmb_free_list, entry) {
    	osal_printk_inf("[%luKB]--[%08lx-%08lx]\n", node->length/SZ_1K, node->start, node->start + node->length);
		total += node->length;
	}

	return 0;
}

SHELL_CMD_REGISTER(mmz_stat, NULL, "Show the mmz memory fragmentation in the system.", mmz_stat);

#ifdef BSP_SOC_SA6941
//=============for remap start==============//
typedef void (*rmap_cb_t)(unsigned int logic_addr, unsigned int len, unsigned int enable, unsigned int region_len, void *param);

extern void osal_mmz_free(unsigned long paddr);
static unsigned int remap_sram_map, remap_psram_map;

static sa_mmb_free_t *find_fixed_region_reserved_bf(sa_mmz_t *mmz_heap, unsigned long addr, unsigned int size)
{
    sa_mmb_free_t *p, *tmp;

    osal_list_for_each_entry_safe(p, tmp, &mmz_heap->mmb_free_list, entry) {
		if (addr >= p->start && (addr + size) <= (p->start + p->length)) {
			osal_list_del(&p->entry);
			return p;
		}
	}

	return NULL;	
}

static int do_mmb_alloc_reserved_bf(sa_mmb_t *mmb, sa_mmb_free_t *p)
{
	sa_mmb_free_t *mmb_free;
	int need_alloc = 1;
	int free_end = (p->start + p->length);
	int mmb_end = (mmb->phys_addr + mmb->length);

	osal_list_add(&mmb->list, &mmb->zone->mmb_list);

	if ((p->start == mmb->phys_addr) && (p->length == mmb->length)) {
		k_free(p);
		return 0;
	}

	if (p->start < mmb->phys_addr) {
		p->length = (mmb->phys_addr - p->start);
		sort_mmz_node(mmb->zone, p);
		need_alloc = 0;
	}

	if (free_end > mmb_end) {
		if (need_alloc) {
			mmb_free = (sa_mmb_free_t *)k_malloc(sizeof(sa_mmb_free_t));
			mmb_free->start = mmb_end;
			mmb_free->length = (free_end - mmb_end);
			sort_mmz_node(mmb->zone, mmb_free);
		} else {
			p->length -= mmb->length;
			p->start += mmb->length;
			sort_mmz_node(mmb->zone, p);
		}
	}

	return 0;
}

sa_mmb_t * mmz_spec_reserved(sa_mmz_t *mmz_heap, unsigned long addr, unsigned int size)
{
    sa_mmb_t *mmb = NULL;
	sa_mmb_free_t *p = NULL;

    if ((size == 0) || (size > sa_max_malloc_size)) {
        return NULL;
    }
    if (mmz_heap == NULL) {
        return NULL;
    }

	p = find_fixed_region_reserved_bf(mmz_heap, addr, size);
	if (!p)
		osal_printk_err("%s can not find sa_mmb_free\n", __func__);

    mmb = (sa_mmb_t *)k_malloc(sizeof(sa_mmb_t));
    if (mmb == NULL) {
        return NULL;
    }

    memset(mmb, 0, sizeof(sa_mmb_t));
    mmb->zone = mmz_heap;
    mmb->phys_addr = addr;
    mmb->length = size;
	strncpy(mmb->name, "remap_unusable", SA_MMB_NAME_LEN);

    if (do_mmb_alloc_reserved_bf(mmb, p)) {
        k_free(mmb);
        mmb = NULL;
    }

	return mmb;
}

static int get_addr_assign_map (unsigned int start, unsigned int logic_addr, unsigned int region_len)
{
	return (logic_addr - start)/region_len;
}

static void remap_cb(unsigned int logic_addr, unsigned int len, unsigned int enable, unsigned int region_len, void *param)
{
	int i, bit = 0;
	int region_num;
	int mmz_region_len, equ = 0;
	struct mmz_zone_node *zone = (struct mmz_zone_node *)param;
	unsigned int *remap_map;

	//osal_printk_err("%s logic_addr=%x, len=%x, enable=%x, region_len=%x [%lx]\n", __func__, logic_addr, len, enable, region_len, (unsigned long)param);

	if (!zone) {
		osal_printk_err("%s [error] zone is NULL\n", __func__);
		return;
	}

	if (!strstr(zone->mmz_heap.name, "remap")) {
		osal_printk_err("%s [error] zone is Not remap\n", __func__);
		return;
	}

	if (strstr(zone->mmz_heap.name, "psram")) {
		remap_map = &remap_psram_map;
		bit = get_addr_assign_map(0x40000000, logic_addr, region_len);
		mmz_region_len = 0x100000;
	} else {
		remap_map = &remap_sram_map;
		bit = get_addr_assign_map(0x21000000, logic_addr, region_len);
		mmz_region_len = 0x10000;
	}

	equ = (mmz_region_len == region_len) ? 0 : 1;

	//osal_printk_err("%s remap_map=%x bit=%x, equ=%d\n", __func__, *remap_map, bit, equ);
	
	region_num = len/mmz_region_len;

	bit = (1U << bit);

	for (i = 0; i < region_num; i++) {
		if ((enable & bit) && !(*remap_map & bit)) { //应该使能但未被使能
			//osal_printk_err("%s0 logic_addr=%x\n", __func__, logic_addr + i * mmz_region_len);
			osal_mmz_free(logic_addr + i * mmz_region_len);
			if (!equ || i%2) {
				*remap_map |= bit;
			}
		} else if (!(enable & bit) && (*remap_map & bit)) { //应该禁能但未被禁能
			//osal_printk_err("%s1 logic_addr=%x\n", __func__, logic_addr + i * mmz_region_len);
			osal_mmz_spec_reserved(&zone->mmz_heap, logic_addr + i * mmz_region_len, mmz_region_len);
			if (!equ || i%2) {
				*remap_map &= ~(bit);
			}
		}

		if (!equ || i%2) {
			bit = (bit<<1);
		}
	}

}

static void acme_remap_set_unusable(struct mmz_zone_node *zone)
{
	int region_len, i;
	int region_num;
	unsigned long phys_start;

	if (!zone) {
		osal_printk_err("%s [error] zone is NULL\n", __func__);
		return;
	}

	phys_start = zone->mmz_heap.phys_start;

	if (strstr(zone->mmz_heap.name, "psram")) {
		region_len = 0x100000;
		remap_psram_map = 0;
		region_num = zone->mmz_heap.nbytes/region_len;
		if (region_num > 64) {
			zone->mmz_heap.nbytes = 0;
			osal_printk_err("%s [error] length is too large\n", zone->mmz_heap.name);
			return;
		}
	} else {
		region_len = 0x10000;
		remap_sram_map = 0;
		region_num = zone->mmz_heap.nbytes/region_len;
		if (region_num > 32) {
			zone->mmz_heap.nbytes = 0;
			osal_printk_err("%s [error] length is too large\n", zone->mmz_heap.name);
			return;
		}
	}

	for (i = 0; i < region_num; i++) {
		osal_mmz_spec_reserved(&zone->mmz_heap, phys_start + i * region_len, region_len);
	}

}

extern void acme_remap_cb_register(rmap_cb_t cb, char is_psram, void *param);
static void mmz_remap (const struct shell *shell, size_t argc, char **argv)
{
	struct mmz_zone_node *p = NULL;

    osal_list_for_each_entry(p, &g_mmz_list, node) {
		if (strstr(p->mmz_heap.name, "remap")) {
			acme_remap_set_unusable(p);

			if (strstr(p->mmz_heap.name, "psram"))
				acme_remap_cb_register(remap_cb, 1, p);
			else
				acme_remap_cb_register(remap_cb, 0, p);
		}
    }
}

SHELL_CMD_REGISTER(mmz_remap, NULL, "mmz assgin remap", mmz_remap);
//------------for remap end---------------//
#endif
