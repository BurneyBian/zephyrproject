#include <string.h>
#include <stdio.h>

#include "osal.h"

static struct k_work_q wq;
static struct k_work_q highpri_wq;

static void work_func(struct k_work *item)
{
	struct osal_work_struct *work = CONTAINER_OF(item, struct osal_work_struct, work);
    if (work && work->func)
		work->func(work);	
}

int osal_init_work(struct osal_work_struct *work, osal_work_func_t func)
{
    if (work == NULL) {
        osal_printk("%s - parameter invalid!\n", __FUNCTION__);
        return -EINVAL;
    }

	work->func = func;
	k_work_init(&work->work, work_func);
    return 0;
}

int osal_schedule_work(struct osal_work_struct *work)
{
    if (work == NULL) {
        osal_printk("%s - parameter invalid!\n", __FUNCTION__);
        return -EINVAL;
    }

	work->wq = &wq;
	k_work_submit_to_queue(&wq, &work->work);

	return 0;
}

int osal_schedule_highpri_work(struct osal_work_struct *work)
{
    if (work == NULL) {
        osal_printk("%s - parameter invalid!\n", __FUNCTION__);
        return -EINVAL;
    }

	//work->wq = &highpri_wq;
	work->wq = &wq;
	k_work_submit_to_queue(&highpri_wq, &work->work);

	return 0;
}

void osal_destroy_work(struct osal_work_struct *work)
{
    if (work == NULL) {
        osal_printk("%s - parameter invalid!\n", __FUNCTION__);
        return;
    }

	k_work_cancel(&work->work);
	work->wq = NULL;
}

#define THREAD_PRIORITY         (CONFIG_NUM_PREEMPT_PRIORITIES/2 + 1)
#define THREAD_PRIORITY_HI      (CONFIG_NUM_PREEMPT_PRIORITIES/2 - 1)
#define THREAD_STACK_SIZE       512

static K_THREAD_STACK_DEFINE(stack_area_0, THREAD_STACK_SIZE);
//static K_THREAD_STACK_DEFINE(stack_area_1, THREAD_STACK_SIZE);
void osal_work_queue_init(void)
{
	k_work_queue_init(&wq);
	//k_work_queue_init(&highpri_wq);

	k_work_queue_start(&wq, stack_area_0, K_THREAD_STACK_SIZEOF(stack_area_0), THREAD_PRIORITY, NULL);
	//k_work_queue_start(&highpri_wq, stack_area_1, K_THREAD_STACK_SIZEOF(stack_area_1), THREAD_PRIORITY_HI, NULL);
}
