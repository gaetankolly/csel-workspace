/* skeleton.c */
#include <linux/cdev.h>        /* needed for char device driver */
#include <linux/fs.h>          /* needed for device drivers */
#include <linux/init.h>        /* needed for macros */
#include <linux/kernel.h>      /* needed for debugging */
#include <linux/module.h>      /* needed by all modules */
#include <linux/moduleparam.h> /* needed for module parameters */
#include <linux/uaccess.h>     /* needed to copy data to/from user */
#include <linux/slab.h>        /* needed for dynamic memory management */

#define BUFFER_SZ 10000
#define NB_INSTANCE_MAX 5

static int instances = 1;
module_param(instances, int, 0);

//static char buffer[BUFFER_SZ];      // memory representing the files
static char** buffers;              // memory representing the files

static dev_t skeleton_dev;          // contain minor and major number, register with alloc_chrdev_region
static struct cdev skeleton_cdev;   // representation of the driver into the kernel, 

static ssize_t skeleton_read(struct file* f,  		// only used to read file attribut
                             char __user* buf,		// buffer from user where to write
                             size_t count,			// number of byte required
                             loff_t* off)			// position in the file, so in buffer
{

    ssize_t remaining = BUFFER_SZ - (ssize_t)(*off); // remaining size
    //char* ptr = buffer + *off;                    // at the position required
    char* ptr = (char*) f->private_data + *off;       // at the position required
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
        //char* ptr = buffer + *off; // at the position required
        char* ptr = (char*) f->private_data + *off; // at the position required
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

    f->private_data = buffers[iminor(i)];
    pr_info("skeleton: private_data=%p\n", f->private_data);

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

static int __init skeleton_init(void)
{   
    int i;
    int status; 
	// skeleton_dev = de type dev_t containt the major and minor number (32bits) which represent the device driver
	// skeleton_cdev = struct cdev 
	// alloc_chrdev_region allocate dymicaly the minor and major number in skeleton_dev
	//int alloc_chrdev_region (dev_t* dev, unsinged baseminor, unsinged count, const char* name);
    if (instances > NB_INSTANCE_MAX){
        instances=NB_INSTANCE_MAX;
        pr_warn("Maximal number of instance is %d, instances set to %d\n",NB_INSTANCE_MAX,NB_INSTANCE_MAX);
    }
    status = alloc_chrdev_region(&skeleton_dev, 0, instances, "mymodule");
    if (status == 0) {
		// init cdev structure, make it ready to add to the system with cdev_add
        cdev_init(&skeleton_cdev, &skeleton_fops);
        skeleton_cdev.owner = THIS_MODULE;
        status = cdev_add(&skeleton_cdev, skeleton_dev, instances); // add character device to the system
    }

    if (status == 0) {
        buffers = kzalloc(sizeof(char*) * instances, GFP_KERNEL);
        for (i = 0; i < instances; i++){
            buffers[i] = kzalloc(BUFFER_SZ, GFP_KERNEL);
        }
    }

    pr_info("Linux module skeleton loaded\n");
    return 0;
}

static void __exit skeleton_exit(void)
{
    int i;
    cdev_del(&skeleton_cdev);
    unregister_chrdev_region(skeleton_dev, instances);

    for (i = 0; i < instances; i++){
        kfree(buffers[i]);
    }
    kfree(buffers);

    pr_info("Linux module skeleton unloaded\n");
}

module_init(skeleton_init);
module_exit(skeleton_exit);

MODULE_AUTHOR("Andrea Enrile, Gaëtan Kolly");
MODULE_DESCRIPTION("Module skeleton");
MODULE_LICENSE("GPL");
