#include <string.h>
#include <stdlib.h>
#include "osal.h"

#define   OSAL_PRINT_YES            1
#define   OSAL_PRINT_NO             0
#define   OSAL_BULK_SIZE            512

typedef struct 
{
    osal_spinlock_t  lock;
    char*       log_buf;
    int         logbuf_size;
	char*       log_head;
	char*       log_tail;
	int         around;
	osal_proc_entry_t *log_entry;
    char*       file_name;
	osal_proc_entry_t *parent_entry;
	int         level;
	int         win_disp;
}osal_print_t;

static char module_close[256] = {0};

static osal_print_t  *osal_print_s = NULL;
static osal_proc_entry_t* log_flag = NULL;
static osal_spinlock_t print_lock;

static void _osal_print(char *buf)
{
	int i = 0;
	char tmp_buf[129];
	tmp_buf[128] = 0;

	int count = strlen(buf)/128;
	int rem = strlen(buf)%128;

	for (i = 0; i < count; i++) {
		memcpy(tmp_buf, buf + i*128, 128);
		osal_printk("%s\n", tmp_buf);
	}

	if (rem) {
		memcpy(tmp_buf, buf + i*128, rem);
		tmp_buf[rem] = 0;
		osal_printk("%s\n", tmp_buf);
	}
}

static int osal_print_read (osal_proc_entry_t *s)
{
	
	//struct seq_file *m = (struct seq_file *)s->seqfile;
	char  p_hand;
	char* p_start = NULL;
   // char* p_end   = NULL;  
	
    osal_print_t *prnt_s =(osal_print_t* )(s->private);   
    if(prnt_s->around == OSAL_PRINT_YES)
    {
        p_start  = prnt_s->log_head;
     //   p_end    = prnt_s->log_head - 1;
        _osal_print(p_start);
        p_hand   = *(prnt_s->log_head);
        *prnt_s->log_head = 0;
        _osal_print(prnt_s->log_buf);
        *prnt_s->log_head = p_hand;
    }
    else
    {
        p_start = prnt_s->log_buf;
   //     p_end = prnt_s->log_head;        
        _osal_print(p_start);
    }

    return 0;
}


static void osal_print_tty(const char *str)
{
    osal_printk("%s\n", str);
}

int osal_print_state_write(osal_proc_entry_t *entry, const char *buf, int count, long long *loggt)
{
	int i = 0;

	if(count == 4)
	{
		if(buf[0] == 'l' && buf[1] == 'e')
		{
			osal_print_s->level = buf[3] - '0';
		}
		if(buf[0] == 'd' && buf[1] == 'i')
		{
			osal_print_s->win_disp = buf[3] - '0';
		}
	}

	if(count >= 4) {
		if(buf[0] == 'm' && buf[1] == 'o')
		{
			if (!strncmp(buf+3, "all", 3)){
				memset(module_close, 0, sizeof(module_close));
			} else {
			i = atoi(buf+3);
			if (i < sizeof(module_close))
				module_close[i] = 0;
		}
		}
		else if(buf[0] == 'm' && buf[1] == 'c')
		{
			if (!strncmp(buf+3, "all", 3)){
				memset(module_close, 1, sizeof(module_close));
			} else {
			i = atoi(buf+3);
			if (i < sizeof(module_close))
				module_close[i] = 1;
			}
		}
	}

	return count;
}


int osal_print_state_show(osal_proc_entry_t *s)
{
    osal_seq_printf(s, "--------------------print flag------------------\n");
	osal_seq_printf(s, "input level le=?\n");
	osal_seq_printf(s, "input disp  di=?\n");
	osal_seq_printf(s, "------------------------------------------------\n");
	osal_seq_printf(s, "level  :%d\n", osal_print_s->level);
	osal_seq_printf(s, "disp   :%d\n", osal_print_s->win_disp);

    return 0;
}

int  osal_print_get_module_state(int idx)
{
	return module_close[idx];
}

int  osal_print_get_level(void)
{
	if(osal_print_s != NULL)
	{
		return osal_print_s->level;
	}
	else
	{
		return -1;
	}
}

int  osal_print_get_disp(void)
{
	if(osal_print_s != NULL)
	{
		return osal_print_s->win_disp;
	}
	else
	{
		return -1;
	}
}

static int osal_print_write(struct osal_proc_dir_entry *entry, const char *buf, int count, long long *ppos)
{

  // osal_print_t*  p = (osal_print_t* )((struct seq_file*)file->private_data)->private;
   return count;


}

osal_print_t*  osal_print_create(const char* file_name, size_t size)
{
	char* buf = NULL;
	int len = 0;
    struct osal_proc_dir_entry *proc_entry  = NULL;
	struct osal_proc_dir_entry *proc_dir    = NULL;
	osal_print_t* ctx                  = (osal_print_t*)k_malloc(sizeof(osal_print_t));
	if(!ctx)
	{
		return NULL;
	}
    memset(ctx, 0, sizeof(osal_print_t));

	proc_dir = osal_proc_mkdir("logdir", NULL);
    if(proc_dir == NULL)
	{
		osal_printk_err("%s - proc_mkdir failed!\n", __FUNCTION__);
	}
	
	buf = (char* )k_malloc(OSAL_BULK_SIZE*size);	
	if(!buf){
		osal_printk_err("%s - malloc failed\n", __FUNCTION__);
		k_free(ctx);
		return NULL;
	}
	memset(buf, 0, OSAL_BULK_SIZE*size);
    ctx->log_buf     = buf;
    ctx->logbuf_size = OSAL_BULK_SIZE*size - 1;
    
	proc_entry = osal_create_proc_entry(file_name, proc_dir);
	if(proc_entry == NULL)
	{	    
	    osal_printk_err("proc_create_data error! \n");
	    k_free(buf);
		k_free(ctx);
		return NULL;
	}

	proc_entry->read  = osal_print_read;
	proc_entry->write = osal_print_write;
	proc_entry->private = ctx;

	ctx->file_name = k_malloc(strlen(file_name)+1);
	memset(ctx->file_name, 0, strlen(file_name)+1);
	ctx->file_name[strlen(file_name)] = 0;
	if(NULL == ctx->file_name)
	{
	    osal_printk_err("malloc filename error\n");
	    osal_remove_proc_entry(file_name,proc_dir);
	    k_free(buf);
		k_free(ctx);
		return NULL;
	    
	}
	len = sizeof(ctx->file_name) - 1;
	if (len > strlen(file_name))
		len = strlen(file_name);
	strncpy(ctx->file_name,file_name,len);
	ctx->parent_entry = proc_dir;
	ctx->log_entry    = proc_entry;
	osal_spin_lock_init(&ctx->lock);
	ctx->around = OSAL_PRINT_NO;
	ctx->log_head = ctx->log_buf;
	//osal_printk("ctx->file_name %s\n",ctx->file_name);
	return ctx;

}

int osal_print_destroy(void)
{
    osal_remove_proc_entry(osal_print_s->file_name,osal_print_s->parent_entry);
	osal_remove_proc_entry("logdir", NULL);
    if(!osal_print_s->file_name)
	{
		k_free(osal_print_s->file_name);
	}
	if(osal_print_s->log_buf)
	{
		k_free(osal_print_s->log_buf);
	}
	if(osal_print_s)
	{
		k_free(osal_print_s);
	}
    if(log_flag != NULL)
	{
		osal_remove_proc_entry("log_flag", NULL);
	}
    
	return 0;
}

int osal_print_store(osal_print_t* ctx, const char *fmt, va_list args)
{
	size_t len;
	size_t remain;
	char *buf = (char *)k_malloc(4096);
    if (buf == NULL) {
		osal_printk_err("print error\n");
        return 0;
    }
	//osal_printk("%s enter\n", __func__);
	osal_spin_lock(&ctx->lock);
	len = vsprintf(buf, fmt, args);
	//check around;
    remain = (ctx->logbuf_size - (ctx->log_head - ctx->log_buf));
    if(len > remain)
    {
        memcpy(ctx->log_head,buf,remain);
        memcpy(ctx->log_buf,buf+remain,len - remain);
        ctx->log_head = ctx->log_buf+len - remain;
        ctx->around   = OSAL_PRINT_YES;
        
    }
    else
    {
        memcpy(ctx->log_head,buf,len);
        ctx->log_head += len;
    }
	if(osal_print_s->win_disp >= OSAL_PRINT_YES)
	{
		osal_print_tty(buf);
	}
	osal_spin_unlock(&ctx->lock);

	k_free(buf);
	//osal_printk("%s exit\n", __func__);
	return len;
}

int osal_print_out(const char *fmt, ...)
{
    int p;
	unsigned long flags;
	va_list args;

	osal_spin_lock_irqsave(&print_lock, &flags);

	va_start(args, fmt);
	p = osal_print_store(osal_print_s, fmt, args);
	va_end(args);

	osal_spin_unlock_irqrestore(&print_lock, &flags);

	return p;
}

static int osal_printk_lvl; 

static int osal_printk_show(osal_proc_entry_t *s)
{
	if (osal_printk_lvl == OSAL_LVL_ERR)
		osal_seq_printf(s, "err\n");
	else if (osal_printk_lvl == OSAL_LVL_WARN)
		osal_seq_printf(s, "war\n");
	else if (osal_printk_lvl == OSAL_LVL_EVENT)
		osal_seq_printf(s, "inf\n");
	else if (osal_printk_lvl == OSAL_LVL_INFO)
		osal_seq_printf(s, "dbg\n");

    return 0;
}

static int osal_printk_write(osal_proc_entry_t *entry, const char *buf, int count, long long *loggt)
{
	if(count == 3)
	{
		if(buf[0] == 'e' && buf[1] == 'r' && buf[2] == 'r')
		{
			osal_printk_lvl = OSAL_LVL_ERR;
		}
		else if(buf[0] == 'w' && buf[1] == 'a' && buf[2] == 'r')
		{
			osal_printk_lvl = OSAL_LVL_WARN;
		}
		else if(buf[0] == 'i' && buf[1] == 'n' && buf[2] == 'f')
		{
			osal_printk_lvl = OSAL_LVL_EVENT;
		}
		else if(buf[0] == 'd' && buf[1] == 'b' && buf[2] == 'g')
		{
			osal_printk_lvl = OSAL_LVL_INFO;
		}
		else {
			osal_printk_err("[%d]pls input err|war|inf|dbg\n", count);
		}
	} else {
		osal_printk_err("[%d]pls input err|war|inf|dbg\n", count);
	}

	return count;
}

static void osal_printk_init(void)
{
    osal_proc_entry_t* log_flag_lvl = osal_create_proc_entry("prtk_lvl", NULL);

	if(log_flag_lvl != NULL)
	{
		log_flag_lvl->read  = osal_printk_show;
		log_flag_lvl->write = osal_printk_write;
	}
}

int osal_printk_get_level(void)
{
	return osal_printk_lvl;
}

int osal_printk_set_level(int lvl)
{
	return osal_printk_lvl = lvl;
}

int osal_print_init(int size)
{
	if (osal_spin_lock_init(&print_lock)) {
		osal_printk_err("%s - init lock error!\n", __FUNCTION__);
		return -1;
	}

    osal_print_s = osal_print_create("print",size);
	if(osal_print_s == NULL)
	{
		return -1;
	}
	log_flag  = osal_create_proc_entry("log_flag",NULL);
	if(log_flag != NULL)
	{
		log_flag->read  = osal_print_state_show;
		log_flag->write = osal_print_state_write;
	}

	osal_print_s->level = 2;

	osal_printk_init();
	osal_printk_lvl = OSAL_LVL_EVENT;

	memset(module_close, 0, sizeof(module_close));

	return 0;
}

