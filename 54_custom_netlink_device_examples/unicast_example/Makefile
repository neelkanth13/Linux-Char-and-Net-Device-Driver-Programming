

KERNEL_DIR=/usr/src/kernel-headers-$(shell uname -r)
obj-m += netlink_driver.o
ccflags-y := -std=gnu99 -Wno-declaration-after-statement

all:
	$(CC) nl_userspace_app.c -o nl_userspace_app
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	rm -f nl_userspace_app
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
