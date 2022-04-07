// skeleton.c
#include <linux/module.h>	// needed by all modules
#include <linux/init.h>		// needed for macros
#include <linux/kernel.h>	// needed for debugging

#include <linux/moduleparam.h>	// needed for module parameters

/* function needed for list */
#include <linux/slab.h>
#include <linux/list.h>		
#include <linux/string.h>

#include <linux/ioport.h> /* needed for memory region handling */
#include <linux/io.h>			/* needed for mmio handling */

#define ID_START (0x01C14200)
#define TEMP_START (0x01C25080)
#define MAC_START (0x01C30050)

#define MY_START_ADDR(addr, pageSize) (addr - (addr % pageSize))

#define PAGE_SIZE (0x1000)

static struct resource *res = 0;

static int __init skeleton_init(void)
{
	unsigned char *myRegisters[3] = {
			[0] = 0,
		};
	unsigned int chipId[4] = {
			[0] = 0,
		};
	unsigned int macAddr[2] = {
			[0] = 0,
	};
	long temp = 0;

	pr_info ("Linux module 5 skeleton loaded\n");

	res = request_mem_region(MY_START_ADDR(ID_START, PAGE_SIZE), PAGE_SIZE, "ChipID");
	/* From solution */
	if ((res == 0))
		pr_info("Error while reserving memory region...\n");

	myRegisters[0] = ioremap(MY_START_ADDR(ID_START, PAGE_SIZE), PAGE_SIZE);
	myRegisters[1] = ioremap(MY_START_ADDR(TEMP_START, PAGE_SIZE), PAGE_SIZE);
	myRegisters[2] = ioremap(MY_START_ADDR(MAC_START, PAGE_SIZE), PAGE_SIZE);

	/* From solution */
	if ((myRegisters[0] == 0) || (myRegisters[1] == 0) || (myRegisters[2] == 0))
	{
		pr_info("Error while trying to map processor register...\n");
		return -EFAULT;
	}

	chipId[0] = ioread32(myRegisters[0] + 0x200);
	chipId[1] = ioread32(myRegisters[0] + 0x204);
	chipId[2] = ioread32(myRegisters[0] + 0x208);
	chipId[3] = ioread32(myRegisters[0] + 0x20c);
	pr_info("chipid=%x %x %x %x\n",
					chipId[0], chipId[1], chipId[2], chipId[3]);

	temp = -1191 * (int)ioread32(myRegisters[1] + 0x80) / 10 + 223000;
	pr_info("temp=%ld\n", temp);

	macAddr[0] = ioread32(myRegisters[2] + 0x50);
	macAddr[1] = ioread32(myRegisters[2] + 0x54);
	/* From solution */
	pr_info("mac-addr=%02x:%02x:%02x:%02x:%02x:%02x\n",
					(macAddr[1] >> 0) & 0xff,
					(macAddr[1] >> 8) & 0xff,
					(macAddr[1] >> 16) & 0xff,
					(macAddr[1] >> 24) & 0xff,
					(macAddr[0] >> 0) & 0xff,
					(macAddr[0] >> 8) & 0xff);

	iounmap(myRegisters[0]);
	iounmap(myRegisters[1]);
	iounmap(myRegisters[2]);

	return 0;
}

static void __exit skeleton_exit(void)
{
	if (res != 0)
		release_mem_region(MY_START_ADDR(ID_START, PAGE_SIZE), PAGE_SIZE);

	pr_info("Linux module 5 skeleton unloaded\n");
}

module_init (skeleton_init);
module_exit (skeleton_exit);

MODULE_AUTHOR ("Andrea Enrile; Gaëtan Kolly");
MODULE_DESCRIPTION ("Module skeleton");
MODULE_LICENSE ("GPL");