#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include<linux/slab.h>                 //kmalloc()
#include<linux/uaccess.h>              //copy_to/from_user()
#include<linux/sysfs.h> 
#include<linux/kobject.h> 
#include <linux/interrupt.h>
#include <asm/io.h>


#define IRQ_NO 11

void tasklet_fn(struct tasklet_struct *unused);

/* Init the Tasklet by Static Method */
DECLARE_TASKLET(tasklet, tasklet_fn);


/* Tasklet Function */
void tasklet_fn(struct tasklet_struct *unused)
{
    pr_info("Executing Tasklet Function\n");
}


// Interrupt handler for IRQ 11. 
static irqreturn_t irq_handler(int irq,void *dev_id) 
{
    pr_info("Shared IRQ: Interrupt Occurred");

    /* Scheduling Task to Tasklet */
    tasklet_schedule(&tasklet); 

    return IRQ_HANDLED;
}


volatile int dummy_value = 0;


dev_t dev = 0;
static struct class *dev_class;
static struct cdev dummy_cdev;
struct kobject *kobj_ref;

static int __init dummy_driver_init(void);
static void __exit dummy_driver_exit(void);

/*************** Driver functions **********************/
static int dummy_open(struct inode *inode, struct file *file);
static int dummy_release(struct inode *inode, struct file *file);
static ssize_t dummy_read(struct file *filp, 
        char __user *buf, size_t len,loff_t * off);
static ssize_t dummy_write(struct file *filp, 
        const char *buf, size_t len, loff_t * off);

/*************** Sysfs functions **********************/
static ssize_t sysfs_show(struct kobject *kobj, 
        struct kobj_attribute *attr, char *buf);
static ssize_t sysfs_store(struct kobject *kobj, 
        struct kobj_attribute *attr,const char *buf, size_t count);

struct kobj_attribute dummy_attr = __ATTR(dummy_value, 0660, sysfs_show, sysfs_store);

/*
 ** File operation sturcture
 */
static struct file_operations fops =
{
    .owner          = THIS_MODULE,
    .read           = dummy_read,
    .write          = dummy_write,
    .open           = dummy_open,
    .release        = dummy_release,
};

/*
 ** This function will be called when we read the sysfs file
 */  
static ssize_t sysfs_show(struct kobject *kobj, 
        struct kobj_attribute *attr, char *buf)
{
    pr_info("Sysfs - Read!!!\n");
    return sprintf(buf, "%d", dummy_value);
}

/*
 ** This function will be called when we write the sysfsfs file
 */  
static ssize_t sysfs_store(struct kobject *kobj, 
        struct kobj_attribute *attr,const char *buf, size_t count)
{
    pr_info("Sysfs - Write!!!\n");
    sscanf(buf,"%d",&dummy_value);
    return count;
}

/*
 ** This function will be called when we open the Device file
 */  
static int dummy_open(struct inode *inode, struct file *file)
{
    pr_info("Device File Opened...!!!\n");
    return 0;
}

/*
 ** This function will be called when we close the Device file
 */   
static int dummy_release(struct inode *inode, struct file *file)
{
    pr_info("Device File Closed...!!!\n");
    return 0;
}

/*
 ** This function will be called when we read the Device file
 */ 
static ssize_t dummy_read(struct file *filp, 
        char __user *buf, size_t len, loff_t *off)
{
    pr_info("Read function\n");
    asm("int $0x3B");  // Corresponding to irq 11
    return 0;
}

/*
 ** This function will be called when we write the Device file
 */
static ssize_t dummy_write(struct file *filp, 
        const char __user *buf, size_t len, loff_t *off)
{
    pr_info("Write Function\n");
    return len;
}

/*
 ** Module Init function
 */ 
static int __init dummy_driver_init(void)
{
    /* Allocating Major number */
    if ((alloc_chrdev_region(&dev, 0, 1, "dummy_Dev")) < 0) {
        pr_info("Cannot allocate major number\n");
        return -1;
    }
    pr_info("Major = %d Minor = %d \n", MAJOR(dev), MINOR(dev));

    /* Creating cdev structure */
    cdev_init(&dummy_cdev, &fops);

    /* Adding character device to the system */
    if ((cdev_add(&dummy_cdev, dev, 1)) < 0) {
        pr_info("Cannot add the device to the system\n");
        goto r_class;
    }

    /* Creating struct class */
    if ((dev_class = class_create(THIS_MODULE, "dummy_class")) == NULL) {
        pr_info("Cannot create the struct class\n");
        goto r_class;
    }

    /* Creating device */
    if ((device_create(dev_class, NULL, dev, NULL, "dummy_device")) == NULL) {
        pr_info("Cannot create the Device 1\n");
        goto r_device;
    }

    /* Creating a directory in /sys/kernel/ */
    kobj_ref = kobject_create_and_add("dummy_sysfs", kernel_kobj);

    /* Creating sysfs file for dummy_value */
    if (sysfs_create_file(kobj_ref, &dummy_attr.attr)) {
        pr_err("Cannot create sysfs file......\n");
        goto r_sysfs;
    }
    if (request_irq(IRQ_NO, irq_handler, IRQF_SHARED, 
        "dummy_device", (void *)(irq_handler))) {
        pr_info("my_device: cannot register IRQ ");
        goto irq;
    }

    pr_info("Device Driver Insert...Done!!!\n");
    return 0;

irq:
    free_irq(IRQ_NO,(void *)(irq_handler));

r_sysfs:
    kobject_put(kobj_ref); 
    sysfs_remove_file(kernel_kobj, &dummy_attr.attr);

r_device:
    class_destroy(dev_class);
r_class:
    unregister_chrdev_region(dev, 1);
    cdev_del(&dummy_cdev);   
    return -1;
}

/*
 ** Module exit function
 */  
static void __exit dummy_driver_exit(void)
{
    /* Kill the Tasklet */ 
    tasklet_kill(&tasklet);
    free_irq(IRQ_NO,(void *)(irq_handler));
    kobject_put(kobj_ref); 
    sysfs_remove_file(kernel_kobj, &dummy_attr.attr);
    device_destroy(dev_class,dev);
    class_destroy(dev_class);
    cdev_del(&dummy_cdev);
    unregister_chrdev_region(dev, 1);
    pr_info("Device Driver Remove...Done!!!\n");
}

module_init(dummy_driver_init);
module_exit(dummy_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Neelkanth Reddy <www.neelkanth.13@gmail.com>");
MODULE_DESCRIPTION("A simple device driver - Tasklet Static");
MODULE_VERSION("2:1.0");
