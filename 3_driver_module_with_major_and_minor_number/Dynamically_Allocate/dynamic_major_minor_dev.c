#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/module.h>
#include<linux/kdev_t.h>
#include<linux/fs.h>
 
dev_t dev = 0;

/*
** Module Init function
*/
static int __init hello_world_init(void)
{
	/*Allocating Major number*/
	if((alloc_chrdev_region(&dev, 0, 1, "dynamic_allocated_dev")) <0){
		pr_info("Cannot allocate major number for device 1\n");
		return -1;
	}
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
MODULE_DESCRIPTION("A Driver example code for passing arguments");
MODULE_VERSION("2:1.0");

