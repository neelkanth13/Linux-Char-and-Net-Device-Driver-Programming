obj-m += static_major_minor_dev.o
 
KDIR = /lib/modules/$(shell uname -r)/build

all:
	make -C $(KDIR)  M=$(shell pwd) modules
 
clean:
	make -C $(KDIR)  M=$(shell pwd) clean
