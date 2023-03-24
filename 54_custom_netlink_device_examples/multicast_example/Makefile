KERNEL_DIR=/usr/src/kernel-headers-$(shell uname -r)
obj-m += netlink_test.o
ccflags-y := -std=gnu99 -Wno-declaration-after-statement

all:
	$(CC) nl_recv.c -o nl_recv
	$(CC) nl_send.c -o nl_send
	$(CC) nl_recv1.c -o nl_recv1
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	rm -f nl_recv nl_send nl_recv1
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
