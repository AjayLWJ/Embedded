#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <asm/io.h>


// 设计一个类型,描述一个设备的信息
struct led_desc{
    unsigned int dev_major;     // 设备号
    static struct class *devcls;
    static struct device *dev;  // 创建设备文件
    void *reg_virt_base;
}

// 物理地址:点亮led灯
#define GPX2_CON 0x11000C40
#define GPX2_SIZE 8

struct led_desc *led_dev;       // (声明)表示一个全局的设备对象,用于全局共享

static int kernel_val = 555;

ssize_t led_dev_read(struct file *filp, char __user *buf, size_t count, loff_t *fpos)
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

ssize_t led_dev_write(struct file *filp, char __user *buf, size_t count, loff_t *fpos)
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
        write(readl(led_dev->reg_virt_base + 4) | (1 << 7), led_dev->reg_virt_base + 4);
    else
        write(readl(led_dev->reg_virt_base + 4) & ~(1 << 7), led_dev->reg_virt_base + 4);
    
    // printk("__KERN__: value = %d\n", value);
    return 0;
}

int led_dev_open(struct inode *inode, struct file *filp)
{
    printk("-----------%S-----------", __FUNCTION__);
    return 0;
}

int led_dev_close(struct inode *inode, struct file *filp)
{
    printk("-----------%S-----------", __FUNCTION__);
    return 0;
}

const struct file_operations my_fops = {
    .open = led_dev_open,
    .read = led_dev_read,
    .write = led_dev_write,
    .release = led_dev_close,
};

static int __init led_dev_init(void)
{
    int ret;

    // 0, 实例化全局的设备对象--分配空间
    // GFP_KERNEL 如果当前内存不够用的时候,该函数会一直阻塞(休眠)
    // #include <linux/slab.h>
    led_dev = kmalloc(sizeof(struct led_desc), GFP_KERNEL);
    if(led_dev == NULL)
    {
        printk(KERN_ERR "malloc error\n");
        return -ENOMEM;
    }

    // 一般都是申请设备号资源
    /* 申请主设备号--内核中用于区和管理不同字符设备 */
    // 动态分配主设备号,只要register_chrdev中的第一个参数为0即可
    led_dev->dev_major = register_chrdev(0, "led_dev_test", &my_fops);
    if (led_dev->dev_major < 0)
    {
        printk(KERN_ERR "register_chrdev error\n");
        ret = -ENODEV;
        goto err_0;
    }
    
    /* 创建一个设备节点文件--为用户提供一个可操作的文件接口--open() */
    led_dev->devcls = class_create(THIS_MODULE, "led_cls");
    if(IS_ERR(led_dev->devcls))
    {
        printk(KERN_ERR "class_create error\n");
        ret = PTR_ERR(led_dev->devcls);         //将指针出错的具体原因转换成一个出错码
        goto err_1;
    }
    // 以下代码是在开发板中的dev文件下创建了led2文件
    led_dev->dev = device_create(led_dev->devcls, NULL, 
                    MKDEV(led_dev->dev_major, 0), NULL, "led%d", 0);
    if(IS_ERR(led_dev->dev))
    {
        printk(KERN_ERR "device_create error\n");
        ret = PTR_ERR(led_dev->dev);         //将指针出错的具体原因转换成一个出错码
        goto err_2;
    }                

    /* 在驱动中实现文件io的接口,应用程序可以调用文件io */

    /* 硬件的初始化-- 1.地址的映射  2.中断的申请    3.实现硬件的寄存器的初始化 */
    // 对物理地址进行虚拟空间的映射
    led_dev->reg_virt_base = ioremap(GPX2_CON, GPX2_SIZE);
    if(led_dev->reg_virt_base = NULL)
    {
        printk(KERN_ERR "ioremap error\n");
        ret = -ENOMEM;
        goto err_3;
    }
    
    // 需要配置gpio功能为输出
    u32 value = readl(led_dev->reg_virt_base);
    value &= ~(0xf << 28);
    value |= (0x1 << 28);
    write(value, led_dev->reg_virt_base);

    
    return 0;

err_3:
    device_destroy(led_dev->devcls,MKDEV(led_dev->dev_major, 0));

err_2:
    class_destroy(led_dev->devcls);

err_1:
    unregister_chrdev(led_dev->dev_major, "led_dev_test");

err_0:
    kfree(led_dev);
    return ret;
}

static void __exit led_dev_exit(void)
{
    // 释放虚拟地址
    iounremap(led_dev->reg_virt_base);

    // 一般都是释放设备号资源
    device_destroy(led_dev->devcls,MKDEV(led_dev->dev_major, 0));
    class_destroy(led_dev->devcls);
    unregister_chrdev(led_dev->dev_major, "led_dev_test");
    kfree(led_dev);
}

module_init(led_dev_init);
module_exit(led_dev_exit);
MODULE_LICENSE("GPL");