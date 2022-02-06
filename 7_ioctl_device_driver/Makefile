obj-m += ioctl_device_driver.o
 
KDIR = /lib/modules/$(shell uname -r)/build
 
 
all:
	make -C $(KDIR)  M=$(shell pwd) modules
	gcc -o user_space_ioctl_app user_space_ioctl_app.c
 
clean:
	make -C $(KDIR)  M=$(shell pwd) clean
