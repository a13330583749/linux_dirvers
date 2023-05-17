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
// #include <uapi/linux>

const int BEEFON = 1;
const int BEEFOFF = 0;
const char Name[] = "gtqbeep";

struct beef_device{
    dev_t decid;
    struct cdev beef_cdev;   //字符型设备
    struct class *class;    //什么类型
    struct device *device;  //什么设备
    int major;
    int minot;
    struct device_node *nd;
    int beef_gpio;
};

struct beef_device beef;    //什么蜂鸣器设备

static int beef_open(struct inode *inode, struct file *filp){
    filp->private_data = &beef;
    return 0;
}

static ssize_t beef_read(struct file *filp, char __user *buf, size_t cnt, loff_t *offt)
{
	return 0;
}

static ssize_t beef_write(struct file *filp, const char __user *buf, size_t cnt, loff_t *offt)
{
    int val;
    int datebuf;
    unsigned char statu;
    struct beef_device *dev = filp->private_data;
    val = copy_from_user(&datebuf, buf, cnt);
    if(val < 0){
        printk("wirte fail\r\n");
        return -EFAULT;
    }
    statu = datebuf;
    if(statu == BEEFON){
        printk("opening!\r\n");
        gpio_set_value(dev->beef_gpio, 0);
    }
    else{
        printk("closing\r\n");
        gpio_set_value(dev->beef_gpio, 1);
    }
    return 0;
}

static int beef_release(struct inode *inode, struct file *filp){
    return 0;
}

static struct file_operations beef_opetations = {
	.owner = THIS_MODULE,	
	.open = beef_open,
	.read = beef_read,
	.write = beef_write,
	.release = beef_release,
};

// static struct beef_fops beef_opetations;



static int __init beef_init(void){
    int ret = 0;
    beef.nd = of_find_node_by_path("/beep");
    if(beef.nd == NULL){
        printk("beep node not find\r\n");
    }else{
        printk("beep node find\r\n");
    }
    beef.beef_gpio = of_get_named_gpio(beef.nd, "beep-gpios", 0);
    if(beef.beef_gpio < 0){
        printk("can't find gpio\r\n");
        return -EINVAL;
    }
    printk("led-gpio num = %d\r\n", beef.beef_gpio);

    ret = gpio_direction_output(beef.beef_gpio, 1);
    beef.major = 0;
    beef.minot = 0;
    if(beef.major){
        beef.decid = MKDEV(beef.major, beef.minot);
        register_chrdev_region(beef.decid, 1, Name);
    }else{
        alloc_chrdev_region(&beef.decid, 0, 1, Name);
        beef.major = MAJOR(beef.decid);
        beef.minot = MINOR(beef.decid);
    }
    printk("beep major=%d,minor=%d\r\n",beef.major, beef.minot);


    //初始化cdev
    beef.beef_cdev.owner = THIS_MODULE;
    cdev_init(&beef.beef_cdev, &beef_opetations);

    cdev_add(&beef.beef_cdev, beef.decid, 1);

    beef.class = class_create(THIS_MODULE, Name);
	if (IS_ERR(beef.class)) {
        printk("class_create fail\r\n");
		return PTR_ERR(beef.class);
	}
    
    beef.device = device_create(beef.class, (struct device *)NULL, beef.decid, NULL, Name);
    if (IS_ERR(beef.device)){
        printk("device_create fail\r\n");
        return PTR_ERR(beef.device);
    }

    return 0;
}

static void __exit beef_exit(void){
    cdev_del(&beef.beef_cdev);
    unregister_chrdev_region(beef.decid, 1);
    device_destroy(beef.class, beef.decid);
    class_destroy(beef.class);
}

module_init(beef_init);
module_exit(beef_exit);
MODULE_LICENSE("GPL");
