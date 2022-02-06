#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

/*
 ** This function will be called when we open the Misc device file
 */
static int dummy_misc_open(struct inode *inode, struct file *file)
{
    pr_info("dummy misc device open\n");
    return 0;
}

/*
 ** This function will be called when we close the Misc Device file
 */
static int dummy_misc_close(struct inode *inodep, struct file *filp)
{
    pr_info("dummy misc device close\n");
    return 0;
}

/*
 ** This function will be called when we write the Misc Device file
 */
static ssize_t dummy_misc_write(struct file *file, const char __user *buf,
        size_t len, loff_t *ppos)
{
    pr_info("dummy misc device write\n");

    /* We are not doing anything with this data now */

    return len; 
}

/*
 ** This function will be called when we read the Misc Device file
 */
static ssize_t dummy_misc_read(struct file *filp, char __user *buf,
        size_t count, loff_t *f_pos)
{
    pr_info("dummy misc device read\n");

    return 0;
}

//File operation structure 
static const struct file_operations fops = {
    .owner          = THIS_MODULE,
    .write          = dummy_misc_write,
    .read           = dummy_misc_read,
    .open           = dummy_misc_open,
    .release        = dummy_misc_close,
    .llseek         = no_llseek,
};

//Misc device structure
struct miscdevice dummy_misc_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "simple_dummy_misc",
    .fops = &fops,
};

/*
 ** Misc Init function
 */
static int __init misc_init(void)
{
    int error;

    error = misc_register(&dummy_misc_device);
    if (error) {
        pr_err("misc_register failed!!!\n");
        return error;
    }

    pr_info("misc_register init done!!!\n");
    return 0;
}

/*
 ** Misc exit function
 */
static void __exit misc_exit(void)
{
    misc_deregister(&dummy_misc_device);
    pr_info("misc_register exit done!!!\n");
}

    module_init(misc_init)
module_exit(misc_exit)

    MODULE_LICENSE("GPL");
    MODULE_AUTHOR("Neelkanth Reddy <www.neelkanth.13@gmail.com>");
    MODULE_DESCRIPTION("A simple device driver - Misc Driver");
    MODULE_VERSION("2:1.0");
