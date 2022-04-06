
#include <linux/module.h>	/* needed by all modules */
#include <linux/init.h>		/* needed for macros */
#include <linux/kernel.h>	/* needed for debugging */

#include <linux/interrupt.h>	// use for interupt function
#include <linux/gpio.h>			  // use for gpio function


#define K1	0
#define K2	2
#define K3	3

static char* K1_name="Button_k1";
static char* K2_name="Button_k2";
static char* K3_name="Button_k3";

//static int* counter;

// data to share with irq
typedef struct data{
    const char name[5];
    int counter;
} DataHandler;

static DataHandler data_k1= { .name = "K1\0",
                        .counter=0};
static DataHandler data_k2= { .name = "K2\0",
                        .counter=0};
static DataHandler data_k3= { .name = "K3\0",
                        .counter=0};

irqreturn_t gpio_isr(int irq, void* handle)
{
  
  DataHandler* data = (DataHandler*) handle;
  data->counter++;
	pr_info ("Button %s, counter= %d\n", data->name,data->counter);
  
	return IRQ_HANDLED;
}


static int __init skeleton_init(void)
{
	int status = 0;

	// install k1
	if (status == 0) status = gpio_request (K1, "k1");
	if (status == 0)
		status = request_irq(gpio_to_irq(K1), gpio_isr,
			IRQF_TRIGGER_FALLING | IRQF_SHARED, K1_name, &data_k1);

	// install k2
	if (status == 0) status = gpio_request (K2, "k2");
	if (status == 0)
		status = request_irq(gpio_to_irq(K2), gpio_isr,
			IRQF_TRIGGER_FALLING | IRQF_SHARED, K2_name, &data_k2);

	// install k3
	if (status == 0) status = gpio_request (K3, "k3");
	if (status == 0)
		status = request_irq(gpio_to_irq(K3), gpio_isr,
			IRQF_TRIGGER_FALLING | IRQF_SHARED, K3_name, &data_k3);

	pr_info ("Linux module skeleton loaded\n");

	return status;
}

static void __exit skeleton_exit(void)
{
	free_irq(gpio_to_irq(K1), &data_k1);
  gpio_free(K1);
	
	free_irq(gpio_to_irq(K2), &data_k2);
  gpio_free(K2);
	
	free_irq(gpio_to_irq(K3), &data_k3);
  gpio_free(K3);

	pr_info ("Linux module skeleton unloaded\n");
}

module_init (skeleton_init);
module_exit (skeleton_exit);

MODULE_AUTHOR("Andrea Enrile; Gaëtan Kolly");
MODULE_DESCRIPTION("Module skeleton");
MODULE_LICENSE("GPL");