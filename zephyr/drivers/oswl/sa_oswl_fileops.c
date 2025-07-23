#include <rtthread.h>
#include <rthw.h>
#include <dfs.h>
#include <dfs_fs.h>

#ifdef DFS_USING_POSIX
extern int open(const char *file, int flags, ...);
extern int close(int fd);
extern ssize_t read(int fd, void *buf, size_t len);
extern ssize_t write(int fd, void *buf, size_t len);
#elif defined(RT_USING_DFS)
#include <dfs_file.h>
#else
#include <stdio.h>
#endif

#include "oswl.h"

void *oswl_open(const char *filename, int flags, int mode)
{
	int dfs_flags = 0;

	if (flags == OSWL_O_RDONLY)
		dfs_flags = O_RDONLY;
	else if (flags == OSWL_O_WRONLY)
		dfs_flags = O_WRONLY;
	else if (flags == OSWL_O_RDWR)
		dfs_flags = O_RDWR;
	else if (flags == OSWL_O_CREAT)
		dfs_flags = O_RDWR | O_CREAT;
	else if (flags == OSWL_O_ACCMODE)
		dfs_flags = O_RDWR | O_APPEND;

#ifdef DFS_USING_POSIX
	int *fd;
	fd = rt_malloc(sizeof(int));
    *fd = open(filename, dfs_flags);
	return (void *)fd;
#elif defined(RT_USING_DFS)
	struct dfs_file *fd;
	fd = rt_malloc(sizeof(struct dfs_file));
	fd_init(fd);

	if (dfs_file_open(fd, filename, dfs_flags) < 0)
	{
		rt_set_errno(-ENOTDIR);
		rt_free(fd);
		return RT_NULL;
	}
	return (void *)fd;
#else
	if (flags == OSAL_O_RDONLY)
		return (void *)fopen(filename, "r");
	else if (flags == OSAL_O_WRONLY)
		return (void *)fopen(filename, "w");
	else if (flags == OSAL_O_RDWR)
		return (void *)fopen(filename, "r+");
	else if (flags == OSAL_O_CREAT)
		return (void *)fopen(filename, "w+");
	else if (flags == OSAL_O_ACCMODE)
		return (void *)fopen(filename, "a+");
#endif
}
RTM_EXPORT(oswl_open);

void oswl_close(void *filp)
{
	if (filp) {
#ifdef DFS_USING_POSIX
		close(*(int *)filp);
		rt_free(filp);
#elif defined(RT_USING_DFS)
		dfs_file_close((struct dfs_file *fd)filp);
		rt_free(filp);
#else
		fclose((FILE *)filp);
#endif
	}
}
RTM_EXPORT(oswl_close);

int oswl_write(const char *buf, int len, void *filp)
{
	if (filp)
#ifdef DFS_USING_POSIX
		return write(*(int *)filp, (void *)buf, len);
#elif defined(RT_USING_DFS)
		return dfs_file_write((struct dfs_file *)filp, buf, len);
#else
		return fread(buf, len, 1, (FILE *)filp);
#endif
	else
		return -1;
}
RTM_EXPORT(oswl_write);

int oswl_read(char *buf, unsigned int len, void *filp)
{
	if (filp)
#ifdef DFS_USING_POSIX
		return read(*(int *)filp, (void *)buf, len);
#elif defined(RT_USING_DFS)
		return dfs_file_write((struct dfs_file *)filp, buf, len);
#else
		return fwrite(buf, len, 1, (FILE *)filp);
#endif
	else
		return -1;
}
RTM_EXPORT(oswl_read);

int oswl_ioctl(void *args, unsigned int cmd, void *filp)
{
	if (filp)
#ifdef DFS_USING_POSIX
		return ioctl(filp, cmd, args);
#elif defined(RT_USING_DFS)
		return dfs_file_ioctl(filp, cmd, args);
#endif

	return -1;
}
RTM_EXPORT(oswl_ioctl);
