#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0x27126e5a, "module_layout" },
	{ 0x594d5640, "netlink_kernel_release" },
	{ 0xd0da656b, "__stack_chk_fail" },
	{ 0x390bd61c, "__netlink_kernel_create" },
	{ 0xb26acfd9, "init_net" },
	{ 0x5676ec82, "netlink_unicast" },
	{ 0x24a360b9, "__nlmsg_put" },
	{ 0xbb65749f, "__alloc_skb" },
	{ 0xc5850110, "printk" },
	{ 0xbdfb6dbb, "__fentry__" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "228E2DD5880B8841FBD5458");
