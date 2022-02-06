/*
 * Working of seqlock
 * When no one is in a critical section then one writer can enter into a 
 * critical section by acquiring its lock. Once it took its lock then the 
 * writer will increment the sequence number by one. Currently, the sequence 
 * number is an odd value. Once done with the writing, again it will increment 
 * the sequence number by one. Now the number is an even value. So, when the 
 * sequence number is an odd value, writing is happening. When the sequence number 
 * is an even value, writing has done. Only one writer thread will be allowed in 
 * the critical section. So other writers will be waiting for the lock.
 * 
 * When the reader wants to read the data, first it will read the sequence number. 
 * If it is an even value, then it will go to a critical section and reads the data. 
 * If it is an odd value (the writer is writing something), the reader will wait for 
 * the writer to finish (the sequence number becomes an even number). The value of the 
 * sequence number while entering into the critical section is called an old sequence number.
 * After reading the data, again it will check the sequence number. If it is equal to 
 * the old sequence number, then everything is okay. Otherwise, it will repeat step 2 
 * again. In this case, readers simply retry (using a loop) until they read the same 
 * even sequence number before and after. The reader never blocks, but it may have to 
 * retry if a write is in progress.

 * When only the reader is reading the data and no writer is in the critical section, 
 * any time one writer can enter into a critical section by taking lock without blocking. 
 * This means the writer cannot be blocked for the reader and the reader has to re-read 
 * the data when the writer is writing. This means seqlock is giving importance to a 
 * writer, not the reader (the reader may have to wait but not the writer).

 * When we have to use seqlock
 * We cannot use this seqlock in any situations like normal spinlock or mutex. 
 * Because this will not be effective in such situations other than the situations 
 * mentioned below.

 * where read operations are more frequent than write.
 * where write access is rare but must be fast.
 * That data is simple (no pointers) that needs to be protected. Seqlocks 
 * generally cannot be used to protect data structures involving pointers, 
 * because the reader may be following a pointer that is invalid while the 
 * writer is changing the data structure.
 *
 */
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
#include <linux/seqlock.h>

//Seqlock variable
seqlock_t dummy_seq_lock;

unsigned long dummy_global_variable = 0;
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

// Thread used for writing
int thread_function1(void *pv)
{
    while (!kthread_should_stop()) {  
        write_seqlock(&dummy_seq_lock);
        dummy_global_variable++;
        pr_info("In Thread Function1 : Write value %lu\n", 
                dummy_global_variable);
        write_sequnlock(&dummy_seq_lock);
        msleep(1000);
    }
    return 0;
}

// Thread used for reading
int thread_function2(void *pv)
{
    unsigned int seq_no;
    unsigned long read_value;
    while (!kthread_should_stop()) {
        do {
            seq_no = read_seqbegin(&dummy_seq_lock);
            read_value = dummy_global_variable;
        } while (read_seqretry(&dummy_seq_lock, seq_no));
        pr_info("In Thread Function2 : Read value %lu\n", read_value);
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
    /*Allocating Major number*/
    if((alloc_chrdev_region(&dev, 0, 1, "dummy_Dev")) <0){
        pr_info("Cannot allocate major number\n");
        return -1;
    }
    pr_info("Major = %d Minor = %d \n",MAJOR(dev), MINOR(dev));

    /*Creating cdev structure*/
    cdev_init(&dummy_cdev,&fops);

    /*Adding character device to the system*/
    if((cdev_add(&dummy_cdev,dev,1)) < 0){
        pr_info("Cannot add the device to the system\n");
        goto r_class;
    }

    /*Creating struct class*/
    if((dev_class = class_create(THIS_MODULE,"dummy_class")) == NULL){
        pr_info("Cannot create the struct class\n");
        goto r_class;
    }

    /*Creating device*/
    if((device_create(dev_class,NULL,dev,NULL,"dummy_device")) == NULL){
        pr_info("Cannot create the Device \n");
        goto r_device;
    }


    /* Creating Thread 1 */
    dummy_thread1 = kthread_run(thread_function1,NULL,"dummy Thread1");
    if(dummy_thread1) {
        pr_err("Kthread1 Created Successfully...\n");
    } else {
        pr_err("Cannot create kthread1\n");
        goto r_device;
    }

    /* Creating Thread 2 */
    dummy_thread2 = kthread_run(thread_function2,NULL,"dummy Thread2");
    if(dummy_thread2) {
        pr_err("Kthread2 Created Successfully...\n");
    } else {
        pr_err("Cannot create kthread2\n");
        goto r_device;
    }

    // Initialize the seqlock
    seqlock_init(&dummy_seq_lock);

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
MODULE_DESCRIPTION("A simple device driver - Seqlock");
MODULE_VERSION("2:1.0");
