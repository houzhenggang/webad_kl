#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = KBUILD_MODNAME,
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
 .arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x14522340, "module_layout" },
	{ 0x806e575f, "kmem_cache_destroy" },
	{ 0x405c1144, "get_seconds" },
	{ 0xd691cba2, "malloc_sizes" },
	{ 0x2124474, "ip_send_check" },
	{ 0xac63b351, "inet_proto_csum_replace4" },
	{ 0xa62174e2, "textsearch_find_continuous" },
	{ 0xaa87dcca, "textsearch_prepare" },
	{ 0x3c2c5af5, "sprintf" },
	{ 0xaa1b9b4e, "__pskb_pull_tail" },
	{ 0x8ce3169d, "netlink_kernel_create" },
	{ 0xde0bdcff, "memset" },
	{ 0xea147363, "printk" },
	{ 0x42224298, "sscanf" },
	{ 0x2fa5a500, "memcmp" },
	{ 0xd4defbf4, "netlink_kernel_release" },
	{ 0xb4390f9a, "mcount" },
	{ 0x7329e40d, "kmem_cache_free" },
	{ 0x16305289, "warn_slowpath_null" },
	{ 0x1e6d26a8, "strstr" },
	{ 0x1c740bd6, "init_net" },
	{ 0x5d0ae648, "nf_unregister_hooks" },
	{ 0xee065ced, "kmem_cache_alloc" },
	{ 0x25421969, "__alloc_skb" },
	{ 0x312919, "netlink_broadcast" },
	{ 0xf0fdf6cb, "__stack_chk_fail" },
	{ 0xebbf1dba, "strncasecmp" },
	{ 0xf5f5d2d9, "pskb_expand_head" },
	{ 0x2044fa9e, "kmem_cache_alloc_trace" },
	{ 0xe4a639f8, "kmem_cache_create" },
	{ 0x24b91dd3, "textsearch_destroy" },
	{ 0x37a0cba, "kfree" },
	{ 0x236c8c64, "memcpy" },
	{ 0xf96f18ba, "nf_register_hooks" },
	{ 0xb4713751, "skb_make_writable" },
	{ 0xa3a5be95, "memmove" },
	{ 0xe113bbbc, "csum_partial" },
	{ 0x207b7e2c, "skb_put" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "5678DD61F12FC0445CCAD78");

static const struct rheldata _rheldata __used
__attribute__((section(".rheldata"))) = {
	.rhel_major = 6,
	.rhel_minor = 5,
};
