/*
 * Copyright (c) 2022-2025, acme system software Team
 *
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-6-30     wth       the first version
 */

#include <zephyr/kernel.h>
#include <errno.h>
#include <zephyr/init.h>
#include <zephyr/fs/fs.h>
#include <zephyr/fs/fs_sys.h>

#include "../osal.h"
#include "./devfs.h"

#define DEV_FS_TYPE (FS_TYPE_EXTERNAL_BASE + 1)
static struct fs_mount_t *mp[DEV_FS_TYPE];
static OSAL_LIST_HEAD(osal_dev_head);
static OSAL_LIST_HEAD(osal_dev_dir_head);
static K_MUTEX_DEFINE(osal_dev_mutex);

struct osal_file_entry {
    osal_dev_t *dev;
	struct osal_pdata_content content;
    struct osal_poll table;
    int f_ref_cnt;
};

void osal_device_register(osal_dev_t *pdev)
{
	if (pdev) {
		k_mutex_lock(&osal_dev_mutex, K_FOREVER);
		osal_list_add_tail(&pdev->list, &osal_dev_head);
		k_mutex_unlock(&osal_dev_mutex);
	}
}

void osal_device_unregister(osal_dev_t *pdev)
{
	if (pdev) {
		k_mutex_lock(&osal_dev_mutex, K_FOREVER);
		osal_list_del(&pdev->list);
		k_mutex_unlock(&osal_dev_mutex);
	}
}

osal_dev_t *dev_find_nodes(char * path)
{
	osal_dev_t *pdev;

    osal_list_for_each_entry(pdev, &osal_dev_head, list) {
        if (!strcmp(pdev->name, path) == 0) {
            return pdev;
        }
    }

	return NULL;
}

int devfs_read(struct fs_file_t *zfp, void *ptr, size_t size)
{
	struct osal_file_entry *entry;
	osal_dev_t *dev;
	long offset = 0;

	if ((zfp == NULL) || (zfp->filep == NULL))
		return -EINVAL;

	entry = (struct osal_file_entry *)zfp->filep;
	dev = (osal_dev_t *)entry->dev;

	if (dev && dev->fops && dev->fops->read)
		dev->fops->read(ptr, size, &offset, (void *)&(entry->content));

    return 0;
}

int devfs_write(struct fs_file_t *zfp, const void *ptr, size_t size)
{
	struct osal_file_entry *entry;
	osal_dev_t *dev;
	long offset = 0;

	if ((zfp == NULL) || (zfp->filep == NULL))
		return -EINVAL;

	entry = (struct osal_file_entry *)zfp->filep;
	dev = (osal_dev_t *)entry->dev;
	
	if (dev && dev->fops && dev->fops->write)
		dev->fops->write(ptr, size, &offset, (void *)&(entry->content));

    return 0;
}

int devfs_ioctl(struct fs_file_t *zfp, unsigned int cmd, unsigned long arg)
{
	struct osal_file_entry *entry;
	osal_dev_t *dev;

	if ((zfp == NULL) || (zfp->filep == NULL))
		return -EINVAL;

	entry = (struct osal_file_entry *)zfp->filep;
	dev = (osal_dev_t *)entry->dev;

	if (dev && dev->fops && dev->fops->unlocked_ioctl)
		dev->fops->unlocked_ioctl(cmd, arg, (void *)&(entry->content));
	else if (dev && dev->fops && dev->fops->compat_ioctl)
		dev->fops->compat_ioctl(cmd, arg, (void *)&(entry->content));

    return 0;
}

#if 0
int devfs_poll(struct fs_file_t *zfp, osal_poll_t *osal_poll)
{
	osal_dev_t *dev = (osal_dev_t *)zfp->filep;

	if (dev && dev->fops && dev->fops->poll)
		dev->fops->poll(osal_poll, dev->priv_data);

    return 0;
}
#endif

off_t devfs_lseek(struct fs_file_t *zfp, off_t offset, int whence)
{
	if (!zfp)
		return -EINVAL;

	osal_printk_err("Error: Not support lssek!\n");

    return 0;
}

int devfs_close(struct fs_file_t *zfp)
{
	osal_dev_t *dev;
	struct osal_file_entry *entry;

	if ((zfp == NULL) || (zfp->filep == NULL))
		return -EINVAL;

	entry = (struct osal_file_entry *)zfp->filep;
	dev = (osal_dev_t *)entry->dev;

	if (dev && dev->fops && dev->fops->release)
		dev->fops->release((void *)&(entry->content));

	k_free(entry);

	zfp->filep = NULL;

    return 0;
}

int devfs_open(struct fs_file_t *zfp, const char *file_name, fs_mode_t flags)
{
	int ret;
	osal_dev_t *dev;
	char *path = (char *)file_name;
	struct osal_file_entry *entry;

	if (zfp == NULL || file_name == NULL)
		return -EINVAL;

	if (zfp->filep) {
		osal_printk("%s - file has been opened!\n", __func__);
		return -EEXIST;
	}

	if (!(flags & FS_O_MASK)) {
		printk("[%s] -- FS_O_MASK\n", __func__);
		return -EINVAL;
	}

	dev = dev_find_nodes(path);

	if (dev && dev->fops && dev->fops->open) {
		entry = k_malloc(sizeof(struct osal_file_entry));
		if (entry) {
			entry->dev = dev;		
			ret = dev->fops->open((void *)&(entry->content));
			if (ret) {
				k_free(entry);
				return ret;
			}
			zfp->filep = entry;
		} else {
			printk("[%s]error: malloc failed\n", __func__);
			return -ENOMEM;
		}

		return 0;	
	}

	return -EINVAL;
}

static int devfs_stat(struct fs_mount_t *mountp, const char *path, struct fs_dirent *entry)
{
	osal_dev_t *dev;
	char *file_name = (char *)path;

	if (mountp == NULL || path == NULL || entry == NULL)
		return -EINVAL;

	if (!strcmp(path, "/dev:/")) {
		entry->type = FS_DIR_ENTRY_DIR;
	} else {
		dev = dev_find_nodes(file_name);
		if (dev)
			entry->type = FS_DIR_ENTRY_FILE;
		else
			return -1;
	}

	return 0;
}

static int devfs_opendir(struct fs_dir_t *zdp, const char *path)
{
	osal_dev_t *pdev;

	if (zdp == NULL || path == NULL) {
		return -EINVAL;
	}

	if (strcmp(path, "/dev:/")) {
		osal_printk_err("[%s]Error: [%s] is Not valid mount-point\n", __func__, path);
		return -EINVAL;
	}

	k_mutex_lock(&osal_dev_mutex, K_FOREVER);
    osal_list_for_each_entry(pdev, &osal_dev_head, list) {
        osal_list_add_tail(&pdev->node, &osal_dev_dir_head);
    }
	k_mutex_unlock(&osal_dev_mutex);

	zdp->dirp = &osal_dev_dir_head;

	return 0;
}

static int devfs_readdir(struct fs_dir_t *zdp, struct fs_dirent *entry)
{
	osal_dev_t *pdev = NULL;
	int found = 0;

	k_mutex_lock(&osal_dev_mutex, K_FOREVER);
	if(!osal_list_empty(&osal_dev_dir_head)) {
		pdev = osal_list_first_entry(&osal_dev_dir_head, osal_dev_t, node);
		osal_list_del(&pdev->node);
		found = 1;
	}
	k_mutex_unlock(&osal_dev_mutex);

	if (found && pdev) {
		entry->type = FS_DIR_ENTRY_FILE;
		strcpy(entry->name, pdev->name);	
	} else {
		entry->name[0] = '\0';
	}

	return 0;
}

static int devfs_closedir(struct fs_dir_t *zdp)
{
	if (zdp && zdp->dirp) {
		zdp->dirp = NULL;
		return 0;
	}

	return -EINVAL;
}

static int devfs_mount(struct fs_mount_t *mountp)
{
	int len;

	if (mountp == NULL) {
		return -EINVAL;
	}

	len = strlen(mountp->mnt_point);

	if (mountp->mnt_point[len - 1] != ':') {
		return -EINVAL;
	}
	mp[mountp->type] = mountp;

	return 0;
}

static int devfs_unmount(struct fs_mount_t *mountp)
{
	if (mountp == NULL) {
		return -EINVAL;
	}

	if (mp[mountp->type] == NULL) {
		return -EINVAL;
	}
	mp[mountp->type] = NULL;
	return 0;
}

static const struct fs_file_system_t devfs_fs_ops = {
	.open = devfs_open,
	.close = devfs_close,
	.read = devfs_read,
	.write = devfs_write,
	.opendir = devfs_opendir,
	.readdir = devfs_readdir,
	.closedir = devfs_closedir,
    .mount = devfs_mount,
	.unmount = devfs_unmount,
	.stat = devfs_stat,
	.ioctl = devfs_ioctl,
};

static struct fs_mount_t mnt = {
	.type = DEV_FS_TYPE,
	.mnt_point = "/dev:",
	.fs_data = NULL,
	.storage_dev = NULL,
};
	
int osal_devfs_init(void)
{
	int res;

    res = fs_register(DEV_FS_TYPE, &devfs_fs_ops);
	if (res < 0) {
		osal_printk("Error fs_register [%d]\n", res);
		return res;
	}

	res = fs_mount(&mnt);
	if (res < 0) {
		osal_printk("Error fs_mount [%d]\n", res);
		return res;
	}

	return 0;
}

