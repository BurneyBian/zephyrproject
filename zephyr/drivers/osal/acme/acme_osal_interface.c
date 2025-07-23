#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/cache.h>
#include <zephyr/drivers/mm/system_mm.h>

//#define ARCH_ARMV8_REMAP

#ifdef BSP_SOC_SA6921
#define CPU_SRAM_ADDR_C_START0 0x20000000
#define CPU_SRAM_ADDR_C_END0 0x23ffffff
#define CPU_SRAM_ADDR_NC_START0 0x24000000
#define CPU_SRAM_ADDR_NC_END0 0x27ffffff
#define CPU_SRAM_ADDR_C_START1 0x38000000
#define CPU_SRAM_ADDR_C_END1 0x3bffffff
#define CPU_SRAM_ADDR_NC_START1 0x3c000000
#define CPU_SRAM_ADDR_NC_END1 0x3fffffff
#define CPU_SRAM_ADDR_MAX 0x40000000
#endif

#ifdef BSP_SOC_SA6941
#define CPU_SRAM_ADDR_C_START0 0x20000000
#define CPU_SRAM_ADDR_C_END0 0x27ffffff
#define CPU_SRAM_ADDR_NC_START0 0x28000000
#define CPU_SRAM_ADDR_NC_END0 0x2fffffff
#define CPU_SRAM_ADDR_C_START1 0x30000000
#define CPU_SRAM_ADDR_C_END1 0x4fffffff
#define CPU_SRAM_ADDR_NC_START1 0x60000000
#define CPU_SRAM_ADDR_NC_END1 0x7fffffff
#define CPU_SRAM_ADDR_MAX 0x80000000
#endif

#if defined (BSP_SOC_SA6921) || defined (BSP_SOC_SA6941)
unsigned long acme_phyaddr_need_remap(unsigned long phys_addr, int cached)
{
	if (cached) {
		if ((phys_addr >= CPU_SRAM_ADDR_NC_START0) && (phys_addr <= CPU_SRAM_ADDR_NC_END0)) {
			return (phys_addr - (CPU_SRAM_ADDR_NC_START0 - CPU_SRAM_ADDR_C_START0));
		}
		if ((phys_addr >= CPU_SRAM_ADDR_NC_START1) && (phys_addr <= CPU_SRAM_ADDR_NC_END1)) {
			return (phys_addr - (CPU_SRAM_ADDR_NC_START1 - CPU_SRAM_ADDR_C_START1));
		}
	} else {
		if ((phys_addr >= CPU_SRAM_ADDR_C_START0) && (phys_addr <= CPU_SRAM_ADDR_C_END0)) {
			return (phys_addr + CPU_SRAM_ADDR_NC_START0 - CPU_SRAM_ADDR_C_START0);
		}
		if ((phys_addr >= CPU_SRAM_ADDR_C_START1) && (phys_addr <= CPU_SRAM_ADDR_C_END1)) {
			return (phys_addr + CPU_SRAM_ADDR_NC_START1 - CPU_SRAM_ADDR_C_START1);
		}
	}

	return phys_addr;
}

#else

unsigned long acme_phyaddr_need_remap(unsigned long phys_addr, int cached)
{
#if defined (BSP_CPU_A55) && defined (ARCH_ARMV8_REMAP)
	if ((phys_addr < 0x40000000) || (phys_addr >= 0x60000000)) {
		if (!cached) {
			phys_addr += 0x80000000L;
		}
	}
	return phys_addr;
#endif
	return phys_addr;
}
#endif

#if defined (ARCH_ARMV8)||defined(ARCH_ARM_CORTEX_A)


void acme_flush_dcache_all(void)
{
	
}

void acme_set_cacheable_addr(unsigned long phys_addr, unsigned long length)
{
#ifndef ARCH_ARMV8_REMAP
	length = (length + 4095)/4096*4096;
	sys_mm_drv_map_region(phys_addr, phys_addr, length, SYS_MM_MEM_CACHE_WB);
#endif
}

void acme_set_noncacheable_addr(unsigned long phys_addr, unsigned long length)
{
#ifndef ARCH_ARMV8_REMAP
	length = (length + 4095)/4096*4096;
	sys_mm_drv_map_region(phys_addr, phys_addr, length, SYS_MM_MEM_CACHE_NONE);
#endif
}

void acme_set_devie_addr(unsigned long phys_addr, unsigned long length)
{
#ifndef ARCH_ARMV8_REMAP
	length = (length + 4095)/4096*4096;
	sys_mm_drv_map_region(phys_addr, phys_addr, length, SYS_MM_MEM_CACHE_NONE);
#endif
}
int acme_get_addr_cachable(unsigned long addr)
{
	return -ENOTSUP;
}
#elif defined (CONFIG_SOC_SA6920_STAR)


void acme_flush_dcache_all(void)
{
	sys_cache_data_flush_all();
}

extern int memory_cachable_set_by_mpu_nonum(unsigned char is_cachable,
                               unsigned long addr_start,
                               unsigned long addr_end);

void acme_set_cacheable_addr(unsigned long phys_addr, unsigned long length)
{

}

void acme_set_noncacheable_addr(unsigned long phys_addr, unsigned long length)
{

}

void acme_set_devie_addr(unsigned long phys_addr, unsigned long length)
{

}

int acme_get_addr_cachable(unsigned long addr)
{
	return -ENOTSUP;
}
#else
void acme_flush_dcache_all(void)
{}

void acme_set_cacheable_addr(unsigned long phys_addr, unsigned long length)
{}

void acme_set_noncacheable_addr(unsigned long phys_addr, unsigned long length)
{}

void acme_set_devie_addr(unsigned long phys_addr, unsigned long length)
{}

int acme_get_addr_cachable(unsigned long addr)
{
	return -ENOTSUP;
}
#endif

void acme_cpuc_flush_dcache_area(void *addr, int size)
{
    sys_cache_data_flush_range(addr, size);
}

void acme_flush_dcache_area(void *kvirt, unsigned long phys_addr, unsigned long length)
{
	if (kvirt)
		 sys_cache_data_flush_range(kvirt, length);
	else if (phys_addr)
		 sys_cache_data_flush_range((void *)phys_addr, length);
}

void acme_invalid_dcache_area(void *kvirt, unsigned long length)
{
	sys_cache_data_invd_range(kvirt, length);
}

int acme_is_sys_user_heap(unsigned long phys_addr)
{
	return 0;
}


#if defined(ARCH_ARMV8) || defined(ARCH_ARM_CORTEX_A)
void acme_mb(void)
{
    __asm__ volatile ("dsb sy":::"memory");
}

void acme_rmb(void)
{
	__asm__ volatile ("dsb ld":::"memory");
}

void acme_wmb(void)
{
	__asm__ volatile ("dsb st":::"memory");
}

void acme_smp_mb(void)
{
	__asm__ volatile ("dmb ish":::"memory");
}

void acme_smp_rmb(void)
{
	__asm__ volatile ("dmb ishld":::"memory");
}

void acme_smp_wmb(void)
{
	__asm__ volatile ("dmb ishst":::"memory");
}

void acme_isb(void)
{
    __asm__ volatile ("isb":::"memory");
}

void acme_dsb(void)
{
	__asm__ volatile ("dsb sy":::"memory");
}

void acme_dmb(void)
{
	__asm__ volatile ("dmb sy":::"memory");
}
#elif defined(ARCH_RISCV)
#define nop() __asm__ __volatile__ ("nop")
#define RISCV_FENCE(p, s) __asm__ __volatile__ ("fence " #p "," #s : : : "memory")
void acme_mb(void)
{
	compiler_barrier();
}

void acme_rmb(void)
{
	compiler_barrier();
}

void acme_wmb(void)
{
	compiler_barrier();
}

void acme_smp_mb(void)
{
	compiler_barrier();
}

void acme_smp_rmb(void)
{
	compiler_barrier();
}

void acme_smp_wmb(void)
{
	compiler_barrier();
}

void acme_dmb(void)
{
	acme_mb();
}

void acme_isb(void)
{
	acme_mb();
}

void acme_dsb(void)
{
	acme_mb();
}
#elif defined(CONFIG_SOC_SA6920_STAR)
void acme_isb(void)
{
  __asm__ volatile("isb 0xF":::"memory");
}
void acme_dsb(void)
{
  __asm__ volatile("dsb 0xF":::"memory");
}
void acme_dmb(void)
{
  __asm__ volatile("dmb 0xF":::"memory");
}
void acme_mb(void){acme_dmb();}
void acme_rmb(void){acme_dmb();}
void acme_wmb(void){acme_dmb();}
void acme_smp_mb(void){acme_dmb();}
void acme_smp_rmb(void){acme_dmb();}
void acme_smp_wmb(void){acme_dmb();}

#else
void acme_mb(void){}
void acme_rmb(void){}
void acme_wmb(void){}
void acme_smp_mb(void){}
void acme_smp_rmb(void){}
void acme_smp_wmb(void){}
void acme_isb(void){}
void acme_dsb(void)
{}
void acme_dmb(void)
{}
#endif


#if defined (BSP_CPU_A55) && defined (ARCH_ARMV8_REMAP)
unsigned long acme_virt_to_phys(void *va)
{
	unsigned long phys_addr = (unsigned long)va;

	if (phys_addr >= 0x80000000) {
		phys_addr &= 0x7fffffff;
	}

	return phys_addr;
}
#else
unsigned long acme_virt_to_phys(void *va)
{
	return (unsigned long)(va);
}
#endif

