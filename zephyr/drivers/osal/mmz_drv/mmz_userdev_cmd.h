#ifndef MMZ_USERDEV_H
#define MMZ_USERDEV_H



#ifndef _DRV_IOCTL_H
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
#endif
struct dirty_area
{
	unsigned long dirty_phys_start;	/* dirty physical address */
	unsigned long dirty_virt_start;	/* dirty virtual  address, must be coherent with dirty_phys_addr */
	unsigned long dirty_size;
};


#define MMB_NAME_LEN           32
#define CACHE_LINE_SIZE        (0x40)
#define MMZ_NAME_LEN           32

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

    //struct osal_list_head *mmb_list;
   // hil_mmb_t *mmb;
    int map_ref;
    int mmb_ref;
};

#define IOC_MMB_ALLOC              _IOWR('m', 10, struct mmb_info)
#define IOC_MMB_ATTR               _IOR('m', 11, struct mmb_info)
#define IOC_MMB_FREE               _IOW('m', 12, struct mmb_info)
#define IOC_MMB_ALLOC_V2           _IOWR('m', 13, struct mmb_info)

#define IOC_MMB_USER_REMAP         _IOWR('m', 20, struct mmb_info)
#define IOC_MMB_USER_REMAP_CACHED  _IOWR('m', 21, struct mmb_info)
#define IOC_MMB_USER_UNMAP         _IOWR('m', 22, struct mmb_info)

#define IOC_MMB_VIRT_GET_PHYS      _IOWR('m', 23, struct mmb_info)

//#define IOC_MMB_ADD_REF            _IO('r', 30) /* ioctl(file, cmd, arg), arg is mmb_addr */
//#define IOC_MMB_DEC_REF            _IO('r', 31) /* ioctl(file, cmd, arg), arg is mmb_addr */

#define IOC_MMB_FLUSH_DCACHE       _IO('c', 40)

#define IOC_MMB_FLUSH_DCACHE_DIRTY _IOW('d', 50, struct dirty_area)
#define IOC_MMB_INV_DCACHE		   _IOW('d', 51, struct dirty_area)

//#define IOC_MMB_TEST_CACHE         _IOW('t', 11, struct mmb_info)

#define IOC_MMB_GET_SHKEY      _IOWR('s', 11, struct mmb_info)
#define IOC_MMB_RM_SHKEY       _IOW('s', 12, int)

#define MMZ_SETUP_CMDLINE_LEN      256


#define MAP_FAILED     ((void *) -1)

#define MAP_SHARED     0x01
#define MAP_PRIVATE    0x02
#define MAP_TYPE       0x0f
#define MAP_FIXED      0x10
#define MAP_ANON       0x20
#define MAP_ANONYMOUS  MAP_ANON
#define MAP_NORESERVE  0x4000
#define MAP_GROWSDOWN  0x0100
#define MAP_DENYWRITE  0x0800
#define MAP_EXECUTABLE 0x1000
#define MAP_LOCKED     0x2000
#define MAP_POPULATE   0x8000
#define MAP_NONBLOCK   0x10000
#define MAP_STACK      0x20000
#define MAP_HUGETLB    0x40000
#define MAP_FILE       0

#define PROT_NONE      0
#define PROT_READ      1
#define PROT_WRITE     2
#define PROT_EXEC      4
#define PROT_GROWSDOWN 0x01000000
#define PROT_GROWSUP   0x02000000

#endif