#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/cdev.h>
#include <linux/fs.h>


#define NewChrName "NewChar_Gtq"
/* 寄存器物理地址 */
#define CCM_CCGR1_BASE				(0X020C406C)	
#define SW_MUX_GPIO1_IO03_BASE		(0X020E0068)
#define SW_PAD_GPIO1_IO03_BASE		(0X020E02F4)
#define GPIO1_DR_BASE				(0X0209C000)
#define GPIO1_GDIR_BASE				(0X0209C004)

/* 映射后的寄存器虚拟地址指针 */
static void __iomem *IMX6U_CCM_CCGR1;
static void __iomem *SW_MUX_GPIO1_IO03;
static void __iomem *SW_PAD_GPIO1_IO03;
static void __iomem *GPIO1_DR;
static void __iomem *GPIO1_GDIR;

struct newchrdev{
    struct cdev  led_cdev;
    dev_t devid; //设备号
    int   major; //主设备号
    int   minor; //次设备号
};

static const struct file_operations gtq_operation ={
    .owner = THIS_MODULE;
}

static struct newchrdev devides; //LED设备
// struct cdev test_cdev;    
// static struct file_operations test_fops = {
//     .owner = THIS_MODULE,
// }
// test_cdev.owner = THIS_MODULE;
// cdev_init(&test_cdev, &test_fops);//初始化cdev结构体变量

static int __init newchrdev_init(void){
    int ret = 0;
    /*完成内存映射*/
  	IMX6U_CCM_CCGR1 = ioremap(CCM_CCGR1_BASE, 4);
	SW_MUX_GPIO1_IO03 = ioremap(SW_MUX_GPIO1_IO03_BASE, 4);
  	SW_PAD_GPIO1_IO03 = ioremap(SW_PAD_GPIO1_IO03_BASE, 4);
	GPIO1_DR = ioremap(GPIO1_DR_BASE, 4);
	GPIO1_GDIR = ioremap(GPIO1_GDIR_BASE, 4);

    if(devides.major){                           /* 主设备号存在 */
        devides.devid = MKDEV(devides.major, 0); //MKDEV(主设备号，次设备号)
        ret = register_chrdev_region(devides.devid, 1, NewChrName);
    }else{
        ret = alloc_chrdev_region(&devides.devid, 0, 1, NewChrName);
        devides.major = MAJOR(devides.devid);
        devides.minor = MINOR(devides.minor);
    }
    printk("major = %d, minor = %d\n", devides.major, devides.minor);

// void cdev_init(struct cdev *, const struct file_operations *);
    devides.led_cdev.owner = THIS_MODULE;
    cdev_init(&devides.led_cdev, &gtq_operation);
    cdev_add(&devides.led_cdev, devides.devid, 1);

    //file_operation


    return 0;
} 

static void __exit newchrdev_exit(void){
    cdev_del(&devides.led_cdev);

    unregister_chrdev_region(devides.devid, 1);

    iounmap(IMX6U_CCM_CCGR1);
    iounmap(SW_MUX_GPIO1_IO03);
    iounmap(SW_PAD_GPIO1_IO03);
    iounmap(GPIO1_DR);
    iounmap(GPIO1_GDIR);
}

module_exit(newchrdev_exit);
module_init(newchrdev_init);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Gtq");