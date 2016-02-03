#include "main.h"

static int __init app_init(void)
{
	printk("Loading Module\n");

	if(http_session_init())
	{
		printk("http_session_init failed\n");
	}
		
	if(-1==bm_search_init())
	{
		printk("bm_search_init failed\n");
	}
	if(-1==mnlk_init())
	{
		printk("mnlk_init failed\n");
	}
	if(-1==plug_init())
	{
		printk("plug_init failed\n");
	}
	
	if(-1==capture_init())
	{
		printk("capture_init failed\n");
	}
	
	printk( "Finished Loading: Ready for capture\n");

	return 0;
}

static void __exit app_exit(void)
{
	printk("Module Unloading\n");

	http_session_fini();	
	bm_search_fini();
	mnlk_fini();
	plug_fini();
	capture_fini();

	printk("Unloaded\n");
	return;
}

module_init(app_init);
module_exit(app_exit);

MODULE_DESCRIPTION("app check");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("yj");


