#ifndef SA_OSAL_H
#define SA_OSAL_H
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "osal_list.h"
#include "osal_module_id.h"
#include "osal_err_id.h"

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>

#ifndef SDK_INIT_DEVICE
#define OSAL_INIT_PREV_EXPORT(fn)            SYS_INIT(fn, PRE_KERNEL_1, CONFIG_KERNEL_INIT_PRIORITY_OBJECTS)
/* device initialization */
#define OSAL_INIT_DEVICE_EXPORT(fn)          SYS_INIT(fn, PRE_KERNEL_1, CONFIG_KERNEL_INIT_PRIORITY_DEVICE)
/* components initialization (dfs, lwip, ...) */
#define OSAL_INIT_COMPONENT_EXPORT(fn)       SYS_INIT(fn, PRE_KERNEL_2, (CONFIG_KERNEL_INIT_PRIORITY_DEVICE +1))
/* environment initialization (mount disk, ...) */
#define OSAL_INIT_ENV_EXPORT(fn)             SYS_INIT(fn, POST_KERNEL, (CONFIG_KERNEL_INIT_PRIORITY_DEVICE +2))
/* application initialization (rtgui application etc ...) */
#define OSAL_INIT_APP_EXPORT(fn)             SYS_INIT(fn, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY)
/* init after mount fs */
#define OSAL_INIT_FS_EXPORT(fn)              SYS_INIT(fn, APPLICATION, (CONFIG_KERNEL_INIT_PRIORITY_DEVICE +1))
/* application SERVICE initialization (service ...) */
#define OSAL_INIT_APP_SERVICE_EXPORT(fn)     SYS_INIT(fn, APPLICATION, (CONFIG_KERNEL_INIT_PRIORITY_DEVICE +2))
/* application SDK initialization (sdk ...) */
#define OSAL_INIT_APP_SDK_EXPORT(fn)         SYS_INIT(fn, APPLICATION, (CONFIG_KERNEL_INIT_PRIORITY_DEVICE +3))
/* application FINAL initialization (app ...) */
#define OSAL_INIT_APP_APPLICATION_EXPORT(fn) SYS_INIT(fn, APPLICATION, (CONFIG_KERNEL_INIT_PRIORITY_DEVICE +4))
#else
#define OSAL_INIT_PREV_EXPORT(fn)
#define OSAL_INIT_DEVICE_EXPORT(fn)
#define OSAL_INIT_COMPONENT_EXPORT(fn)
#define OSAL_INIT_ENV_EXPORT(fn)
#define OSAL_INIT_APP_EXPORT(fn)
#define OSAL_INIT_FS_EXPORT(fn)
#define OSAL_INIT_APP_SERVICE_EXPORT(fn)
#define OSAL_INIT_APP_SDK_EXPORT(fn)
#define OSAL_INIT_APP_APPLICATION_EXPORT(fn)
#endif /* SDK_INIT_DEVICE */


#define OSAL_ALIGN(x, a) ((x+a-1)/a*a)
#define OSAL_ALIGN_DOWN(x, a) ((x)/a*a)
	
struct osal_module_symtab
{
    void       *addr;
    const char *name;
};


#define ERESTARTSYS        512

#ifndef rt_container_of
#define rt_container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - (unsigned long)(&((type *)0)->member)))
#endif

#define NAME_LENGTH 24
#ifndef _IOC_TYPECHECK
#include "osal_ioctl.h"
#endif

#define OSAL_MAX_DEV_NAME_LEN 32
struct osal_pdata_content {
    char name[OSAL_MAX_DEV_NAME_LEN];
    void *data;
};

// proc
struct seq_file {
	char *buf;
	int size;
	int count;
	void *private;
};

typedef struct osal_proc_dir_entry {
    char name[50];
    void *proc_dir_entry;
    int (*open)(struct osal_proc_dir_entry *entry);
    int (*read)(struct osal_proc_dir_entry *entry);
    int (*write)(struct osal_proc_dir_entry *entry, const char *buf, int count, long long *);
    void *private;
    void *seqfile;
    struct osal_list_head node;
} osal_proc_entry_t;
extern osal_proc_entry_t *osal_create_proc_entry(const char *name, osal_proc_entry_t *parent);
extern osal_proc_entry_t *osal_proc_mkdir(const char *name, osal_proc_entry_t *parent);
extern void osal_remove_proc_entry(const char *name, osal_proc_entry_t *parent);
extern int osal_seq_printf(osal_proc_entry_t *entry, const char *fmt, ...) __attribute__((format(printf, 2, 3)));

#define OSAL_SEEK_SET      0
#define OSAL_SEEK_CUR      1
#define OSAL_SEEK_END      2

typedef struct osal_poll {
    void *poll_table;
    void *data;
} osal_poll_t;

#define OSAL_POLLIN        0x0001
#define OSAL_POLLPRI       0x0002
#define OSAL_POLLOUT       0x0004
#define OSAL_POLLERR       0x0008
#define OSAL_POLLHUP       0x0010
#define OSAL_POLLNVAL      0x0020
#define OSAL_POLLRDNORM    0x0040
#define OSAL_POLLRDBAND    0x0080
#define OSAL_POLLWRNORM    0x0100

typedef struct osal_fileops {
    int (*open)(void *private_data);
    int (*read)(char *buf, int size, long *offset, void *private_data);
    int (*write)(const char *buf, int size, long *offset, void *private_data);
    long (*llseek)(long offset, int whence, void *private_data);
    int (*release)(void *private_data);
    long (*unlocked_ioctl)(unsigned int cmd, unsigned long arg, void *private_data);
    unsigned int (*poll)(osal_poll_t *osal_poll, void *private_data);
    //int (*mmap)(osal_vm_t *vm, unsigned long start, unsigned long end, unsigned long vm_pgoff, void *private_data);
    long (*compat_ioctl)(unsigned int cmd, unsigned long arg, void *private_data);
} osal_fileops_t;

typedef struct osal_dev {
    char name[48];
    const struct osal_fileops *fops;
	struct osal_list_head list;
	struct osal_list_head node;
#ifdef RT_USING_PM
    struct osal_pmops *osal_pmops;
#endif
	void *priv_data;
} osal_dev_t;

typedef struct osal_vm {
    void *vm;
} osal_vm_t;

typedef struct osal_pmops {
    int (*pm_prepare)(osal_dev_t *dev);
    void (*pm_complete)(osal_dev_t *dev);
    int (*pm_suspend)(osal_dev_t *dev);
    int (*pm_resume)(osal_dev_t *dev);
    int (*pm_freeze)(osal_dev_t *dev);
    int (*pm_thaw)(osal_dev_t *dev);
    int (*pm_poweroff)(osal_dev_t *dev);
    int (*pm_restore)(osal_dev_t *dev);
    int (*pm_suspend_late)(osal_dev_t *dev);
    int (*pm_resume_early)(osal_dev_t *dev);
    int (*pm_freeze_late)(osal_dev_t *dev);
    int (*pm_thaw_early)(osal_dev_t *dev);
    int (*pm_poweroff_late)(osal_dev_t *dev);
    int (*pm_restore_early)(osal_dev_t *dev);
    int (*pm_suspend_noirq)(osal_dev_t *dev);
    int (*pm_resume_noirq)(osal_dev_t *dev);
    int (*pm_freeze_noirq)(osal_dev_t *dev);
    int (*pm_thaw_noirq)(osal_dev_t *dev);
    int (*pm_poweroff_noirq)(osal_dev_t *dev);
    int (*pm_restore_noirq)(osal_dev_t *dev);
} osal_pmops_t;

typedef struct osal_mutex {
    struct k_mutex	mutex;
} osal_mutex_t;

typedef struct osal_spinlock {
    k_spinlock_key_t key;
	struct k_spinlock lock;
} osal_spinlock_t;

extern osal_dev_t *osal_createdev(const char *name);
extern int osal_destroydev(osal_dev_t *pdev);
extern int osal_registerdevice(osal_dev_t *pdev);
extern void osal_deregisterdevice(osal_dev_t *pdev);

extern void *osal_dma_alloc_coherent(osal_dev_t *dev, int size, unsigned long *dma_handle);
extern void osal_dma_free_coherent(osal_dev_t *dev, int size, void *cpu_addr, unsigned long dma_handle);
extern void osal_dma_map_single(osal_dev_t *dev, void *ptr, int size, int dir_to_dev);
extern void osal_dma_unmap_single(osal_dev_t *dev, void *ptr, int size, int dir_to_dev);
extern void osal_poll_requested_events(osal_poll_t *poll, unsigned int *event);

extern int osal_mutex_init(osal_mutex_t *mutex);
extern int osal_mutex_lock(osal_mutex_t *mutex);
extern int osal_mutex_trylock(osal_mutex_t *mutex);
extern int osal_mutex_lock_interruptible(osal_mutex_t *mutex);
extern void osal_mutex_unlock(osal_mutex_t *mutex);
extern void osal_mutex_destory(osal_mutex_t *mutex);

extern int osal_spin_lock_init(osal_spinlock_t *lock);
extern void osal_spin_lock(osal_spinlock_t *lock);
extern int osal_spin_trylock(osal_spinlock_t *lock);
extern void osal_spin_unlock(osal_spinlock_t *lock);
extern void osal_spin_lock_irqsave(osal_spinlock_t *lock, unsigned long *flags);
extern void osal_spin_unlock_irqrestore(osal_spinlock_t *lock, unsigned long *flags);
// notice:must be called when kmod exit, other wise will lead to memory leak;
extern void osal_spin_lock_destory(osal_spinlock_t *lock);

// debug
extern void __osal_assert(void);
#define OSAL_BUG() __osal_assert()

#define OSAL_ASSERT(expr)                       \
    do {                                        \
        if (!(expr)) {                          \
            osal_printk_err("ASSERT: failure at %s:%d/%s()!\n", __FILE__, __LINE__, __func__); \                       \
            __osal_assert();                         \
        }                                       \
    } while (0)

#define OSAL_BUG_ON(expr)                                                               \
    do {                                                                                \
        if (expr) {                                                                     \
            osal_printk_err("BUG: failure at %s:%d/%s()!\n", __FILE__, __LINE__, __func__); \
            __osal_assert();                                                                 \
        }                                                                               \
    } while (0)

// timer
typedef struct osal_timer {
    void *timer;
    void (*function)(unsigned long);
    unsigned long data;
	unsigned int count; //仅用于osal内部使用
} osal_timer_t;

typedef struct osal_timeval {
    long tv_sec;
    long tv_usec;
} osal_timeval_t;

typedef struct osal_rtc_time {
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year;
    int tm_wday;
    int tm_yday;
    int tm_isdst;
} osal_rtc_time_t;

/* Return values for the timer callback function */
typedef enum hiOSAL_HRTIMER_RESTART_E {
    OSAL_HRTIMER_NORESTART, /* < The timer will not be restarted. */
    OSAL_HRTIMER_RESTART /* < The timer must be restarted. */
} OSAL_HRTIMER_RESTART_E;

/* hrtimer struct */
typedef struct osal_hrtimer {
    void *timer;
    OSAL_HRTIMER_RESTART_E (*function)(void *timer);
    unsigned long interval; /* Unit ms */
	unsigned int count; //仅用于osal内部使用
} osal_hrtimer_t;

extern int osal_hrtimer_create(osal_hrtimer_t *phrtimer);
extern int osal_hrtimer_start(osal_hrtimer_t *phrtimer);
extern int osal_hrtimer_cancel(osal_hrtimer_t *phrtimer);
extern int osal_hrtimer_destory(osal_hrtimer_t *phrtimer);

extern int osal_timer_create(osal_timer_t *timer);
extern int osal_timer_set(osal_timer_t *timer, unsigned long interval);  // ms
extern int osal_timer_del(osal_timer_t *timer);
extern int osal_timer_destory(osal_timer_t *timer);

#define osal_timer_init osal_timer_create
#define osal_set_timer osal_timer_set
#define osal_del_timer osal_timer_del

extern unsigned long osal_msleep(unsigned int msecs);
extern void osal_udelay(unsigned int usecs);
extern void osal_mdelay(unsigned int msecs);

extern unsigned int osal_get_tickcount(void);
extern unsigned long long osal_sched_clock(void);
extern void osal_gettimeofday(osal_timeval_t *tv);
extern void osal_rtc_time_to_tm(unsigned long time, osal_rtc_time_t *tm);
extern void osal_rtc_tm_to_time(osal_rtc_time_t *tm, unsigned long *time);
extern int osal_rtc_valid_tm(struct osal_rtc_time *tm);
extern void osal_getjiffies(unsigned long long *pjiffies);

#define OSAL_O_ACCMODE     00000003
#define OSAL_O_RDONLY      00000000
#define OSAL_O_WRONLY      00000001
#define OSAL_O_RDWR        00000002
#define OSAL_O_CREAT       00000100

extern void *osal_klib_fopen(const char *filename, int flags, int mode);
extern void osal_klib_fclose(void *filp);
extern int osal_klib_fwrite(char *buf, int len, void *filp);
extern int osal_klib_fread(char *buf, unsigned int len, void *filp);

typedef unsigned int gfp_t;

#define osal_gfp_kernel    0
#define osal_gfp_atomic    1
extern void *osal_vmalloc(unsigned long size);
extern void osal_vfree(const void *addr);
extern void *osal_kmalloc(unsigned long size, unsigned int osal_gfp_flag);
extern void *osal_kzalloc(unsigned long size, unsigned int osal_gfp_flag);
extern void osal_kfree(const void *addr);
extern void *osal_kmalloc_align(unsigned long size, unsigned int osal_gfp_flag, unsigned int align);
extern void osal_kfree_align(void *addr);

#define MALLOC_DEBUG

extern void *osal_vmalloc_debug(int module, unsigned long size);
extern void *osal_vmalloc_debug_ext(int module,char *mod_name, unsigned long size);
extern void osal_vfree_debug(int module, const void *addr);
extern void *osal_kmalloc_debug(int module, unsigned long size, unsigned int osal_gfp_flag);
extern void *osal_kmalloc_debug_ext(int module,char *mod_name, unsigned long size, unsigned int osal_gfp_flag);
extern void osal_kfree_debug(int module, const void *addr);

// addr translate
extern void *osal_ioremap(unsigned long phys_addr, unsigned int size);
extern void *osal_ioremap_nocache(unsigned long phys_addr, unsigned int size);
extern void *osal_ioremap_cached(unsigned long phys_addr, unsigned int size);
extern void *osal_ioremap_wc(unsigned long phys_addr, unsigned int size);
extern void osal_iounmap(void *addr);

#define osal_readl(x) (*((volatile unsigned int *)(x)))
#define osal_writel(v, x) (*((volatile unsigned int *)(x)) = (v))

extern unsigned long osal_copy_from_user(void *to, const void *from, unsigned long n);
extern unsigned long osal_copy_to_user(void *to, const void *from, unsigned long n);
extern int osal_access_ok(const void *addr, unsigned long size);
extern unsigned long osal_virt_to_phys(void *va);
extern void *osal_phys_to_virt(unsigned long pa);

#define OSAL_VERIFY_READ   0
#define OSAL_VERIFY_WRITE  1

// semaphore api
#define EINTR              4
typedef struct osal_semaphore {
    struct k_sem sem;
} osal_semaphore_t;
extern int osal_sema_init(osal_semaphore_t *sem, int val);
extern int osal_down(osal_semaphore_t *sem);
extern int osal_down_interruptible(osal_semaphore_t *sem);
extern int osal_down_trylock(osal_semaphore_t *sem);
extern void osal_up(osal_semaphore_t *sem);
// notice:must be called when kmod exit, other wise will lead to memory leak;
extern void osal_sema_destory(osal_semaphore_t *sem);

// task api
typedef int (*threadfn_t)(void *data);
typedef struct osal_task {
    void *task_struct;
} osal_task_t;
extern osal_task_t *osal_kthread_create(threadfn_t thread, void *data, const char *name);
extern void osal_kthread_destory(osal_task_t *task, unsigned int stop_flag);

// shedule
extern void osal_yield(void);

// interrupt api
enum osal_irqreturn {
    OSAL_IRQ_NONE = (0 << 0),
    OSAL_IRQ_HANDLED = (1 << 0),
    OSAL_IRQ_WAKE_THREAD = (1 << 1),
};
typedef int (*osal_irq_handler_t)(int, void *);

#ifndef CONFIG_DYNAMIC_INTERRUPTS
extern void osal_isr_handler(const void *param);
extern void osal_isr_register(unsigned int irq, osal_irq_handler_t handler, osal_irq_handler_t thread_fn, const char *name, void *dev);
#define osal_request_irq(irq, handler, thread_fn, name, dev) do { \
	osal_isr_register(irq, handler, thread_fn, name, dev);\
	IRQ_CONNECT(irq, 2, osal_isr_handler, irq, 0) \
	irq_enable(irq); \
	} while (0); 
#else
extern int osal_request_irq(unsigned int irq, osal_irq_handler_t handler, osal_irq_handler_t thread_fn, const char *name, void *dev);
#endif

extern void osal_free_irq(unsigned int irq, void *dev);
extern int osal_in_interrupt(void);

typedef struct osal_workqueue {
	struct k_work work;//内部使用
	void (*workq_func)(void *workq_data);
	void *param;
} osal_workqueue_t;
extern void osal_workq_init(osal_workqueue_t *wq);
extern int osal_workq_schedule(osal_workqueue_t *wq);
extern int osal_workq_cancel(osal_workqueue_t *wq);


extern void osal_interrupt_mask(int irq);
extern void osal_interrupt_umask(int irq);


// string api
extern char *osal_strcpy(char *s1, const char *s2);
extern int osal_strlcpy(char *s1, const char *s2, int size);
extern char *osal_strcat(char *s1, const char *s2);
extern char *osal_strncat(char *s1, const char *s2, int size);
extern int osal_strlcat(char *s1, const char *s2, int size);
extern int osal_strnicmp(const char *s1, const char *s2, int size);
extern int osal_strncasecmp(const char *s1, const char *s2, int n);
extern char *osal_strchr(const char *s, int n);
extern char *osal_strnchr(const char *s, int count, int c);
extern char *osal_strrchr(const char *s, int c);
extern char *osal_strnstr(const char *s1, const char *s2, int n);
extern char *osal_strpbrk(const char *s1, const char *s2);
extern char *osal_strsep(char **s, const char *ct);
extern int osal_strspn(const char *s, const char *accept);
extern int osal_strcspn(const char *s, const char *reject);
extern void *osal_memscan(void *addr, int c, int size);
extern void *osal_memchr(const void *s, int c, int n);
extern void *osal_memchr_inv(const void *s, int c, int n);

extern void *osal_memcpy(void *dst, const void *src, unsigned long count);
extern int osal_memcmp(const void *cs, const void *ct, size_t count);
extern void *osal_memset(void *src, int c, unsigned long n);
extern void *osal_memset_io(void *src, int c, unsigned long n);
extern void *osal_memmove(void *dest, const void *src, unsigned long n);
extern char *osal_strstr(const char *str1, const char *str2);
extern char *osal_strncpy(char *dest, const char *src, size_t n);
extern int osal_strncmp(const char *cs, const char *ct, size_t count);
extern int osal_strcmp(const char *cs, const char *ct);
extern int osal_strcasecmp(const char *a, const char *b);
extern size_t osal_strlen(const char *src);
extern size_t osal_strnlen(const char *s, unsigned long maxlen);


#define osal_va_list       va_list
extern unsigned long long osal_strtoull(const char *cp, char **endp, unsigned int base);
extern unsigned long osal_strtoul(const char *cp, char **endp, unsigned int base);
extern long osal_strtol(const char *cp, char **endp, unsigned int base);
extern long long osal_strtoll(const char *cp, char **endp, unsigned int base);
extern int osal_scnprintf(char *buf, int size, const char *fmt, ...) __attribute__((format(printf, 3, 4)));
extern int osal_sscanf(const char *buf, const char *fmt, ...);

// cache api
extern void osal_flush_cache_all(void);
extern void osal_cpuc_flush_dcache_area(void *addr, int size);
extern void osal_flush_dcache_area(void *kvirt, unsigned long phys_addr, unsigned long length);
extern int osal_flush_dcache_all(void);
extern void osal_invalid_dcache_area(void *kvirt, unsigned long length);

// workqueue api
typedef struct osal_work_struct {
    struct k_work work;//内部使用
	void *wq;
    void (*func)(struct osal_work_struct *work);
} osal_work_struct_t;
typedef void (*osal_work_func_t)(struct osal_work_struct *work);

extern int osal_init_work(struct osal_work_struct *work, osal_work_func_t func);

#define OSAL_INIT_WORK(_work, _func)      \
    do {                                  \
        osal_init_work((_work), (_func)); \
    } while (0)

extern int osal_schedule_work(struct osal_work_struct *work);
extern void osal_destroy_work(struct osal_work_struct *work);
extern int osal_schedule_highpri_work(struct osal_work_struct *work);

// atomic api
typedef struct {
    void *atomic;
} osal_atomic_t;

#define OSAL_ATOMIC_INIT(i)  { (i) }

extern int osal_atomic_init(osal_atomic_t *atomic);
extern void osal_atomic_destory(osal_atomic_t *atomic);
extern int osal_atomic_read(osal_atomic_t *v);
extern void osal_atomic_set(osal_atomic_t *v, int i);
extern int osal_atomic_inc_return(osal_atomic_t *v);
extern int osal_atomic_dec_return(osal_atomic_t *v);

// wait api
typedef int (*osal_wait_cond_func_t)(const void *param);

typedef struct osal_wait {
    void *wait;
} osal_wait_t;
#define ERESTARTSYS        512

extern unsigned long osal_msecs_to_jiffies(const unsigned int m);
extern int osal_wait_init(osal_wait_t *wait);
extern int osal_wait_interruptible(osal_wait_t *wait, osal_wait_cond_func_t func, void *param);
extern int osal_wait_uninterruptible(osal_wait_t *wait, osal_wait_cond_func_t func, void *param);
extern int osal_wait_timeout_interruptible(osal_wait_t *wait, osal_wait_cond_func_t func, void *param,
                                           int ms);
extern int osal_wait_timeout_uninterruptible(osal_wait_t *wait, osal_wait_cond_func_t func, void *param,
                                             int ms);

#define osal_wait_event_interruptible(wait, func, param)                       \
    ({                                                                         \
        int __ret = 0;                                                         \
                                                                               \
        for (;;) {                                                             \
            if (func(param)) {                                                 \
                __ret = 0;                                                     \
                break;                                                         \
            }                                                                  \
            __ret = osal_wait_interruptible(wait, (func), param); \
            if (__ret < 0)                                                     \
                break;                                                         \
        }                                                                      \
        __ret;                                                                 \
    })

#define osal_wait_event_uninterruptible(wait, func, param)          \
    ({                                                              \
        int __ret = 0;                                              \
                                                                    \
        for (;;) {                                                  \
            if (func(param)) {                                      \
                __ret = 0;                                          \
                break;                                              \
            }                                                       \
            __ret = osal_wait_uninterruptible(wait, (func), param); \
            if (__ret < 0)                                          \
                break;                                              \
        }                                                           \
        __ret;                                                      \
    })

#define osal_wait_event_timeout_interruptible(wait, func, param, timeout)        \
    ({                                                                           \
        int __ret = timeout;                                                     \
                                                                                 \
        if ((func(param)) && !timeout) {                                         \
            __ret = 1;                                                           \
        }                                                                        \
                                                                                 \
        for (;;) {                                                               \
            if (func(param)) {                                                   \
                __ret = osal_msecs_to_jiffies(__ret);                            \
                break;                                                           \
            }                                                                    \
            __ret = osal_wait_timeout_interruptible(wait, (func), param, __ret); \
            if (!__ret || __ret < 0)                                 \
                break;                                                           \
        }                                                                        \
        __ret;                                                                   \
    })

#define osal_wait_event_timeout_uninterruptible(wait, func, param, timeout)        \
    ({                                                                             \
        int __ret = timeout;                                                       \
                                                                                   \
        if ((func(param)) && !timeout) {                                           \
            __ret = 1;                                                             \
        }                                                                          \
                                                                                   \
        for (;;) {                                                                 \
            if (func(param)) {                                                     \
                __ret = osal_msecs_to_jiffies(__ret);                              \
                break;                                                             \
            }                                                                      \
            __ret = osal_wait_timeout_uninterruptible(wait, (func), param, __ret); \
            if (!__ret || __ret < 0)                                 \
                break;                                              \
        }                                                           \
        __ret;                                                      \
    })

#define OSAL_POLLIN        0x0001
#define OSAL_POLLPRI       0x0002
#define OSAL_POLLOUT       0x0004
#define OSAL_POLLERR       0x0008
#define OSAL_POLLHUP       0x0010
#define OSAL_POLLNVAL      0x0020
#define OSAL_POLLRDNORM    0x0040
#define OSAL_POLLRDBAND    0x0080
#define OSAL_POLLWRNORM    0x0100

extern void osal_wakeup(osal_wait_t *wait);  // same as wake_up_all
extern void osal_wait_destory(osal_wait_t *wait);
extern void osal_poll_wait(osal_poll_t *table, osal_wait_t *wait);

//PRINT

#define OSAL_LVL_INFO    3
#define OSAL_LVL_EVENT   2
#define OSAL_LVL_WARN    1
#define OSAL_LVL_ERR     0
#define OSAL_MODULEL     "MPP"

int  osal_print_get_level(void);
int  osal_print_get_module_state(int idx);
int osal_print_out(const char *fmt, ...);

#define osal_print_dbg(level, module, fmt, ...) \
	do { \
		if ((osal_print_get_level() >= level) && !osal_print_get_module_state(module)) \
			osal_print_out("[MOD]:%s [Func]:%s [Line]:%d [Info]:"fmt"", \
									#module, __func__, __LINE__, ##__VA_ARGS__);\
	} while(0)

extern int osal_printk_get_level(void);
//extern int osal_printk_raw(const char *fmt, ...);

#define __osal_printk(lvl, fmt, ...) \
	do { \
		if (osal_printk_get_level() >= lvl) \
			printk(fmt, ##__VA_ARGS__);\
	} while(0)

#define osal_printk_err(fmt, ...) __osal_printk(OSAL_LVL_ERR, "\033[1;31m"fmt"\033[0m", ##__VA_ARGS__)
#define osal_printk_war(fmt, ...) __osal_printk(OSAL_LVL_WARN, fmt, ##__VA_ARGS__)
#define osal_printk_inf(fmt, ...) __osal_printk(OSAL_LVL_EVENT, fmt, ##__VA_ARGS__)
#define osal_printk_dbg(fmt, ...) __osal_printk(OSAL_LVL_INFO, fmt, ##__VA_ARGS__)
#define osal_printk osal_printk_inf

extern int osal_gpio_request(unsigned int gpio, const char *label);
extern void osal_gpio_free(unsigned int gpio);
extern void osal_gpio_set_value(unsigned int gpio, int value);
extern int osal_gpio_get_value(unsigned int gpio);
extern int osal_gpio_direction_output(unsigned int gpio, int value);
extern int osal_gpio_direction_input(unsigned int gpio);

typedef struct osal_i2c_data_w
{
    unsigned int reg_addr;
    unsigned int data;
}osal_i2c_data_w;

extern int osal_i2c_write(unsigned char i2c_dev, unsigned char dev_addr,
                               unsigned int reg_addr, unsigned int reg_addr_num,
                               unsigned int data, unsigned int data_byte_num);

extern int osal_i2c_read(unsigned char i2c_dev, unsigned char dev_addr,
                               unsigned int reg_addr, unsigned int reg_addr_num,
                               unsigned int *data, unsigned int data_byte_num);

int osal_i2c_write_burst(unsigned char i2c_dev, unsigned char dev_addr,
                               unsigned int reg_addr, unsigned int reg_addr_num,
                               unsigned char *data, unsigned int data_burst_byte_len);

int osal_i2c_write_batch(unsigned char i2c_dev, unsigned char dev_addr,
                            struct osal_i2c_data_w* data_w, unsigned int data_len,
                            unsigned int reg_addr_num,  unsigned int data_byte_num);
                            
/* Hibernation and suspend events */
#define OSAL_PM_HIBERNATION_PREPARE  0x0001 /* Going to hibernate */
#define OSAL_PM_POST_HIBERNATION     0x0002 /* Hibernation finished */
#define OSAL_PM_SUSPEND_PREPARE      0x0003 /* Going to suspend the system */
#define OSAL_PM_POST_SUSPEND         0x0004 /* Suspend finished */
#define OSAL_PM_RESTORE_PREPARE      0x0005 /* Going to restore a saved image */
#define OSAL_PM_POST_RESTORE         0x0006 /* Restore failed */

//noyifier
#define OSAL_NOTIFY_DONE             0x0000          /* Don't care */
#define OSAL_NOTIFY_OK               0x0001          /* Suits me */
#define OSAL_NOTIFY_STOP_MASK        0x8000          /* Don't call further */
#define OSAL_NOTIFY_BAD              (OSAL_NOTIFY_STOP_MASK|0x0002)
#define OSAL_NOTIFY_STOP             (OSAL_NOTIFY_OK|OSAL_NOTIFY_STOP_MASK)

struct osal_notifier_block {
    int (*notifier_call)(struct osal_notifier_block *nb, unsigned long action, void *data);
    void *notifier_block;
};
typedef int (*osal_notifier_fn_t)(struct osal_notifier_block *nb, unsigned long action, void *data);
int osal_register_notifier(void *list, struct osal_notifier_block *nb);
int osal_unregister_notifier(void *list, struct osal_notifier_block *nb);
int osal_notifier_call(void *list, unsigned long val, void *v);
#define OSAL_BLOCKING_NOTIFIER_HEAD(name)	OSAL_LIST_HEAD(name)

extern int osal_get_random_value(void);

extern unsigned long osal_mmz_alloc(char *name, unsigned int size, unsigned int align);
extern void osal_mmz_free(unsigned long paddr);
extern void *osal_mmz_remap(unsigned long paddr, unsigned int size, unsigned char cached);
extern void osal_mmz_unmap(void *vaddr);

extern unsigned long osal_mmz_sram_alloc(char *name, unsigned int size, unsigned int align);
extern void osal_mmz_sram_free(unsigned long paddr);
extern void *osal_mmz_sram_remap(unsigned long paddr, unsigned int size, unsigned char cached);
extern void osal_mmz_sram_unmap(void *vaddr);

// math
extern unsigned long long osal_div_u64(unsigned long long dividend, unsigned int divisor);
extern long long osal_div_s64(long long dividend, int divisor);
extern unsigned long long osal_div64_u64(unsigned long long dividend, unsigned long long divisor);
extern long long osal_div64_s64(long long dividend, long long divisor);
extern unsigned long long osal_div_u64_rem(unsigned long long dividend, unsigned int divisor);
extern long long osal_div_s64_rem(long long dividend, int divisor);
extern unsigned long long osal_div64_u64_rem(unsigned long long dividend, unsigned long long divisor);

#define osal_random osal_get_random_value

#define osal_max(x, y) ({                            \
        __typeof__(x) _max1 = (x);                  \
        __typeof__(y) _max2 = (y);                  \
        (void) (&_max1 == &_max2);              \
        _max1 > _max2 ? _max1 : _max2; })

#define osal_min(x, y) ({                \
    __typeof__(x) _min1 = (x);          \
     __typeof__(y) _min2 = (y);          \
     (void) (&_min1 == &_min2);      \
     _min1 < _min2 ? _min1 : _min2; })

#define osal_abs(x) ({                \
    long ret;                         \
    if (sizeof(x) == sizeof(long)) {  \
        long __x = (x);               \
        ret = (__x < 0) ? -__x : __x; \
    } else {                          \
        int __x = (x);                \
        ret = (__x < 0) ? -__x : __x; \
    }                                 \
    ret;                              \
})

/*dmacpy functions*/
typedef void (*osal_dma_cb)(void *param);
struct osal_dmacpy_sg_e {
	int size;
	unsigned long data_addr;
};

struct osal_dmacpy_info {
	unsigned long dev;
	unsigned long channel;
	int to_dev;
	unsigned char dst_addr_width;
	unsigned char dst_maxburst;
	unsigned char src_addr_width;
	unsigned char src_maxburst;

	unsigned long src; //slave_single or cyclic
	unsigned long dst; //slave_single or cyclic
	size_t size; //slave_single or cyclic

	size_t period_len; //cyclic
	unsigned long buf_addr; //cyclic

	int desc_num; //slave_sg
	unsigned long fifo_addr; //slave_sg
	struct osal_dmacpy_sg_e e[0];
};
extern unsigned long osal_dmacpy_init(unsigned long dev);
extern void osal_dmacpy_deinit(int channel);
extern int osal_dmacpy_start(int fd, unsigned long sar, unsigned long dar, unsigned int len);

extern void osal_mb(void);
extern void osal_rmb(void);
extern void osal_wmb(void);
extern void osal_smp_mb(void);
extern void osal_smp_rmb(void);
extern void osal_smp_wmb(void);
extern void osal_isb(void);
extern void osal_dsb(void);
extern void osal_dmb(void);

typedef struct chip_info {
	uint8_t no[2];
	uint8_t package;
	struct {
        uint8_t year;
        uint8_t month;
        uint8_t day;
        uint8_t hour;
        uint8_t minute;
        uint8_t second;
    } date;
    uint8_t lot_id[8];
    uint8_t wafer;
    uint8_t wafer_x;
    uint8_t wafer_y;
    union {
        struct {
            uint8_t cp_good_die_flag    :1;
            uint8_t ft_good_die_flag    :1;
            uint8_t reserved            :6;
        };
        uint8_t val;
    } flag;
    uint8_t reserved[11];
    uint8_t crc16[2];
} chip_info_t;

extern int osal_get_soc_info(chip_info_t *info);

#define OS_TYPE_LINUX 1
#define OS_TYPE_RTTHREAD 2
#define OS_TYPE_ZEPHYR 3
#define OS_TYPE_NUTTX 4

extern unsigned int osal_get_os_type(void);

#ifdef SA6920_SOC
#define A_GPIO(pin)			(pin)
#define D_GPIO(pin)			(pin + 0x8)
#define B_GPIO(pin)			(pin + 0x19)
#define E_GPIO(pin)			(pin + 0x1B)
#define C_GPIO(pin)			(pin + 0x29)
#define F_GPIO(pin)			(pin + 0x2F)
#define H_GPIO(pin)			(pin + 0x4A)
#endif

extern int osal_gpio_request(unsigned int gpio, const char *label);
extern void osal_gpio_free(unsigned int gpio);
extern int osal_gpio_direction_input(unsigned int gpio);
extern int osal_gpio_direction_output(unsigned int gpio, int value);
extern int osal_gpio_get_value(unsigned int gpio);
extern void osal_gpio_set_value(unsigned int gpio, int value);

//-----------------platform--------------------//
#define OSAL_IORESOURCE_TYPE_BITS	0x00001f00	/* Resource type */
#define OSAL_IORESOURCE_IO		0x00000100	/* PCI/ISA I/O ports */
#define OSAL_IORESOURCE_MEM		0x00000200
#define OSAL_IORESOURCE_REG		0x00000300	/* Register offsets */
#define OSAL_IORESOURCE_IRQ		0x00000400
#define OSAL_IORESOURCE_DMA		0x00000800
#define OSAL_IORESOURCE_BUS		0x00001000

struct osal_resource {
	unsigned long start;
	unsigned long end;
};

#define OSAL_PLATFORM_NAME_SIZE	20
struct osal_platform_device {
	void *data;
	void *private_data;
	struct osal_resource *rc;
};

struct osal_of_device_id {
	char	name[32];
	char	type[32];
	char	compatible[128];
	const void *data;
};

struct osal_platform_driver {
	struct osal_list_head entry;
	char name[OSAL_PLATFORM_NAME_SIZE];
	void *data;
	struct osal_platform_device *dev;
	int (*probe)(struct osal_platform_device *);
	int (*remove)(struct osal_platform_device *);
	void (*shutdown)(struct osal_platform_device *);
	int (*suspend)(struct osal_platform_device *, int event); //not support
	int (*resume)(struct osal_platform_device *); //not support
	struct osal_of_device_id	*osal_of_match_table;
};
typedef unsigned long osal_pinctrl;
typedef unsigned long osal_pinctrl_state;
typedef unsigned long osal_clk;
typedef void* osal_device;
typedef void* osal_device_node;
typedef void* osal_reset_control ;
typedef int osal_dma_cookie_t;
typedef void* osal_dma_chan;
typedef void* osal_dma_async_tx_descriptor;

#define OSAL_BLK_TASK_NAME_LEN  128
#define OSAL_BLK_WQ_NAME_LEN  128

enum osal_dma_transfer_direction {
        OSAL_DMA_MEM_TO_MEM,
        OSAL_DMA_MEM_TO_DEV,
        OSAL_DMA_DEV_TO_MEM,
        OSAL_DMA_DEV_TO_DEV,
        OSAL_DMA_TRANS_NONE,
};

enum osal_dma_data_direction {
        OSAL_DMA_BIDIRECTIONAL = 0,
        OSAL_DMA_TO_DEVICE = 1,
        OSAL_DMA_FROM_DEVICE = 2,
        OSAL_DMA_NONE = 3,
};


enum osal_dma_slave_buswidth {
        OSAL_DMA_SLAVE_BUSWIDTH_UNDEFINED = 0,
        OSAL_DMA_SLAVE_BUSWIDTH_1_BYTE = 1,
        OSAL_DMA_SLAVE_BUSWIDTH_2_BYTES = 2,
        OSAL_DMA_SLAVE_BUSWIDTH_3_BYTES = 3,
        OSAL_DMA_SLAVE_BUSWIDTH_4_BYTES = 4,
        OSAL_DMA_SLAVE_BUSWIDTH_8_BYTES = 8,
        OSAL_DMA_SLAVE_BUSWIDTH_16_BYTES = 16,
        OSAL_DMA_SLAVE_BUSWIDTH_32_BYTES = 32,
        OSAL_DMA_SLAVE_BUSWIDTH_64_BYTES = 64,
};

enum osal_dma_ctrl_flags {
        OSAL_DMA_PREP_INTERRUPT = (1 << 0),
        OSAL_DMA_CTRL_ACK = (1 << 1),
        OSAL_DMA_PREP_PQ_DISABLE_P = (1 << 2),
        OSAL_DMA_PREP_PQ_DISABLE_Q = (1 << 3),
        OSAL_DMA_PREP_CONTINUE = (1 << 4),
        OSAL_DMA_PREP_FENCE = (1 << 5),
        OSAL_DMA_CTRL_REUSE = (1 << 6),
        OSAL_DMA_PREP_CMD = (1 << 7),
        OSAL_DMA_PREP_REPEAT = (1 << 8),
        OSAL_DMA_PREP_LOAD_EOT = (1 << 9),
};

enum osal_dma_status {
        OSAL_DMA_COMPLETE,
        OSAL_DMA_IN_PROGRESS,
        OSAL_DMA_PAUSED,
        OSAL_DMA_ERROR,
        OSAL_DMA_OUT_OF_ORDER,
};

struct osal_dma_slave_config{
    unsigned int data[32];
};
typedef int osal_dma_cookie_t;
struct osal_dma_tx_state {
        osal_dma_cookie_t last;
        osal_dma_cookie_t used;
        unsigned int residue;
        unsigned int in_flight_bytes;
};

struct osal_cpumask{
   unsigned long bits;
};

#define osal_module_driver_new(__driver, __register, ...) \
static int __driver##_init(void) \
{ \
	return __register(&(__driver) , ##__VA_ARGS__); \
} \
INIT_DEVICE_EXPORT(__driver##_init);

//driver: 类型为 struct osal_platform_driver
#define osal_module_platform_driver_new(driver)                   \
    osal_module_driver_new(driver, osal_platform_driver_register_new)

unsigned long long osal_ktime_get_boottime_ns(void);//
unsigned long long osal_ktime_get_boottime(void);//
unsigned long long osal_ktime_get(void);//
unsigned long long osal_ktime_to_ms(unsigned long long kt);//

//---------------------------fake linux macro---------------------------------//
#define MODULE_AUTHOR(x)
#define MODULE_DESCRIPTION(x)
#define MODULE_VERSION(x)
#define MODULE_LICENSE(x)
#define MODULE_VERSION(x)
#define MODULE_DEVICE_TABLE(x,y)

#ifndef IS_ERR
#define IS_ERR(ptr)     ((unsigned long)(ptr) > (unsigned long)(-4095))
#endif

#ifndef ERR_PTR
#define ERR_PTR(err)    ((void *)((long)(err)))
#endif

#ifndef PTR_ERR
#define PTR_ERR(ptr)    ((long)(ptr)) 
#endif

#ifndef IS_ERR_VALUE
#define IS_ERR_VALUE(x) ((unsigned long)(void *)(x) >= (unsigned long)(-4095))
#endif

#ifndef IS_ERR_OR_NULL
#define IS_ERR_OR_NULL(ptr) ((!ptr) || IS_ERR_VALUE((unsigned long)ptr))
#endif

#ifndef __GFP_COMP
#define __GFP_COMP 0
#endif

#ifndef __GFP_NORETRY
#define __GFP_NORETRY 0
#endif

#ifndef __GFP_NOWARN
#define __GFP_NOWARN 0
#endif

#ifndef GFP_ATOMIC
#define GFP_ATOMIC osal_gfp_atomic
#endif

#ifndef GFP_KERNEL
#define GFP_KERNEL osal_gfp_kernel
#endif

#ifndef PAGE_SHIFT
#define PAGE_SHIFT 12
#endif

#ifndef phys_addr_t
#define phys_addr_t unsigned long
#endif

#ifndef dma_addr_t
#define dma_addr_t unsigned int
#endif
//----------------------------fake linux end-----------------------------//

extern int osal_platform_driver_register_new(struct osal_platform_driver *drv);
extern void osal_platform_driver_unregister_new(struct osal_platform_driver *drv);
extern struct osal_resource *osal_platform_get_resource_byname_new(struct osal_platform_device *dev, unsigned int type,
                                        const char *name);
extern struct osal_resource *osal_platform_get_resource_new(struct osal_platform_device *dev, unsigned int type,
                                 unsigned int num);
extern int osal_platform_get_irq_new(struct osal_platform_device *dev, unsigned int num);
extern int osal_platform_get_irq_byname_new(struct osal_platform_device *dev, const char *name);
extern void *osal_platform_ioremap_resource_new(struct osal_platform_device *dev, struct osal_resource *osal_resource);

extern void osal_platform_set_drvdata(struct osal_platform_device *dev, void *data);
extern void *osal_platform_get_drvdata(const struct osal_platform_device *dev);
extern void *osal_devm_platform_ioremap_resource(struct osal_platform_device *dev,unsigned int index);
extern int osal_devm_request_irq(struct osal_platform_device *dev, unsigned int irq, osal_irq_handler_t handler, unsigned long irqflags, const char *devname, void *dev_id);
extern void osal_devm_free_irq(struct osal_platform_device *dev, unsigned int irq, void *dev_id);

osal_pinctrl osal_pinctrl_get(struct osal_platform_device *dev);
osal_pinctrl_state osal_pinctrl_lookup_state(osal_pinctrl p, const char *name);
int osal_pinctrl_select_state(osal_pinctrl p, osal_pinctrl_state ps);

extern osal_clk osal_clk_get(struct osal_platform_device *dev, const char *name);
extern int osal_clk_prepare_enable(osal_clk clk);
extern void osal_clk_disable_unprepare(osal_clk clk);
extern int osal_clk_set_rate(osal_clk clk, unsigned long rate);
extern unsigned long osal_clk_get_rate(osal_clk clk);
extern int osal_clk_set_parent(osal_clk clk, osal_clk parent);
extern int osal_sa6920_clk_set_isp_rate(unsigned long rate);
extern unsigned long osal_sa6920_clk_get_isp_rate(void);

/*--------------------------------------sync from linux begain----------------------------------------*/
extern void osal_dev_set_drvdata(struct osal_platform_device *dev, void *data);//
extern void *osal_dev_get_drvdata(const struct osal_platform_device *dev);//
extern osal_device osal_get_device(struct osal_platform_device *dev);//

osal_device_node osal_of_find_node_by_name(osal_device_node from,const char *name);//
osal_device_node osal_of_get_node(struct osal_platform_device* drv); //
osal_device_node osal_of_find_compatible_node(osal_device_node from, const char *type, const char *compat);//
void osal_of_node_put(osal_device_node node);//
int osal_of_irq_get(osal_device_node dev, int index);//
osal_clk osal_of_clk_get(osal_device_node np, int index); //Not
int osal_of_property_read_u32_array(osal_device_node node,const char *propname,unsigned int *out_values, size_t sz);//
int osal_of_property_read_u32(osal_device_node node,const char *propname,unsigned int *out_value);//
int osal_of_device_is_compatible(const osal_device_node from, const char *name);


//reset
osal_reset_control osal_devm_reset_control_get_optional_exclusive(struct osal_platform_device *dev, const char *id);
int osal_reset_control_deassert(osal_reset_control rstc);
int osal_reset_control_reset(osal_reset_control rstc);

//task
unsigned long osal_get_pid(void);//
unsigned long osal_get_tgid(void);//
void osal_cpumask_clear(struct osal_cpumask* dstp);//
void osal_cpumask_set_cpu(unsigned int cpu, struct osal_cpumask *dstp);//
int  osal_irq_set_affinity_hint(unsigned int irq, const struct osal_cpumask *m);//

//dma
osal_dma_chan osal_dma_request_chan(osal_device dev, const char *name);
void osal_dma_release_channel(osal_dma_chan chan);
void *osal_dma_alloc_coherent_new(osal_dma_chan chan, size_t size,unsigned long *dma_handle, unsigned int osal_gfp_flag);
void osal_set_dma_slave_config(struct osal_dma_slave_config* config,enum osal_dma_transfer_direction direction,unsigned long src_addr,unsigned long dst_addr,enum osal_dma_slave_buswidth src_addr_width,
                               enum osal_dma_slave_buswidth dst_addr_width,unsigned int src_maxburst,unsigned int dst_maxburst,unsigned int src_port_window_size,
                               unsigned int dst_port_window_size,int device_fc, unsigned int slave_id);

int osal_dmaengine_slave_config(osal_dma_chan chan,struct osal_dma_slave_config *config);
osal_dma_async_tx_descriptor osal_dmaengine_prep_slave_single(
        osal_dma_chan *chan, unsigned long buf, size_t len,
        enum osal_dma_transfer_direction dir, unsigned long flags);
void osal_set_dma_async_tx_descriptor(osal_dma_async_tx_descriptor des,void (*dma_cb)(void*),void *callback_param);
osal_dma_cookie_t osal_dmaengine_submit(osal_dma_async_tx_descriptor desc);
void osal_dma_async_issue_pending(osal_dma_chan chan);
void osal_dma_sync_single_for_device(osal_dma_chan chan, unsigned long addr,size_t size, enum osal_dma_data_direction dir);
void osal_dma_free_coherent_new(osal_device dev, size_t size, void *cpu_addr, dma_addr_t dma_handle);
int osal_dmaengine_terminate_async(osal_dma_chan chan);

osal_dma_async_tx_descriptor  osal_dmaengine_prep_dma_cyclic(
                osal_dma_chan chan, unsigned long buf_addr, size_t buf_len,
                size_t period_len, enum osal_dma_transfer_direction dir, unsigned long flags);

enum osal_dma_status osal_dmaengine_tx_status(osal_dma_chan chan, osal_dma_cookie_t cookie,struct osal_dma_tx_state *state);

//misc -- linux wq调试统计函数，不必实现
int osal_get_blk_wq_item_count(void);
void osal_get_blk_wq_item(char* buf,int len);
void osal_clear_blk_wq_list(void);
int osal_get_blk_task_item_count(void);
void osal_get_blk_task_item(char* buf,int len);
void osal_clear_blk_task_list(void);


#endif
