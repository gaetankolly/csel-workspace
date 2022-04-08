
#include <linux/cdev.h>			 /* needed for char device driver */
#include <linux/fs.h>				 /* needed for device drivers */
#include <linux/gpio.h>			 /* needed for i/o handling */
#include <linux/init.h>			 /* needed for macros */
#include <linux/interrupt.h> /* needed for interrupt handling */
#include <linux/io.h>				 /* needed for mmio handling */
#include <linux/ioport.h>		 /* needed for memory region handling */
#include <linux/kernel.h>		 /* needed for debugging */
#include <linux/miscdevice.h>
#include <linux/module.h>	 /* needed by all modules */
#include <linux/poll.h>		 /* needed for polling handling */
#include <linux/sched.h>	 /* needed for scheduling constants */
#include <linux/uaccess.h> /* needed to copy data to/from user */
#include <linux/wait.h>		 /* needed for wating */

#define K1	0
#define K2	2
#define K3	3

static char* K1_name="Button_k1";
static char* K2_name="Button_k2";
static char* K3_name="Button_k3";

// queue variables
DECLARE_WAIT_QUEUE_HEAD(myQueue);
static atomic_t request_can_be_processed;

irqreturn_t gpio_isr(int irq, void* handle)
{
	atomic_set(&request_can_be_processed, 1);
	wake_up_interruptible(&myQueue);
	pr_info("IRQ unknown\n");
  
	return IRQ_HANDLED;
}

static ssize_t skeleton_read(struct file *f,
														 char __user *buf,
														 size_t sz,
														 loff_t *off)
{
	return 0;
}

static unsigned int skeleton_poll(struct file *f, poll_table *wait)
{
	unsigned int mask = 0;
	poll_wait(f, &myQueue, wait);
	if (atomic_read(&request_can_be_processed) != 0)
	{
		mask |= POLLIN | POLLRDNORM;
		atomic_set(&request_can_be_processed, 0);
		pr_info("Poll called from user space.\n");
	}
	return mask;
}

static struct file_operations skeleton_fops = {
		.owner = THIS_MODULE,
		.read = skeleton_read,
		.poll = skeleton_poll,
};

struct miscdevice my_misc_device = {
		.minor = MISC_DYNAMIC_MINOR,
		.fops = &skeleton_fops,
		.name = "mymodule",
		.mode = 0777,
};

static int __init skeleton_init(void)
{
	int status = 0;

	atomic_set(&request_can_be_processed, 0);

	pr_info("Linux module skeleton loaded\n");

	status = misc_register(&my_misc_device);

	// install k1
	if (status == 0)
		status = devm_request_irq(my_misc_device.this_device,
															gpio_to_irq(K1),
															gpio_isr,
										 					IRQF_TRIGGER_FALLING | IRQF_SHARED,
															K1_name,
															K1_name);

	// install k2
	if (status == 0)
		status = devm_request_irq(my_misc_device.this_device,
															gpio_to_irq(K2),
															gpio_isr,
															IRQF_TRIGGER_FALLING | IRQF_SHARED,
															K2_name,
															K2_name);

	// install k3
	if (status == 0)
		status = devm_request_irq(my_misc_device.this_device,
															gpio_to_irq(K3),
															gpio_isr,
															IRQF_TRIGGER_FALLING | IRQF_SHARED,
															K3_name,
															K3_name);

	return status;
}

static void __exit skeleton_exit(void)
{
	misc_deregister(&my_misc_device);

	pr_info ("Linux module skeleton unloaded\n");
}

module_init (skeleton_init);
module_exit (skeleton_exit);

MODULE_AUTHOR("Andrea Enrile; Gaëtan Kolly");
MODULE_DESCRIPTION("Module skeleton");
MODULE_LICENSE("GPL");