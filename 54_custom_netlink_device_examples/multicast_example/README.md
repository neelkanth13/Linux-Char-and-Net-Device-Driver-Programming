# Multicast Example
####################################################################################
In this project, we have 3 userspace processes and 1 kernel device driver. 
All of them are part of the netlink multicast group. 
one userspace process nl_send sends message to other 2 userspace processes
and linux kernel driver. 

Now, If the three userspace processes are subscribed to the same multicast group 
using the same netlink socket and are part of the same multicast group, then 
when one of the userspace processes sends a message to the multicast group, 
the message will be delivered to all the other subscribed processes via the 
kernel driver.

In a multicast group, the kernel driver acts as a message router, forwarding the 
messages to all the subscribed processes. When a process sends a message to the 
multicast group, the message is sent to the kernel driver, which then delivers the 
message to all the subscribed processes.

Therefore, when one of the userspace processes sends a 
message to the multicast group, the message will be delivered to the kernel driver, 
which will then forward the message to all the subscribed processes, including the 
other two userspace processes.


Example adapted to newer Linux kernel versions.

Compile kernel module and user space program.

```
make
```

Load kernel module:

```
insmod ./netlink_test.ko
```

Also check kernel log `dmesg` for module debug output.

```
./nl_recv "Hello you!"
Listen for message...
Received from kernel: Hello you!
Listen for message...
```

Execute `./nl_recv` in another console as well and see how the message is send to the kernel and back to all running nl_recv instances. Note: Only root or the kernel can send a message to a multicast group!

Unload kernel module:
```
rmmod netlink_test.ko
```
