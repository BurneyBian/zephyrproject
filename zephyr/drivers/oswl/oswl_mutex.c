#include <string.h>
#include <stdio.h>
#include <rtthread.h>

#include "oswl.h"

#ifndef RT_USING_PTHREADS
int oswl_mutex_init(oswl_mutex_t *mutex)
{
	char mutexName[NAME_LENGTH] = {'\0'};

	snprintf(mutexName, sizeof(mutexName),"mtx_%lx", (long)mutex);

    mutex->mutex = rt_mutex_create(mutexName, RT_IPC_FLAG_PRIO);

	if (mutex->mutex == RT_NULL)
    {
        OSWL_LOG_E("create dynamic mutex failed.\n");
        return -1;
    }

    return 0;
}

int oswl_mutex_lock(oswl_mutex_t *mutex)
{
    if (mutex == NULL || mutex->mutex == NULL) {
        OSWL_LOG_E("%s - parameter invalid!\n", __FUNCTION__);
        return -1;
    }
	return rt_mutex_take((rt_mutex_t)mutex->mutex, RT_WAITING_FOREVER);
}

void oswl_mutex_unlock(oswl_mutex_t *mutex)
{
    if (mutex == NULL || mutex->mutex == NULL) {
        OSWL_LOG_E("%s - parameter invalid!\n", __FUNCTION__);
        return;
    }

	rt_mutex_release((rt_mutex_t)mutex->mutex);
}

void oswl_mutex_destory(oswl_mutex_t *mutex)
{
    if (mutex == NULL || mutex->mutex == NULL) {
        OSWL_LOG_E("%s - parameter invalid!\n", __FUNCTION__);
        return;
    }
	rt_mutex_delete((rt_mutex_t)mutex->mutex);

    mutex->mutex = NULL;
}

#else
#include <pthread.h>
int oswl_mutex_init(oswl_mutex_t *mutex)
{
#ifndef MUTEX_STATIC_DEFINE
	pthread_mutex_t *p_mutex = (pthread_mutex_t *)rt_malloc(sizeof(pthread_mutex_t));
	rt_memset(p_mutex, 0, sizeof(pthread_mutex_t));
	mutex->mutex = p_mutex;
	return pthread_mutex_init(p_mutex, RT_NULL);
#else
	return pthread_mutex_init(mutex, RT_NULL);
#endif
}

int oswl_mutex_lock(oswl_mutex_t *mutex)
{
#ifndef MUTEX_STATIC_DEFINE
	return pthread_mutex_lock((pthread_mutex_t *)mutex->mutex);
#else
	return pthread_mutex_lock(mutex);
#endif
}
 
void oswl_mutex_unlock(oswl_mutex_t *mutex)
{
#ifndef MUTEX_STATIC_DEFINE
	pthread_mutex_unlock((pthread_mutex_t *)mutex->mutex);
#else
	pthread_mutex_unlock(mutex);
#endif
}

void oswl_mutex_destory(oswl_mutex_t *mutex)
{
#ifndef MUTEX_STATIC_DEFINE
	pthread_mutex_destroy((pthread_mutex_t *)mutex->mutex);
	rt_free(mutex->mutex);
#else
	pthread_mutex_destroy(mutex);
#endif
}
#endif
