#include "osal.h"

#define THREAD_PRIORITY         (CONFIG_NUM_PREEMPT_PRIORITIES/2 + 1)
#define THREAD_STACK_SIZE       512

struct thread_user_data {
	struct k_thread ptask;
	threadfn_t thread_func;
	K_KERNEL_STACK_MEMBER(stack, THREAD_STACK_SIZE);
	k_tid_t tid;
	void *data;
};

static void thread_entry(void *p1, void *p2, void *p3)
{
	struct thread_user_data *thread_data = (struct thread_user_data *)p1;
	if (thread_data && thread_data->thread_func)
		thread_data->thread_func(thread_data->data);
}

__attribute ((visibility("default"))) 
osal_task_t *osal_kthread_create(threadfn_t thread, void *data, const char *name)
{
	struct thread_user_data *thread_data;

	thread_data = (struct thread_user_data *)k_malloc(sizeof(struct thread_user_data));
	if (thread_data == NULL) {
        osal_printk("%s - rt_malloc thread_data error!\n", __FUNCTION__);
        return NULL;
    }

    memset(thread_data, 0, sizeof(struct thread_user_data));

	thread_data->thread_func = thread;
	thread_data->data = data;
	thread_data->tid = k_thread_create(&thread_data->ptask, thread_data->stack, THREAD_STACK_SIZE, (k_thread_entry_t)thread_entry,
		thread_data, NULL, NULL, THREAD_PRIORITY, 0, K_NO_WAIT);

	if (thread_data->tid)
		k_thread_name_set(thread_data->tid, name);

    return (osal_task_t *)thread_data;
}

__attribute ((visibility("default"))) 
void osal_kthread_destory(osal_task_t *task, unsigned int stop_flag)
{
	struct thread_user_data *thread_data;

    if (task == NULL) {
        osal_printk("%s - task parameter invalid!\n", __FUNCTION__);
        return;
    }
	thread_data = (struct thread_user_data *)task;

	if (stop_flag)
		k_thread_abort (thread_data->tid);

    k_free(thread_data);
}

