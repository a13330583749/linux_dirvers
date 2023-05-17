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

/* 映射后的寄存器虚拟地址指针 */
static void __iomem *IMX6U_CCM_CCGR1;
static void __iomem *SW_MUX_GPIO1_IO03;
static void __iomem *SW_PAD_GPIO1_IO03;
static void __iomem *GPIO1_DR;
static void __iomem *GPIO1_GDIR;

struct leddev_gtq{
    dev_t devid;
    struct cdev led_chardev;
    struct class class;
    struct device *device;
    int major;
    int minor;
};

struct leddev_gtq ledder;

void led0_switch(u8 sta)
{
    u32 val = 0;
    if(sta == LED_ON){
        val = readl(GPIO1_DR);
        val &= ~(1 << 3);
        writel(val, GPIO1_DR);
    }else if(sta == LED_OFF){
        val = readl(GPIO1_DR);
        val |= (1 << 3);
        writel(val, GPIO1_DR);
    }
}

static int led_open(struct inode *inode, struct file *filp)
{
    filp->private_data = &ledder;
    printk("opening!\r\n");
    return 0;
}

static ssize_t led_write(struct file *filp, const char __user *buf, size_t cnt, loff_t *offt)
{
    int ret;
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
    int i = 0;
    int resize[5];
    u32 val = 0;
    struct resource *ledsource[5]; //返回值是一个指针数组

    printk("led driver and device has matched!\r\n");
    //使用platform_get_resource获得资源
    for(i = 0; i < 5; i++){
        ledsource[i] = platform_get_resource(dev, IORESOURCE_MEM, i);
        if(!ledsource[i]){
            dev_err(&dev->dev, "No MEM resource for always on\r\n");
            return -1;
        }
        printk("get resource of %d\r\n", i);
        resize[i] = resource_size(ledsource[i]);
    }

    /* 初始化LED */
    IMX6U_CCM_CCGR1 = ioremap(ledsource[0]->start, resize[0]);
    SW_MUX_GPIO1_IO03 = ioremap(ledsource[1]->start, resize[1]);
    SW_PAD_GPIO1_IO03 = ioremap(ledsource[2]->start, resize[2]);
    GPIO1_DR = ioremap(ledsource[3]->start, resize[3]);
    GPIO1_GDIR = ioremap(ledsource[4]->start, resize[4]);

    	/* 2、使能GPIO1时钟 */
	val = readl(IMX6U_CCM_CCGR1);
	val &= ~(3 << 26);	/* 清楚以前的设置 */
	val |= (3 << 26);	/* 设置新值 */
	writel(val, IMX6U_CCM_CCGR1);

	/* 3、设置GPIO1_IO03的复用功能，将其复用为
	 *    GPIO1_IO03，最后设置IO属性。
	 */
	writel(5, SW_MUX_GPIO1_IO03);
	
	/*寄存器SW_PAD_GPIO1_IO03设置IO属性
	 *bit 16:0 HYS关闭
	 *bit [15:14]: 00 默认下拉
     *bit [13]: 0 kepper功能
     *bit [12]: 1 pull/keeper使能
     *bit [11]: 0 关闭开路输出
     *bit [7:6]: 10 速度100Mhz
     *bit [5:3]: 110 R0/6驱动能力
     *bit [0]: 0 低转换率
	 */
	writel(0x10B0, SW_PAD_GPIO1_IO03);

	/* 4、设置GPIO1_IO03为输出功能 */
	val = readl(GPIO1_GDIR);
	val &= ~(1 << 3);	/* 清除以前的设置 */
	val |= (1 << 3);	/* 设置为输出 */
	writel(val, GPIO1_GDIR);

	/* 5、默认关闭LED */
	val = readl(GPIO1_DR);
	val |= (1 << 3);	
	writel(val, GPIO1_DR);

	/* 6、注册字符设备驱动 */
    if(ledder.major == 0){
        retvalue = alloc_chrdev_region(&ledder.device, 0, 1, NAME);
        ledder.major = MAJOR(ledder.device);
        ledder.minor = MINOR(ledder.device);
    }else{
        ledder.device = MKDEV(ledder.major, ledder.minor);
	    retvalue = register_chrdev(ledder.device, 1, NAME);
    }
	if(retvalue < 0){
		printk("register chrdev failed!\r\n");
		return -EIO;
	}

    ledder.led_chardev.owner = THIS_MODULE;
    cdev_init(&ledder.led_chardev, &led_fops);//将这两个结构体绑定在一起

    cdev_add()

    return 0;
}

static int led_remove(struct platform_driver *dev)
{
    iounmap(IMX6U_CCM_CCGR1);
    iounmap(SW_MUX_GPIO1_IO03);
    iounmap(SW_PAD_GPIO1_IO03);
    printk("led driver remove\r\n");
    return 0;
}

struct platform_driver led_driver = {
    .probe = led_probes,
    .driver = {
        .name = "gtq-led",
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