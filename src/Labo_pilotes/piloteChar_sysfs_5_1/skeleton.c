/* skeleton.c */
#include <linux/cdev.h>        /* needed for char device driver */
#include <linux/fs.h>          /* needed for device drivers */
#include <linux/init.h>        /* needed for macros */
#include <linux/kernel.h>      /* needed for debugging */
#include <linux/module.h>      /* needed by all modules */
#include <linux/moduleparam.h> /* needed for module parameters */
#include <linux/uaccess.h>     /* needed to copy data to/from user */
#include <linux/string.h>
#include <linux/miscdevice.h>

#define BUFFER_SZ 10000

static char buffer[BUFFER_SZ];      // memory representing the file
static dev_t skeleton_dev;          // contain minor and major number, register with alloc_chrdev_region
static struct cdev skeleton_cdev;   // representation of the driver into the kernel, 

/* operation sysfs */
ssize_t show_attribut_buffer(struct device* dev, struct device_attribute* attr, char* buf)
{
    sprintf(buf, "%s\n", buffer);
    return strlen(buf);
}
ssize_t store_attribut_buffer(struct device* dev, struct device_attribute* attr, const char* buf,size_t count)
{
    if(count>=BUFFER_SZ){
        count=BUFFER_SZ-1;
        pr_info("forced count to %d\n",(int)count);
    }
    memcpy(buffer,buf,count);

    pr_debug("Value recieved= %s\n",buffer);
    return count;
}

DEVICE_ATTR(buffer, 0664, show_attribut_buffer, store_attribut_buffer);

static struct class* sysfs_class;
static struct device* sysfs_device;

/* operation sur /dev */
static ssize_t skeleton_read(struct file* f,  		// only used to read file attribut
                             char __user* buf,		// buffer from user where to write
                             size_t count,			// number of byte required
                             loff_t* off)			// position in the file, so in buffer
{

    ssize_t remaining = BUFFER_SZ - (ssize_t)(*off); // remaining size
    char* ptr         = buffer + *off;               // position where to read
    if (count > remaining) count = remaining;        // security against buffer over read
    *off += count;                                   // update offset for the user
    // copy required number of bytes
    if (copy_to_user(buf, ptr, count) != 0) count = -EFAULT;

    pr_info("skeleton: read operation... read=%ld\n", count);

    return count;
}

static ssize_t skeleton_write(struct file* f,           // only used to read file attribut
                              const char __user* buf,   // buffer from user where to read
                              size_t count,             // number of byte required
                              loff_t* off)              // position in the file, so in buffer
{

    ssize_t remaining = BUFFER_SZ - (ssize_t)(*off);    // remaining size in the buffer

    pr_info("skeleton: at%ld\n", (unsigned long)(*off));

    // check the space remaining
    if (count >= remaining){
        count = remaining-1;
    }
    // store additional bytes into internal buffer
    if (count <= 0) {
        count =0;
    }
    else{
		int i;
        char* ptr = buffer + *off; // at the position required
        *off += count;             // update offset in case of append for the user
        ptr[count] = 0;  // make sure string is null terminated
        if (copy_from_user(ptr, buf, count)) count = -EFAULT;

		for(i=0; i<count;i++){
			if(ptr[i]>=97 && ptr[i]<=122)
                ptr[i]-='a'-'A';
		}	
    }
    pr_info("skeleton: write operation... written=%ld\n", count);
    

    return count;
}

static int skeleton_open(struct inode* i, struct file* f)
{
    pr_info("skeleton : open operation... major:%d, minor:%d\n",
            imajor(i),
            iminor(i));

    if ((f->f_flags & (O_APPEND)) != 0) {
        pr_info("skeleton : opened for appending...\n");
    }
    if ((f->f_mode & (FMODE_READ | FMODE_WRITE)) != 0) {
        pr_info("skeleton : opened for reading & writing...\n");
    } else if ((f->f_mode & FMODE_READ) != 0) {
        pr_info("skeleton : opened for reading...\n");
    } else if ((f->f_mode & FMODE_WRITE) != 0) {
        pr_info("skeleton : opened for writing...\n");
    }
    return 0;
}

static int skeleton_release(struct inode* i, struct file* f)
{
    pr_info("skeleton: release operation...\n");

    return 0;
}


static struct file_operations skeleton_fops = {
    .owner   = THIS_MODULE,
    .open    = skeleton_open,
    .read    = skeleton_read,
    .write   = skeleton_write,
    .release = skeleton_release,
};

struct miscdevice my_misc_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .fops = &skeleton_fops,
    .name = "mymodule",
    .mode = 0664,
};

static int __init skeleton_init(void)
{
	// skeleton_dev = de type dev_t containt the major and minor number (32bits) which represent the device driver
	// skeleton_cdev = struct cdev 
	// alloc_chrdev_region allocate dymicaly the minor and major number in skeleton_dev
	//int alloc_chrdev_region (dev_t* dev, unsinged baseminor, unsinged count, const char* name);
    int status = alloc_chrdev_region(&skeleton_dev, 0, 1, "mymodule");
    if (status == 0) {
		// init cdev structure, make it ready to add to the system with cdev_add
        cdev_init(&skeleton_cdev, &skeleton_fops);
        skeleton_cdev.owner = THIS_MODULE;
        status              = cdev_add(&skeleton_cdev, skeleton_dev, 1); // add character device to the system
    }

    sysfs_class  = class_create(THIS_MODULE, "my_sysfs_class");
    sysfs_device = device_create(sysfs_class, NULL, skeleton_dev, NULL, "my_sysfs_device");
    if (status == 0) status = device_create_file(sysfs_device, &dev_attr_buffer);

    status = misc_register(&my_misc_device);

    pr_info("Linux module skeleton loaded\n");
    return 0;
}

static void __exit skeleton_exit(void)
{
    device_remove_file(sysfs_device, &dev_attr_buffer);
    device_destroy(sysfs_class, skeleton_dev);
    class_destroy(sysfs_class); 
    cdev_del(&skeleton_cdev);
    unregister_chrdev_region(skeleton_dev, 1);

    misc_deregister(&my_misc_device);

    pr_info("Linux module skeleton unloaded\n");
}

module_init(skeleton_init);
module_exit(skeleton_exit);

MODULE_AUTHOR("Andrea Enrile, Gaëtan Kolly");
MODULE_DESCRIPTION("Module skeleton");
MODULE_LICENSE("GPL");