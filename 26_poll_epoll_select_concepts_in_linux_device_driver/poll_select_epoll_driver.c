/*
 *    I/O Multiplexing
 */

/*
 * Poll is one of the  solution is to use a 'kernel mechanism' for polling over 
 * a set of file descriptors. 
 */


/*
 * Driver Concept
 * ===============================================================================
 * When you write data using the sysfs entry (/sys/kernel/dummy_sysfs/dummy_value), 
 * that means data is available in the kernel. So, we have to inform the userspace application 
 * to read the available data (Driver gives the permission to the app for reading).
 * When you read the data using the sysfs entry (/sys/kernel/dummy_sysfs/dummy_value), 
 * that means, data has been read by the kernel, and the userspace app has to write the data 
 * into the kernel space. So that driver can read the data. (Driver gives the permission to the 
 * app for writing).
 * So, whenever the app gets read permission, it can read the data from the driver and 
 * whenever the app gets the write permission, it can write some data into the driver.
 *
 * Application concept
 * =============================================================================
 * The application will open the driver’s device file (/dev/dummy_device). 
 * Then register that descriptor with reading and writing operation using Poll 
 * ( 5 Seconds timeout ). So, every 5 seconds it will exit the poll and again it polls.
 * 
 * If this app gets the read permission from the poll Linux kernel driver, 
 *   then it reads the data.
 * If this app gets the write permission from the poll Linux kernel driver, 
 *  then it writes the data to the driver.
 */


#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>                 //kmalloc()
#include <linux/uaccess.h>              //copy_to/from_user()
#include <linux/kthread.h>
#include <linux/wait.h>                 //Required for the wait queues
#include <linux/poll.h>
#include <linux/sysfs.h> 
#include <linux/kobject.h>

/* Waitqueue */
DECLARE_WAIT_QUEUE_HEAD(wait_queue_dummy_data);

dev_t dev = 0;
static struct class *dev_class;
static struct cdev dummy_cdev;
struct kobject *kobj_ref;

static bool can_write = false;
static bool can_read  = false;
static char dummy_value[20];

/*
 ** Function Prototypes
 */
static int      __init dummy_driver_init(void);
static void     __exit dummy_driver_exit(void);

/*************** Driver functions **********************/
static int      dummy_open(struct inode *inode, struct file *file);
static int      dummy_release(struct inode *inode, struct file *file);
static ssize_t  dummy_read(struct file *filp, char __user *buf, size_t len,loff_t * off);
static ssize_t  dummy_write(struct file *filp, const char *buf, size_t len, loff_t * off);
static unsigned int dummy_poll(struct file *filp, struct poll_table_struct *wait);

/*************** Sysfs functions **********************/
static ssize_t  sysfs_show(struct kobject *kobj, 
        struct kobj_attribute *attr, char *buf);
static ssize_t  sysfs_store(struct kobject *kobj, 
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
    .poll           = dummy_poll
};

/*
 ** This function will be called when we read the sysfs file
 */
static ssize_t sysfs_show(struct kobject *kobj, 
        struct kobj_attribute *attr, 
        char *buf)
{
    pr_info("Sysfs Show - Write Permission Granted!!!\n");

    can_write = true;

    //wake up the waitqueue
    wake_up(&wait_queue_dummy_data);

    return sprintf(buf, "%s", "Success\n");
}

/*
 ** This function will be called when we write the sysfsfs file
 */
static ssize_t sysfs_store(struct kobject *kobj, 
        struct kobj_attribute *attr,
        const char *buf, 
        size_t count)
{
    pr_info("Sysfs Store - Read Permission Granted!!!\n");

    strcpy(dummy_value, buf);

    can_read = true;

    //wake up the waitqueue
    wake_up(&wait_queue_dummy_data);

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
static ssize_t dummy_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
    pr_info("Read Function : dummy_value = %s\n",dummy_value);   

    len = strlen(dummy_value);

    strcpy(buf, dummy_value);

#if 0  
    if( copy_to_user(buf, dummy_value, len) > 0)
    {
        pr_err("ERROR: Not all the bytes have been copied to user\n");
    }
#endif

    return 0;
}

/*
 ** This function will be called when we write the Device file
 */
static ssize_t dummy_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
    strcpy(dummy_value, buf);

    pr_info("Write function : dummy_value = %s\n", dummy_value);

    return len;
}

/*
 ** This function will be called when app calls the poll function
 */
static unsigned int dummy_poll(struct file *filp, struct poll_table_struct *wait)
{
    __poll_t mask = 0;

    /*************************************************************************/
    poll_wait(filp, &wait_queue_dummy_data, wait);
    /*************************************************************************/

    pr_info("Poll function\n");

    if (can_read) {
        can_read = false;
        mask |= ( POLLIN | POLLRDNORM );
    }

    if (can_write) {
        can_write = false;
        mask |= ( POLLOUT | POLLWRNORM );
    }

    return mask;
}

/*
 ** Module Init function
 */
static int __init dummy_driver_init(void)
{
    /* Allocating Major number */
    if ((alloc_chrdev_region(&dev, 0, 1, "dummy_Dev")) < 0) {
        pr_err("Cannot allocate major number\n");
        return -1;
    }
    pr_info("Major = %d Minor = %d \n", MAJOR(dev), MINOR(dev));

    /* Creating cdev structure */
    cdev_init(&dummy_cdev,&fops);
    dummy_cdev.owner = THIS_MODULE;
    dummy_cdev.ops = &fops;

    /* Adding character device to the system */
    if ((cdev_add(&dummy_cdev,dev,1)) < 0) {
        pr_err("Cannot add the device to the system\n");
        goto r_class;
    }

    /* Creating struct class */
    if ((dev_class = class_create(THIS_MODULE, "dummy_class")) ==
        NULL) {
        pr_err("Cannot create the struct class\n");
        goto r_class;
    }

    /* Creating device */
    if ((device_create(dev_class, NULL, dev, NULL, "dummy_device")) == 
        NULL) {
        pr_err("Cannot create the Device 1\n");
        goto r_device;
    }

    /* Creating a directory in /sys/kernel/ */
    kobj_ref = kobject_create_and_add("dummy_sysfs",kernel_kobj);

    /* Creating sysfs file for dummy_value */
    if(sysfs_create_file(kobj_ref,&dummy_attr.attr))
    {
        pr_err("Cannot create sysfs file......\n");
        goto r_sysfs;
    }

    /* Initialize wait queue */
    init_waitqueue_head(&wait_queue_dummy_data);

    pr_info("Device Driver Insert...Done!!!\n");
    return 0;

r_sysfs:
    kobject_put(kobj_ref); 
    sysfs_remove_file(kernel_kobj, &dummy_attr.attr);
r_device:
    class_destroy(dev_class);
r_class:
    unregister_chrdev_region(dev,1);
    return -1;
}

/*
 ** Module exit function
 */ 
static void __exit dummy_driver_exit(void)
{
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
MODULE_DESCRIPTION("Simple linux driver for Poll / Select / E-Poll demonstration)");
MODULE_VERSION("2:1.0");
