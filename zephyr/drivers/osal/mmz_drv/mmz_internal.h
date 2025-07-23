#ifndef _MMZ_INTERNAL_H
#define _MMZ_INTERNAL_H

#include "osal_list.h"

#define MMZ_DBG_LEVEL              0x0
#define MMZ_GRAIN                  4096
#define SA_MMB_NAME_LEN		32
#define SA_MMZ_NAME_LEN     32
#define SZ_1K 1024

#define mmz_align2low(x, g) (((x) / (g)) * (g))
#define mmz_align2(x, g) ((((x) + (g)-1) / (g)) * (g))
#define mmz_grain_align(x) mmz_align2(x, MMZ_GRAIN)
#define mmz_length2grain(len) (mmz_grain_align(len) / MMZ_GRAIN)

struct sa_media_memory_zone {
    char name[SA_MMZ_NAME_LEN];

    unsigned long phys_start;
    unsigned long nbytes;

    //struct osal_list_head list;
    struct osal_list_head mmb_list;
	struct osal_list_head mmb_free_list;

    unsigned int alloc_type;
    unsigned long block_align;
	unsigned int order;
};
typedef struct sa_media_memory_zone sa_mmz_t;

struct mmz_idle_node {
    struct osal_list_head entry;
    unsigned long start;
    unsigned long length;
};
typedef struct mmz_idle_node sa_mmb_free_t;

struct hil_media_memory_block {
    char name[SA_MMB_NAME_LEN];
    struct sa_media_memory_zone *zone;
    struct osal_list_head list;

    unsigned long phys_addr;
    unsigned long length;
};

typedef struct hil_media_memory_block sa_mmb_t;

struct mmz_region {
	unsigned long phys_addr;
	unsigned long size;
	int order;
	struct osal_list_head list;
	sa_mmb_t *mmb;
	char mmb_name[SA_MMB_NAME_LEN];
};

extern sa_mmb_t *rt_mmz_malloc_bf(char *mmz_name, char *mmb_name, int size);
extern void rt_mmz_free_bf(sa_mmb_t *mmb);
#ifdef BSP_SOC_SA6941
extern sa_mmb_t * mmz_spec_reserved(sa_mmz_t *mmz_heap, unsigned long addr, unsigned int size);
extern void osal_mmz_spec_reserved(sa_mmz_t *mmz_heap, unsigned int addr, unsigned int size);
#endif
#endif
