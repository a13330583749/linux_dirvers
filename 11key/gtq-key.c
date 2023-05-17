#include <linux/module.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/miscdevice.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/mutex.h>
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
#define KEYVALUE 0XF0
#define INVALUE  0X00
static const char Name[] = "Gtq_key";

struct gtq_devices{
    dev_t devid;    //设备号
    struct cdev chrdev;//字符型设备
    struct class *class; //设备中的哪一类
    struct device *gtq_device; //这一类中的什么具体设备
    int major;             //主设备号
    int minor;              //次设备号
    struct device_node *nd; //设备节点
    int device_gpio;        //所使用的GPIO编号
    atomic_t key_numbers;
};

struct gtq_devices gtq_key;

static int key_open(struct inode *inode, struct file *filp)
{
    // int ret = 0;
    filp->private_data = &gtq_key;
    // gpio_request(gtq_key.device_gpio, "key_gpio");
    // ret = key_init();
    return 0;
}


static int key_release(struct inode *inode, struct file *filp)
{
    // struct gtq_devices *dev = filp->private_data;
    printk("closing fd");
	return 0;
}

static ssize_t key_read (struct file* filp, char __user* buf , size_t cnt, loff_t * offt){
    int ret = 0;
    struct gtq_devices *dev = filp->private_data;
    unsigned char value;
    if(gpio_get_value(dev->device_gpio) == 0){
        while(!gpio_get_value(dev->device_gpio)){
            atomic_set(&dev->key_numbers, INVALUE);
        }
    }else{
        atomic_set(&dev->key_numbers, KEYVALUE);
    }
    value = atomic_read(&dev->key_numbers);
    ret = copy_to_user(buf, &value, sizeof value);
    return 0;
}

static struct file_operations devise_fd = {
    .owner = THIS_MODULE,
    .open  = key_open,
    .read  = key_read,
    .release = key_release,
};


static int __init key_init(void)
{
    int ret = 0;
    gtq_key.nd = of_find_node_by_path("/key");
    if(gtq_key.nd == NULL){
        printk("ERROR FIND NODE\r\n");
        return -EINVAL;
    }else
        printk("find devise node!\r\n");
    //获取设备树中的gpio属性，得到设备所使用的设备编号
    gtq_key.device_gpio = of_get_named_gpio(gtq_key.nd, "key-gpios", 0);
    if(gtq_key.device_gpio < 0) {
		printk("can't get beep-gpio");
		return -EINVAL;
	}
    printk("devise-gpio num = %d\r\n", gtq_key.device_gpio);
    //设置GPIO的模式
    gpio_request(gtq_key.device_gpio, "key0");
    ret = gpio_direction_input(gtq_key.device_gpio);

    //注册字符设备驱动
    if(gtq_key.major){
        gtq_key.devid = MKDEV(gtq_key.major, gtq_key.minor);
        register_chrdev_region(gtq_key.devid, 1, Name);
    }else{
        alloc_chrdev_region(&gtq_key.devid, 0, 1, Name);
        gtq_key.major = MAJOR(gtq_key.devid);
        gtq_key.minor = MINOR(gtq_key.devid);
    }
    printk("gtq_devices major=%d,minor=%d\r\n",gtq_key.major, gtq_key.minor);

    gtq_key.chrdev.owner = THIS_MODULE;
    cdev_init(&gtq_key.chrdev, &devise_fd);
    cdev_add(&gtq_key.chrdev, gtq_key.devid, 1);

    gtq_key.class = class_create(THIS_MODULE, Name);
    if(IS_ERR(gtq_key.class)){
        return PTR_ERR(gtq_key.class);
    }

    gtq_key.gtq_device = device_create(gtq_key.class, NULL, gtq_key.devid, NULL, Name);
    if (IS_ERR(gtq_key.gtq_device)) {
		return PTR_ERR(gtq_key.gtq_device);
	}

    return 0;
}
static void __exit key_exit(void)
{
    cdev_del(&gtq_key.chrdev);
    unregister_chrdev_region(gtq_key.devid, 1);
    device_destroy(gtq_key.class, gtq_key.devid);
    class_destroy(gtq_key.class);
}

module_init(key_init);
module_exit(key_exit);
MODULE_LICENSE("GPL");