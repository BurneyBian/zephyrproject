#include "osal.h"
#include "osal_proc.h"
#include <zephyr/shell/shell.h>
static struct osal_proc_desc *g_pdesc = NULL;
static osal_spinlock_t proc_lock;
static struct osal_list_head g_head;

__attribute ((visibility("default"))) 
osal_proc_entry_t *osal_create_proc_entry(const char *name, osal_proc_entry_t *parent)
{
	unsigned long flags;
	int len = 0;
	struct osal_proc_desc *p = NULL;
    if (name == NULL) {
        osal_printk_err("%s - parameter invalid!\n", __FUNCTION__);
        return NULL;
    }

	struct osal_proc_desc *pdesc = (struct osal_proc_desc *)k_malloc(sizeof(struct osal_proc_desc));
	if (!pdesc) {
		osal_printk_err("%s - malloc error!\n", __FUNCTION__);
		return NULL;
	}

	memset(pdesc, 0, sizeof(struct osal_proc_desc));

	len = sizeof(pdesc->proc_entry.name) - 1;
	if (len > strlen(name))
		len = strlen(name);
	strncpy(pdesc->proc_entry.name, name, len);

	pdesc->proc_entry.proc_dir_entry = parent;

	if (parent)
		p = rt_container_of(parent, struct osal_proc_desc, proc_entry);
	else
		p = g_pdesc;
	
	osal_spin_lock_irqsave(&proc_lock, &flags);
	osal_list_add_tail(&pdesc->proc_entry.node, &p->f_head);
	osal_spin_unlock_irqrestore(&proc_lock, &flags);				

    return &pdesc->proc_entry;
}

__attribute ((visibility("default"))) 
void osal_remove_proc_entry(const char *name, osal_proc_entry_t *parent)
{
	struct osal_proc_desc *pdesc = NULL;
	struct osal_proc_desc *p = NULL;
	struct osal_proc_dir_entry *p_entry = NULL;
	unsigned long flags;
    if (name == NULL) {
        osal_printk_err("%s - parameter invalid!\n", __FUNCTION__);
        return;
    }

	if (parent)
		p = rt_container_of(parent, struct osal_proc_desc, proc_entry);
	else
		p = g_pdesc;

	osal_spin_lock_irqsave(&proc_lock, &flags);

    osal_list_for_each_entry(p_entry, &p->f_head, node) {
        if (strcmp(p_entry->name, name) == 0) {
            osal_list_del(&(p_entry->node));
			pdesc = rt_container_of(p_entry, struct osal_proc_desc, proc_entry);
            break;
        }
    }

	osal_spin_unlock_irqrestore(&proc_lock, &flags);

	if (pdesc)
		k_free(pdesc);
}

__attribute ((visibility("default"))) 
osal_proc_entry_t *osal_proc_mkdir(const char *name, osal_proc_entry_t *parent)
{
	unsigned long flags;
	int len = 0;
	struct osal_proc_desc *p = NULL;
    if (name == NULL) {
        osal_printk_err("%s - parameter invalid!\n", __FUNCTION__);
        return NULL;
    }

	struct osal_proc_desc *pdesc = (struct osal_proc_desc *)k_malloc(sizeof(struct osal_proc_desc));
	if (!pdesc) {
		osal_printk_err("%s - malloc error!\n", __FUNCTION__);
		return NULL;
	}

	memset(pdesc, 0, sizeof(struct osal_proc_desc));

	len = sizeof(pdesc->proc_entry.name) - 1;
	if (len > strlen(name))
		len = strlen(name);
	strncpy(pdesc->proc_entry.name, name, len);

	strcat(pdesc->proc_entry.name, "/");
	pdesc->is_dir = 1;
	pdesc->proc_entry.proc_dir_entry = parent;
	OSAL_INIT_LIST_HEAD(&pdesc->f_head);
	OSAL_INIT_LIST_HEAD(&pdesc->d_head);

	if (parent)
		p = rt_container_of(parent, struct osal_proc_desc, proc_entry);
	else
		p = g_pdesc;

	osal_spin_lock_irqsave(&proc_lock, &flags);
	osal_list_add_tail(&pdesc->proc_entry.node, &p->d_head);
	osal_spin_unlock_irqrestore(&proc_lock, &flags);	

    return &pdesc->proc_entry;
}


void osal_remove_proc_root(const char *name, osal_proc_entry_t *parent)
{
	osal_remove_proc_entry(name, parent);
}

osal_proc_entry_t *osal_create_proc(const char *name, osal_proc_entry_t *parent)
{
    return osal_create_proc_entry(name, parent);
}

void osal_remove_proc(const char *name, osal_proc_entry_t *parent)
{
	osal_remove_proc_entry(name, parent);
}

#ifdef SMART_VERSION
static int reallocate_larger_buf(struct seq_file *seqfilep, size_t oldcount)
{
    char *largerbuf = (char *)NULL;

    if ((seqfilep == NULL) || (seqfilep->buf == NULL)) {
        return -1;
    }

    /* limitted the size of the seq file, small than 1M. */
    if (seqfilep->size >= 256 * PAGE_SIZE) {
        k_free(seqfilep->buf);
        seqfilep->buf = (char *)NULL;
        return -1;
    }

    largerbuf = (char *)k_malloc(seqfilep->size <<= 1);
    if (largerbuf == NULL) {
        k_free(seqfilep->buf);
        seqfilep->buf = (char *)NULL;
        return -1;
    }
    memset(largerbuf, 0, seqfilep->size);

    memcpy(largerbuf, seqfilep->buf, oldcount);
    seqfilep->count = oldcount;

    k_free(seqfilep->buf);
    seqfilep->buf = largerbuf;

    return 0;
}

__attribute ((visibility("default"))) 
int osal_seq_printf(osal_proc_entry_t *entry, const char *fmt, ...)
{
    int buflen = 0;
    size_t oldcount = 0;
    va_list arglist;
    char needreprintf = RT_FALSE;  // if need re-printf.

	struct seq_file *seqfilep = (struct seq_file *)entry->seqfile;

    if (seqfilep == NULL) {
		osal_printk_err("%s seqfilep is NULL\n", __func__);
        return -1;
    }

    if (seqfilep->buf == NULL) {
        seqfilep->buf = (char *)k_malloc(PAGE_SIZE);
		seqfilep->size = PAGE_SIZE;

        if (seqfilep->buf == NULL) {
			osal_printk_err("%s buf is NULL\n", __func__);
            return -1;
        }
        memset(seqfilep->buf, 0, seqfilep->size);
        seqfilep->count = 0;
    }

    do {
        oldcount = seqfilep->count;

        va_start(arglist, fmt);
        buflen = vsnprintf(seqfilep->buf + seqfilep->count, seqfilep->size - seqfilep->count, fmt, arglist);
        va_end(arglist);

        if (buflen < 0) {
			osal_printk_err("%s buflen error\n", __func__);
            k_free(seqfilep->buf);
            seqfilep->buf = (char *)NULL;
            return -1;
        }

        if (seqfilep->count + buflen < seqfilep->size) { //succeed write
            seqfilep->count += buflen;
            return 0;
        }

        needreprintf = RT_TRUE;

        if (reallocate_larger_buf(seqfilep, oldcount) != 0) {
            return -1;
        }
    } while (needreprintf);

    return 0;
}
#else
__attribute ((visibility("default"))) 
int osal_seq_printf(osal_proc_entry_t *entry, const char *fmt, ...)
{
	va_list arglist;
	char *buf = (char *)k_malloc(128);
    if (buf == NULL) {
		osal_printk_err("print error\n");
        return 0;
    }

	va_start(arglist, fmt);
	vsprintf(buf, fmt, arglist);
	va_end(arglist);

	osal_printk("%s", buf);

	k_free(buf);

	return 0;
}
#endif

/*=========================================================================================================*/

static struct osal_proc_desc * __proc_find_nodes(char * name, struct osal_list_head *head)
{
	struct osal_proc_dir_entry *p_entry = NULL;
	struct osal_proc_desc *pdesc = NULL;
	int tmp_len = 0;

    osal_list_for_each_entry(p_entry, head, node) {
		tmp_len = strlen(p_entry->name);
		if (p_entry->name[tmp_len - 1] == '/')
			tmp_len = tmp_len - 1;
        if (!strncmp(p_entry->name, name, tmp_len)) {
			pdesc = rt_container_of(p_entry, struct osal_proc_desc, proc_entry);
            break;
        }
    }

	return pdesc;
}

struct osal_proc_desc *proc_find_nodes(char * path, int is_ls)
{
	char *name = NULL;
	struct proc_name p_name;
	int name_depth = 0;
	int i = 0, pos = 0;
	struct osal_proc_desc *pdesc = NULL;
	struct osal_list_head *head = &g_head;
	struct osal_list_head *tmp_head = NULL;
	int is_dir = 0;
	int len = strlen(path);

	memset(&p_name, 0, sizeof(struct proc_name));

	for (i = 0; i < len; i++) {
		if (path[i] == '/') {
			p_name.name[name_depth] = &path[i];
			if (name_depth)
				p_name.len[name_depth - 1] = (i - pos);
			pos = i;
			name_depth++;
		}
	}

	if (path[len - 1] == '/')
		is_dir = 1;

	if(!is_dir) {
		p_name.len[name_depth - 1] = (len - pos - 1);
	}

	if (is_ls) {
		is_dir = 1;
	}

	for (i = 0; i < name_depth; i++) {
		if (p_name.len[i]) {
			name = (p_name.name[i] + 1);
			//osal_printk_inf("%s  [%s]\n", __func__, name);
			pdesc = __proc_find_nodes(name, head);
			if (!pdesc) {
				break;
			}
			head = &pdesc->d_head;

			if (i == (name_depth - 2)) {
				if (!is_dir)
					head = &pdesc->f_head;
				else if (is_ls)
					tmp_head = &pdesc->f_head;
			}
		}
	}

	if (!pdesc && tmp_head) {
		name = (p_name.name[name_depth - 1] + 1);
		pdesc = __proc_find_nodes(name, tmp_head);
	}

	if (!pdesc) {
		osal_printk_err("Please check whether the file or path you entered is correct [%s]\n", path);
	}

	return pdesc;
}

static int proc_ls_func(struct osal_proc_desc *pdesc, struct proc_ioctl_data *data)
{
	struct osal_proc_dir_entry *p_entry = NULL;

	if (!pdesc) {
		osal_printk_inf("%s - parameter invalid!\n", __FUNCTION__);
		return -EINVAL;	
	}
#ifdef SMART_VERSION
	if (!data->buf) {
		osal_printk_inf("%s - parameter invalid!\n", __FUNCTION__);
		return -EINVAL;	
	}
#endif
	if (!pdesc->is_dir) {
		osal_printk_inf("Yes!! it exsit\n");
		return -EEXIST;			
	}

    osal_list_for_each_entry(p_entry, &pdesc->d_head, node) {
#ifdef SMART_VERSION
        strcat(data->buf, p_entry->name);
		strcat(data->buf, "\n");
#else
		osal_printk_inf("%s\n", p_entry->name);
#endif
    }

    osal_list_for_each_entry(p_entry, &pdesc->f_head, node) {
#ifdef SMART_VERSION
        strcat(data->buf, p_entry->name);
		strcat(data->buf, "\n");
#else
		osal_printk_inf("%s\n", p_entry->name);
#endif
    }

	osal_printk_inf("\n");

	return 0;	
}

static int proc_read_func(struct osal_proc_desc *pdesc, struct proc_ioctl_data *data)
{
	int ret = 0;
#ifdef SMART_VERSION
	unsigned long *paddr = (unsigned long *)data->buf;
	struct seq_file *seqfilep = NULL;
#endif

	if (!pdesc) {
		osal_printk_inf("%s - parameter invalid!\n", __FUNCTION__);
		return -EINVAL;	
	}

	if (pdesc->is_dir) {
		osal_printk_inf("%s - Not a file!\n", __FUNCTION__);
		return -EINVAL;			
	}

	if (pdesc->proc_entry.read)
		ret = pdesc->proc_entry.read(&pdesc->proc_entry);

#ifdef SMART_VERSION
	seqfilep = (struct seq_file *)pdesc->proc_entry.seqfile;
    if (seqfilep == NULL) {
		osal_printk_inf("%s - seqfilep NULL!\n", __FUNCTION__);
        return -1;
    }

	if (ret >= 0) {
		data->count = seqfilep->count;
		*paddr = (unsigned long)seqfilep->buf;
		//memcpy(data->buf, seqfilep->buf, seqfilep->count);
	}
#endif

	return ret;
}

static int proc_write_func(struct osal_proc_desc *pdesc, struct proc_ioctl_data *data)
{
	int ret = 0;
	long long pos = 0;

	if (!pdesc) {
		osal_printk_inf("%s - parameter invalid!\n", __FUNCTION__);
		return -EINVAL;	
	}

	if (pdesc->is_dir) {
		osal_printk_inf("%s - Not a file!\n", __FUNCTION__);
		return -1;			
	}

	if (pdesc->proc_entry.write)
		ret = pdesc->proc_entry.write(&pdesc->proc_entry, data->buf, data->count, &pos);

	return ret;
}

static int proc_ioctl(int cmd, struct proc_ioctl_data *data)
{
	int ret = 0;
	struct osal_proc_desc *pdesc = NULL;

	if (!data && !data->path) {
		osal_printk_err("%s - invalid param!\n", __FUNCTION__);
		return -1;
	}

	switch (cmd) {
        case PROC_READ:
			pdesc = proc_find_nodes(data->path, 0);
			ret = proc_read_func(pdesc, data);
			break;
        case PROC_WRITE:
			pdesc = proc_find_nodes(data->path, 0);
			ret = proc_write_func(pdesc, data);
			break;
        case PROC_LS:
			pdesc = proc_find_nodes(data->path, 1);
			ret = proc_ls_func(pdesc, data);
			break;
        default:
            osal_printk_err("invalid ioctl cmd = %d\n", cmd);
            ret = -EINVAL;
            break;
    }

	return ret;
}

void osal_proc_init(void)
{
	g_pdesc = (struct osal_proc_desc *)k_malloc(sizeof(struct osal_proc_desc));
	if (!g_pdesc) {
		osal_printk_err("%s - malloc error!\n", __FUNCTION__);
		return;
	}

	memset(g_pdesc, 0, sizeof(struct osal_proc_desc));

	OSAL_INIT_LIST_HEAD(&g_pdesc->f_head);
	OSAL_INIT_LIST_HEAD(&g_pdesc->d_head);
	OSAL_INIT_LIST_HEAD(&g_head);
	g_pdesc->is_dir = 1;
	strcpy(g_pdesc->proc_entry.name, "proc/");

	if (osal_spin_lock_init(&proc_lock)) {
		osal_printk_err("%s - init lock error!\n", __FUNCTION__);
	}

	osal_list_add_tail(&g_pdesc->proc_entry.node, &g_head);
}
void osal_proc_exit(void)
{
	//release all nodes on g_pdesc
	//todo:
	if (g_pdesc) {
		osal_spin_lock_destory(&proc_lock);
		k_free(g_pdesc);
		g_pdesc = NULL;
	}
}

__attribute ((visibility("default"))) 
unsigned int osal_get_os_type(void)
{
	return OS_TYPE_ZEPHYR;
}

static void do_help(void)
{
	osal_printk("Usage: proc [ls|cat|echo] <file path>\n");
}

static int cmd_proc_ls(const struct shell *sh, size_t argc, char **argv)
{
	struct proc_ioctl_data proc_data;
	char path_name[32];

	memset(&path_name, 0, 32);
	memset(&proc_data, 0, sizeof(proc_data));	

	if (strncmp(argv[1], "/proc/", 6)) {
		osal_printk_err("file path is Not right\n");
		return -EINVAL;		
	} else {
		proc_data.path = argv[1];
		proc_ioctl(PROC_LS, &proc_data);
	}

	return 0;
}

static int cmd_proc_cat(const struct shell *sh, size_t argc, char **argv)
{
	struct proc_ioctl_data proc_data;
	char path_name[32];

	memset(&path_name, 0, 32);
	memset(&proc_data, 0, sizeof(proc_data));	

	if (strncmp(argv[1], "/proc/", 6)) {
		osal_printk_err("file path is Not right\n");
		return -EINVAL;		
	} else {
		proc_data.path = argv[1];
		proc_ioctl(PROC_READ, &proc_data);
	}

    return 0;
}


static int cmd_proc_echo(const struct shell *sh, size_t argc, char **argv)
{
	struct proc_ioctl_data proc_data;
	char path_name[32];

	memset(&path_name, 0, 32);
	memset(&proc_data, 0, sizeof(proc_data));	

	if (!strcmp(argv[2], ">") && !strncmp(argv[3], "/proc/", 6)) {
		proc_data.path = argv[3];
		proc_data.buf = argv[1];
		proc_data.count = strlen(argv[1]);
		proc_ioctl(PROC_WRITE, &proc_data);
	}
	else
	{
		do_help();
		return -1;
	}

	return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(sub_proc,
	SHELL_CMD_ARG(ls, NULL, "List files in current directory", cmd_proc_ls, 2, 0),
	SHELL_CMD_ARG(cat, NULL, "Concatenate files and print on the standard output", cmd_proc_cat, 2, 0),
	SHELL_CMD_ARG(echo, NULL, "Read file", cmd_proc_echo, 4, 0),
	SHELL_SUBCMD_SET_END
);

SHELL_CMD_REGISTER(proc, &sub_proc, "proc commands", NULL);

