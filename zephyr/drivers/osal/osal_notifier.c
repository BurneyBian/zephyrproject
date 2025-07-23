#include "osal.h"
#include <string.h>
struct nb_node {
    struct osal_notifier_block nb;
    struct osal_list_head node;
};

int osal_register_notifier(void *list, struct osal_notifier_block *nb)
{
    struct nb_node *nb_node = NULL;
    unsigned long flags;

    if (nb == NULL) {
        osal_printk("nb is NULL!\n");
        return -EINVAL;
    }
    nb_node = osal_kmalloc(sizeof(struct nb_node), osal_gfp_kernel);
    if (nb_node == NULL) {
        osal_printk("osal_register_notifier kmalloc nb_node failed!\n");
        return -ENOMEM;
    }
    memset(nb_node, 0, sizeof(struct nb_node));
    nb->notifier_block = nb_node;
    osal_memcpy(&nb_node->nb, nb, sizeof(struct osal_notifier_block));
    flags = irq_lock();
    osal_list_add_tail(&nb_node->node, (struct osal_list_head *)list);
    irq_unlock(flags);

    return 0;
}

int osal_unregister_notifier(void *list, struct osal_notifier_block *nb)
{
    unsigned long flags;
    if (nb == NULL) {
        osal_printk("nb is NULL!\n");
        return -EINVAL;
    }
    flags = irq_lock();
    osal_list_del(&((struct nb_node *)nb->notifier_block)->node);
    osal_kfree((struct nb_node *)nb->notifier_block);
    irq_unlock(flags);

    return 0;
}

int osal_notifier_call(void *list, unsigned long val, void *v)
{
    unsigned long flags;
    struct nb_node *nb_node = NULL;
	int ret = OSAL_NOTIFY_DONE;

    flags = irq_lock();
    osal_list_for_each_entry(nb_node, (struct osal_list_head *)list, node) {
        if (nb_node->nb.notifier_call) {
            ret = nb_node->nb.notifier_call(&nb_node->nb, val, v);
			if (ret & OSAL_NOTIFY_STOP_MASK)
				break;
		}
    }
    irq_unlock(flags);

    return 0;
}

