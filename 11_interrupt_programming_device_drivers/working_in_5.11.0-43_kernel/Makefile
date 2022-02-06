obj-m += simulate_hw_interrupt_via_sw.o
 
KDIR = /lib/modules/$(shell uname -r)/build
 
 
all:
	make -C $(KDIR)  M=$(shell pwd) modules
 
clean:
	make -C $(KDIR)  M=$(shell pwd) clean
