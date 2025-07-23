#include "osal.h"
__attribute ((visibility("default"))) 
void osal_yield(void)
{
    k_yield();
}
