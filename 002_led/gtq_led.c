#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/init.h>
#include <linux/module.h>

#define Major 200 //主设备号
#define Name  "Gtq" //设备名

#define CCM_CCGR1_BASE 				(0x020C406C)
#define SW_MUX_GPIO1_I003_BASE		(0X020E0068)
#define SW_PAD_GPIO1_I003_BASE		(0X020E02F4)
#define GPI01_DR_BASE				(0X0209C000)
#define GPIO1_GDIR_BASE				(0X0209C004)

//使能GPIO模块   ccm：clock control module   使得某个引脚使能
static void __iomem *IMX6U_CCM_CCGR1;

//IO控制寄存器
//MUX_MODE：选择pin的功能    
//PAD     ：是否使用上拉电阻、下拉电阻
static void __iomem *SW_MUX_GPIO1_IO03;
static void __iomem *SW_PAD_GPIO1_IO03;

//输出模式把值写在该引脚，让该引脚输出高低电平
static void __iomem *GPIO1_DR;
//让这个引脚处于输入模式还是输出模式
static void __iomem *GPIO1_GDIR;

char buf = '1';

int led_open (struct inode * inode_, struct file * file_){
	return 0;
}

// @__
ssize_t led_write (struct file *file_, const char __user *buf, size_t n, loff_t * loff_t_){
	return 0;
}

int led_release (struct inode * inode_, struct file * file_){
	return 0;
}


static const struct file_operations led_fops = {
        .owner = THIS_MODULE,
        .open = led_open,
        .write = led_write,
        .release = led_release,
};

static int __init led_init(void){
	int retvalue = 0;
	/* GPIO初始化，初始化寄存器，地址映射 */



	retvalue = register_chrdev(Major, Name, &led_fops);
	printk("led_init");
	return 0;

}

static void __exit led_exit(void)
{
	unregister_chrdev(Major, Name);
	printk("exit");
}


module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Gtq");
