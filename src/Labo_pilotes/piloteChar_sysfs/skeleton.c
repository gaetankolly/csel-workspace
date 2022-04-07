/* skeleton.c */
#include <linux/cdev.h>        /* needed for char device driver */
#include <linux/fs.h>          /* needed for device drivers */
#include <linux/init.h>        /* needed for macros */
#include <linux/kernel.h>      /* needed for debugging */
#include <linux/module.h>      /* needed by all modules */
#include <linux/moduleparam.h> /* needed for module parameters */
#include <linux/uaccess.h>     /* needed to copy data to/from user */
#include <linux/slab.h>        /* needed for dynamic memory management */
#include <linux/device.h>       /* needed for sysfs handling */
#include <linux/string.h>


static int attribut_1 =0;

ssize_t show_attribut_1(struct device* dev, struct device_attribute* attr, char* buf)
{
    sprintf(buf, "%d\n", attribut_1);
    return strlen(buf);
}
ssize_t store_attribut_1(struct device* dev, struct device_attribute* attr, const char* buf,size_t count)
{
    attribut_1 = simple_strtol(buf, 0, 10);
    pr_debug("Value recieved= %d\n",attribut_1);
    return count;
}

DEVICE_ATTR(attribut_1, 0664, show_attribut_1, store_attribut_1);

static struct class* sysfs_class;
static struct device* sysfs_device;

static int __init skeleton_init(void)
{   
 
    int status = 0;
    sysfs_class  = class_create(THIS_MODULE, "my_sysfs_class");
    sysfs_device = device_create(sysfs_class, NULL, 0, NULL, "my_sysfs_device");
    if (status == 0) status = device_create_file(sysfs_device, &dev_attr_attribut_1);

    pr_info("Linux module skeleton loaded\n");
    return 0;
}

static void __exit skeleton_exit(void)
{

    device_remove_file(sysfs_device, &dev_attr_attribut_1);
    device_destroy(sysfs_class, 0);
    class_destroy(sysfs_class); 

    pr_info("Linux module skeleton unloaded\n");
}

module_init(skeleton_init);
module_exit(skeleton_exit);

MODULE_AUTHOR("Andrea Enrile, Gaëtan Kolly");
MODULE_DESCRIPTION("Module skeleton");
MODULE_LICENSE("GPL");
