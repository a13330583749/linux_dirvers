#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/semaphore.h>
#include <linux/timer.h>
#include <linux/of_irq.h>
#include <linux/irq.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/mach-types.h>
#include <linux/fcntl.h>
#include <linux/platform_device.h>

#define DEVICE_COUNT 1
#define NAME         "LED_gtq"
#define LED_ON       1
#define LED_OFF      0

struct leddev_gtq{
    dev_t devid;
    struct cdev led_chardev;
    struct class *class;
    struct device *device;
    int major;
    int minor;
    struct device_node* node;
    int led0;                   //led灯GPIO号
};

struct leddev_gtq ledder;

void led0_switch(u32 sta)
{
    if(sta == LED_ON){
        printk("open led!\r\n");
        gpio_set_value(ledder.led0, 0);
    }else if(sta == LED_OFF){
        printk("close led!\r\n");
        gpio_set_value(ledder.led0, 1);
    }else
        printk("invailue!\r\n");
}

static int led_open(struct inode *inode, struct file *filp)
{
    filp->private_data = &ledder;
    printk("opening!\r\n");
    return 0;
}

static ssize_t led_write(struct file *filp, const char __user *buf, size_t cnt, loff_t *offt)
{

    unsigned char led_buf[1];
    if(copy_from_user(led_buf, buf, cnt )){
        printk("copy_from_user error\r\n");
        return -1;
    }
    led0_switch(led_buf[0]);
    return 0;
}

static struct file_operations led_fops = {
    .owner = THIS_MODULE,
    .open = led_open,
    .write = led_write,
};

static int led_probes(struct platform_device *dev)
{
    int retvalue;
    // struct resource *ledsource[5]; //返回值是一个指针数组
	/* 6、注册字符设备驱动 */
    if(ledder.major == 0){
        retvalue = alloc_chrdev_region(&ledder.devid, 0, 1, NAME);
        ledder.major = MAJOR(ledder.devid);
        ledder.minor = MINOR(ledder.devid);
    }else{
        ledder.devid = MKDEV(ledder.major, 0);
	    retvalue = register_chrdev_region(ledder.devid, 1, NAME);
        ledder.minor = 0;
    }
	if(retvalue < 0){
		printk("register chrdev failed!\r\n");
		return -EIO;
	}

    ledder.led_chardev.owner = THIS_MODULE;
    cdev_init(&ledder.led_chardev, &led_fops);//将这两个结构体绑定在一起

    cdev_add(&ledder.led_chardev, ledder.devid, 1);

    ledder.class = class_create(THIS_MODULE, NAME);
    ledder.device = device_create(ledder.class, NULL, ledder.devid, NULL, NAME);

    ledder.node = of_find_node_by_path("/gpioled");
    if(!ledder.node)
        printk("can't find ledder.node\r\n");
    ledder.led0 = of_get_named_gpio(ledder.node, "led-gpios", 0);
    if(ledder.led0 < 0){
        printk("can't find led-gpio\r\n");
        return -EINVAL;
    }

    gpio_request(ledder.led0, "led!!!");
    gpio_direction_output(ledder.led0, 1);

    return 0;
}

static int led_remove(struct platform_device *dev)
{
    gpio_set_value(ledder.led0, 1);
    unregister_chrdev_region(ledder.major, 1);
    device_destroy(ledder.class, ledder.devid);
    class_destroy(ledder.class);
    
    printk("led driver remove\r\n");
    return 0;
}

static const struct of_device_id led_match_list[] = {
    {
        .compatible = "alientek,gpioled",
    }
};

struct platform_driver led_driver = {
    .probe = led_probes,
    .driver = {
        .name = "gtq-led",
        .of_match_table = led_match_list,
    },
    .remove = led_remove,
};

static int __init leddriver_init(void)
{
    return platform_driver_register(&led_driver);
    // return 0;
}

static void __exit leddriver_exit(void)
{
    platform_driver_register(&led_driver);
}
module_init(leddriver_init);
module_exit(leddriver_exit);
MODULE_LICENSE("GPL v2");