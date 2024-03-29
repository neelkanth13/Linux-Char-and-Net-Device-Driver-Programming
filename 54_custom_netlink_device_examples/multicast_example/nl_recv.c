#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <linux/netlink.h>

#define MYPROTO NETLINK_USERSOCK
#define MYGRP 17

#define MAX_PAYLOAD 1024  /* maximum payload size*/

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

void read_event(int sock)
{
  struct sockaddr_nl nladdr;
  struct msghdr msg;
  struct iovec iov;
  char buffer[65536];
  int ret;

  iov.iov_base = (void *) buffer;
  iov.iov_len = sizeof(buffer);
  msg.msg_name = (void *) &(nladdr);
  msg.msg_namelen = sizeof(nladdr);
  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;

  printf("Listen for message...\n");
  ret = recvmsg(sock, &msg, 0);
  if (ret < 0)
      return;

  char *payload = NLMSG_DATA((struct nlmsghdr *) &buffer);
  printf("Received from kernel: %s\n", payload);
}

int main(int argc, char **argv)
{
  struct sockaddr_nl src_addr;
  struct sockaddr_nl dest_addr;
  struct nlmsghdr *nlh;
  struct msghdr msg;
  struct iovec iov;
  int sock_fd;
  int rc;

  if (argc != 2) {
    printf("usage: %s <message>\n", argv[0]);
    return 1;
  }

  sock_fd = socket(PF_NETLINK, SOCK_RAW, MYPROTO);
  if (sock_fd < 0) {
    printf("socket(): %s\n", strerror(errno));
    return 1;
  }

  memset(&src_addr, 0, sizeof(src_addr));
  src_addr.nl_family = AF_NETLINK;
  src_addr.nl_pid = getpid();  /* self pid */
  //src_addr.nl_groups = 0;  /* not in mcast groups */
  bind(sock_fd, (struct sockaddr*)&src_addr, sizeof(src_addr));

  memset(&dest_addr, 0, sizeof(dest_addr));
  dest_addr.nl_family = AF_NETLINK;
  dest_addr.nl_pid = 0;   /* For Linux Kernel */

  nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));

  /* Fill the netlink message header */
  nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
  nlh->nlmsg_pid = getpid();  /* self pid */
  nlh->nlmsg_flags = 0;

  /* Fill in the netlink message payload */
  strcpy(NLMSG_DATA(nlh), argv[1]);

  memset(&iov, 0, sizeof(iov));
  iov.iov_base = (void *)nlh;
  iov.iov_len = nlh->nlmsg_len;

  memset(&msg, 0, sizeof(msg));
  msg.msg_name = (void *)&dest_addr;
  msg.msg_namelen = sizeof(dest_addr);
  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;

  int group = MYGRP;
  if (setsockopt(sock_fd, SOL_NETLINK, NETLINK_ADD_MEMBERSHIP, &group, sizeof(group)) < 0) {
    printf("setsockopt(NETLINK_ADD_MEMBERSHIP): %s \n", strerror(errno));
    close(sock_fd);
    return 1;
  }

  printf("Send to kernel: %s\n", argv[1]);

  rc = sendmsg(sock_fd, &msg, 0);
  if (rc < 0) {
    printf("sendmsg(): %s\n", strerror(errno));
    close(sock_fd);
    return 1;
  }

  while (1) {
    read_event(sock_fd);
  }

  close(sock_fd);

  return 0;
}
