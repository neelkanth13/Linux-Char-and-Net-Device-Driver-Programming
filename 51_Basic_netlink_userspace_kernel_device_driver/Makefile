obj-m += netlinkKernel.o
 
KDIR = /lib/modules/$(shell uname -r)/build
 
 
all:
	make -C $(KDIR)  M=$(shell pwd) modules
	gcc -o netlink_userspace_app netlinkUser.c
 
clean:
	make -C $(KDIR)  M=$(shell pwd) clean
	rm netlink_userspace_app
