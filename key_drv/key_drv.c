#include <linux/module.h>
#include <linux/init.h>

static int __init key_drv_init(void)
{

}

static void __exit key_drv_exit(void)
{

}

module_init(key_drv_init);
module_exit(key_drv_exit);
MODULE_LICENSE("GPL");
