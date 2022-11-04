/*
 * Building a Basic Network Driver Stub
 *
 * Write a basic network device driver.
 *
 * It should register itself upon loading, and unregister upon
 * removal.
 *
 * Supply minimal open() and stop() methods.
 *
 * You should be able to exercise it with:
 *
 *     insmod network_device_driver.ko
 *     ifconfig neel_netif0 up 192.168.3.197
 *     ifconfig neel_netif0
 *
 * Make sure your chosen address is not being used by anything else.
 *
 * Warning: Depending on kernel version, your stub driver may crash if
 * you try to bring it up or ping it.  If you put in a trivial
 * transmit function, such as
 *
 * static int stub_start_xmit (struct sk_buff *skb, struct net_device *dev)
 * {
 *   dev_kfree_skb (skb);
 *   return 0;
 * }
 * this should avoid the problems.
 *
 */
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/init.h>

static struct net_device *dev;

static int my_open(struct net_device *dev)
{
    pr_info("Hit: my_open(%s)\n", dev->name);

    /* start up the transmission queue */

    netif_start_queue(dev);
    return 0;
}

static int my_close(struct net_device *dev)
{
    pr_info("Hit: my_close(%s)\n", dev->name);

    /* shutdown the transmission queue */

    netif_stop_queue(dev);
    return 0;
}

/* Note this method is only needed on some; without it
   module will fail upon removal or use. At any rate there is a memory
   leak whenever you try to send a packet through in any case*/

static int stub_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
    dev_kfree_skb(skb);
    return 0;
}


static struct net_device_ops ndo = {
    .ndo_open = my_open,
    .ndo_stop = my_close,
    .ndo_start_xmit = stub_start_xmit,
};

static void my_setup(struct net_device *dev)
{
    int j;
    pr_info("my_setup(%s)\n", dev->name);

    /* Fill in the MAC address with a phoney */

    for (j = 0; j < ETH_ALEN; ++j) {
        dev->dev_addr[j] = (char)j;
    }

    ether_setup(dev);
    dev->netdev_ops = &ndo;
}

static int __init my_init(void)
{
    pr_info("Loading stub network module:....");

    /*
     * alloc_netdev allocates the private data area and the net device structure.
     * It also initializes the name field in the net_device structure to the base
     * string for the name, such as neel_netif%d, as done below.
     *
     * Sample example
     * $ ifconfig neel_netif0 <== name of the newly created net device interface.
     *   neel_netif0: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
     *   inet6 fe80::56ef:2353:5d:77db  prefixlen 64  scopeid 0x20<link>
     *   ether 00:01:02:03:04:05  txqueuelen 1000  (Ethernet)
     *   RX packets 0  bytes 0 (0.0 B)
     *   RX errors 0  dropped 0  overruns 0  frame 0
     *   TX packets 0  bytes 0 (0.0 B)
     *   TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0
     */
    dev = alloc_netdev(0, "neel_netif%d", NET_NAME_UNKNOWN, my_setup);
    if (register_netdev(dev)) {
        pr_info(" Failed to register\n");
        free_netdev(dev);
        return -1;
    }
    pr_info("Succeeded in loading %s!\n\n", dev_name(&dev->dev));
    return 0;
}

static void __exit my_exit(void)
{
    pr_info("Unloading stub network module\n\n");
    unregister_netdev(dev);
    free_netdev(dev);
}

module_init(my_init);
module_exit(my_exit);

MODULE_AUTHOR("Neelkanth Reddy");
MODULE_DESCRIPTION("Basic network device driver");
MODULE_LICENSE("GPL v2");

