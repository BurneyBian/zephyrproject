/* userdev.c
*
* Copyright (c) 2022 microbt Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
*
*/

#include <string.h>
#include "mmz_userdev_cmd.h"

#include "osal_list.h"
#include "osal.h"

#define MMZ_PAGE_SIZE      4096

#include "mmz_internal.h"
#include "../acme/acme_interface.h"
//#define MMZ_REDZONE_DEBUG

struct mmz_userdev_info {
	int pid;
    struct k_mutex mutex;
    struct osal_list_head list;
};

typedef struct key_list_head
{
	int key_num;
	void *phys_addr;
} shkey_list_head_t;

typedef struct mmz_zone_node
{
	char name[MMZ_NAME_LEN];
	int block_number;
	//int is_valid;
	struct osal_list_head list;
	unsigned long phys_addr;
	unsigned long size;
	unsigned long used;
} mmz_zone_node_t;

static mmz_zone_node_t *mmz_zone[64];

typedef struct mmb_stat
{
	char mmb_name[MMB_NAME_LEN];
	char mmz_name[MMZ_NAME_LEN];
	unsigned long phys_addr;
	unsigned long size;
    struct osal_list_head mmb_list;
	unsigned int cached;
	sa_mmb_t *mmb;
#ifdef MMZ_REDZONE_DEBUG
	struct osal_list_head entry;
#endif
} mmb_stat_t;

static osal_dev_t *mmz_userdev;
static osal_dev_t *mmz_sram_userdev;
static osal_spinlock_t mmz_lock;

static void init_mmz_zone_name(int is_sram, char *name)
{
	if (is_sram) {
		if (!strlen(name))
			strcpy(name, "sram");
	} else {
		if (!strlen(name))
			strcpy(name, "anonymous");
	}
}

static mmz_zone_node_t *find_mmz_zone_by_name(char *name)
{
	int i = 0;

	while (mmz_zone[i])
	{
		if (!strcmp(mmz_zone[i]->name, name))
			return mmz_zone[i];
		i++;
	}

	return mmz_zone[0];
}

static mmb_stat_t *search_mmb_stat(unsigned long phys_addr)
{
	int is_found = 0, i = 0;
	mmb_stat_t *mmb_stat = NULL;

    osal_spin_lock(&mmz_lock);

	while (mmz_zone[i])
	{
		osal_list_for_each_entry(mmb_stat, &mmz_zone[i]->list, mmb_list) {
			if ((mmb_stat->phys_addr <= phys_addr) && (phys_addr < (mmb_stat->phys_addr + mmb_stat->size)))
			{
				is_found = 1;
				break;
			}
		}
		i++;
		if (is_found)
			break;
	}

	osal_spin_unlock(&mmz_lock);

	if (!is_found)
		mmb_stat = NULL;

	return mmb_stat;
}


#define SHKEY_BIT_NUM 128
#define SHKEY_BIT_DEF 16
static shkey_list_head_t *shmem_key[SHKEY_BIT_NUM];

static shkey_list_head_t *search_shkey(int key_num)
{
    return shmem_key[key_num-1];
}

static int alloc_shkey_num(struct mmb_info *pmi)
{
	int i;
	shkey_list_head_t *key = NULL;
	int num = pmi->share_key;
	int size = mmz_grain_align(pmi->size);
	mmb_stat_t *mmb_stat = NULL;
	mmz_zone_node_t *zone;
	sa_mmb_t *mmb;

	if (num <= 0)
	{
		for (i = SHKEY_BIT_DEF; i < SHKEY_BIT_NUM; i++) {
			if (!shmem_key[i])
			{
				num = i + 1;
				break;
			}
		}

		if (i == SHKEY_BIT_NUM)
		{
			return -1;
		}
		pmi->share_key = num;
	}
	else
	{
		i = num - 1;
		if (shmem_key[i])
		{
			osal_printk_inf("%s old key=%d\n", __func__, num);
			return 0;			
		}
	}
	osal_printk_inf("%s new key=%d\n", __func__, num);
	key = (shkey_list_head_t *)k_malloc(sizeof(shkey_list_head_t));
	if (key)
	{
		memset(key, 0, sizeof(shkey_list_head_t));
		key->key_num = num;

		mmb = rt_mmz_malloc_bf(pmi->mmz_name, pmi->mmb_name, size);

		if (mmb)
			key->phys_addr = (void *)mmb->phys_addr;
		else
			return -ENOMEM;

		if (key->phys_addr == NULL) {
			osal_printk_err("sa_mmb_alloc(%s, %lu, 0x%lx, %lu, %s) failed!\n",
              pmi->mmb_name, pmi->size, pmi->align,
              pmi->gfp, pmi->mmz_name);

			return -ENOMEM;
		}
		shmem_key[i] = key;

		mmb_stat = (mmb_stat_t *)k_malloc(sizeof(mmb_stat_t));
		if (mmb_stat)
		{
			memset(mmb_stat, 0, sizeof(mmb_stat_t));
			mmb_stat->phys_addr = (unsigned long)key->phys_addr;
			mmb_stat->size = mmz_grain_align(size);
			mmb_stat->mmb = mmb;

			memcpy(mmb_stat->mmb_name, pmi->mmb_name, MMB_NAME_LEN);
			zone = find_mmz_zone_by_name(pmi->mmz_name);
			if (zone)
			{
				osal_spin_lock(&mmz_lock);
				osal_list_add_tail(&mmb_stat->mmb_list, &zone->list);
				osal_spin_unlock(&mmz_lock);
			}
		}

		return 0;
	}
	else
	{
		shmem_key[i] = NULL;
	}

	return -2;
}

static int remove_shkey_num(int num)
{
	int ret;
	shkey_list_head_t *key = search_shkey(num);
	mmb_stat_t *mmb_stat;

	if (key)
	{
		osal_printk_inf("%s key=%d\n", __func__, key->key_num);
		shmem_key[key->key_num-1] = NULL;

		mmb_stat = search_mmb_stat((unsigned long)key->phys_addr);
		if (mmb_stat)
		{
			rt_mmz_free_bf(mmb_stat->mmb);
			//osal_printk_inf("%s mmb_stat=%lx\n", __func__, (unsigned long)mmb_stat);
			osal_spin_lock(&mmz_lock);
			osal_list_del(&mmb_stat->mmb_list);
			osal_spin_unlock(&mmz_lock);
		}

		key->phys_addr = NULL;
		k_free(key);
	}
	else
	{
		osal_printk_err("%s Not find [%d] Shmem\n", __func__, num);
		return -EINVAL;
	}

	return ret;
}

#ifdef MMZ_REDZONE_DEBUG
#define MMZ_REDZONE_SIZE MMZ_PAGE_SIZE
struct osal_list_head redzone_list;
static osal_spinlock_t redzone_lock;
static void	mmz_fill_redzone(mmb_stat_t *mmb_stat)
{
	unsigned char *p;

	p = (unsigned char *)(mmb_stat->phys_addr + (mmz_grain_align(mmb_stat->size) - MMZ_REDZONE_SIZE));

	memset(p, 0x5a, 16);//right overflow
}

static void	mmz_check_redzone(mmb_stat_t *mmb_stat, osal_proc_entry_t *s)
{
	unsigned long long *p;
	int status = 0;
	unsigned long phys_addr = mmb_stat->phys_addr + (mmz_grain_align(mmb_stat->size) - MMZ_REDZONE_SIZE);

	p = (unsigned long long *)phys_addr;
	if (!p)
		return;

	if (mmb_stat->cached)
	{
		osal_cpuc_flush_dcache_area(p, MMZ_REDZONE_SIZE);
	}

	if (*p != 0x5a5a5a5a5a5a5a5a)
	{
		status = 1;
		osal_printk_err("[error]%s data overflow:[%s],start=%lx,len=%lx,%llx!=0x5a5a5a5a5a5a5a5a\n", __func__, mmb_stat->mmb_name,
			mmb_stat->phys_addr, (mmz_grain_align(mmb_stat->size) - MMZ_REDZONE_SIZE), *p);				
	}

	if (status)
	{
		osal_printk_inf("[0]: %llx\n", *p);
		osal_printk_inf("[8]: %llx\n", *(++p));
	}
}

__attribute ((visibility("default"))) 
void mmz_check_memerr(void)
{
    mmb_stat_t *p = NULL, *n = NULL;
	osal_spin_lock(&redzone_lock);

	osal_list_for_each_entry_safe(p, n, &redzone_list, entry) {
		if (p)
		{
			mmz_check_redzone(p, NULL);
		}
	}

	osal_spin_unlock(&redzone_lock);
}


static int redzone_proc_show(osal_proc_entry_t *s)
{
    mmb_stat_t *p = NULL, *n = NULL;

	osal_spin_lock(&redzone_lock);

	osal_list_for_each_entry_safe(p, n, &redzone_list, entry) {
		if (p)
		{
			mmz_check_redzone(p, s);
		}
	}

	osal_spin_unlock(&redzone_lock);

	return 0;
}

#endif

static int mmz_flush_dcache_mmb_dirty(struct mmb_info *pmi)
{
    if (pmi == NULL) {
		osal_printk_err("%s NULL\n", __func__);
        return -EINVAL;
    }

	osal_flush_dcache_area((void*)pmi->mapped, pmi->phys_addr, pmi->size);

    return 0;
}

static int mmz_inv_dcache_mmb(struct mmb_info *pmi)
{
    if (pmi == NULL) {
		osal_printk_err("%s NULL\n", __func__);
        return -EINVAL;
    }

	osal_invalid_dcache_area((void*)pmi->mapped, pmi->size);

    return 0;
}

static int mmz_userdev_open(void *private_data)
{
	struct osal_pdata_content *content = (struct osal_pdata_content *)private_data;
	osal_printk("%s enter\n", __func__);
	struct mmz_userdev_info *pmu = (struct mmz_userdev_info *)k_malloc(sizeof(struct mmz_userdev_info));
    if (pmu == NULL) {
		osal_printk_err("%s malloc failed\n", __func__);
        return -ENOMEM;
    }
	osal_printk("%s 1\n", __func__);
	k_mutex_init(&pmu->mutex);
	osal_printk("%s 2\n", __func__);
	pmu->pid = getpid();
	osal_printk("%s 3 [%d]\n", __func__, pmu->pid);
	content->data = (void *)pmu;
	osal_printk("%s exit\n", __func__);
    return 0;
}

static int ioctl_mmb_alloc(struct osal_pdata_content *content, unsigned int iocmd, struct mmb_info *pmi)
{
	void *addr = NULL;
	int size;
	shkey_list_head_t *key = NULL;
	mmb_stat_t *mmb_stat = NULL;
	mmz_zone_node_t *zone;
	sa_mmb_t *mmb;


#ifdef MMZ_REDZONE_DEBUG
	pmi->size += MMZ_REDZONE_SIZE;
#endif
	size = mmz_grain_align(pmi->size);

	if (pmi->share_key && (pmi->share_key <= SHKEY_BIT_NUM))
	{
		key = search_shkey(pmi->share_key);
		if (!key)
		{
			osal_printk_err("%s Not find [%d] Shmem\n", __func__, pmi->share_key);
			return -EINVAL;
		}
	}

	if (key)
	{
		addr = key->phys_addr;
		osal_printk_inf("%s key=%d\n", __func__, key->key_num);
	}
	else
	{
		pmi->share_key = 0;

		mmb = rt_mmz_malloc_bf(pmi->mmz_name, pmi->mmb_name, size);
		if (mmb)
			addr = (void *)mmb->phys_addr;

		if (addr)
		{
			mmb_stat = (mmb_stat_t *)k_malloc(sizeof(mmb_stat_t));
			if (mmb_stat)
			{
				memset(mmb_stat, 0, sizeof(mmb_stat_t));
				mmb_stat->phys_addr = (unsigned long)addr;
				mmb_stat->size = size;
				mmb_stat->mmb = mmb;

				memcpy(mmb_stat->mmb_name, pmi->mmb_name, MMB_NAME_LEN);
				memcpy(mmb_stat->mmz_name, pmi->mmz_name, MMZ_NAME_LEN);
				zone = find_mmz_zone_by_name(pmi->mmz_name);
				if (zone)
				{
					osal_spin_lock(&mmz_lock);
					osal_list_add_tail(&mmb_stat->mmb_list, &zone->list);
					osal_spin_unlock(&mmz_lock);
				}
			}
		}
	}

    if (addr == NULL) {
        osal_printk_inf("mmb_alloc(%s, %u, 0x%08lX, %lu, %s) failed!\n",
              pmi->mmb_name, size, pmi->align, pmi->gfp, pmi->mmz_name);
        return -ENOMEM;
    }

    pmi->phys_addr = (unsigned long)addr;
	pmi->mapped = 0;
#ifdef MMZ_REDZONE_DEBUG
	if (mmb_stat)
	{
		osal_spin_lock(&redzone_lock);
		osal_list_add_tail(&mmb_stat->entry, &redzone_list);
		osal_spin_unlock(&redzone_lock);
		mmz_fill_redzone(mmb_stat);
	}
#endif
    return 0;
}

static int ioctl_mmb_free(struct osal_pdata_content *content, unsigned int iocmd, struct mmb_info *pmi)
{
    int ret = 0;
	shkey_list_head_t *key = NULL;
	mmb_stat_t *mmb_stat = NULL;

	if (pmi->share_key)
	{
		key = search_shkey(pmi->share_key);
		if (!key)
		{
			osal_printk_err("%s Not find [%d] Shmem\n", __func__, pmi->share_key);
		}
	}
	else
	{
		mmb_stat = search_mmb_stat(pmi->phys_addr);
		if (mmb_stat)
		{
			osal_spin_lock(&mmz_lock);
			osal_list_del(&mmb_stat->mmb_list);
			osal_spin_unlock(&mmz_lock);

			rt_mmz_free_bf(mmb_stat->mmb);
#ifdef MMZ_REDZONE_DEBUG
			osal_spin_lock(&redzone_lock);
			osal_list_del(&mmb_stat->entry);
			osal_spin_unlock(&redzone_lock);
			mmz_check_redzone(mmb_stat, NULL);
#endif
		}
		else
		{
			osal_printk_err("%s mmb=%s failed\n", __func__, pmi->mmz_name);
			return -EINVAL;
		}
	}

	pmi->phys_addr = 0;
	pmi->mapped = 0;

    return ret;
}

static int ioctl_mmb_attr(struct osal_pdata_content *content, unsigned int iocmd, struct mmb_info *pmi)
{
	mmb_stat_t *mmb_stat = NULL;

	mmb_stat = search_mmb_stat(pmi->phys_addr);
    if (mmb_stat == NULL) {
        return -ENOMEM;//-EPERM;
    }

	pmi->map_cached = mmb_stat->cached;

#ifdef ARCH_ARM_CORTEX_M
	extern int memory_cachable_get_by_mpu(unsigned long addr);
	pmi->map_cached = memory_cachable_get_by_mpu((unsigned long)pmi->phys_addr);
#endif

    return 0;
}

static int ioctl_mmb_user_remap(struct osal_pdata_content *content, unsigned int iocmd, struct mmb_info *pmi, int cached)
{
	mmb_stat_t *mmb_stat = NULL;

	if (cached) {
		acme_set_cacheable_addr(pmi->phys_addr, mmz_grain_align(pmi->size));
		pmi->map_cached = 1;
    } else {
		acme_set_noncacheable_addr(pmi->phys_addr, mmz_grain_align(pmi->size));
		pmi->map_cached = 0;
    }

	pmi->mapped = (void *)acme_phyaddr_need_remap(pmi->phys_addr, cached);

	mmb_stat = search_mmb_stat(pmi->phys_addr);
	if (mmb_stat)
	{
		mmb_stat->cached = pmi->map_cached;
	}

    return 0;
}

static int ioctl_mmb_user_unmap(struct osal_pdata_content *content, unsigned int iocmd, struct mmb_info *pmi)
{
#ifdef ARCH_ARM_CORTEX_M
	mmb_stat_t *mmb_stat = search_mmb_stat(pmi->phys_addr);
	if (mmb_stat && (!mmb_stat->cached))
		acme_set_cacheable_addr(mmb_stat->phys_addr, mmb_stat->size);
#endif

	pmi->mapped = 0;

	return 0;
}

unsigned long usr_virt_to_phys(unsigned long virt)
{
    return virt;
}

static int ioctl_mmb_virt2phys(struct osal_pdata_content *content, unsigned int iocmd, struct mmb_info *pmi)
{
    int ret = 0;
    unsigned long virt = 0, phys = 0;
    //unsigned long offset = 0;

    virt = (unsigned long)pmi->mapped;
    phys = usr_virt_to_phys(virt);

    if (!phys) {
        ret = -ENOMEM;
    }

    pmi->phys_addr = phys;

    return ret;
}

static int mmz_userdev_ioctl_m(struct osal_pdata_content *content, unsigned int cmd, struct mmb_info *pmi)
{
    int ret = 0;
    //osal_printk_inf("mmz_userdev_ioctl_m enter\n");
    switch (_IOC_NR(cmd)) {
        case _IOC_NR(IOC_MMB_ALLOC):
            //osal_printk_inf("%s--alloc\n", __func__);
            ret = ioctl_mmb_alloc(content, cmd, pmi);
            break;

        case _IOC_NR(IOC_MMB_ALLOC_V2):
            // ret = ioctl_mmb_alloc_v2(content, cmd, pmi);
            osal_printk_inf("[%s], [%d] do not support in liteos.\n", __func__, __LINE__);
            break;

        case _IOC_NR(IOC_MMB_ATTR):
			//osal_printk_inf("%s--attr\n", __func__);
            ret = ioctl_mmb_attr(content, cmd, pmi);
            break;

        case _IOC_NR(IOC_MMB_FREE):
			//osal_printk_inf("%s--free\n", __func__);
            ret = ioctl_mmb_free(content, cmd, pmi);
            break;

        case _IOC_NR(IOC_MMB_USER_REMAP):
			//osal_printk_inf("%s--remap\n", __func__);
            ret = ioctl_mmb_user_remap(content, cmd, pmi, 0);
            break;

        case _IOC_NR(IOC_MMB_USER_REMAP_CACHED):
			//osal_printk_inf("%s--remap_cache\n", __func__);
            ret = ioctl_mmb_user_remap(content, cmd, pmi, 1);
            break;

        case _IOC_NR(IOC_MMB_USER_UNMAP):
			//osal_printk_inf("%s--unmap\n", __func__);
            ret = ioctl_mmb_user_unmap(content, cmd, pmi);
            break;

        case _IOC_NR(IOC_MMB_VIRT_GET_PHYS):
			//osal_printk_inf("%s--get phys\n", __func__);
            ret = ioctl_mmb_virt2phys(content, cmd, pmi);
            break;

        default:
            osal_printk_err("invalid ioctl cmd = %08X\n", cmd);
            ret = -EINVAL;
            break;
    }

    return ret;
}

static int __mmz_userdev_ioctl(struct osal_pdata_content *content, int cmd, void *args, int is_sram)
{
    int ret = 0;
	struct mmb_info mi;
	int num = 0;
    struct mmz_userdev_info *pmu = content->data;

	k_mutex_lock(&pmu->mutex, K_FOREVER);

    if (_IOC_TYPE(cmd) == 'm') {
        
        if ((_IOC_SIZE(cmd) > sizeof(mi)) || (!args)) {
            osal_printk_err("_IOC_SIZE(cmd)=%d, args==0x%08lX\n", _IOC_SIZE(cmd), (unsigned long)args);
            ret = -EINVAL;
            goto __error_exit;
        }

        memset(&mi, 0, sizeof(mi));
        memcpy(&mi, args, _IOC_SIZE(cmd));
		init_mmz_zone_name(is_sram, mi.mmz_name);
			
        mi.mmz_name[MMZ_NAME_LEN - 1] = '\0';
        mi.mmb_name[MMB_NAME_LEN - 1] = '\0';

        ret = mmz_userdev_ioctl_m(content, cmd, &mi);

        if (!ret && (cmd & IOC_OUT)) {
            memcpy(args, &mi, _IOC_SIZE(cmd));
        }

    } else if (_IOC_TYPE(cmd) == 's') {
        switch (_IOC_NR(cmd)) {
            case _IOC_NR(IOC_MMB_GET_SHKEY):
				if ((_IOC_SIZE(cmd) > sizeof(mi)) || (args == 0)) {
					osal_printk_err("_IOC_SIZE(cmd)=%d, %d, arg==0x%08lX\n", _IOC_SIZE(cmd), sizeof(mi), (unsigned long)args);
					ret = -EINVAL;
					goto __error_exit;
				}
				memset(&mi, 0, sizeof(mi));
				memcpy(&mi, args, _IOC_SIZE(cmd));
				init_mmz_zone_name(is_sram, mi.mmz_name);
				mi.mmz_name[MMZ_NAME_LEN - 1] = '\0';
				mi.mmb_name[MMB_NAME_LEN - 1] = '\0';

				ret = alloc_shkey_num(&mi);
				if (!ret && (cmd & IOC_OUT))
				{
					memcpy(args, &mi, _IOC_SIZE(cmd));
				}
				break;
            case _IOC_NR(IOC_MMB_RM_SHKEY):
				memcpy(&num, args, _IOC_SIZE(cmd));
				remove_shkey_num(num);
				break;
			default:
                ret = -EINVAL;
                break;
        }
    } else if (_IOC_TYPE(cmd) == 'd') {
        //osal_printk_inf("%s--d\n", __func__);
		struct dirty_area *d_area = (struct dirty_area *)(unsigned long *)args;
        if (!d_area) {
            goto __error_exit;
        }

		mi.phys_addr = d_area->dirty_phys_start;
		mi.mapped = (void *)d_area->dirty_virt_start;
		mi.size = d_area->dirty_size;
		switch (_IOC_NR(cmd)) {
			case _IOC_NR(IOC_MMB_FLUSH_DCACHE_DIRTY):
				mmz_flush_dcache_mmb_dirty(&mi);
				break;
			case _IOC_NR(IOC_MMB_INV_DCACHE):
				mmz_inv_dcache_mmb(&mi);
				break;
			default:
				ret = -ENOTSUP;
				break;
		}

    } else {
		osal_printk_err("%s Not support cmd[%x]\n", __func__, cmd);
        ret = -EINVAL;
    }

__error_exit:
	k_mutex_unlock(&pmu->mutex);

    return ret;
}

static int mmz_userdev_release(void *private_data)
{
	struct osal_pdata_content *content = (struct osal_pdata_content *)private_data;
    struct mmz_userdev_info *pmu = (struct mmz_userdev_info *)content->data;

    content->data = NULL;
    k_free(pmu);

    return 0;
}

static long mmz_sram_userdev_ioctl(unsigned int cmd, unsigned long arg, void *private_data)
{
	struct osal_pdata_content *content = (struct osal_pdata_content *)private_data;
	return __mmz_userdev_ioctl(content, cmd, (void *)arg, 1);
}

static long mmz_userdev_ioctl(unsigned int cmd, unsigned long arg, void *private_data)
{
	struct osal_pdata_content *content = (struct osal_pdata_content *)private_data;
	return __mmz_userdev_ioctl(content, cmd, (void *)arg, 0);
}


static struct osal_fileops mmz_user_fops = {
    .open = mmz_userdev_open,
    .release = mmz_userdev_release,
    .compat_ioctl = mmz_userdev_ioctl,
};
static struct osal_fileops mmz_sram_user_fops = {
    .open = mmz_userdev_open,
    .release = mmz_userdev_release,
    .compat_ioctl = mmz_sram_userdev_ioctl,
};

static int mmz_stat_show(osal_proc_entry_t *s)
{
	int i = 0;
    unsigned int block_number = 0;
    unsigned int used_size = 0;
    unsigned int free_size = 0;
	unsigned int len = 0;
	unsigned int mmz_total_size = 0;
	unsigned int zone_free_size = 0;
	mmb_stat_t *mmb_stat = NULL;

	osal_spin_lock(&mmz_lock);

	while (mmz_zone[i])
	{
		zone_free_size = 0;
		mmz_zone[i]->block_number = 0;
		mmz_zone[i]->used = 0;
		osal_printk("+---ZONE: PHYS(%lx) nBytes=%ldKB NAME=%s\n", 
			mmz_zone[i]->phys_addr, mmz_zone[i]->size, mmz_zone[i]->name);
		osal_list_for_each_entry(mmb_stat, &mmz_zone[i]->list, mmb_list) {
			len = mmz_grain_align(mmb_stat->size);
			osal_printk("|-MMB: phys(0x%lx, 0x%lx), length=0x%x, name=%s\n",
				mmb_stat->phys_addr, mmb_stat->phys_addr+len, len, mmb_stat->mmb_name);
				used_size += len /1024;
				++block_number;
				mmz_zone[i]->block_number++;
				mmz_zone[i]->used += len /1024;

		}
		zone_free_size = mmz_zone[i]->size - mmz_zone[i]->used;
		osal_printk("    used=%ldKB(%ldMB + %ldKB),remain=%dKB(%dMB + %dKB),block_number=%d\n",
			mmz_zone[i]->used, mmz_zone[i]->used / 1024, mmz_zone[i]->used % 1024,
			zone_free_size, zone_free_size / 1024, zone_free_size % 1024,
			mmz_zone[i]->block_number);
		mmz_total_size += mmz_zone[i]->size;
		i++;
	}
	osal_spin_unlock(&mmz_lock);


	free_size = mmz_total_size - used_size;

	osal_printk("---MMZ_USE_INFO: total size=%dKB(%dMB),used=%dKB,remain=%dKB,zone_number=%d,block_number=%d\n",
		mmz_total_size, mmz_total_size /1024,
			used_size, free_size, i, block_number);

	return 0;
}

static int mmz_sram_userdev_init(void)
{
    mmz_sram_userdev = osal_createdev("mmz_sram_dev");
    mmz_sram_userdev->fops = &mmz_sram_user_fops;
    osal_registerdevice(mmz_sram_userdev);

    return 0;
}
__attribute ((visibility("default"))) 
int mmz_userdev_init(void)
{
#ifdef MMZ_REDZONE_DEBUG
	struct osal_proc_dir_entry *redzone_proc_entry;
#endif
	struct osal_proc_dir_entry *proc_p;
	osal_printk("[%s] enter\n", __func__);
    mmz_userdev = osal_createdev("mmz_dev");
	//osal_printk("[%s] 1\n", __func__);
    mmz_userdev->fops  = &mmz_user_fops;
    osal_registerdevice(mmz_userdev);
	//osal_printk("[%s] 2\n", __func__);
	mmz_sram_userdev_init();
	//osal_printk("[%s] 3\n", __func__);
	proc_p = osal_create_proc_entry("media-mem", NULL);
	//osal_printk("[%s] 4\n", __func__);
	if (proc_p)
	{
		proc_p->read = mmz_stat_show;
		proc_p->private = NULL;
	}
	 
	if (osal_spin_lock_init(&mmz_lock)) {
		osal_printk_err("%s - init stat lock error!\n", __func__);
	}
	//osal_printk("[%s] 5\n", __func__);
#ifdef MMZ_REDZONE_DEBUG
	OSAL_INIT_LIST_HEAD(&redzone_list);
	if (osal_spin_lock_init(&redzone_lock)) {
		osal_printk_err("%s - init redzone lock error!\n", __func__);
	}
    redzone_proc_entry = osal_create_proc_entry("mmz_memerr", NULL);
    if (redzone_proc_entry)
	{
		redzone_proc_entry->read = redzone_proc_show;
		redzone_proc_entry->private = NULL;
	}
#endif
	osal_printk("[%s] exit\n", __func__);
    return 0;
}
//SYS_INIT(mmz_userdev_init, PRE_KERNEL_1, CONFIG_KERNEL_INIT_PRIORITY_OBJECTS);

__attribute ((visibility("default"))) 
void mmz_userdev_exit(void)
{
    osal_deregisterdevice(mmz_userdev);
	osal_deregisterdevice(mmz_sram_userdev);
#ifdef MMZ_REDZONE_DEBUG
	osal_spin_lock_destory(&redzone_lock);
	osal_remove_proc_entry("mmz_memerr", NULL);
#endif
}

static void fill_mmb_info(struct mmb_info *pmi, char *name,
	unsigned int size, unsigned int align, int is_sram)
{
	int len;

	memset(pmi, 0, sizeof(struct mmb_info));

	if (name) {
		len = strlen(name);
		len = (len > MMB_NAME_LEN) ? MMB_NAME_LEN : len;
		memcpy(pmi->mmb_name, name, len);
		init_mmz_zone_name(is_sram, pmi->mmz_name);
		pmi->mmz_name[MMB_NAME_LEN - 1] = '\0';
		pmi->mmb_name[MMB_NAME_LEN - 1] = '\0';
	}

	pmi->size = size;
	pmi->align = align;
}
__attribute ((visibility("default"))) 
unsigned long osal_mmz_alloc(char *name, unsigned int size, unsigned int align)
{
	int ret;
	struct mmb_info mi;

	fill_mmb_info(&mi, name, size, align, 0);

	ret = ioctl_mmb_alloc(NULL, 0, &mi);
	if (ret)
		return 0;
	else
		return mi.phys_addr;

}
#ifdef BSP_SOC_SA6941
__attribute ((visibility("default"))) 
void osal_mmz_spec_reserved(sa_mmz_t *mmz_heap, unsigned int addr, unsigned int size)
{
	sa_mmb_t *mmb;
	mmb_stat_t *mmb_stat;
	mmz_zone_node_t *zone;

	mmb = mmz_spec_reserved(mmz_heap, addr, size);
	if (mmb) {
		mmb_stat = (mmb_stat_t *)k_malloc(sizeof(mmb_stat_t));
		if (mmb_stat)
		{
			memset(mmb_stat, 0, sizeof(mmb_stat_t));
			mmb_stat->phys_addr = (unsigned long)addr;
			mmb_stat->size = size;
			mmb_stat->mmb = mmb;

			memcpy(mmb_stat->mmb_name, "remap_unusable", 16);
			strcpy(mmb_stat->mmz_name, mmz_heap->name);

			zone = find_mmz_zone_by_name(mmz_heap->name);
			if (zone)
			{
				osal_spin_lock(&mmz_lock);
				osal_list_add_tail(&mmb_stat->mmb_list, &zone->list);
				osal_spin_unlock(&mmz_lock);
			}
		}
	}
}
#endif
__attribute ((visibility("default"))) 
void osal_mmz_free(unsigned long paddr)
{
	struct mmb_info mi;

	memset(&mi, 0, sizeof(struct mmb_info));
	mi.phys_addr = paddr;
	init_mmz_zone_name(0, mi.mmz_name);
	mi.mmz_name[MMB_NAME_LEN - 1] = '\0';

	ioctl_mmb_free(NULL, 0, &mi);
}
__attribute ((visibility("default"))) 
void *osal_mmz_remap(unsigned long paddr, unsigned int size, unsigned char cached)
{
	mmb_stat_t *mmb_stat = search_mmb_stat(paddr);
	if (mmb_stat) {
		mmb_stat->cached = cached;
	}

	if (cached) {
		acme_set_cacheable_addr(paddr, size);
	} else {
		acme_set_noncacheable_addr(paddr, size);
	}

	return (void *)acme_phyaddr_need_remap(paddr, cached);
}
__attribute ((visibility("default"))) 
void osal_mmz_unmap(void *vaddr)
{
#ifdef ARCH_ARM_CORTEX_M
	mmb_stat_t *mmb_stat = search_mmb_stat(vaddr);
	if (mmb_stat && (!mmb_stat->cached))
		acme_set_cacheable_addr(mmb_stat->phys_addr, mmb_stat->size);
#endif
}
__attribute ((visibility("default"))) 
unsigned long osal_mmz_sram_alloc(char *name, unsigned int size, unsigned int align)
{
	int ret;
	struct mmb_info mi;

	fill_mmb_info(&mi, name, size, align, 1);

	ret = ioctl_mmb_alloc(NULL, 0, &mi);
	if (ret)
		return 0;
	else
		return mi.phys_addr;
}
__attribute ((visibility("default"))) 
void osal_mmz_sram_free(unsigned long paddr)
{
	struct mmb_info mi;

	memset(&mi, 0, sizeof(struct mmb_info));
	mi.phys_addr = paddr;
	init_mmz_zone_name(1, mi.mmz_name);
	mi.mmz_name[MMB_NAME_LEN - 1] = '\0';

	ioctl_mmb_free(NULL, 0, &mi);
}
__attribute ((visibility("default"))) 
void *osal_mmz_sram_remap(unsigned long paddr, unsigned int size, unsigned char cached)
{
	return osal_mmz_remap(paddr, size, cached);
}
__attribute ((visibility("default"))) 
void osal_mmz_sram_unmap(void *vaddr)
{
	return;
}

__attribute ((visibility("default"))) 
void mmz_heap_cb(char *name, unsigned long phys_addr, unsigned long size)
{
	static char g_mmz_index = 0;

	osal_printk("[%s] enter [%d]\n", __func__, (int)g_mmz_index);
	mmz_zone[(int)g_mmz_index] = (mmz_zone_node_t *)k_malloc(sizeof(mmz_zone_node_t));
	if (!mmz_zone[(int)g_mmz_index]) {
		osal_printk("[%s] malloc failed\n", __func__);
		return;
	}

	OSAL_INIT_LIST_HEAD(&mmz_zone[(int)g_mmz_index]->list);
	mmz_zone[(int)g_mmz_index]->phys_addr = phys_addr;
	mmz_zone[(int)g_mmz_index]->size = size / 1024;
	mmz_zone[(int)g_mmz_index]->used = 0;
	mmz_zone[(int)g_mmz_index]->block_number = 0;
	strcpy(mmz_zone[(int)g_mmz_index]->name, name);
	osal_printk("[%s] exit\n", __func__);
	g_mmz_index++;
}
