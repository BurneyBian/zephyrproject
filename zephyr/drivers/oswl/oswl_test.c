#include "sa_oswl_mmz.h"
#include <string.h>
static int __mmz_test_open(int is_sram)
{
	int mmz_fd;

	if (is_sram)
		mmz_fd = sa_oswl_mmz_sram_OpenDev();
	else
		mmz_fd = sa_oswl_mmz_OpenMmzDev();
	if (mmz_fd <= 0)
	{
		printk("sa_oswl_mmz_OpenMmzDev failed mmz_fd=%d\n", mmz_fd);
		return -1;
	}
	printk("sa_oswl_mmz_OpenMmzDev ok\n");		

	return mmz_fd;
}

static void __mmz_test_alloc(struct mmb_info *pmi, int mmz_fd)
{
	int ret;

	ret = sa_oswl_mmz_AllocMmb(mmz_fd, pmi);
	if (ret)
	{
		printk("sa_oswl_mmz_AllocMmb failed ret=%d\n", ret);
		return;
	}
	printk("[%s]sa_oswl_mmz_AllocMmb ok! phys_addr=%lx\n", pmi->mmb_name, pmi->phys_addr);

	ret = sa_oswl_mmz_RemapMmb(mmz_fd, pmi, 1);
	if (ret)
	{
		printk("sa_oswl_mmz_RemapMmb failed ret=%d\n", ret);
		return;
	}
	printk("sa_oswl_mmz_RemapMmb ok! phys_addr=%lx [%lx]\n", pmi->phys_addr, (unsigned long)pmi->mapped);

}

static void __mmz_test_free(struct mmb_info *pmi, int mmz_fd)
{
	int ret;

	ret = sa_oswl_mmz_UnremapMmb(mmz_fd, pmi);
	if (ret)
	{
		printk("sa_oswl_mmz_UnremapMmb failed ret=%d\n", ret);
	}
	printk("sa_oswl_mmz_UnremapMmb ok\n");

	ret = sa_oswl_mmz_FreeMmb(mmz_fd, pmi);
	if (ret)
	{
		printk("sa_oswl_mmz_FreeMmb failed ret0=%d\n", ret);
	}
	printk("sa_oswl_mmz_FreeMmb ok\n");
}

static void __mmz_test_close(int mmz_fd)
{
	int ret;

	ret = sa_oswl_mmz_CloseMmzDev(mmz_fd);
	if (ret)
	{
		printk("sa_oswl_mmz_CloseMmzDev failed ret=%d\n", ret);
	}
	printk("sa_oswl_mmz_CloseMmzDev ok\n");	
}

static int mmz_test(void)
{
	struct mmb_info pmi0, pmi1, pmi2, pmi3, pmi4, pmi5, pmi6, pmi7, pmi8, pmi9, pmi10;
	int mmz_fd, mmz_fd_tmp;

	memset(&pmi0, 0, sizeof(pmi0));
	pmi0.size = 8222;
	pmi0.align = 8022;
	pmi0.gfp = 0;
	memcpy(&pmi3, &pmi0, sizeof(pmi0));

	strcpy(pmi0.mmz_name, "anonymous");
	memcpy(&pmi1, &pmi0, sizeof(pmi0));
	memcpy(&pmi2, &pmi0, sizeof(pmi0));

	strcpy(pmi3.mmz_name, "sram");
	memcpy(&pmi4, &pmi3, sizeof(pmi3));
	memcpy(&pmi5, &pmi3, sizeof(pmi3));
	memcpy(&pmi6, &pmi3, sizeof(pmi3));
	memcpy(&pmi7, &pmi3, sizeof(pmi3));
	memcpy(&pmi8, &pmi3, sizeof(pmi3));
	memcpy(&pmi9, &pmi3, sizeof(pmi3));
	memcpy(&pmi10, &pmi3, sizeof(pmi3));

	strcpy(pmi3.mmb_name, "Rshutdown_test");
	strcpy(pmi4.mmb_name, "Rshutdown_test");
	strcpy(pmi5.mmb_name, "Risp_test");
	strcpy(pmi6.mmb_name, "Risp_test");
	strcpy(pmi7.mmb_name, "Risp_test");
	strcpy(pmi8.mmb_name, "Raiisp_test");
	strcpy(pmi9.mmb_name, "Raiisp_test");
	strcpy(pmi10.mmb_name, "Rvpu_test");

	mmz_fd = __mmz_test_open(0);  		//anonymous
	mmz_fd_tmp = __mmz_test_open(1); 	//sram

	__mmz_test_alloc(&pmi0, mmz_fd);
	__mmz_test_alloc(&pmi1, mmz_fd);
	__mmz_test_alloc(&pmi2, mmz_fd);

	__mmz_test_alloc(&pmi3, mmz_fd_tmp);
	__mmz_test_alloc(&pmi4, mmz_fd_tmp);
	__mmz_test_alloc(&pmi5, mmz_fd_tmp);
	__mmz_test_alloc(&pmi6, mmz_fd_tmp);
	__mmz_test_alloc(&pmi7, mmz_fd_tmp);
	__mmz_test_alloc(&pmi8, mmz_fd_tmp);
	__mmz_test_alloc(&pmi9, mmz_fd_tmp);
	__mmz_test_alloc(&pmi10, mmz_fd_tmp);

	//system("cat /proc/media-mem");

	__mmz_test_free(&pmi3, mmz_fd_tmp);
	__mmz_test_free(&pmi4, mmz_fd_tmp);
	__mmz_test_free(&pmi5, mmz_fd_tmp);
	__mmz_test_free(&pmi6, mmz_fd_tmp);
	__mmz_test_free(&pmi7, mmz_fd_tmp);
	__mmz_test_free(&pmi8, mmz_fd_tmp);
	__mmz_test_free(&pmi9, mmz_fd_tmp);
	__mmz_test_free(&pmi10, mmz_fd_tmp);

	__mmz_test_free(&pmi0, mmz_fd);
	__mmz_test_free(&pmi1, mmz_fd);
	__mmz_test_free(&pmi2, mmz_fd);

	__mmz_test_close(mmz_fd);
	__mmz_test_close(mmz_fd_tmp);

	return 0;
}

void oswl_test(void)
{
	printk("[%s] enter\n", __func__);
	mmz_test();
	printk("[%s] exit\n", __func__);
}

