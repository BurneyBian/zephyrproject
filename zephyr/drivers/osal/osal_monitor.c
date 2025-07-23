#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>

#include <dtb_notify.h>
#include <osal.h>

static dtb_notifier dtbo_notifier;
static int log_verbose=0;

//extern int osal_i2c_init(void);
//extern void osal_i2c_print(void);

void process_i2c_change(char* name)
{
    if(rt_strncmp(name,"i2c1ACME",8)==0){
       // osal_i2c_init();
       // osal_i2c_print();
    }
}

static int dtbo_inserted(const char *devname)
{
    if(log_verbose==1){
        osal_printk( "DTBO inserted: %s\n",  devname);
    }
    return 0;
}

static void dtbo_removed(const char *devname)
{
    if(log_verbose==1){
        osal_printk( "DTBO removed: %s\n", devname);
    }
}

static void dtbo_add_prop(const char *devname)
{
    if(log_verbose==1){
        osal_printk( "DTBO add prop: %s\n", devname);
    }
}

static void dtbo_remove_prop(const char *devname)
{
    if(log_verbose==1){
        osal_printk( "DTBO remove prop: %s\n", devname);
    }
}

static void dtbo_update_prop(const char *devname)
{
    if(log_verbose==1){
        osal_printk( "DTBO update prop: %s\n", devname);
    }
    process_i2c_change(devname);
}
 
static int dtbo_device_notifier(const char *devname, int event)
{
    switch (event) {
        case OF_RECONFIG_ATTACH_NODE:{
            dtbo_inserted(devname);
            break;
        }
        case OF_RECONFIG_DETACH_NODE:{
            dtbo_removed(devname);
            break;
        }
        case OF_RECONFIG_ADD_PROPERTY:{
            dtbo_add_prop(devname);
            break;
        }
	    case OF_RECONFIG_REMOVE_PROPERTY:{
            dtbo_remove_prop(devname);
            break;
        }
	    case OF_RECONFIG_UPDATE_PROPERTY:{
            dtbo_update_prop(devname);
            break;
        }
        default:{
            osal_printk( "DTBO opt: %s %ld\n", devname, event);
        }

    }

    return 0;
}


int  dtbo_monitor_init(void) {
    int ret;
    dtbo_notifier.cb = dtbo_device_notifier;
	of_reconfig_notifier_init();
    ret = of_reconfig_notifier_register(&dtbo_notifier);
    if (ret!=0) {
        osal_printk_err("Failed to register platform bus notifier: %d\n", ret);
        return ret;
    }
 
    osal_printk("DTBO monitor registered\n");
    return 0;
}
 
void  dtbo_monitor_exit(void) {
    of_reconfig_notifier_unregister(&dtbo_notifier);
	of_reconfig_notifier_deinit();
    osal_printk("DTBO monitor unregistered\n");
}
 

 