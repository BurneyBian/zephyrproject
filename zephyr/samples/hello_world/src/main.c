/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>

#include <ff.h>
#include <errno.h>
#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/__assert.h>
#include <zephyr/device.h>
#include <zephyr/storage/disk_access.h>
#include <ff.h>
#include <zephyr/fs/fs.h>

#include <string.h>
#include <fcntl.h>
#include <ff.h>
#include <zephyr/fs/fs.h>
#include <zephyr/posix/unistd.h>

/* for mount using FS api */
#define FATFS_MNTP	"/RAM:"


/* FatFs work area */
static FATFS fat_fs;

/* mounting info */
static struct fs_mount_t fatfs_mnt = {
	.type = FS_FATFS,
	.mnt_point = FATFS_MNTP,
	.fs_data = &fat_fs,
};

static int ram_mount(void)
{
	int res;

	res = fs_mount(&fatfs_mnt);
	if (res < 0) {
		printk("Error mounting fs [%d]\n", res);
		return -1;
	}
	printk("ramfs mount %s success \n",FATFS_MNTP);

	return 0;
}
// static const char test_str[] = "Hello World! hhhh\n";
// #define TEST_FILE  FATFS_MNTP "/testfile.txt"
// static int file_open(void)
// {
// 	int res;

// 	res = open(TEST_FILE, O_CREAT | O_RDWR, 0660);
	
// 	return res;
// }

// static int file_write(int file)
// {
// 	ssize_t brw;
// 	off_t res;

// 	res = lseek(file, 0, SEEK_SET);
// 	brw = write(file, (char *)test_str, strlen(test_str));
	
// 	return res;
// }
extern int osal_init(void);
#define DISK_DRIVE_NAME "SD2"
int main(void)
{
	printk("Hello World! %s\n", CONFIG_BOARD);

	// do {
	// 	static const char *disk_pdrv = DISK_DRIVE_NAME;
	// 	uint64_t memory_size_mb;
	// 	uint32_t block_count;
	// 	uint32_t block_size;

	// 	if (disk_access_ioctl(disk_pdrv,
	// 			DISK_IOCTL_CTRL_INIT, NULL) != 0) {
	// 		printk("11Storage init ERROR!");
	// 		break;
	// 	}

	// 	if (disk_access_ioctl(disk_pdrv,
	// 			DISK_IOCTL_GET_SECTOR_COUNT, &block_count)) {
	// 		printk("Unable to get sector count");
	// 		break;
	// 	}
	// 	printk("Block count %u", block_count);

	// 	if (disk_access_ioctl(disk_pdrv,
	// 			DISK_IOCTL_GET_SECTOR_SIZE, &block_size)) {
	// 		printk("Unable to get sector size");
	// 		break;
	// 	}
	// 	printk("Sector size %u\n", block_size);

	// 	memory_size_mb = (uint64_t)block_count * block_size;
	// 	printk("Memory Size(MB) %u\n", (uint32_t)(memory_size_mb >> 20));

	// 	if (disk_access_ioctl(disk_pdrv,
	// 			DISK_IOCTL_CTRL_DEINIT, NULL) != 0) {
	// 		printk("Storage deinit ERROR!");
	// 		break;
	// 	}
	// } while (0);


	ram_mount();
	// int file = file_open();

	// file_write(file);
	// fsync(file);
	osal_init();

	return 0;
}
