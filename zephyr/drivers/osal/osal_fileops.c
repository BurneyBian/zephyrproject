#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <zephyr/fs/fs.h>
#include <zephyr/posix/unistd.h>
#include "osal.h"

void *osal_klib_fopen(const char *filename, int flags, int mode)
{
	int dfs_flags = 0;
	int fd;

	if (flags == OSAL_O_RDONLY)
		dfs_flags = O_RDONLY;
	else if (flags == OSAL_O_WRONLY)
		dfs_flags = O_WRONLY;
	else if (flags == OSAL_O_RDWR)
		dfs_flags = O_RDWR;
	else if (flags == OSAL_O_CREAT)
		dfs_flags = O_RDWR | O_CREAT;
	else if (flags == OSAL_O_ACCMODE)
		dfs_flags = O_RDWR | O_APPEND;

    fd = open(filename, dfs_flags);

	return (void *)fd;
}

void osal_klib_fclose(void *filp)
{
	if (filp) {
		close((int)filp);
	}
}

int osal_klib_fwrite(char *buf, int len, void *filp)
{
	if (filp)
		return write((int)filp, (void *)buf, len);
	else
		return -EINVAL;
}

int osal_klib_fread(char *buf, unsigned int len, void *filp)
{
	if (filp)
		return read((int)filp, (void *)buf, len);
	else
		return -EINVAL;
}
