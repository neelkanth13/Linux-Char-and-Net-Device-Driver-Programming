#include <net/sock.h> 
#include <linux/module.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>

#define NETLINK_TEST 17

/*
 * The nlmsghdr structure is used to send and receive Netlink messages 
 * between user-space and kernel-space processes in Linux. The Netlink 
 * protocol is used for various purposes such as configuring network 
 * interfaces, managing network devices, and monitoring network events.
 */

/*
 * In Linux networking, skb_buff is a data structure used to 
 * represent network packets as they are processed through the 
 * kernel's network stack.
 *
 * In the context of network transmission (tx), skb_buff is used 
 * to represent the packet as it moves through the various layers 
 * of the network stack, from the application layer down to the 
 * physical layer. As the packet moves through each layer, metadata 
 * is added to the skb_buff to describe the packet's characteristics, 
 * such as the source and destination MAC addresses, IP addresses, 
 * and protocol type. The skb_buff is also used to store the actual 
 * packet data.
 *
 * In the context of network reception (rx), skb_buff is used to 
 * represent the incoming packet as it is received by the network 
 * interface card (NIC) and processed by the kernel's network stack. 
 * The skb_buff is used to store the packet data and associated 
 * metadata, such as the source and destination MAC addresses, 
 * IP addresses, and protocol type. The skb_buff is then passed up 
 * through the network stack to the appropriate protocol layer for 
 * further processing.
 *
 * Overall, skb_buff is an important data structure in Linux networking 
 * that enables efficient processing of network packets as they move 
 * through the kernel's network stack, both in transmission and reception.
 */

struct sock *nl_sock = NULL;

static void netlink_test_recv_msg(struct sk_buff *skb)
{
    struct sk_buff *skb_out;
    struct nlmsghdr *nlh;
    int msg_size;
    char *msg;
    int pid;
    int res;

    nlh = (struct nlmsghdr *)skb->data;
    pid = nlh->nlmsg_pid; /* pid of sending process */
    msg = (char *)nlmsg_data(nlh);
    msg_size = strlen(msg);

    printk(KERN_INFO "netlink_test: Received from pid %d: %s\n", pid, msg);

    // create reply
    skb_out = nlmsg_new(msg_size, 0);
    if (!skb_out) {
      printk(KERN_ERR "netlink_test: Failed to allocate new skb\n");
      return;
    }

    // put received message into reply
    nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, msg_size, 0);
    NETLINK_CB(skb_out).dst_group = 0; /* not in mcast group */
    strncpy(nlmsg_data(nlh), msg, msg_size);

    printk(KERN_INFO "netlink_test: Send %s\n", msg);

    res = nlmsg_unicast(nl_sock, skb_out, pid);
    if (res < 0)
      printk(KERN_INFO "netlink_test: Error while sending skb to user\n");
}

static int __init netlink_test_init(void)
{
  printk(KERN_INFO "netlink_test: Init module\n");

  struct netlink_kernel_cfg cfg = {
    .input = netlink_test_recv_msg,
  };

  nl_sock = netlink_kernel_create(&init_net, NETLINK_TEST, &cfg);
  if (!nl_sock) {
    printk(KERN_ALERT "netlink_test: Error creating socket.\n");
    return -10;
  }

  return 0;
}

static void __exit netlink_test_exit(void)
{
  printk(KERN_INFO "netlink_test: Exit module\n");

  netlink_kernel_release(nl_sock);
}

module_init(netlink_test_init);
module_exit(netlink_test_exit);

MODULE_LICENSE("GPL");
