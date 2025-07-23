#include <string.h>
#include <stdio.h>
#include <rtthread.h>

#include "oswl.h"
#include <getopt.h>
#include <ctype.h>

int g_CloseRttOswlPrint = 0;
int g_CloseRttOswlSampleCrg = 0;
int g_CloseRttBgYuvOut = 0;
#ifndef RT_USING_PTHREADS
static void thread_entry(void *p)
{
	oswl_thread_t *thread = (oswl_thread_t *)p;
	if (thread->callback)
		thread->callback(thread->params);
}

int oswl_thread_init(oswl_thread_t *thread)
{
	rt_thread_t ptask;

    if (thread == RT_NULL) {
        OSWL_LOG_E("%s - thread invalid!\n", __FUNCTION__);
        return -1;
    }

	if (thread->priority >= OSWL_THREAD_PRIORITY_MAX)
		thread->priority = OSWL_THREAD_PRIORITY_MAX;
	else if (thread->priority < OSWL_THREAD_PRIORITY_MIN)
		thread->priority = OSWL_THREAD_PRIORITY_MIN;
	
	ptask = rt_thread_create(thread->name, thread_entry, thread, thread->stack_size, thread->priority, thread->tick);
	if (ptask == RT_NULL) {
        OSWL_LOG_E("%s - rt_thread_create error!\n", __FUNCTION__);
        return -1;
    }

	thread->thread = ptask;

	rt_thread_startup(ptask);

    return 0;
}

void oswl_kthread_destory(oswl_thread_t *thread)
{
    if (thread == RT_NULL) {
        OSWL_LOG_E("%s - thread invalid!\n", __FUNCTION__);
        return;
    }

	rt_thread_delete((rt_thread_t)thread->thread);

	thread->thread = RT_NULL;
}

int oswl_pthread_cond_init(oswl_pthread_cond_t *p_cond)
{
	OSWL_LOG_E("[%s] - Not support!\n", __FUNCTION__);
	return -1;
}

int oswl_pthread_cond_wait(oswl_pthread_cond_t *p_cond, oswl_mutex_t *mutex)
{
	OSWL_LOG_E("[%s] - Not support!\n", __FUNCTION__);
	return -1;
}

int oswl_pthread_cond_timedwait(oswl_pthread_cond_t *p_cond, oswl_mutex_t *mutex,
	struct oswl_timespec *abstime)
{
	OSWL_LOG_E("[%s] - Not support!\n", __FUNCTION__);
	return -1;
}

int oswl_pthread_cond_signal(oswl_pthread_cond_t *p_cond)
{
	OSWL_LOG_E("[%s] - Not support!\n", __FUNCTION__);
	return -1;
}

int oswl_pthread_cond_destroy(oswl_pthread_cond_t *p_cond)
{
	OSWL_LOG_E("[%s] - Not support!\n", __FUNCTION__);
	return -1;
}

int oswl_pthread_cond_broadcast(oswl_pthread_cond_t *p_cond)
{
	OSWL_LOG_E("[%s] - Not support!\n", __FUNCTION__);
	return -1;
}

#else
#include <pthread.h>

static void *thread_entry(void *p)
{
	oswl_thread_t *thread = (oswl_thread_t *)p;
	if (thread->callback)
		thread->callback(thread->params);

	return NULL;
}

extern int pthread_setname_np(pthread_t thread, char *name);

int oswl_thread_init(oswl_thread_t *thread)
{
	int ret = 0;
	pthread_t *ptask = RT_NULL;
	pthread_attr_t attr;
	struct sched_param sched;
	int policy;

    if (thread == RT_NULL) {
        OSWL_LOG_E("Error: %s - thread param is NULL!\n", __func__);
        return -1;
    }

	ptask = (pthread_t *)rt_malloc(sizeof(pthread_t));
	if (ptask == RT_NULL) {
		OSWL_LOG_E("Error: %s - malloc thread failed!\n", __func__);
		return -1;
	}

	ret = pthread_attr_init(&attr);
	if (ret) {
		OSWL_LOG_E("Error:  pthread_attr_init()!");
		return -1;
	}

	pthread_attr_setstacksize(&attr, thread->stack_size);
//	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_attr_getschedpolicy(&attr, &policy);

	if (thread->priority >= OSWL_THREAD_PRIORITY_MAX)
		thread->priority = OSWL_THREAD_PRIORITY_MAX;
	else if (thread->priority < OSWL_THREAD_PRIORITY_MIN)
		thread->priority = OSWL_THREAD_PRIORITY_MIN;

	sched.sched_priority = thread->priority;
	pthread_attr_setschedparam(&attr, &sched);

	ret = pthread_create(ptask, &attr, thread_entry, thread);
	if (ret) {
        OSWL_LOG_E("Error:  %s - rt_thread_create error!\n", __func__);
		rt_free(ptask);
        return -1;
    }

	thread->thread = ptask;

	pthread_setname_np(*ptask, (char *)thread->name);

	pthread_setschedparam(*ptask, policy, &sched);

    return 0;
}

void oswl_kthread_destory(oswl_thread_t *thread)
{
	pthread_t *ptask;

    if ((thread == RT_NULL) && (thread->thread == RT_NULL)) {
        OSWL_LOG_E("%s - thread invalid!\n", __FUNCTION__);
        return;
    }

	ptask = (pthread_t *)thread->thread;
	if (ptask)
	{
		pthread_join(*ptask, RT_NULL);
		rt_free(ptask);
	}
}

int oswl_pthread_cond_init(oswl_pthread_cond_t *p_cond)
{
	pthread_cond_t *cond;
	cond = rt_malloc(sizeof(pthread_cond_t));
	p_cond->data = cond;
	memset(cond, 0, sizeof(pthread_cond_t));
	
	return pthread_cond_init(cond, RT_NULL);
}

int oswl_pthread_cond_wait(oswl_pthread_cond_t *p_cond, oswl_mutex_t *mutex)
{
#ifndef MUTEX_STATIC_DEFINE
	pthread_cond_t *cond = (pthread_cond_t *)p_cond->data;	
	return pthread_cond_wait(cond, (pthread_mutex_t *)(mutex->mutex));
#else
	return pthread_cond_wait(cond, (pthread_mutex_t *)(mutex));
#endif
}

int oswl_pthread_cond_timedwait(oswl_pthread_cond_t *p_cond, oswl_mutex_t *mutex,
	struct oswl_timespec *abstime)
{
	struct timespec time;
	pthread_cond_t *cond = (pthread_cond_t *)p_cond->data;

	time.tv_sec = abstime->tv_sec;
	time.tv_nsec = abstime->tv_nsec;
#ifndef MUTEX_STATIC_DEFINE
	return pthread_cond_timedwait(cond, (pthread_mutex_t *)(mutex->mutex), &time);
#else
	return pthread_cond_timedwait(cond, (pthread_mutex_t *)(mutex), &time);	
#endif
}


int oswl_pthread_cond_signal(oswl_pthread_cond_t *p_cond)
{
	pthread_cond_t *cond = (pthread_cond_t *)p_cond->data;
	return pthread_cond_signal(cond);
}

int oswl_pthread_cond_destroy(oswl_pthread_cond_t *p_cond)
{
	pthread_cond_t *cond = (pthread_cond_t *)p_cond->data;
	int ret = pthread_cond_destroy(cond);

	rt_free(cond);

	return ret;
}

int oswl_pthread_cond_broadcast(oswl_pthread_cond_t *p_cond)
{
	pthread_cond_t *cond = (pthread_cond_t *)p_cond->data;
	return pthread_cond_broadcast(cond);
}

#endif

char *oswl_get_optarg(void)
{
    return optarg;
}


void oswl_reset_optind(void)
{
    optind = 0;
}


char oswl_toupper(char *str) {
    char ch;
    ch = toupper(*str);
    return ch;
}

int oswl_isdigit(int ch) {
    return isdigit(ch);
}

RTM_EXPORT(oswl_thread_init);
RTM_EXPORT(oswl_kthread_destory);
RTM_EXPORT(oswl_pthread_cond_init);
RTM_EXPORT(oswl_pthread_cond_wait);
RTM_EXPORT(oswl_pthread_cond_timedwait);
RTM_EXPORT(oswl_pthread_cond_signal);
RTM_EXPORT(oswl_pthread_cond_destroy);
RTM_EXPORT(oswl_pthread_cond_broadcast);

RTM_EXPORT(oswl_get_optarg);
RTM_EXPORT(oswl_reset_optind);
//RTM_EXPORT(oswl_getopt_long);
RTM_EXPORT(oswl_toupper);
RTM_EXPORT(oswl_isdigit);