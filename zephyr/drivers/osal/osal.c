#include "osal.h"
#define SA_OSAL_VERSION "v0.5"

extern void osal_work_queue_init(void);
extern void osal_proc_init(void);
extern int osal_procfs_init(void);
extern int osal_print_init(int size);
extern void osal_test(void);
extern void osal_i2c_init(void);
extern void sdk_osal_heap_init(char *name, void *begin_addr, void *end_addr);
extern int osal_printk_set_level(int lvl);
extern int mmz_userdev_init(void);
extern int osal_devfs_init(void);
extern void rt_mmz_heap_init_bf(char *name, void *begin_addr, void *end_addr);

int osal_init(void)
{
	osal_printk_set_level(5);

	osal_printk("[%d]sa_osal enter\n", osal_get_tickcount());

	sdk_osal_heap_init("osal_test", (void *)0x20120000, (void *)0x2061ffff); //5MB

	osal_proc_init();
	//osal_work_queue_init();
	osal_procfs_init();
	osal_devfs_init();
	mmz_userdev_init();
	rt_mmz_heap_init_bf("sram", (void *)0x20120000, (void *)0x20420000);
	rt_mmz_heap_init_bf("test", (void *)0x20420000, (void *)0x20620000);
	osal_test();
	
	osal_printk("sa_osal %s init success!\n", SA_OSAL_VERSION);

	return 0;

#ifdef BSP_USING_STAR
	osal_print_init(10);
#else
	osal_print_init(100);
#endif

#ifdef RT_USING_I2C
	osal_i2c_init();
#ifdef RT_USING_FDT
    dtbo_monitor_init();
#endif
#endif

#ifdef RT_USING_MMZ
#endif
    osal_printk("sa_osal %s init success!\n", SA_OSAL_VERSION);

    return 0;
}

//OSAL_INIT_PREV_EXPORT(osal_init);

