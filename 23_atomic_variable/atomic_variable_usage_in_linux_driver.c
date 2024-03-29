#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include<linux/slab.h>                 //kmalloc()
#include<linux/uaccess.h>              //copy_to/from_user()
#include <linux/kthread.h>             //kernel threads
#include <linux/sched.h>               //task_struct 
#include <linux/delay.h>

atomic_t dummy_global_variable = ATOMIC_INIT(0);      //Atomic integer variable
unsigned int etc_bit_check = 0;

dev_t dev = 0;
static struct class *dev_class;
static struct cdev dummy_cdev;

static int __init dummy_driver_init(void);
static void __exit dummy_driver_exit(void);

static struct task_struct *dummy_thread1;
static struct task_struct *dummy_thread2; 

/*************** Driver functions **********************/
static int dummy_open(struct inode *inode, struct file *file);
static int dummy_release(struct inode *inode, struct file *file);
static ssize_t dummy_read(struct file *filp, 
        char __user *buf, size_t len,loff_t * off);
static ssize_t dummy_write(struct file *filp, 
        const char *buf, size_t len, loff_t * off);
/******************************************************/

int thread_function1(void *pv);
int thread_function2(void *pv);

/*
 ** kernel thread function 2
 */
int thread_function1(void *pv)
{
    unsigned int prev_value = 0;

    while(!kthread_should_stop()) {
        atomic_inc(&dummy_global_variable);
        prev_value = test_and_change_bit(1, (void*)&etc_bit_check);
        pr_info("Function1 [value : %u] [bit:%u]\n", atomic_read(&dummy_global_variable), prev_value);
        msleep(1000);
    }
    return 0;
}

/*
 ** kernel thread function 2
 */
int thread_function2(void *pv)
{
    unsigned int prev_value = 0;
    while(!kthread_should_stop()) {
        atomic_inc(&dummy_global_variable);
        prev_value = test_and_change_bit(1,(void*) &etc_bit_check);
        pr_info("Function2 [value : %u] [bit:%u]\n", atomic_read(&dummy_global_variable), prev_value);
        msleep(1000);
    }
    return 0;
}

//File operation structure 
static struct file_operations fops =
{
    .owner          = THIS_MODULE,
    .read           = dummy_read,
    .write          = dummy_write,
    .open           = dummy_open,
    .release        = dummy_release,
};

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
    pr_info("Major = %d Minor = %d \n",MAJOR(dev), MINOR(dev));

    /* Creating cdev structure */
    cdev_init(&dummy_cdev,&fops);

    /* Adding character device to the system */
    if ((cdev_add(&dummy_cdev,dev,1)) < 0) {
        pr_info("Cannot add the device to the system\n");
        goto r_class;
    }

    /* Creating struct class */
    if ((dev_class = class_create(THIS_MODULE,"dummy_class")) == 
         NULL) {
        pr_info("Cannot create the struct class\n");
        goto r_class;
    }

    /* Creating device */
    if ((device_create(dev_class,NULL,dev,NULL,"dummy_device")) == 
         NULL){
        pr_info("Cannot create the Device \n");
        goto r_device;
    }


    /* Creating Thread 1 */
    dummy_thread1 = kthread_run(thread_function1, NULL, "dummy Thread1");
    if (dummy_thread1) {
        pr_err("Kthread1 Created Successfully...\n");
    } else {
        pr_err("Cannot create kthread1\n");
        goto r_device;
    }

    /* Creating Thread 2 */
    dummy_thread2 = kthread_run(thread_function2, NULL, "dummy Thread2");
    if (dummy_thread2) {
        pr_err("Kthread2 Created Successfully...\n");
    } else {
        pr_err("Cannot create kthread2\n");
        goto r_device;
    }

    pr_info("Device Driver Insert...Done!!!\n");
    return 0;


r_device:
    class_destroy(dev_class);
r_class:
    unregister_chrdev_region(dev,1);
    cdev_del(&dummy_cdev);
    return -1;
}

/*
 ** Module exit function
 */ 
static void __exit dummy_driver_exit(void)
{
    kthread_stop(dummy_thread1);
    kthread_stop(dummy_thread2);
    device_destroy(dev_class,dev);
    class_destroy(dev_class);
    cdev_del(&dummy_cdev);
    unregister_chrdev_region(dev, 1);
    pr_info("Device Driver Remove...Done!!\n");
}

module_init(dummy_driver_init);
module_exit(dummy_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Neelkanth Reddy <www.neelkanth.13@gmail.com>");
MODULE_DESCRIPTION("A simple device driver - Atomic Variables");
MODULE_VERSION("2:1.0");
