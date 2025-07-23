#include <string.h>
#include <stdio.h>
#include "oswl.h"
#include <rtthread.h>

int oswl_sem_init(oswl_sem_t *sem, int val)
{
    rt_sem_t p = RT_NULL;

    if (sem == RT_NULL) {
        OSWL_LOG_E("%s - parameter invalid!\n", __FUNCTION__);
        return -1;
    }

	char semName[NAME_LENGTH] = {'\0'};

	snprintf(semName, sizeof(semName),"sem_%lx", (long)sem);

    p = rt_sem_create(semName, val, RT_IPC_FLAG_PRIO);
    if (p == RT_NULL) {
        OSWL_LOG_E("%s - rt_malloc error!\n", __FUNCTION__);
        return -1;
    }

    sem->sem = p;
    return 0;
}

int oswl_sem_wait(oswl_sem_t *sem, oswl_mutex_t *mutex)
{
    rt_sem_t  p = RT_NULL;
    if (sem == RT_NULL) {
        OSWL_LOG_E("%s - parameter invalid!\n", __FUNCTION__);
        return -1;
    }
    p = (rt_sem_t)(sem->sem);
    return rt_sem_take(p, RT_WAITING_FOREVER);
}

void oswl_sem_signal(oswl_sem_t *sem)
{
    rt_sem_t  p = RT_NULL;
    p = (rt_sem_t)(sem->sem);
    rt_sem_release(p);
}
void oswl_sem_destory(oswl_sem_t *sem)
{
    rt_sem_t  p = RT_NULL;
    p = (rt_sem_t)(sem->sem);

    rt_sem_delete(p);

    sem->sem = RT_NULL;
}

int oswl_sem_wait_timeout(oswl_sem_t *sem, oswl_mutex_t *mutex, int ms)
{
    if ((sem == NULL) || (sem->sem == NULL)) {
        OSWL_LOG_E("%s - parameter invalid!\n", __FUNCTION__);
        return -1;
    }

	if (ms < 0) {
        OSWL_LOG_E("%s - ms parameter invalid!\n", __FUNCTION__);
        return -1;
	}

    while (1)
	{
        if (rt_sem_trytake((rt_sem_t)(sem->sem)) == RT_EOK)
            return 0;

		if (ms == 0)
			return -1;
		
		ms--;
        oswl_mdelay(1);
    }

    return 0;
}