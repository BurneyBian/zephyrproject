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
#include "../osal_proc.h"

#define PROC_FS_TYPE (FS_TYPE_EXTERNAL_BASE + 0)
static struct fs_mount_t *mp[PROC_FS_TYPE];

struct proc_dirent {
	struct osal_list_head head;
};

int procfs_read(struct fs_file_t *zfp, void *ptr, size_t size)
{
	struct osal_proc_desc *pdesc = (struct osal_proc_desc *)zfp->filep;

	if (pdesc->proc_entry.read)
		pdesc->proc_entry.read(&pdesc->proc_entry);

    return 0;
}

int procfs_write(struct fs_file_t *zfp, const void *ptr, size_t size)
{
	int ret = 0;
	long long pos = 0;
	struct osal_proc_desc *pdesc = (struct osal_proc_desc *)zfp->filep;

	if (pdesc->proc_entry.write)
		ret = pdesc->proc_entry.write(&pdesc->proc_entry, ptr, size, &pos);

    return ret;
}


off_t procfs_lseek(struct fs_file_t *zfp, off_t offset, int whence)
{
	if (!zfp) {
		return -EINVAL;
	}

    return 0;
}

int procfs_close(struct fs_file_t *zfp)
{
	if (zfp == NULL) {
		return -EINVAL;
	}
	
	zfp->filep = NULL;

    return 0;
}

int procfs_open(struct fs_file_t *zfp, const char *file_name, fs_mode_t flags)
{
	struct osal_proc_desc *pdesc = NULL;
	char *path = (char *)file_name;

	if (zfp == NULL || file_name == NULL) {
		return -EINVAL;
	}

	if (zfp->filep) {
		osal_printk("%s - file has been opened!\n", __func__);
		return -EEXIST;
	}

	if (!(flags & FS_O_MASK)) {
		return -EINVAL;
	}
	path[0] = '/';
	path[1] = 'p';
	path[2] = 'r';
	path[3] = 'o';
	path[4] = 'c';

	pdesc = proc_find_nodes(path, 0);
	if (pdesc && !pdesc->is_dir) {
		zfp->filep = pdesc;
		return 0;	
	}

	return -EINVAL;
}

static int procfs_stat(struct fs_mount_t *mountp, const char *path, struct fs_dirent *entry)
{
	struct osal_proc_desc *pdesc = NULL;
	char *file_name = (char *)path;

	if (mountp == NULL || path == NULL || entry == NULL) {
		return -EINVAL;
	}

	file_name[0] = '/';
	file_name[1] = 'p';
	file_name[2] = 'r';
	file_name[3] = 'o';
	file_name[4] = 'c';

	pdesc = proc_find_nodes(file_name, 0);
	if (pdesc) {
		entry->type = pdesc->is_dir ? FS_DIR_ENTRY_DIR : FS_DIR_ENTRY_FILE;
	}

	return 0;
}

static int procfs_opendir(struct fs_dir_t *zdp, const char *path)
{
	struct proc_dirent *current_dirent;
	struct osal_proc_desc *pdesc = NULL, *tmp_pdesc;
	struct osal_proc_dir_entry *p_entry;

	if (zdp == NULL || path == NULL) {
		return -EINVAL;
	}

	pdesc = proc_find_nodes(path, 1);

	current_dirent = (struct proc_dirent *)k_malloc(sizeof(struct proc_dirent));
	if ((current_dirent != NULL) && pdesc)
	{
		OSAL_INIT_LIST_HEAD(&current_dirent->head);

		k_sched_lock();

		if (!pdesc->is_dir) {
			osal_list_add_tail(&pdesc->list, &current_dirent->head);

			k_sched_unlock();
			zdp->dirp = current_dirent;
			return 0;
		}

		osal_list_for_each_entry(p_entry, &pdesc->d_head, node) {
			tmp_pdesc = rt_container_of(p_entry, struct osal_proc_desc, proc_entry);
			osal_list_add_tail(&tmp_pdesc->list, &current_dirent->head);				
		}

		osal_list_for_each_entry(p_entry, &pdesc->f_head, node) {
			tmp_pdesc = rt_container_of(p_entry, struct osal_proc_desc, proc_entry);
			osal_list_add_tail(&tmp_pdesc->list, &current_dirent->head);
		}

		k_sched_unlock();

		zdp->dirp = current_dirent;
		return 0;
	}

	return 0;
}

static int procfs_readdir(struct fs_dir_t *zdp, struct fs_dirent *entry)
{
	int found = 0;
	struct osal_proc_desc *pdesc = NULL;
    struct proc_dirent *current_dirent = (struct proc_dirent *)zdp->dirp;

    if (current_dirent == NULL)
		 return -EINVAL;

	k_sched_lock();

	if(!osal_list_empty(&current_dirent->head)) {
		pdesc = osal_list_first_entry(&current_dirent->head, struct osal_proc_desc, list);
		osal_list_del(&pdesc->list);
		found = 1;
	}

	k_sched_unlock();

	if (found && pdesc) {
		entry->type = pdesc->is_dir ? FS_DIR_ENTRY_DIR : FS_DIR_ENTRY_FILE;
		strncpy(entry->name, pdesc->proc_entry.name, MAX_FILE_NAME);	
	} else {
		entry->name[0] = '\0';
	}

	return 0;
}

static int procfs_closedir(struct fs_dir_t *zdp)
{
	if (zdp && zdp->dirp) {
		k_free(zdp->dirp);
		zdp->dirp = NULL;
		return 0;
	}

	return -EINVAL;
}

static int procfs_mount(struct fs_mount_t *mountp)
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

static int procfs_unmount(struct fs_mount_t *mountp)
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

static const struct fs_file_system_t procfs_fs_ops = {
	.open = procfs_open,
	.close = procfs_close,
	.read = procfs_read,
	.write = procfs_write,
	.opendir = procfs_opendir,
	.readdir = procfs_readdir,
	.closedir = procfs_closedir,
    .mount = procfs_mount,
	.unmount = procfs_unmount,
	.stat = procfs_stat,
};

static struct fs_mount_t mnt = {
	.type = PROC_FS_TYPE,
	.mnt_point = "/proc:",
	.fs_data = NULL,
	.storage_dev = NULL,
};
	
int osal_procfs_init(void)
{
	int res;

    res = fs_register(PROC_FS_TYPE, &procfs_fs_ops);
	if (res < 0) {
		osal_printk("Error fs_register [%d]\n", res);
		return -1;
	}

	res = fs_mount(&mnt);
	if (res < 0) {
		osal_printk("Error fs_mount [%d]\n", res);
		return res;
	}
	osal_printk("osal_procfs_init success \n");

	return 0;
}

