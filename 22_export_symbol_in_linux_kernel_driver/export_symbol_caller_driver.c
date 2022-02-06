#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>

dev_t dev = 0;
static struct class *dev_class;
static struct cdev dummy_cdev;

static int __init dummy_driver_init(void);
static void __exit dummy_driver_exit(void);

/*************** Driver Functions **********************/
static int dummy_open(struct inode *inode, struct file *file);
static int dummy_release(struct inode *inode, struct file *file);
static ssize_t dummy_read(struct file *filp, char __user *buf, size_t len,loff_t * off);
static ssize_t dummy_write(struct file *filp, const char *buf, size_t len, loff_t * off);

/* 
 * The below symbols (variable and function) are exported from 
 * export_symbol_definition_driver.ko driver and definitions 
 * are in export_symbol_definition_driver.c file.  
 */
extern int dummy_count;
void dummy_shared_func(void); //Function declaration is by defalut extern

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
    dummy_shared_func();
    pr_info("%d time(s) shared function called!\n", dummy_count);
    pr_info("Data Read : Done!\n");
    return 0;
}

/*
 ** This function will be called when we write the Device file
 */
static ssize_t dummy_write(struct file *filp, 
        const char __user *buf, size_t len, loff_t *off)
{
    pr_info("Data Write : Done!\n");
    return len;
}

/*
 ** Module Init function
 */
static int __init dummy_driver_init(void)
{
    /*Allocating Major number*/
    if((alloc_chrdev_region(&dev, 0, 1, "dummy_Dev2")) <0){
        pr_err("Cannot allocate major number\n");
        return -1;
    }
    pr_info("Major = %d Minor = %d \n",MAJOR(dev), MINOR(dev));

    /*Creating cdev structure*/
    cdev_init(&dummy_cdev,&fops);

    /*Adding character device to the system*/
    if((cdev_add(&dummy_cdev,dev,1)) < 0){
        pr_err("Cannot add the device to the system\n");
        goto r_class;
    }

    /*Creating struct class*/
    if((dev_class = class_create(THIS_MODULE,"dummy_class2")) == NULL){
        pr_err("Cannot create the struct class\n");
        goto r_class;
    }

    /*Creating device*/
    if((device_create(dev_class,NULL,dev,NULL,"dummy_device2")) == NULL){
        pr_err("Cannot create the Device 1\n");
        goto r_device;
    }
    pr_info("Device Driver 2 Insert...Done!!!\n");
    return 0;

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
    device_destroy(dev_class,dev);
    class_destroy(dev_class);
    cdev_del(&dummy_cdev);
    unregister_chrdev_region(dev, 1);
    pr_info("Device Driver 2 Remove...Done!!!\n");
}

module_init(dummy_driver_init);
module_exit(dummy_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Neelkanth Reddy <www.neelkanth.13@gmail.com>");
MODULE_DESCRIPTION("EXPORT_SYMBOL Driver - 2");
MODULE_VERSION("2:1.0");
