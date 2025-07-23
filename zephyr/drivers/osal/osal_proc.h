#ifndef OSAL_PROC_H
#define OSAL_PROC_H

#define PAGE_SIZE 64

struct osal_proc_desc {
    char is_dir;
    struct osal_list_head d_head;
    struct osal_list_head f_head;
    osal_proc_entry_t  proc_entry;
	osal_spinlock_t proc_lock;
	struct osal_list_head list;
};

enum iocmd {
    PROC_READ,
	PROC_WRITE,
	PROC_LS
};

struct proc_ioctl_data {
	char *path;
	char *buf;
	int count;
};

struct proc_name {
	char *name[8];
	int len[8];
};

struct osal_proc_desc *proc_find_nodes(char * path, int is_ls);
int dfs_proc_init(void);
#endif