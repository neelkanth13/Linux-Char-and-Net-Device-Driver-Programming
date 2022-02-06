obj-m += export_symbol_definition_driver.o
obj-m += export_symbol_caller_driver.o
 
KDIR = /lib/modules/$(shell uname -r)/build
 
 
all:
	make -C $(KDIR)  M=$(shell pwd) modules
 
clean:
	make -C $(KDIR)  M=$(shell pwd) clean
