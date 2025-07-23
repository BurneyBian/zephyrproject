#ifndef SA_ACME_INTF_H
#define SA_ACME_INTF_H

void acme_flush_dcache_all(void);
void acme_cpuc_flush_dcache_area(void *addr, int size);
void acme_flush_dcache_area(void *kvirt, unsigned long phys_addr, unsigned long length);

void acme_set_cacheable_addr(unsigned long phys_addr, unsigned long length);
void acme_set_noncacheable_addr(unsigned long phys_addr, unsigned long length);
void acme_cpuc_flush_dcache_area(void *addr, int size);
void acme_invalid_dcache_area(void *kvirt, unsigned long length);
void acme_set_devie_addr(unsigned long phys_addr, unsigned long length);
int acme_is_sys_user_heap(unsigned long phys_addr);
unsigned long acme_phyaddr_need_remap(unsigned long phys_addr, int cached);

void acme_mb(void);
void acme_rmb(void);
void acme_wmb(void);
void acme_isb(void);
void acme_dsb(void);
void acme_dmb(void);
void acme_smp_mb(void);
void acme_smp_rmb(void);
void acme_smp_wmb(void);
unsigned long acme_virt_to_phys(void *va);

#endif