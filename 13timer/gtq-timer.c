#include <linux/module.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/miscdevice.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/stat.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/tty.h>
#include <linux/kmod.h>
#include <linux/gfp.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <asm/io.h>
#include <linux/timer.h>
#include <linux/jiffies.h>

#define KEYVALUE 0XF0
#define INVALUE  0X00
//ioctl的定义规则
#ifdef 0
#define _IO()
#endif

static const char Name[] = "Gtq_timer";

struct gtq_devices{
    dev_t devid;    //设备号
    struct cdev chrdev;//字符型设备
    struct class *class; //设备中的哪一类
    struct device *gtq_device; //这一类中的什么具体设备
    int major;             //主设备号
    int minor;              //次设备号
    struct device_node *nd; //设备节点
    int device_gpio;        //所使用的GPIO编号
    int timerperiod;        //定时周期,单位为ms
    struct timer_list timer;
};

struct gtq_devices gtq_timer;

static int key_open(struct inode *inode, struct file *filp)
{
    int ret = 0;
    filp->private_data = &gtq_timer;
    gtq_timer.timerperiod = 1000;
    ret = led_init(&gtq_timer);
    if(ret != 0){
        printk("led_init fail\r\n");
    }else   
        printk("initing success!\r\n");
    return 0;
}


static int key_release(struct inode *inode, struct file *filp)
{
    // struct gtq_devices *dev = filp->private_data;
    printk("closing fd");
	return 0;
}

static ssize_t key_read (struct file* filp, char __user* buf , size_t cnt, loff_t * offt){

    return 0;
}
long device_unlocked_ioctl(struct file *filp, unsigned int cmd, unsigned long arg){
    int ret = 0;
    switch(cmd)
}
static struct file_operations devise_fd = {
    .owner = THIS_MODULE,
    .open  = key_open,
    .read  = key_read,
    .release = key_release,
    .unlocked_ioctl = device_unlocked_ioctl,
};

static int led_init(struct  gtq_devices* dev){
    int ret = 0;
    gtq_timer.nd = of_find_node_by_path("/gpioled");
    if(gtq_timer.nd == NULL){
        printk("ERROR FIND NODE\r\n");
        return -EINVAL;
    }else
        printk("find devise node!\r\n");
    //获取设备树中的gpio属性，得到设备所使用的设备编号
    gtq_timer.device_gpio = of_get_named_gpio(gtq_timer.nd, "led-gpios", 0);
    if(gtq_timer.device_gpio < 0) {
		printk("can't get beep-gpio");
		return -EINVAL;
	}
    printk("devise-gpio num = %d\r\n", gtq_timer.device_gpio);
    //设置GPIO的模式
    gpio_request(gtq_timer.device_gpio, "key0");
    ret = gpio_direction_output(gtq_timer.device_gpio, 1);//默认高电平，关灯
    return 0;
}

int timer_init_function(struct gtq_devices* dev){

}


static int __init key_init(void)
{
    init_timer(&gtq_timer.timer);
    gtq_timer.timer.function = ;
    add_timer(&gtq_timer.timer);

    //注册字符设备驱动
    if(gtq_timer.major){
        gtq_timer.devid = MKDEV(gtq_timer.major, gtq_timer.minor);
        register_chrdev_region(gtq_timer.devid, 1, Name);
    }else{
        alloc_chrdev_region(&gtq_timer.devid, 0, 1, Name);
        gtq_timer.major = MAJOR(gtq_timer.devid);
        gtq_timer.minor = MINOR(gtq_timer.devid);
    }
    printk("gtq_devices major=%d,minor=%d\r\n",gtq_timer.major, gtq_timer.minor);

    gtq_timer.chrdev.owner = THIS_MODULE;
    cdev_init(&gtq_timer.chrdev, &devise_fd);
    cdev_add(&gtq_timer.chrdev, gtq_timer.devid, 1);

    gtq_timer.class = class_create(THIS_MODULE, Name);
    if(IS_ERR(gtq_timer.class)){
        return PTR_ERR(gtq_timer.class);
    }

    gtq_timer.gtq_device = device_create(gtq_timer.class, NULL, gtq_timer.devid, NULL, Name);
    if (IS_ERR(gtq_timer.gtq_device)) {
		return PTR_ERR(gtq_timer.gtq_device);
	}

    return 0;
}
static void __exit key_exit(void)
{
    gpio_set_value(gtq_timer.device_gpio, 1);
    del_timer(&gtq_timer.timer);
    cdev_del(&gtq_timer.chrdev);
    unregister_chrdev_region(gtq_timer.devid, 1);
    device_destroy(gtq_timer.class, gtq_timer.devid);
    class_destroy(gtq_timer.class);

    gpio_free()
}

module_init(key_init);
module_exit(key_exit);
MODULE_LICENSE("GPL");