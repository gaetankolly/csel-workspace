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

static struct task_struct *sleeper;
static struct task_struct *awakener;

DECLARE_WAIT_QUEUE_HEAD(queue);
static int flag = 0;

static int firstTime = 1;

static char *wakeText = "Thread 1:  Wakey Wakey!! !!!!!(o≧ω≦)○))ｏ(＿＿*)Ｚｚｚ";
static char *sleepText = "Thread 2:  Uuuh... ＼ (~o~)／  I'm awake";

static int skeleton_sleeper(void *message)
{
    pr_info("skeleton sleeper activated!!!\n");
    while (!kthread_should_stop())
    {
        if (firstTime != 1)
            pr_info("INFOS:   skeleton_sleeper falls asleep again........\n");
        else
            firstTime = 1;
        
        wait_event_interruptible(queue, flag == 1);
        printk(message);
        flag = 0;
    }
    pr_info("skeleton task deactivated!!!\n");
    return 0;
}

static int skeleton_awakener(void *message)
{
    pr_info("skeleton task activated!!!\n");
    while (!kthread_should_stop())
    {
        pr_info("INFOS:   skeleton_awakener falls asleep for 5s\n");
        ssleep(5);
        flag = 1;
        wake_up_interruptible(&queue);
        printk(message);
    }
    pr_info("skeleton task deactivated!!!\n");
    return 0;
}

static int __init skeleton_init(void)
{
    pr_info ("Linux module 6 skeleton loaded\n");

    sleeper = kthread_run(skeleton_sleeper, (void *)sleepText, "SkeletonSleeper");
    awakener = kthread_run(skeleton_awakener, (void *)wakeText, "SkeletonAwakener");

    return 0;
}
static void __exit skeleton_exit(void)
{
    kthread_stop(sleeper);
    kthread_stop(awakener);
    pr_info ("Linux module 6 skeleton unloaded\n");
}
module_init (skeleton_init);
module_exit (skeleton_exit);

MODULE_AUTHOR ("Andrea Enrile; Gaëtan Kolly");
MODULE_DESCRIPTION ("Module skeleton");
MODULE_LICENSE ("GPL");