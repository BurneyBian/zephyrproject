//hanqiyi temp #include <dfs_posix.h>

#include <zephyr/posix/unistd.h>
#include <zephyr/posix/fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../sa_oswl_mmz.h"

////////////////////////////  COPY FROM MMZ-DRIVER(mmz-userdev.h)  ////////////////////////////
#ifndef _DRV_fcntl_H
#define _IOC_NRBITS    8
#define _IOC_TYPEBITS  8

#ifndef _IOC_SIZEBITS
#define _IOC_SIZEBITS  14
#endif

#define _IOC_NRSHIFT    0
#define _IOC_TYPESHIFT    (_IOC_NRSHIFT+_IOC_NRBITS)
#define _IOC_NRMASK    ((1 << _IOC_NRBITS)-1)
#define _IOC_TYPEMASK    ((1 << _IOC_TYPEBITS)-1)
#define _IOC_SIZESHIFT    (_IOC_TYPESHIFT+_IOC_TYPEBITS)
#define _IOC_SIZEMASK    ((1 << _IOC_SIZEBITS)-1)
#define _IOC_DIRSHIFT    (_IOC_SIZESHIFT+_IOC_SIZEBITS)

#define _IOC_TYPE(nr)        (((nr) >> _IOC_TYPESHIFT) & _IOC_TYPEMASK)
#define _IOC_NR(nr)        (((nr) >> _IOC_NRSHIFT) & _IOC_NRMASK)
#define _IOC_SIZE(nr)        (((nr) >> _IOC_SIZESHIFT) & _IOC_SIZEMASK)
#endif

#define _IOC(dir, type, nr, size)    \
    (((dir) << _IOC_DIRSHIFT) |  \
        ((type) << _IOC_TYPESHIFT) |  \
        ((nr) << _IOC_NRSHIFT) |  \
        ((size) << _IOC_SIZESHIFT))

#ifndef _IOC_NONE
#define _IOC_NONE      0U
#endif

#ifndef _IOC_WRITE
#define _IOC_WRITE     1U
#endif

#ifndef _IOC_READ
#define _IOC_READ      2U
#endif

#define _IOC_TYPECHECK(t) (sizeof(t))

#define _IO(type,nr)        _IOC(_IOC_NONE,(type),(nr),0)
#define _IOR(type,nr,size)    _IOC(_IOC_READ,(type),(nr),(_IOC_TYPECHECK(size)))
#define _IOW(type,nr,size)    _IOC(_IOC_WRITE,(type),(nr),(_IOC_TYPECHECK(size)))
#define _IOWR(type,nr,size)    _IOC(_IOC_READ|_IOC_WRITE,(type),(nr),(_IOC_TYPECHECK(size)))
#define _IOR_BAD(type,nr,size)    _IOC(_IOC_READ,(type),(nr),sizeof(size))
#define _IOW_BAD(type,nr,size)    _IOC(_IOC_WRITE,(type),(nr),sizeof(size))
#define _IOWR_BAD(type,nr,size)    _IOC(_IOC_READ|_IOC_WRITE,(type),(nr),sizeof(size))

#define IOC_IN        (_IOC_WRITE << _IOC_DIRSHIFT)
#define IOC_OUT        (_IOC_READ << _IOC_DIRSHIFT)
#define IOC_INOUT    ((_IOC_WRITE|_IOC_READ) << _IOC_DIRSHIFT)
#define IOCSIZE_MASK    (_IOC_SIZEMASK << _IOC_SIZESHIFT)
#define IOCSIZE_SHIFT    (_IOC_SIZESHIFT)

#define IOC_MMB_ALLOC					_IOWR('m', 10,  struct mmb_info)
#define IOC_MMB_ATTR					_IOR('m',  11,  struct mmb_info)
#define IOC_MMB_FREE					_IOW('m',  12,  struct mmb_info)
#define IOC_MMB_ALLOC_V2				_IOWR('m', 13,  struct mmb_info)
#define IOC_MMB_FREE_KERNEL				_IOW('m',  14,  struct mmb_info)

#define IOC_MMB_USER_REMAP				_IOWR('m', 20,  struct mmb_info)
#define IOC_MMB_USER_REMAP_CACHED 		_IOWR('m', 21,  struct mmb_info)
#define IOC_MMB_USER_UNMAP				_IOWR('m', 22,  struct mmb_info)

#define IOC_MMB_VIRT_GET_PHYS			_IOWR('m',  23,  struct mmb_info)

#define IOC_MMB_ADD_REF					_IO('r', 30)	/* fcntl(file, cmd, arg), arg is mmb_addr */
#define IOC_MMB_DEC_REF					_IO('r', 31)	/* fcntl(file, cmd, arg), arg is mmb_addr */

#define IOC_MMB_FLUSH_DCACHE			_IO('c', 40)

#define IOC_MMB_FLUSH_DCACHE_DIRTY		_IOW('d', 50, struct dirty_area)
#define IOC_MMB_INV_DCACHE				_IOW('d', 51, struct dirty_area)
#define IOC_MMB_TEST_CACHE				_IOW('t',  11,  struct mmb_info)

#define IOC_MMB_GET_SHKEY      _IOWR('s', 11, struct mmb_info)
#define IOC_MMB_RM_SHKEY       _IOW('s', 12, int)
////////////////////////////////////////////  END  ////////////////////////////////////////////


#ifndef	PAGE_SHIFT
#define PAGE_SHIFT						12
#endif
#ifndef	PAGE_SIZE
#define PAGE_SIZE						( 1UL << PAGE_SHIFT )
#endif
#ifndef	PAGE_MASK
#define PAGE_MASK						(~( PAGE_SIZE - 1 ))
#endif


#define MMZ_DEVICE					"/dev:/mmz_dev"
#define MMZ_SRMA_DEVICE				"/dev:/mmz_sram_dev"
#define	MEM_DEVICE					"mem"


#define MODULE_TAG  "oswl mmz"

extern int fcntl(int fildes, int cmd, ...);
extern int close(int fd);

SA_BLK		g_blockID = 0;
SA_POOL		g_poolID  = 0;
sa_pool_s  *g_poolList= NULL;
sa_block_s *g_blockList = NULL;
sa_mmb_info_s *g_mmblist = NULL;

static int g_mmzFD = -1;

int sa_oswl_mmz_OpenMmzDev(void)
{
	int s32Ret;
	printk("[%s] enter\n", __func__);
	s32Ret = open(MMZ_DEVICE, O_RDWR,  0);

	g_mmzFD = s32Ret;
	printk("[%s] exit\n", __func__);
	return (int)s32Ret;
}

int sa_oswl_mmz_sram_OpenDev(void)
{
	int s32Ret;
	printk("[%s] enter\n", __func__);
	s32Ret = open(MMZ_SRMA_DEVICE, O_RDWR,  0);

	g_mmzFD = s32Ret;
	printk("[%s] exit\n", __func__);
	return (int)s32Ret;
}

int sa_oswl_mmz_CloseMmzDev(int fd)
{
	int s32Ret = 0;

	s32Ret = close(fd);

	return s32Ret;
}

int sa_oswl_mmz_GetShmKey(int mmz_fd, struct mmb_info *mmb_info)
{
	return fcntl(mmz_fd,IOC_MMB_GET_SHKEY, mmb_info);
}

int sa_oswl_mmz_RemoveShmKey(int mmz_fd, int num)
{
	return fcntl(mmz_fd,IOC_MMB_RM_SHKEY, &num);
}

int sa_oswl_mmz_Init(void)
{
	int s32Ret = 0;

	if(g_mmzFD > 0)//mmz dev has opend
	{
		return 0;
	}

	s32Ret = sa_oswl_mmz_OpenMmzDev();
	if(s32Ret < 0)
	{
		printk("sa_oswl_mmz_Init failed\n");
	}

	g_mmzFD = s32Ret;

	return 0;
}

void sa_oswl_mmz_Dinit(void)
{
	sa_oswl_mmz_CloseMmzDev(g_mmzFD);
}

unsigned int sa_oswl_mmz_AllocMmb(int mmz_fd, struct mmb_info *info)
{
	int s32Ret = 0;

	/*malloc memory for pool*/
	s32Ret = fcntl(mmz_fd,IOC_MMB_ALLOC, info);

	return s32Ret;
}

unsigned int sa_oswl_mmz_FreeMmb(int mmz_fd, struct mmb_info *mmb_info)
{
	int s32Ret = 0;
	/*malloc memory for pool*/

	s32Ret = fcntl(mmz_fd,IOC_MMB_FREE,mmb_info);

	return s32Ret;
}

unsigned int sa_oswl_mmz_FreeKernelMmb(int mmz_fd, struct mmb_info *mmb_info)
{
	int s32Ret = 0;
	/*malloc memory for pool*/

	s32Ret = fcntl(mmz_fd,IOC_MMB_FREE_KERNEL,mmb_info);

	return s32Ret;
}

int sa_oswl_mmz_GetMmbInfo( int mmz_fd, struct mmb_info *mmb_info)
{
	int s32Ret = 0;
	/*malloc memory for pool*/

	s32Ret = fcntl(mmz_fd,IOC_MMB_ATTR,mmb_info);

	return s32Ret;
}


int sa_oswl_mmz_RemapMmb(int fd, struct mmb_info *info, int cached)
{
    int ret;

    if (cached == 0)
		ret = fcntl( fd, IOC_MMB_USER_REMAP, info);
    else
		ret = fcntl( fd, IOC_MMB_USER_REMAP_CACHED, info);

    return ret;
}

int sa_oswl_mmz_UnremapMmb( int fd, struct mmb_info *info)
{
    int ret;

	ret = fcntl( fd, IOC_MMB_USER_UNMAP, info);

    return ret;
}


int sa_oswl_mmz_FlushCache( int fd, struct dirty_area *area)
{
    int ret;

	ret = fcntl( fd, IOC_MMB_FLUSH_DCACHE_DIRTY, area );

    return ret;
}

int sa_oswl_mmz_InvalidCache( int fd, struct dirty_area *area)
{
    int ret;

	ret = fcntl( fd, IOC_MMB_INV_DCACHE, area );

    return ret;
}

#if 0
int sa_oswl_mmz_OpenMemDev( void )
{
    int ret;
	ret = open( MEM_DEVICE, ( O_RDWR ));
    return ret;

}

int sa_oswl_mmz_CloseMemDev( int fd)
{
    int ret;
	ret = close(fd);
    return ret;
}

void* sa_oswl_mmz_MapMem( int fd, unsigned int paddr, unsigned int size)
{
	void *pVirAddr = (void *)paddr;

	if( paddr & ~PAGE_MASK )
	{
		paddr &= PAGE_MASK;
	}

	if( size & ~PAGE_MASK )
	{
		size += PAGE_SIZE - 1;
		size &= PAGE_MASK;
	}

	pVirAddr = (void *)mmap((void *)0, size, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, paddr );
    return pVirAddr;
}

int sa_oswl_mmz_UnmapMem(void* vaddr, unsigned int size)
{
    int ret = 0;

	if( size & ~PAGE_MASK )
	{
		size += PAGE_SIZE - 1;
		size &= PAGE_MASK;
	}
	ret = munmap( vaddr, size);
    return ret;
}
#endif

int sa_oswl_mmz_SetReg(int fd, unsigned int u32Addr, unsigned int u32Value )
{
	int ret = 0;
	char *pVirAddr = NULL;
	unsigned int page_addr = 0, page_offset = 0;

	page_addr   = u32Addr & PAGE_MASK;
	page_offset = u32Addr & ~PAGE_MASK;


	pVirAddr = (char *)(unsigned long)page_addr;

	if( pVirAddr == (void*)-1)
	{
		printk("Unable to mmap page 0x%08x\n", page_addr);
		return -1;
	}

	*((unsigned int *)( pVirAddr + page_offset )) = u32Value;

	return ret;
}

int sa_oswl_mmz_GetReg(int fd, unsigned int u32Addr, unsigned int *pu32Value )
{
	int ret = 0;
	char *pVirAddr = NULL;
	unsigned int page_addr = 0, page_offset = 0;

	if( !pu32Value )
	{
		printk("val addr is NULL\n");
		return -1;
	}

	if( fd < 0 )
	{
		printk("fd is NULL\n");
		return -1;
	}

	page_addr   = u32Addr & PAGE_MASK;
	page_offset = u32Addr & ~PAGE_MASK;

	pVirAddr = (char *)(unsigned long)page_addr;

	if( pVirAddr == (void*)-1)
	{
		printk("Unable to mmap page 0x%08x\n", page_addr);
		return -1;
	}

	*pu32Value = *((unsigned int *)( pVirAddr + page_offset ));

	return ret;
}

sa_mmb_info_s *sa_oswl_mmz_CreateMmbinfolist(void)
{
	sa_mmb_info_s *headNode = (sa_mmb_info_s *)malloc(sizeof(sa_mmb_info_s));
	if(NULL == headNode)
	{
		printk("malloc mem for blocklist failed\n");
		return NULL;
	}
	headNode->pNext = NULL;

	return headNode;
}

sa_mmb_info_s *sa_oswl_mmz_CreateMmbinfoNode(struct mmb_info *Mmbinfo)
{
	sa_mmb_info_s *newMmbinfoNode = (sa_mmb_info_s *)malloc(sizeof(sa_mmb_info_s));
	if(NULL == newMmbinfoNode)
	{
		printk("create newMmbinfoNode failed\n");
		return NULL;
	}
	memset(newMmbinfoNode, 0, sizeof(sa_mmb_info_s));

	newMmbinfoNode->poolId = g_poolID;

	/*todo : cp MmbinfoNode to */
	newMmbinfoNode->Mmbinfo.phys_addr = Mmbinfo->phys_addr;
	newMmbinfoNode->Mmbinfo.size = Mmbinfo->size;
	newMmbinfoNode->Mmbinfo.align = Mmbinfo->align;
	newMmbinfoNode->Mmbinfo.flags = Mmbinfo->flags;
	newMmbinfoNode->Mmbinfo.order = Mmbinfo->order;
	memcpy(newMmbinfoNode->Mmbinfo.mmb_name, Mmbinfo->mmb_name, strlen(Mmbinfo->mmb_name));

	return newMmbinfoNode;
}

int sa_oswl_mmz_InsertMmbinfoNode(sa_mmb_info_s * MmbinfoNode, struct mmb_info *Mmbinfo)
{
	sa_mmb_info_s *newMmbinfoNode = sa_oswl_mmz_CreateMmbinfoNode(Mmbinfo);
	sa_mmb_info_s *pTaiMmbinfoNodel = MmbinfoNode;

	while(NULL != pTaiMmbinfoNodel->pNext)
	{
		pTaiMmbinfoNodel = pTaiMmbinfoNodel->pNext;
	}

	pTaiMmbinfoNodel->pNext = newMmbinfoNode;
	newMmbinfoNode->pNext = NULL;

	printk("pTailPool->pNext->poolId = %d \n", pTaiMmbinfoNodel->pNext->poolId);

	return 0;
}


sa_pool_s *sa_oswl_mmz_CreatePoolList(void)
{
	sa_pool_s *headNode = (sa_pool_s *)malloc(sizeof(sa_pool_s));
	if(NULL == headNode)
	{
		printk("malloc mem for blocklist failed\n");
		return NULL;
	}
	headNode->pNext = NULL;

	return headNode;
}

sa_pool_s *sa_oswl_mmz_CreatePoolNode(sa_pool_s * poolNode)
{
	sa_pool_s *newPoolNode = (sa_pool_s *)malloc(sizeof(sa_pool_s));
	if(NULL == newPoolNode)
	{
		printk("create newPoolNode failed\n");
		return NULL;
	}
	memset(newPoolNode, 0, sizeof(sa_pool_s));

	newPoolNode->poolId = poolNode->poolId;
	newPoolNode->totalBlkCnt = poolNode->totalBlkCnt;
	newPoolNode->poolSize = poolNode->poolSize;

	return newPoolNode;

}

int sa_oswl_mmz_InsertPoolNode(sa_pool_s * pPool, sa_pool_s *poolNode)
{
	sa_pool_s *newPoolNode = sa_oswl_mmz_CreatePoolNode(poolNode);
	sa_pool_s *pTailPool = pPool;

	while(NULL != pTailPool->pNext)
	{
		pTailPool = pTailPool->pNext;
	}

	pTailPool->pNext = newPoolNode;
	newPoolNode->pNext = NULL;

	printk("pTailPool->pNext->poolId = %d \n", pTailPool->pNext->poolId);

	return 0;
}

sa_block_s *sa_oswl_mmz_CreateBlockList(void)
{
	sa_block_s *headNode = (sa_block_s *)malloc(sizeof(sa_block_s));
	if(NULL == headNode)
	{
		printk("malloc mem for blocklist failed\n");
		return NULL;
	}
	headNode->pNext = NULL;

	return headNode;
}

sa_block_s *sa_oswl_mmz_CreateBlockNode(sa_block_s *pblock)
{
	sa_block_s *blockNode = (sa_block_s *)malloc(sizeof(sa_block_s));
	if(NULL == blockNode)
	{
		printk("create blocknode failed\n");
		return NULL;
	}
	memset(blockNode, 0, sizeof(sa_block_s));

	blockNode->blockId = pblock->blockId;
	blockNode->blockSize = pblock->blockSize;
	blockNode->phys_addr = pblock->phys_addr;
	blockNode->poolId = pblock->poolId;
	blockNode->size = pblock->size;
	blockNode->used = pblock->used;
	blockNode->pNext = NULL;

	return blockNode;

}

void sa_oswl_mmz_InsertBlockNode(sa_block_s *pblock, sa_block_s *pblockNode)
{
	sa_block_s* pNewBlock = sa_oswl_mmz_CreateBlockNode(pblockNode);
	sa_block_s* pTailBlock = pblock;

	while(NULL != pTailBlock->pNext)
	{
		pTailBlock = pTailBlock->pNext;
	}

	pNewBlock->pNext = pTailBlock->pNext;
	pTailBlock->pNext = pNewBlock;

}

void sa_oswl_mmz_deleteMmbinfoNode(SA_POOL Pool)
{
	sa_mmb_info_s* posFrontNode = g_mmblist;
	sa_mmb_info_s* posNode = g_mmblist->pNext;
	struct mmb_info mmbinfo;

	while (posNode != NULL && posNode->poolId != Pool)
	{
		posFrontNode = posNode;
		posNode = posFrontNode->pNext;
	}
	if (posNode == NULL)
	{
		printk("no poolid node\n");
		return;
	}
	else
	{
		posFrontNode->pNext = posNode->pNext;
		/* free mmb mem*/
		mmbinfo.phys_addr = posNode->Mmbinfo.phys_addr;
		mmbinfo.size = posNode->Mmbinfo.size;
		mmbinfo.align = posNode->Mmbinfo.align;
		mmbinfo.flags = posNode->Mmbinfo.flags;
		mmbinfo.order = posNode->Mmbinfo.order;
		memcpy(mmbinfo.mmb_name,posNode->Mmbinfo.mmb_name, strlen(posNode->Mmbinfo.mmb_name));

		sa_oswl_mmz_FreeMmb(g_mmzFD, &mmbinfo);
		
		free(posNode);
	}
}

void sa_oswl_mmz_deletePoolNode(SA_POOL Pool)
{
	sa_pool_s* posFrontNode = g_poolList;
	sa_pool_s* posNode = g_poolList->pNext;

	while (posNode != NULL && posNode->poolId != Pool)
	{
		posFrontNode = posNode;
		posNode = posFrontNode->pNext;
	}
	if (posNode == NULL)
	{
		printk("no poolid node\n");
		return;
	}
	else
	{
		posFrontNode->pNext = posNode->pNext;
		free(posNode);
	}
}

void sa_oswl_mmz_deleteBlockNode(SA_BLK Block)
{
	sa_block_s* posFrontNode = g_blockList;
	sa_block_s* posNode = g_blockList->pNext;

	while (posNode != NULL && posNode->blockId != Block)
	{
		posFrontNode = posNode;
		posNode = posFrontNode->pNext;
	}
	if (posNode == NULL)
	{
		printk("no Block id node\n");
		return;
	}
	else
	{
		posFrontNode->pNext = posNode->pNext;
		free(posNode);
	}
}



SA_POOL sa_oswl_mmz_CreatePoolMemoey(sa_pool_config_s *pstPoolCfg)
{
	int s32Ret;
	struct mmb_info *mmb_info = NULL;
	sa_pool_s *pool_info = NULL;

	if(g_mmzFD < 0)
	{
		printk("mmz not init \n");
		return 0;
	}
	
	if(NULL == pstPoolCfg)
	{
		printk("input parameter invalid\n");
		return 0;
	}

	pool_info = (sa_pool_s *)malloc(sizeof(sa_pool_s));
	if(NULL == pool_info)
	{
		printk("malloc failed \n");
		return 0;			
	}
	memset(pool_info, 0, sizeof(sa_pool_s));

	/*fill data to pool_info*/
	g_poolID = g_poolID + 1;
	pool_info->poolId = g_poolID ;
	pool_info->totalBlkCnt = pstPoolCfg->u32BlkCnt;
	pool_info->poolSize = pstPoolCfg->u32BlkCnt * pstPoolCfg->u64BlkSize;
	pool_info->pNext = NULL;

	if(NULL == g_blockList)
	{
		g_blockList = sa_oswl_mmz_CreateBlockList();
		if(NULL == g_blockList)
		{
			printk("malloc for g_blockList failed\n");
			return 0;
		}
		memset(g_blockList,0,sizeof(sa_block_s));
	}


	/*add pool_info to g_poolList*/
	if(NULL == g_poolList)
	{
		printk("g_poolList is NULL \n");
		g_poolList = sa_oswl_mmz_CreatePoolList();
		if(NULL == g_poolList)
		{
			printk("malloc failed\n");
			return 0;
		}
		memset(g_poolList,0,sizeof(sa_pool_s));

	}
	
	sa_oswl_mmz_InsertPoolNode(g_poolList, pool_info);

	/*fill data to mmb_info*/
	mmb_info = (struct mmb_info *)malloc(sizeof(struct mmb_info));
	if(NULL == mmb_info)
	{
		printk("malloc failed \n");
		return 0;
	}

	mmb_info->size = pstPoolCfg->u64BlkSize * pstPoolCfg->u32BlkCnt;
	memcpy(mmb_info->mmb_name, pstPoolCfg->acMmbName, strlen(pstPoolCfg->acMmbName));
	
	s32Ret = sa_oswl_mmz_AllocMmb(g_mmzFD, mmb_info);
	if(s32Ret != 0)
	{
		printk("malloc mem for pool failed\n");
		return 0;
	}

	if(NULL == g_mmblist)
	{
		g_mmblist = sa_oswl_mmz_CreateMmbinfolist();
		if(NULL == g_mmblist)
		{
			printk("malloc for g_mmblist failed\n");
			return 0;
		}
		memset(g_mmblist,0,sizeof(sa_mmb_info_s));
	}
	sa_oswl_mmz_InsertMmbinfoNode(g_mmblist, mmb_info);


	sa_block_s *pBlock = NULL;
	pBlock = (sa_block_s *)malloc(sizeof(sa_block_s));
	if(NULL == pBlock)
	{
		printk("malloc mem for pBlock failed \n");
		free(pBlock);
	}
	memset(pBlock, 0, sizeof(sa_block_s));

	unsigned int u32Cnt = 0;
	for(u32Cnt = 0; u32Cnt < pstPoolCfg->u32BlkCnt; u32Cnt ++)
	{
		/*malloc memory for per blk*/
		g_blockID = g_blockID + 1;
		pBlock->blockId = g_blockID;
		pBlock->poolId = g_poolID;
		pBlock->phys_addr = mmb_info->phys_addr + (pstPoolCfg->u64BlkSize)*u32Cnt;
		pBlock->used = 0;
		pBlock->size = pstPoolCfg->u64BlkSize;
		//printk(" blk mmb_info->phys_addr =0x%lx blockid=%d\n",pBlock->phys_addr, pBlock->blockId);
		/*inset blk to pool*/
		sa_oswl_mmz_InsertBlockNode(g_blockList, pBlock);

		//printk("g_poolID = %d g_blockID =%d \n", g_poolID, g_blockID);
	}

	free(pBlock);
	pBlock = NULL;

	return pool_info->poolId;
}


SA_BLK sa_oswl_mmz_GetBlock(SA_POOL Pool)
{
	sa_pool_s * pMove = g_poolList->pNext;
	sa_block_s *pBlock = g_blockList->pNext;
	unsigned int freeBlkID = 0;
	
	printk("Pool is =%d pMove->poolId = %d totalBlkCnt =%d \n", Pool, pMove->poolId, pMove->totalBlkCnt);

	while (Pool != pMove->poolId && NULL != pMove->pNext)
	{
		pMove = pMove->pNext;
	}

	if(NULL == pMove)
	{
		printk("Pool was not found\n");
		return 0;	
	}

	while(NULL != pBlock->pNext)
	{
		if(Pool == pBlock->poolId && pBlock->used == 0)
		{
			freeBlkID = pBlock->blockId;
			pBlock->used = 1;
			break;
		}
		pBlock = pBlock->pNext;
	}

	if(NULL == pBlock->pNext && pBlock->used == 0)
	{
		freeBlkID = pBlock->blockId;
		pBlock->used = 1;	
	}

	return freeBlkID;
}

int sa_oswl_mmz_RemapPool(SA_POOL pool_id, int cached)
{
	sa_mmb_info_s * pMove = g_mmblist->pNext;

	while (pool_id != pMove->poolId)
	{
		pMove = pMove->pNext;	
	}

	if(NULL == pMove)
	{
		printk("Pool was not found\n");
		return -1;	
	}

	sa_oswl_mmz_UnremapMmb(g_mmzFD, &(pMove->Mmbinfo));

	return 0;
}


int sa_oswl_mmz_UnremapPool(SA_POOL pool_id, int cached)
{
	sa_mmb_info_s * pMove = g_mmblist->pNext;

	while (pool_id != pMove->poolId)
	{
		pMove = pMove->pNext;	
	}

	if(NULL == pMove)
	{
		printk("Pool was not found\n");
		return -1;	
	}

	sa_oswl_mmz_RemapMmb(g_mmzFD, &(pMove->Mmbinfo), cached);

	return 0;
}

unsigned int sa_oswl_mmz_GetPoolVirtAddr(SA_POOL pool_id,void **virt_addr )
{
	sa_mmb_info_s * pMove = g_mmblist->pNext;

	while (pool_id != pMove->poolId)
	{
		pMove = pMove->pNext;	
	}

	if(NULL == pMove)
	{
		printk("Pool was not found\n");
		return -1;	
	}

	virt_addr = pMove->Mmbinfo.mapped;

	return 0;
}

int sa_oswl_mmz_ReleaseBlock(SA_BLK Block , SA_POOL Pool)
{
	sa_pool_s * pMove = g_poolList->pNext;
	sa_block_s *pBlock = g_blockList->pNext;

	//printk("Pool is =%d pMove->poolId = %d totalBlkCnt =%d \n", Pool, pMove->poolId, pMove->totalBlkCnt);

	while (Pool != pMove->poolId && NULL != pMove->pNext)
	{
		pMove = pMove->pNext;
	}

	if(NULL == pMove)
	{
		printk("Pool was not found\n");
		return -1;
	}

	while(NULL != pBlock->pNext)
	{
		if(Pool == pBlock->poolId && pBlock->blockId == Block)
		{
			pBlock->used = 0;
			break;
		}
		pBlock = pBlock->pNext;
	}

	if(NULL == pBlock->pNext && pBlock->blockId == Block)
	{
		pBlock->used = 0;	
	}
	
	return 0;
}


int sa_oswl_mmz_DestroyPool(SA_POOL Pool)
{	
	int s32Ret = 0;
	sa_block_s *pBlock = g_blockList->pNext;

	while(NULL != pBlock->pNext)
	{
		if(Pool == pBlock->poolId)
		{
			sa_oswl_mmz_deleteBlockNode(pBlock->blockId);
		}

		pBlock = pBlock->pNext;
	}

	sa_oswl_mmz_deleteMmbinfoNode(Pool);

	sa_oswl_mmz_deletePoolNode(Pool);

	return s32Ret;
}


SA_U64 sa_oswl_mmz_GetBlockPhysAddr(SA_BLK Block, SA_POOL Pool)
{
	sa_pool_s * pMove = g_poolList->pNext;
	sa_block_s *pBlock = g_blockList->pNext;
	SA_U64 block_pyhsaddr = 0;
	
	//printk("Pool is =%d pMove->poolId = %d totalBlkCnt =%d \n", Pool, pMove->poolId, pMove->totalBlkCnt);

	while (Pool != pMove->poolId)
	{
		pMove = pMove->pNext;
	}

	if(NULL == pMove)
	{
		printk("Pool was not found\n");
		return 0;	
	}
	
	while(NULL != pBlock->pNext)
	{
		if(Pool == pBlock->poolId && pBlock->blockId == Block)
		{
			block_pyhsaddr = pBlock->phys_addr;
			break;
		}
		pBlock = pBlock->pNext;
	}

	if(NULL == pBlock->pNext && pBlock->blockId == Block)
	{
		block_pyhsaddr = pBlock->phys_addr;		
	}


	return block_pyhsaddr;
}

SA_U64 sa_oswl_mmz_GetPoolPhysAddr(SA_POOL Pool)
{
	sa_pool_s * pMove = g_poolList->pNext;

	while (Pool != pMove->poolId)
	{
		pMove = pMove->pNext;
	}

	if(NULL == pMove)
	{
		printk("Pool was not found\n");
		return 0;	
	}

	return pMove->phys_addr;	
}

SA_POOL sa_oswl_mmz_GetPoolId(SA_BLK Block)
{
	sa_block_s *pBlock = g_blockList->pNext;
	SA_POOL poolID = 0;

	
	while(NULL != pBlock->pNext)
	{
		if(pBlock->blockId == Block)
		{
			poolID = pBlock->poolId;
			break;
		}
		pBlock = pBlock->pNext;
	}

	if(NULL == pBlock->pNext && pBlock->blockId == Block)
	{
		poolID = pBlock->poolId;	
	}
	
	return poolID;
}