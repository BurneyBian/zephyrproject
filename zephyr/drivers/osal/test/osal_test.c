#include "osal.h"
#include <string.h>
#include <zephyr/shell/shell.h>

static int osal_show(osal_proc_entry_t *s)
{
	osal_printk("%s\n", __func__);
	return 0;
}
static int osal_write(osal_proc_entry_t *entry, const char *buf, int count, long long *loggt)
{
	osal_printk("count=%d\n", count);
	if (count == 1)
		osal_printk("%s\n", buf);

	return 0;
}

static osal_spinlock_t osal_lock;
static int osal_a;
static int consumer_thread_entry(void *data)
{
	int i = 0;
	osal_spinlock_t *lock = (osal_spinlock_t *)data;
	osal_printk("[%d]%s\n", osal_get_tickcount(), __func__);
    while (i < 30)
    {
		osal_spin_lock(lock);
		if (osal_a > 0)
			osal_a--;
		osal_spin_unlock(lock);
		osal_udelay(1);
		i++;
    }

	osal_msleep(1000);

	return 0;
}

static int producer_thread_entry(void *data)
{
	osal_spinlock_t *lock = (osal_spinlock_t *)data;
	int i = 0;
	int tmp = 0;
	osal_printk("[%d]%s\n", osal_get_tickcount(), __func__);
	osal_spin_lock(lock);

    while (i < 10)
    {
		osal_a++;
		osal_udelay(2);
		i++;
    }

	if (osal_a == 10)
		tmp = 1;

	osal_spin_unlock(lock);

	if (!tmp)
		osal_printk("[%d]osal_spinlock_test is failed [%d]\n", osal_get_tickcount(), osal_a);
	else
		osal_printk("[%d]osal_spinlock_test is successful\n", osal_get_tickcount());

	return 0;
}

static int osal_spinlock_test(void)
{
	osal_task_t *pthread, *cthread;
	osal_a = 0;
	osal_spin_lock_init(&osal_lock);
    cthread = osal_kthread_create(consumer_thread_entry, &osal_lock, "osal_consumer");
    pthread = osal_kthread_create(producer_thread_entry, &osal_lock, "osal_producer");

	osal_msleep(100);
	osal_printk("[%d]--1\n", osal_get_tickcount());
	osal_kthread_destory(pthread, 0);
	osal_kthread_destory(cthread, 1);

	osal_spin_lock_destory(&osal_lock);

    return 0;
}

extern void *sdk_osal_malloc(char *name, int size);
extern void sdk_osal_free(char *name, void *rmem);

static int timer_occur = 0;

OSAL_HRTIMER_RESTART_E hrtimer_func(void *timer)
{
	osal_printk("[%d]%s\n", osal_get_tickcount(), __func__);
	timer_occur++;
	return OSAL_HRTIMER_RESTART;
}

static void test_hrtimer(void)
{
	osal_hrtimer_t hrtimer;
	timer_occur = 0;
	hrtimer.function = hrtimer_func;
	hrtimer.interval = 1000;
	osal_hrtimer_create(&hrtimer);
	
	osal_printk("[%d]%s start\n", osal_get_tickcount(), __func__);
	
	osal_hrtimer_start(&hrtimer);

	osal_msleep(2100);

	if (timer_occur == 2)
		osal_printk("[%d]%s is successful\n", osal_get_tickcount(), __func__);
	else
		osal_printk("[%d]%s failed\n", osal_get_tickcount(), __func__);

	osal_hrtimer_cancel(&hrtimer);
	osal_hrtimer_destory(&hrtimer);
}

void timer_func(unsigned long data)
{
	osal_timer_t *timer = (osal_timer_t *)data;
	osal_printk("[%d]%s\n", osal_get_tickcount(), __func__);
	timer_occur++;
	osal_timer_set(timer, 1000);
}

static void test_timer(void)
{
	osal_timer_t timer;

	timer.function = timer_func;
	timer.data = (unsigned long)&timer;
	timer_occur = 0;
	osal_timer_create(&timer);
	osal_printk("[%d]%s start\n", osal_get_tickcount(), __func__);
	osal_timer_set(&timer, 1000);

	osal_msleep(2100);

	if (timer_occur == 2)
		osal_printk("[%d]%s is successful\n", osal_get_tickcount(), __func__);
	else
		osal_printk("[%d]%s failed\n", osal_get_tickcount(), __func__);

	osal_timer_del(&timer);
	osal_timer_destory(&timer);
}

static void test_time(void)
{
	unsigned long long now1 = 0, now2 = 0; 
	now1 = osal_get_tickcount();
	osal_mdelay(15);
	now2 = osal_get_tickcount();
	osal_printk("Test:osal_get_tickcount [%d]--[%d]:[%d]=15\n", (int)now1, (int)now2, (int)(now2 - now1));

	now1 = osal_sched_clock();
	osal_msleep(15);
	now2 = osal_sched_clock();
	osal_printk("Test:osal_sched_clock [%d]--[%d]:[%d]=15\n", (int)now1, (int)now2, (int)(now2 - now1)/(1000*1000));
}

static void test_math(void)
{
	unsigned long long value, dividend1 = 10875631*2 + 109;
	unsigned int divisor1 = 10875631;

	value = osal_div_u64(dividend1, divisor1);
	if (value == 2)
		osal_printk("[%d]osal_div_u64 is successful\n", osal_get_tickcount());

	value = osal_div_u64_rem(dividend1, divisor1);
	if (value == 109)
		osal_printk("[%d]osal_div_u64_rem is successful\n", osal_get_tickcount());
}

static void test_work_func(struct osal_work_struct *work)
{
	osal_printk("[%d]%s is successful\n", osal_get_tickcount(), __func__);
}

static osal_workqueue_t wq;
static void __workq_func(void *workq_data)
{
	if (workq_data == &wq)
		osal_printk("[%d]%s is successful\n", osal_get_tickcount(), __func__);
	else
		osal_printk("[%d]%s is failed\n", osal_get_tickcount(), __func__);
}

static void test_workq(void)
{
	struct osal_work_struct work;
	
	osal_init_work(&work, test_work_func);
	osal_msleep(100);
	osal_schedule_work(&work);
	osal_msleep(100);

	wq.workq_func = __workq_func;
	wq.param = &wq;
	osal_workq_init(&wq);
	osal_workq_schedule(&wq);
	osal_msleep(100);
	osal_workq_cancel(&wq);
}

static void test_malloc(void)
{
	char *p = osal_vmalloc(16);
	if (!p) {
		osal_printk("[%d]%s osal_vmalloc failed\n", osal_get_tickcount(), __func__);
		return;
	}

	p[10] = 8;
	osal_vfree(p);

	p = osal_kmalloc(16, osal_gfp_kernel);
	if (!p) {
		osal_printk("[%d]%s osal_kmalloc failed\n", osal_get_tickcount(), __func__);
		return;
	}
	p[15] = 8;
	osal_kfree(p);

	osal_printk("[%d]%s is successful\n", osal_get_tickcount(), __func__);
}

static unsigned int notify_val;
static int osal_notifier_test(struct osal_notifier_block *nb, unsigned long action, void *data)
{
	if ((action == 2) && (data == &notify_val))
		osal_printk("[%d]%s is successful\n", osal_get_tickcount(), __func__);
	else
		osal_printk("[%d]%s is failed\n", osal_get_tickcount(), __func__);

	return 0;
}
static void osal_notify_test(void)
{
	struct osal_notifier_block nb;
	struct osal_list_head head;
	OSAL_INIT_LIST_HEAD(&head);
	nb.notifier_call = osal_notifier_test;

	osal_register_notifier(&head, &nb);
	osal_notifier_call(&head, 2, &notify_val);
	osal_msleep(20);
	osal_unregister_notifier(&head, &nb);
}

static osal_mutex_t mutex;
static int val = 5;
static osal_wait_t wait;
static volatile int idx = 5;
static osal_semaphore_t osal_sem;

static int thread_func(void *data)
{
	osal_printk("[%d]%s -- 1\n", osal_get_tickcount(), __func__);
	if(osal_down(&osal_sem))
		osal_sema_destory(&osal_sem);
	idx = 5;
	osal_printk("[%d]%s -- 2 [%d]\n", osal_get_tickcount(), __func__, idx);

	while (idx)
	{
		osal_mutex_lock(&mutex);
		val++;
		osal_mutex_unlock(&mutex);
		osal_printk("[%d]%s == %d\n", osal_get_tickcount(), __func__, idx);
		osal_msleep(10);
		idx--;
	}

	osal_wakeup(&wait);

	osal_printk("[%d]%s -- 3\n", osal_get_tickcount(), __func__);
	return 0;
}

static int wait_func(const void *param)
{
	return !idx;
}

static void test_thread(void)
{
	osal_task_t *thread;
	osal_printk("[%d]%s -- 1\n", osal_get_tickcount(), __func__);
	osal_memset(&osal_sem, 0, sizeof(osal_semaphore_t));
	osal_sema_init(&osal_sem, 0);

	osal_mutex_init(&mutex);
	osal_wait_init(&wait);
	osal_printk("[%d]%s -- 2\n", osal_get_tickcount(), __func__);
	thread = osal_kthread_create(thread_func, &osal_sem, "osal_test_thread");
	osal_printk("[%d]%s -- 3\n", osal_get_tickcount(), __func__);
	osal_mutex_lock(&mutex);
	val = 3;
	osal_mutex_unlock(&mutex);
	osal_printk("[%d]%s -- 4\n", osal_get_tickcount(), __func__);
	osal_msleep(1000);
	osal_up(&osal_sem);
	osal_printk("[%d]%s -- 5\n", osal_get_tickcount(), __func__);

	osal_wait_timeout_uninterruptible(&wait, wait_func, NULL, 2000);

	osal_printk("[%d]%s -- 6\n", osal_get_tickcount(), __func__);
	osal_kthread_destory(thread, 0);
	osal_sema_destory(&osal_sem);
	osal_mutex_destory(&mutex);
	osal_wait_destory(&wait);

	if (val == 8)
		osal_printk("[%d]%s is successful\n", osal_get_tickcount(), __func__);
	else
		osal_printk("[%d]%s is failed\n", osal_get_tickcount(), __func__);
}

#define ACME_WDT_CR 	0x00
#define ACME_WDT_TORR 	0x04
#define ACME_WDT_CCVR 	0x08
#define ACME_WDT_CRR 	0x0c
#define ACME_WDT_STAT 	0x10
#define ACME_WDT_EOI 	0x14
#define ACME_WDT_CRR_RESET    0x76

#define ACME_WDT_IRQ 0
#define ACME_WDT_BASE 0x48030000
#define RESET_CRG_ADDR 0x4807000c
#define RESET_CRG_BIT 11

static int wdt_irq_handler(int irq, void *data)
{
	unsigned int value;

	value = osal_readl(ACME_WDT_BASE + ACME_WDT_EOI);
	osal_printk("WDT irq! %d\n", value);

	osal_writel(ACME_WDT_CRR_RESET, ACME_WDT_BASE + ACME_WDT_CRR);

	value = osal_readl(RESET_CRG_ADDR);
	value |= (1U << RESET_CRG_BIT);
	osal_writel(value, RESET_CRG_ADDR);

	return 0;
}

static void osal_irq_test()
{
	unsigned int value;

	osal_request_irq(ACME_WDT_IRQ, wdt_irq_handler, NULL, "wdt", &value);

	value = osal_readl(RESET_CRG_ADDR);
	value &= ~(1U << RESET_CRG_BIT);
	osal_writel(value, RESET_CRG_ADDR);
	osal_msleep(2);

	value = osal_readl(ACME_WDT_BASE + ACME_WDT_CR);
	value &= ~(1);
	osal_writel(value, ACME_WDT_BASE + ACME_WDT_CR);

	osal_writel(0x48, ACME_WDT_BASE + ACME_WDT_TORR);

	value = osal_readl(ACME_WDT_BASE + ACME_WDT_CR);
	value |= 0x3;
	osal_writel(value, ACME_WDT_BASE + ACME_WDT_CR);
}

static char test_str[] = "acme, you are the best!!!\n";
static void osal_fileops_test()
{
	int ret;
	char buf[32];

	void *fd = osal_klib_fopen("/RAM:/testfile.txt", OSAL_O_RDWR, 0);

	memset(buf, 0, 32);

	osal_klib_fwrite(test_str, strlen(test_str), fd);
	ret = osal_klib_fread(buf, strlen(test_str), fd);
	osal_klib_fclose(fd);

	if (!strcmp(buf, test_str))
		osal_printk("[%d]%s is successful\n", osal_get_tickcount(), __func__);
	else
		osal_printk("[%d]%s is failed[%s][%d]\n", osal_get_tickcount(), __func__, buf, ret);
}

static void test_mmz(void)
{
	unsigned long addr1, addr2;
	void *p1, *p2;

	addr1 = osal_mmz_alloc("osal_test", 4096, 4096);
	if (!addr1) {
		osal_printk("[%d]%s osal_mmz_alloc failed\n", osal_get_tickcount(),__func__);
		return;
	}
	osal_printk("[%d]%s addr1=%lx\n", osal_get_tickcount(),__func__, addr1);

	addr2 = osal_mmz_alloc("osal_test", 4096, 4096);
	if (!addr2) {
		osal_printk("[%d]%s osal_mmz_alloc failed\n", osal_get_tickcount(),__func__);
		return;
	}
	osal_printk("[%d]%s addr2=%lx\n", osal_get_tickcount(),__func__, addr2);

	p1 = osal_mmz_remap(addr1, 4096, 1);
	if (!p1) {
		osal_printk("[%d]%s osal_mmz_remap failed\n", osal_get_tickcount(),__func__);
		osal_mmz_free(addr1);
		return;
	}
	osal_printk("[%d]%s p1=%lx\n", osal_get_tickcount(), __func__, (unsigned long)p1);

	p2 = osal_mmz_remap(addr2, 4096, 1);
	if (!p2) {
		osal_printk("[%d]%s osal_mmz_remap failed\n", osal_get_tickcount(),__func__);
		osal_mmz_free(addr2);
		return;
	}
	osal_printk("[%d]%s p2=%lx\n", osal_get_tickcount(),__func__, (unsigned long)p2);

	//此处添加查看proc信息
	//system("cat /proc/media-mem");

	osal_mmz_unmap(p1);
	osal_mmz_free(addr1);

	osal_mmz_unmap(p2);
	osal_mmz_free(addr2);

	osal_printk("[%d]%s is successful\n", osal_get_tickcount(), __func__);

}

extern void oswl_test(void);

void osal_test(void)
{
	struct osal_proc_dir_entry *proc_p;

	osal_printk("[%d]osal_test enter\n", osal_get_tickcount());

	//p = sdk_osal_malloc("osal_test", 2048);
	//if (p) {
	//	memset(p,0x5a,2048);
	//	osal_cpuc_flush_dcache_area(p, 2048);
	//	osal_printk("sdk_osal_malloc v=[%lx]\n", *(unsigned long *)p);
	//	sdk_osal_free("osal_test", p);
	//} else {
	//	osal_printk("sdk_osal_malloc failed\n");
	//}
	
	proc_p = osal_create_proc_entry("test", NULL);
	if (proc_p)
	{
		proc_p->read = osal_show;
		proc_p->write = osal_write;
	}
	oswl_test();
#if 1
	test_mmz();
	osal_fileops_test();
	osal_spinlock_test();
	test_time();
	test_hrtimer();
	test_timer();
	test_malloc();
	test_math();
	osal_notify_test();
	test_workq();
	test_thread();
	osal_msleep(2000);
	osal_irq_test();
#endif
}

#if 0
int osal_test_cmd_handler(const struct shell *shell, size_t argc, char **argv)
{
	osal_test();
    return 0;
}

SHELL_CMD_REGISTER(osal_test, NULL, "osal test", osal_test_cmd_handler);
#endif
