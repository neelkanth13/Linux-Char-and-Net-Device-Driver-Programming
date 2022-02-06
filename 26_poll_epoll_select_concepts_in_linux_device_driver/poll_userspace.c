#include <assert.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main()
{
    char kernel_val[20];
    int fd, ret;
    struct pollfd pfd;

    fd = open("/dev/dummy_device", O_RDWR | O_NONBLOCK);

    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    pfd.fd = fd;
    pfd.events = (POLLIN | POLLRDNORM | POLLOUT | POLLWRNORM);

    /*
     * What is a poll and its use? The poll allows a process to determine 
     * whether it can read from or write to one or more open files without 
     * blocking. The poll is used in applications that must use multiple input 
     * or output streams without blocking any one of them. 
     *
     * If you want to use the poll in the application, the driver must support 
     * that poll. 
     * The poll will monitor multiple file descriptors or single file descriptors 
     * with multiple events.
     *
     * What are the events available? 
     * There are multiple events available. 
     *
     * POLLIN  – This bit must be set, if the device can be read without blocking. 
     * POLLOUT – This bit is set in the return value, if the device can be written 
     * to without blocking.
     */
    while (1) {
        puts("Starting poll...");
        ret = poll(&pfd, (unsigned long)1, 5000);   //wait for 5secs

        if (ret < 0) {
            perror("poll");
            assert(0);
        }


        if ((pfd.revents & POLLIN)  == POLLIN) {
            read(pfd.fd, &kernel_val, sizeof(kernel_val));
            printf("POLLIN : Kernel_val = %s\n", kernel_val);
        }

        if ((pfd.revents & POLLOUT )  == POLLOUT) {
            strcpy( kernel_val, "User Space");
            write(pfd.fd, &kernel_val, strlen(kernel_val));
            printf("POLLOUT : Kernel_val = %s\n", kernel_val);
        }
    }

    if (close(fd)) {
        perror("Failed to close file descriptor\n");
        return 1;
    }

    return 0;
}
