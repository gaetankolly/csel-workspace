// skeleton.c
#include <linux/module.h> /* needed by all modules */
#include <linux/init.h>   /* needed for macros */
#include <linux/kernel.h> /* needed for debugging */

#include <linux/moduleparam.h> /* needed for module parameters */

#include <linux/slab.h>   /* needed for dynamic memory allocation */
#include <linux/list.h>   /* needed for linked list processing */
#include <linux/string.h> /* needed for string handling */

#include <linux/ioport.h> /* needed for memory region handling */
#include <linux/io.h>     /* needed for mmio handling */

#include <linux/kthread.h> /* needed for kernel thread management */
#include <linux/delay.h>   /* needed for delay fonctions */

static struct task_struct *myTask;

static char *text = "Helloooooo, someone there???";

static int skeleton_task(void *message)
{
    pr_info("skeleton task activated!!!\n");
    while (!kthread_should_stop())
    {
        ssleep(5);
        printk(message);
    }
    pr_info("skeleton task deactivated!!!\n");
    return 0;
}

static int __init skeleton_init(void)
{
    pr_info ("Linux module 6 skeleton loaded\n");

    myTask = kthread_run(skeleton_task, (void*)text, "mySkeletonThread");

    return 0;
}
static void __exit skeleton_exit(void)
{
    kthread_stop(myTask);
    pr_info ("Linux module 6 skeleton unloaded\n");
}
module_init (skeleton_init);
module_exit (skeleton_exit);

MODULE_AUTHOR ("Andrea Enrile; Gaëtan Kolly");
MODULE_DESCRIPTION ("Module skeleton");
MODULE_LICENSE ("GPL");