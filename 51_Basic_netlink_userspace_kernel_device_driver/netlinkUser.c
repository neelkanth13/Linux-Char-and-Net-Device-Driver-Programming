#include <sys/socket.h>
#include <linux/netlink.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>


#define NETLINK_USER 31

#define MAX_PAYLOAD 1024 /* maximum payload size*/
struct sockaddr_nl src_addr, dest_addr;
struct nlmsghdr *nlh = NULL;
struct iovec iov;
int sock_fd = 0;
struct msghdr msg;

int main(void)
{
    memset(&src_addr, 0, sizeof(src_addr));
    memset(&dest_addr, 0, sizeof(dest_addr));

    printf("My process ID: %d\n", getpid());

    /* 
     * Create a netlink socket for communicating with kernel
     * driver module.
     */
    sock_fd=socket(PF_NETLINK, SOCK_RAW, NETLINK_USER);
    if (sock_fd < 0) {
        return -1;
    }

    /*
     * Set the parameters of netlink socket for sending unicast
     * message to linux kernel.  
     */
    src_addr.nl_family = AF_NETLINK;
    /* self pid */
    src_addr.nl_pid = getpid();   
    dest_addr.nl_family = AF_NETLINK;
    /* For Linux Kernel, PID will be 0 */
    dest_addr.nl_pid = 0;     
     /* 
      * For unicast message to Linux Kernel driver, 
      * set the below to 0 
      */
    dest_addr.nl_groups = 0; 

    /*  
     * Bind 'src addr' to netlink socket.  
     */
    bind(sock_fd, (struct sockaddr*)&src_addr, sizeof(src_addr));

    /*
     * Build netlink message Header from userspace process
     * to linux kernel driver Module. 
     */
    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
    memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
    nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
    nlh->nlmsg_pid = getpid();
    nlh->nlmsg_flags = 0;

    strcpy(NLMSG_DATA(nlh), "This is neelkanth from userspace");

    #if 0
    Build Final Message header.
     
    struct msghdr {
       void             __user *msg_name;    /* ptr to socket address structure */
       int              msg_namelen;         /* size of socket address structure */
       struct iovec     __user *msg_iov;     /* scatter/gather array */
       __kernel_size_t  msg_iovlen;          /* # elements in msg_iov */
       void             __user *msg_control; /* ancillary data */
       __kernel_size_t  msg_controllen;      /* ancillary data buffer length */
    unsigned int        msg_flags;           /* flags on received message */

    };

    #endif

    iov.iov_base = (void *)nlh;
    iov.iov_len = nlh->nlmsg_len;
    msg.msg_name = (void *)&dest_addr;
    msg.msg_namelen = sizeof(dest_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    /* Send Message to Kernel */
    printf("Sending message to kernel\n");
    sendmsg(sock_fd, &msg, 0);

    /* Read message from kernel */
    printf("Waiting for message from kernel\n");
    recvmsg(sock_fd, &msg, 0);
    printf("Received message payload: %s\n", (char *)NLMSG_DATA(nlh));
    close(sock_fd);

    return 0;
}
