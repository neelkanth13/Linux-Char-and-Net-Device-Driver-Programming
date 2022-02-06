obj-m += kernel_driver_interrupt_handler_generate_signal.o
 
KDIR = /lib/modules/$(shell uname -r)/build
 
 
all:
	make -C $(KDIR)  M=$(shell pwd) modules
	gcc -o userspace_app userspace_app_receiving_signal_from_device_drvr.c
 
clean:
	make -C $(KDIR)  M=$(shell pwd) clean
