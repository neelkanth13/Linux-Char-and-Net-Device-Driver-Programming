obj-m +=poll_select_epoll_driver.o

KDIR = /lib/modules/$(shell uname -r)/build


all:
	gcc -o poll_app poll_userspace.c
	gcc -o select_app select_userspace.c
	gcc -o epoll_app epoll_userspace.c
	make -C $(KDIR)  M=$(shell pwd) modules
 
clean:
	make -C $(KDIR)  M=$(shell pwd) clean
