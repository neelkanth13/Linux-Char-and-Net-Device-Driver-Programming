#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/module.h>
#include<linux/moduleparam.h>
 
int   argv_in_int, 
      arr_argv_in_int[4];
char *argv_in_str;
int   cb_argv_in_int = 0;
 
module_param(argv_in_int, int, S_IRUSR|S_IWUSR);                       // Integer value
module_param(argv_in_str, charp, S_IRUSR|S_IWUSR);                     // String
module_param_array(arr_argv_in_int, int, NULL, S_IRUSR|S_IWUSR);       // Array of integers
 
/*----------------------Module_param_cb()--------------------------------*/
int notify_param(const char *val, const struct kernel_param *kp)
{
        int res = param_set_int(val, kp); // Use helper for write variable
        if(res==0) {
                pr_info("Call back function called...\n");
                pr_info("New value of cb_argv_in_int = %d\n", cb_argv_in_int);
                return 0;
        }
        return -1;
}
 
const struct kernel_param_ops my_param_ops = 
{
        .set = &notify_param, // Use our setter ...
        .get = &param_get_int, // .. and standard getter
};
 
module_param_cb(cb_argv_in_int, &my_param_ops, &cb_argv_in_int, S_IRUGO|S_IWUSR );
/*-------------------------------------------------------------------------*/

/*
** Module init function
*/
static int __init hello_world_init(void)
{
        int i;
        pr_info("Value in int = %d  \n", argv_in_int);
        pr_info("cb_argv_in_int = %d  \n", cb_argv_in_int);
        pr_info("Value in String = %s \n", argv_in_str);
        for (i = 0; i < (sizeof arr_argv_in_int / sizeof (int)); i++) {
                pr_info("Arr_value[%d] = %d\n", i, arr_argv_in_int[i]);
        }
        pr_info("Kernel Module Inserted Successfully...\n");
    return 0;
}

/*
** Module Exit function
*/
static void __exit hello_world_exit(void)
{
    pr_info("Kernel Module Removed Successfully...\n");
}
 
module_init(hello_world_init);
module_exit(hello_world_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Neelkanth Reddy <www.neelkanth.13@gmail.com>");
MODULE_DESCRIPTION("A Driver example code for passing arguments");
MODULE_VERSION("2:1.0");
