#include <net/sock.h>
#include <linux/module.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>

#define MYPROTO NETLINK_USERSOCK
#define MYGRP 17

/*
 * In this project, we have 3 userspace processes and 1 kernel device driver.
All of them are part of the netlink multicast group.
one userspace process nl_send sends message to other 2 userspace processes
and linux kernel driver.

Now, If the three userspace processes are subscribed to the same multicast group
using the same netlink socket and are part of the same multicast group, then
when one of the userspace processes sends a message to the multicast group,
the message will be delivered to all the other subscribed processes via the
kernel driver.

In a multicast group, the kernel driver acts as a message router, forwarding the
messages to all the subscribed processes. When a process sends a message to the
multicast group, the message is sent to the kernel driver, which then delivers the
message to all the subscribed processes.

Therefore, when one of the userspace processes sends a
message to the multicast group, the message will be delivered to the kernel driver,
which will then forward the message to all the subscribed processes, including the
other two userspace processes.
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
  //NETLINK_CB(skb_out).dst_group = MYGRP; /* in multicast group */
  strncpy(nlmsg_data(nlh), msg, msg_size);

  printk(KERN_INFO "netlink_test: Send %s\n", msg);
  res = nlmsg_multicast(nl_sock, skb_out, 0, MYGRP, GFP_KERNEL);

  if (res < 0)
    printk(KERN_INFO "netlink_test: Error while sending skb to user\n");
}

static int __init netlink_test_init(void)
{
  printk(KERN_INFO "netlink_test: Init module\n");

  struct netlink_kernel_cfg cfg = {
    .input = netlink_test_recv_msg,
  };

  nl_sock = netlink_kernel_create(&init_net, MYPROTO, &cfg);
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
