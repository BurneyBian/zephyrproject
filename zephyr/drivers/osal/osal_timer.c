#include "osal.h"

//------------------------timer------------------------------//
static void hrtimer_out(struct k_timer *timer)
{
	OSAL_HRTIMER_RESTART_E ret;

	osal_hrtimer_t *phrtimer = (osal_hrtimer_t *)timer->user_data;
	if (phrtimer && phrtimer->function) {
		ret = phrtimer->function(phrtimer);
		//osal_printk("%s - %lx interval=%ld\n", __func__, (unsigned long)timer, phrtimer->interval);
		if (ret == OSAL_HRTIMER_RESTART) {
			k_timer_start(timer, K_MSEC(phrtimer->interval), K_NO_WAIT);
		}
	}
}

__attribute ((visibility("default"))) 
int osal_hrtimer_create(osal_hrtimer_t *phrtimer)
{
	struct k_timer *timer;

    if (phrtimer == NULL) {
        osal_printk("%s - phrtimer parameter invalid!\n", __FUNCTION__);
        return -EINVAL;
    }

	timer = (struct k_timer *)k_malloc(sizeof(struct k_timer));
    if (timer == NULL) {
        osal_printk("%s - hrtimer malloc failed!\n", __FUNCTION__);
        return -1;
    }

	phrtimer->timer = timer;
	k_timer_init(timer, hrtimer_out, NULL);
	timer->user_data = phrtimer;

	//osal_printk("%s - %lx\n", __func__, (unsigned long)timer);
	return 0;
}

__attribute ((visibility("default"))) 
int osal_hrtimer_start(osal_hrtimer_t *phrtimer)
{
    if (phrtimer == NULL) {
        osal_printk("%s - phrtimer parameter invalid!\n", __FUNCTION__);
        return -EINVAL;
    }

	if (phrtimer->timer) {
		//osal_printk("%s - %lx interval=%ld\n", __func__, (unsigned long)phrtimer->timer, phrtimer->interval);
		k_timer_start((struct k_timer *)phrtimer->timer, K_MSEC(phrtimer->interval), K_NO_WAIT);
		phrtimer->count = 1;
	}

	return 0;
}

__attribute ((visibility("default"))) 
int osal_hrtimer_cancel(osal_hrtimer_t *phrtimer)
{
    if (phrtimer == NULL) {
        osal_printk("%s - phrtimer parameter invalid!\n", __FUNCTION__);
        return -EINVAL;
    }

	if (phrtimer->timer) {
		k_timer_stop((struct k_timer *)phrtimer->timer);
		phrtimer->count = 0;
	}

    return 0;
}

__attribute ((visibility("default"))) 
int osal_hrtimer_destory(osal_hrtimer_t *phrtimer)
{
    if (phrtimer == NULL) {
        osal_printk("%s - phrtimer parameter invalid!\n", __FUNCTION__);
        return -EINVAL;
    }

	if (phrtimer->count) {
		k_timer_stop((struct k_timer *)phrtimer->timer);
		phrtimer->count = 0;
	}

	if (phrtimer->timer)
		k_free(phrtimer->timer);

    return 0;
}

static void timeout_func(struct k_timer *ktimer)
{
	osal_timer_t *timer = (osal_timer_t *)ktimer->user_data;

	if (timer && timer->function)
		timer->function((unsigned long)timer);
}

__attribute ((visibility("default"))) 
int osal_timer_create(osal_timer_t *timer)
{
	struct k_timer *ktimer;

    if (timer == NULL) {
        osal_printk("%s - timer parameter invalid!\n", __FUNCTION__);
        return -EINVAL;
    }

	ktimer = (struct k_timer *)k_malloc(sizeof(struct k_timer));
    if (ktimer == NULL) {
        osal_printk("%s - timer malloc failed!\n", __FUNCTION__);
        return -ENOMEM;
    }

	timer->timer = ktimer;
	k_timer_init(ktimer, timeout_func, NULL);
	ktimer->user_data = timer;

    return 0;
}

__attribute ((visibility("default"))) 
int osal_timer_set(osal_timer_t *timer, unsigned long interval)
{
    if (timer == NULL) {
        osal_printk("%s - timer parameter invalid!\n", __FUNCTION__);
        return -EINVAL;
    }

	if (timer->timer) {
		k_timer_start((struct k_timer *)timer->timer, K_MSEC(interval), K_NO_WAIT);
		timer->count = 1;
	}

    return 0;
}

__attribute ((visibility("default"))) 
int osal_timer_del(osal_timer_t *timer)
{
    if (timer == NULL) {
        osal_printk("%s - timer parameter invalid!\n", __FUNCTION__);
        return -EINVAL;
    }

	if (timer->timer) {
		k_timer_stop((struct k_timer *)timer->timer);
		timer->count = 0;
	}

    return 0;
}

__attribute ((visibility("default"))) 
int osal_timer_destory(osal_timer_t *timer)
{
    if (timer == NULL) {
        osal_printk("%s - timer parameter invalid!\n", __FUNCTION__);
        return -EINVAL;
    }

	if (timer->count) {
		k_timer_stop((struct k_timer *)timer->timer);
		timer->count = 0;
	}

	if (timer->timer)
		k_free(timer->timer);

    return 0;
}
//------------------------timer end---------------------------//


//------------------------delay-------------------------------//
__attribute ((visibility("default"))) 
unsigned long osal_msleep(unsigned int msecs)
{
    k_msleep(msecs);
    return 0;
}

__attribute ((visibility("default"))) 
void osal_udelay(unsigned int usecs)
{
	k_busy_wait(usecs);
}

__attribute ((visibility("default"))) 
void osal_mdelay(unsigned int msecs)
{
    k_busy_wait(msecs*1000);
}
//------------------------delay end-----------------------------//


//------------------------time-------------------------------//
__attribute ((visibility("default"))) 
void osal_rtc_time_to_tm(unsigned long time, osal_rtc_time_t *tm)
{
	osal_printk("%s Not support!\n", __FUNCTION__);
}

__attribute ((visibility("default"))) 
void osal_rtc_tm_to_time(osal_rtc_time_t *tm, unsigned long *time)
{
	osal_printk("%s Not support!\n", __FUNCTION__);
}

__attribute ((visibility("default"))) 
int osal_rtc_valid_tm(struct osal_rtc_time *tm)
{
	osal_printk("%s Not support!\n", __FUNCTION__);
	return 0;
}

__attribute ((visibility("default"))) 
void osal_getjiffies(unsigned long long *pjiffies)
{
    *pjiffies = (unsigned long long)k_uptime_ticks();
}

__attribute ((visibility("default"))) 
unsigned int osal_get_tickcount()
{
    return k_uptime_get_32();
}

__attribute ((visibility("default"))) 
unsigned long long osal_sched_clock()
{
    return k_uptime_get()*1000*1000;
}

void osal_gettimeofday(osal_timeval_t *tv)
{
}
__attribute ((visibility("default"))) 
unsigned long long osal_ktime_get_boottime_ns(void)
{
    return osal_sched_clock();
}
__attribute ((visibility("default"))) 
unsigned long long osal_ktime_get_boottime(void)
{
    return osal_sched_clock();
}
__attribute ((visibility("default"))) 
unsigned long long osal_ktime_get(void)
{
    return osal_sched_clock();
}
__attribute ((visibility("default"))) 
unsigned long long osal_ktime_to_ms(unsigned long long kt)
{
    return kt / (1000000);
}

__attribute ((visibility("default"))) 
int osal_is_enable_hrtimer(void)
{
    return 0;
}
//------------------------time end-----------------------------//
