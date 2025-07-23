#ifndef __SA_OSWL_MMZ_H__
#define __SA_OSWL_MMZ_H__

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>


#include "osal_list.h"
typedef unsigned int            SA_U32;
typedef int                     SA_S32;
typedef unsigned long long      SA_U64;
#define SA_VOID                 void
typedef char                    SA_CHAR;

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define SAL_MMZ_NAME_LEN				32
#define SAL_MMB_NAME_LEN				16

#define SA_MMZ_NAME_LEN           32
#define SA_MMB_NAME_LEN           16
#define SA_POOL_MAX_LEN			  128
#define SA_BLK_MAX_LEN			  128

#define MMB_NAME_LEN           32
#define CACHE_LINE_SIZE        (0x40)
#define MMZ_NAME_LEN           32

typedef SA_U32 SA_POOL;
typedef SA_U32 SA_BLK;

struct mmb_info {
	unsigned long phys_addr;	/* phys-memory address */
	unsigned long align;		/* if you need your phys-memory have special align size */
	unsigned long size;		/* length of memory you need, in bytes */
	unsigned int order;
	int share_key;  /* for shmem */

	void *mapped;			/* userspace mapped ptr */

	union {
		struct {
			unsigned long prot  :8;	/* PROT_READ or PROT_WRITE */
			unsigned long flags :12;/* MAP_SHARED or MAP_PRIVATE */
      unsigned long reserved : 8; /* reserved, do not use */
      unsigned long delayed_free : 1;
      unsigned long map_cached : 1;
		};
		unsigned long w32_stuf;
	};

	char mmb_name[MMB_NAME_LEN];
	char mmz_name[MMZ_NAME_LEN];
	unsigned long gfp;		/* reserved, do set to 0 */

    //struct osal_list_head list;
   // hil_mmb_t *mmb;
    int map_ref;
    int mmb_ref;
};

struct dirty_area
{
	unsigned long dirty_phys_start;	/* dirty physical address */
	unsigned long dirty_virt_start;	/* dirty virtual  address, must be coherent with dirty_phys_addr */
	unsigned long dirty_size;
};

#define MASTER_PORT_NUM   8 //7+1(add one virtual DDR salve prot all）
#define SUB_PORT_MAX_NUM   15

#define MPORT_ID_LEN   10
#define SPORT_ID_LEN   13

struct port_result
{
    unsigned int port;
    unsigned int RCFG;
    unsigned int WCFG;
    long long RADDR_TX_NUM;
    long long RDATA_TX_NUM;
    long long WADDR_TX_NUM;
    long long WDATA_TX_NUM;
	long long TOT_RD_LATENCY;
	long long TOT_WR_LATENCY;
	float RDATA_THROUGHPUT;
	float WDATA_THROUGHPUT;
    float AVG_READ_BURST_LEN;
    float AVG_WRITE_BURST_LEN;
	long long  AVG_RD_LATENCY;
	long long AVG_WR_LATENCY;
};

#define USERDMA_MAX_CH 4
#define MAX_LLI_NUM   16


struct userdma_lli {
    unsigned int src;
    unsigned int dst;
    unsigned int len;
};

struct userdma_user_info {
    unsigned int ch;
    unsigned int listnr;
    struct userdma_lli llis[MAX_LLI_NUM];
};

typedef struct pool_config {
    SA_U64 u64BlkSize;
    SA_U32 u32BlkCnt;
    SA_CHAR acMmbName[SAL_MMB_NAME_LEN];
} sa_pool_config_s;

typedef struct block{
	SA_POOL	poolId;
	SA_U32	blockId;
	SA_U32	blockSize;
	SA_U32	used;
	SA_U64  size;
	SA_U64  phys_addr; /* phys-memory address */
	struct block	*pNext;
}sa_block_s;

typedef struct pool{
	SA_U64  poolSize;
	SA_POOL	poolId;
	SA_U32	totalBlkCnt;
	SA_U32  usedBlkCnt;
	SA_U64  freesize;
	SA_BLK  blockID[SA_BLK_MAX_LEN];
	SA_U64  phys_addr; /* phys-memory address */
	struct pool *pNext;
}sa_pool_s;

typedef struct sa_oswl_mmb
{
	SA_POOL poolId;
	struct mmb_info Mmbinfo;
	struct sa_oswl_mmb *pNext;
}sa_mmb_info_s;


SA_S32 sa_oswl_mmz_OpenMmzDev(SA_VOID);

SA_S32 sa_oswl_mmz_CloseMmzDev(SA_S32 fd);

SA_S32 sa_oswl_mmz_GetShmKey(SA_S32 mmz_fd, struct mmb_info *mmb_info);

SA_S32 sa_oswl_mmz_RemoveShmKey(SA_S32 mmz_fd, SA_S32 num);

SA_U32 sa_oswl_mmz_AllocMmb(SA_S32 mmz_fd, struct mmb_info *info);

SA_U32 sa_oswl_mmz_FreeMmb(SA_S32 mmz_fd, struct mmb_info *mmb_info);

SA_U32 sa_oswl_mmz_FreeKernelMmb(SA_S32 mmz_fd, struct mmb_info *mmb_info);

SA_S32 sa_oswl_mmz_RemapMmb(SA_S32 fd, struct mmb_info *info, SA_S32 cached);

SA_S32 sa_oswl_mmz_UnremapMmb( SA_S32 fd, struct mmb_info *info);

SA_S32 sa_oswl_mmz_FlushCache( SA_S32 fd, struct dirty_area *area);

SA_S32 sa_oswl_mmz_OpenMemDev( SA_VOID );

SA_S32 sa_oswl_mmz_CloseMemDev( SA_S32 fd);

SA_VOID* sa_oswl_mmz_MapMem( SA_S32 fd, SA_U32 paddr, SA_U32 size);

SA_S32 sa_oswl_mmz_UnmapMem(SA_VOID* vaddr, SA_U32 size);

SA_S32 sa_oswl_mmz_SetReg(SA_S32 fd, SA_U32 u32Addr, SA_U32 u32Value );

SA_S32 sa_oswl_mmz_GetReg(SA_S32 fd, SA_U32 u32Addr, SA_U32 *pu32Value );

SA_U32 sa_oswl_mmz_remap_mode(SA_S32 mmz_fd, int is_debug);
/*SA_S32 sa_oswl_mmz_Init(SA_VOID);

SA_VOID sa_oswl_mmz_Dinit(SA_VOID);

SA_U32 sa_oswl_mmz_AllocMmb(struct mmb_info *info);

SA_U32 sa_oswl_mmz_FreeMmb(struct mmb_info *mmb_info);

SA_S32 sa_oswl_mmz_RemapMmb(struct mmb_info *info, SA_S32 cached);

SA_S32 sa_oswl_mmz_UnremapMmb(struct mmb_info *info);

SA_S32 sa_oswl_mmz_FlushCache(struct dirty_area *area);

SA_VOID* sa_oswl_mmz_MapMem(SA_U32 paddr, SA_U32 size);

SA_S32 sa_oswl_mmz_SetReg(SA_U32 u32Addr, SA_U32 u32Value );

SA_S32 sa_oswl_mmz_GetReg(SA_U32 u32Addr, SA_U32 *pu32Value );*/




SA_POOL sa_oswl_mmz_CreatePoolMemoey(sa_pool_config_s *pstPoolCfg);

SA_S32 sa_oswl_mmz_DestroyPool(SA_POOL Pool);

SA_BLK sa_oswl_mmz_GetBlock(SA_POOL Pool);

SA_S32 sa_oswl_mmz_ReleaseBlock(SA_BLK Block , SA_POOL Pool);

SA_U64 sa_oswl_mmz_GetBlockPhysAddr(SA_BLK Block, SA_POOL Pool);

SA_POOL sa_oswl_mmz_GetPoolId(SA_BLK Block);

#ifdef BSP_DDR_LESS
#define sa_oswl_mmz_sram_OpenDev sa_oswl_mmz_OpenMmzDev
#else
SA_S32 sa_oswl_mmz_sram_OpenDev(SA_VOID);
#endif
//SA_POOL sa_oswl_mmz_sram_CreatePoolMemoey(sa_pool_config_s *pstPoolCfg);
#define sa_oswl_mmz_sram_CreatePoolMemoey sa_oswl_mmz_CreatePoolMemoey

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* __SA_OSWL_MMZ_H__ */
