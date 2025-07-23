#include "osal.h"
__attribute ((visibility("default"))) 
void osal_mb(void)
{
	compiler_barrier();
}
__attribute ((visibility("default"))) 
void osal_rmb(void)
{
	compiler_barrier();
}
__attribute ((visibility("default"))) 
void osal_wmb(void)
{
	compiler_barrier();
}
__attribute ((visibility("default"))) 
void osal_smp_mb(void)
{
	compiler_barrier();
}
__attribute ((visibility("default"))) 
void osal_smp_rmb(void)
{
	compiler_barrier();
}
__attribute ((visibility("default"))) 
void osal_smp_wmb(void)
{
	compiler_barrier();
}
__attribute ((visibility("default"))) 
void osal_isb(void)
{
	compiler_barrier();
}
__attribute ((visibility("default"))) 
void osal_dsb(void)
{
	compiler_barrier();
}
__attribute ((visibility("default"))) 
void osal_dmb(void)
{
	compiler_barrier();
}

