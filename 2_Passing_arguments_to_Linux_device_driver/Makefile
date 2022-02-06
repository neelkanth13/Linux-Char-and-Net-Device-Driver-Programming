obj-m += passing_args_to_linux_driver_via_sysfs.o
 
KDIR = /lib/modules/$(shell uname -r)/build
 
all:
	make -C $(KDIR)  M=$(shell pwd) modules
 
clean:
	make -C $(KDIR)  M=$(shell pwd) clean
