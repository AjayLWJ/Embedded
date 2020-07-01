#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <asm/uaccess.h>
#include <asm/io.h>

// 物理地址:点亮led灯
#define GPX2_CON 0x11000C40
#define GPX2_SIZE 8

// 存放虚拟地址的指针
volatile unsigned long *gpx2conf;
volatile unsigned long *gpx2dat;

// 静态的指定
static unsigned int dev_major = 250;
static struct class *devcls;
static struct device *dev;

static int kernel_val = 555;

ssize_t chr_dev_read(struct file *filp, char __user *buf, size_t count, loff_t *fpos)
{
    printk("-----------%S-----------", __FUNCTION__);
    int ret;

    ret = copy_to_usr(buf, &kernel_val, count);
    if (ret > 0)
    {
        printk("copy_to_usr error\n");
        return -EFAULT;
    }
    return 0;
}

ssize_t chr_dev_write(struct file *filp, char __user *buf, size_t count, loff_t *fpos)
{
    // printk("-----------%S-----------", __FUNCTION__);
    int ret;
    int value;

    ret = copy_from_user(&value, buf, count);
    if (ret > 0)
    {
        printk("copy_to_usr error\n");
        return -EFAULT;
    }

    // 实现对灯的亮灭控制
    if(value)
        *gpx2dat |= (1 << 7);
    else
        *gpx2dat &= ~(1 << 7);
    
    // printk("__KERN__: value = %d\n", value);
    return 0;
}

int chr_dev_open(struct inode *inode, struct file *filp)
{
    printk("-----------%S-----------", __FUNCTION__);
    return 0;
}

int chr_dev_close(struct inode *inode, struct file *filp)
{
    printk("-----------%S-----------", __FUNCTION__);
    return 0;
}

const struct file_operations my_fops = {
    .open = chr_dev_open,
    .read = chr_dev_read,
    .write = chr_dev_write,
    .release = chr_dev_close,
};

static int __init chr_dev_init(void)
{
    // 一般都是申请设备号资源
    /* 申请设备号 */
    int ret;

    ret = register_chrdev(dev_major, "chr_dev_test", &my_fops);
    if (ret == 0)
    {
        printk("register ok\n");
    }
    else
    {
        printk("register failed\n");

        return -EINVAL;
    }
    /* 创建一个设备文件 */
    devcls = class_create(THIS_MODULE, "chr_cls");
    // 以下代码是在开发板中的dev文件下创建了chr2文件
    dev = device_create(devcls, NULL, MKDEV(dev_major, 0), NULL, "chr2");

    /* 在驱动中实现文件io的接口,应用程序可以调用文件io */

    // 对物理地址进行虚拟空间的映射
    gpx2conf = ioremap(GPX2_CON, GPX2_SIZE);
    gpx2dat = gpx2conf + 1;

    // 需要配置gpio功能为输出
    *gpx2conf &= ~(0xf << 28);
    *gpx2conf |= (0x1 << 28);

    return 0;
}

static void __exit chr_dev_exit(void)
{
    // 释放虚拟地址
    iounremap(gpx2conf);

    // 一般都是释放设备号资源
    device_destroy(devcls, MKDEV(dev_major, 0));
    class_destroy(devcls);

    unregister_chrdev(dev_major, "chr_dev_test");
}

module_init(chr_dev_init);
module_exit(chr_dev_exit);
MODULE_LICENSE("GPL");