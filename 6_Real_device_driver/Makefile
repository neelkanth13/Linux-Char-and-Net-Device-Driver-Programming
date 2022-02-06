obj-m += char_device_driver.o
 
KDIR = /lib/modules/$(shell uname -r)/build
 
 
all:
	make -C $(KDIR)  M=$(shell pwd) modules
	gcc -o userspace_app userspace_app.c
 
clean:
	make -C $(KDIR)  M=$(shell pwd) clean
