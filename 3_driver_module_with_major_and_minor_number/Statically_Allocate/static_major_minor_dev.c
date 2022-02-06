#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/module.h>
#include <linux/fs.h>

//creating the dev with our custom major and minor number
dev_t dev = MKDEV(300, 0);

/*
** Module Init function
*/
static int __init hello_world_init(void)
{
    register_chrdev_region(dev, 1, "statically_allocated_major_minor_dev");
    pr_info("Major = %d Minor = %d \n",MAJOR(dev), MINOR(dev));
    pr_info("Kernel Module Inserted Successfully...\n");
    return 0;
}

/*
** Module exit function
*/
static void __exit hello_world_exit(void)
{
    unregister_chrdev_region(dev, 1);
    pr_info("Kernel Module Removed Successfully...\n");
}
 
module_init(hello_world_init);
module_exit(hello_world_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Neelkanth Reddy <www.neelkanth.13@gmail.com>");
MODULE_DESCRIPTION("A Driver example code for statically assigned major and minor dev number");
MODULE_VERSION("2:1.0");

