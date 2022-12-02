/*
 ************************************************************************** 
 *    Steps to create netlink socket in kernel driver. Read and Write     *
 *    messages to userspace process via netlink socket.                   *
 **************************************************************************
 * 1. In 'module_init', create a netlink socket using                     
 *    "netlink_kernel_create". The ouput of this function returns a 
 *    "netlink socket id".
 *
 * 2. Pass "init_net" structure to "netlink_kernel_create" as 1st 
 *    argument. A struct net contains information about the network 
 *    namespace, a set of network resources available to processes.
 *    Note that there could be multiple network namespaces (i.e. 
 *    multiple instances of the networking stack), but most drivers 
 *    use the init_net namespace. 
 *    File location: "net/core/net_namespace.c"
 *
 * 3. Pass "netlink_kernel_cfg" structure as 3rd argument to 
 *    "netlink_kernel_create". These are the optional kernel netlink 
 *    parameters. 
 *
 * 4. In structure "netlink_kernel_cfg" passed to "netlink_kernel_create"
 *    register a callback function to receive and send messages to 
 *    userspace process over netlink socket.    
 *    "hello_nl_recv_msg"
 *
 **************************************************************************     
 */

/*
 ********************************************************************
 * How to receive messages from userspace process to kernel driver  * 
 * module over netlink socket and reply back to user space process  *
 ********************************************************************
 * 1. "hello_nl_recv_msg" is the registered callback function 
 *    that would receive incoming messages over netlink socket 
 *    from userspace process and can repond back over the same 
 *    netlink socket.
 *
 * 2. The message received by "hello_nl_recv_msg" is via structure
 *    sk_buff. 
 *    [this is "socket buffer" File: "include/linux/skbuff.h"] 
 *
 *    "sk_buff is socket buffer organized in form of double linked
 *    list and has some of the below members."
 *    struct sk_buff {
 *       struct sk_buff * next;
 *       struct sk_buff * prev;
 *       struct sock * sk;
 *       struct skb_timeval tstamp;
 *       struct net_device * dev;
 *       struct net_device * input_dev;
 *    }
 * 
 * 3. 
 *   Netlink messages consist of a byte stream with one or multiple
 *   'nlmsghdr' headers and associated payload.
 *   Extact 'nlmsghdr' from sk_buff and print the incoming message
 *   from user space process.
 *
 * 4. 'nlmsg_new' and 'sk_buff' -- create a new message to be sent to 
 *    userspace process over netlink socket.
 *
 * 5. nlmsg_put --> nlmsg_data --> nlmsg_unicast   
 *
 ********************************************************************     
 */

#include <linux/module.h>
#include <net/sock.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>


#define NETLINK_USER 31
struct sock *netlink_socket = NULL;

static void hello_nl_recv_msg(struct sk_buff *skb) {
    struct nlmsghdr *nlh;
    int pid;
    struct sk_buff *skb_out;
    int msg_size;
    char *msg="Hello..This is neelkanth from kernel driver module...";
    int res;

    printk(KERN_INFO "Entering: %s\n", __FUNCTION__);

    /* Read the message content from 'sk_buff' structure */
    /*
     * Message received from User space process via Net link socket 
     */
    nlh = (struct nlmsghdr*)skb->data;
    /*
     * Print the message received from the user space process over NL socket
     */
    printk(KERN_INFO "Netlink received msg payload(from userspace):%s\n",
            (char*)nlmsg_data(nlh));
    pid = nlh->nlmsg_pid; /*pid of sending process */
    printk("PID of userspace process from which message is received: %d\n", pid);


    /*
     * Message sent from Kernel driver to Userspace process over Netlink socket
     */
    msg_size=strlen(msg);
    /* Create new message using Netlink msg new API */
    skb_out = nlmsg_new(msg_size, 0);
    if (!skb_out) {
        printk(KERN_ERR "Failed to allocate new skb\n");
        return;
    }
    /* write the message content into 'sk_buff' structure */
    nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, msg_size, 0); 
    NETLINK_CB(skb_out).dst_group = 0; /* not in mcast group */
    strncpy(nlmsg_data(nlh), msg, msg_size);
    /* Send a Unicast message to userspace process over netlink socket */
    res = nlmsg_unicast(netlink_socket, skb_out, pid);
    if (res < 0) {
        printk(KERN_INFO "Error while sending bak to user\n");
    }

    return;
}

/*
 * When insmod of the kernel module is done, this function is invoked.
 */
static int __init hello_init(void)
{
    printk("Entering: %s\n",__FUNCTION__);
    /* 
     * This is for 3.6 kernels and above.
     */
    struct netlink_kernel_cfg cfg = {
        .input = hello_nl_recv_msg,
    };

    /* Create netlink socket in the kernel driver */
    netlink_socket = netlink_kernel_create(&init_net, NETLINK_USER, &cfg);
    if (!netlink_socket) {
        printk(KERN_ALERT "Error creating socket.\n");
        return -10;
    }

    return 0;
}

/*
 * When rmmod of kernel module is done, this function is invoked. 
 */
static void __exit hello_exit(void)
{
    printk(KERN_INFO "exiting hello module\n");
    netlink_kernel_release(netlink_socket);

    return;
}

/* insmod */
module_init(hello_init);
/* rmmod  */
module_exit(hello_exit);

MODULE_LICENSE("GPL");
