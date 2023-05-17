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
#include <linux/irq.h>
#include <linux/of_irq.h>
// #include <linux/irqs.h>
#include <asm/mach/map.h>
#include <linux/interrupt.h>

#define IRQ_COUNT 1
#define YES       0x01
#define NO        0xFF
#define Name      "Interrupt"
/* 按键结构体 */
struct irq_keydesc{
    int gpio;
    int irqnum;            //中断号
    unsigned char value;   //键值
    char name[100];
    irqreturn_t (*handler)(int, void*);
};

struct gtq_devices{
    dev_t devid;        //设备号
    struct cdev chrdev; //字符型设备
    struct class *class; //设备中的哪一类
    struct device *gtq_device; //这一类中的什么具体设备
    int major;             //主设备号
    int minor;              //次设备号
    struct device_node *nd; //设备节点
    int device_gpio;        //所使用的GPIO编号
    int timerperiod;        //定时周期,单位为ms
    struct timer_list timer;
    unsigned char curkeynum;  //当前的按键号
    struct irq_keydesc irqkeydesc[IRQ_COUNT];   //按键结构体
    atomic_t keyvalue;          /* 有效的按键键值 */
    atomic_t releasekey;        /* 标记是否完成一次完成的按键*/
};

struct gtq_devices gtq_irq;
//中断服务函数
//这里是按键0的中断服务函数
// para:@
static irqreturn_t key0_handler(int iqr, void *dev_id){
    struct gtq_devices *dev = (struct gtq_devices *)dev_id;
    dev->curkeynum = 0;
    dev->timer.data = (volatile long)dev_id; //传递给function函数的参数
    mod_timer(&dev->timer, jiffies + msecs_to_jiffies(1000));
    printk("timer happen!\r\n");
    return IRQ_RETVAL(IRQ_HANDLED);
}

static void timer_funcion(unsigned long arg){
    unsigned char value;
    unsigned char num;
    struct irq_keydesc* keydesc;
    struct gtq_devices* dev = (struct gtq_devices*)arg;
    printk("reply interrupt!\r\n!");
    num = dev->curkeynum; //是哪一个按键
    keydesc = &dev->irqkeydesc[num];
    
    value = gpio_get_value(keydesc->gpio);
    if(value == 0){
        //将中断设备的值赋予设备
        atomic_set(&dev->keyvalue, value);  //value按键对应的键值
    }
    else{           
        atomic_set(&dev->keyvalue, 0x80 | value);
        atomic_set(&dev->releasekey, 1);
    }
}


// 按键初始化
static int key_init(struct gtq_devices* dev){
    int ret = 0;
    int i;
    // struct gtq_devices* dev = 
    dev->nd = of_find_node_by_path("/key");
    if(dev->nd == NULL){
        printk("can't find key node!\r\n");
        return -1;
    }
    for(i = 0; i < IRQ_COUNT; i++){
        dev->irqkeydesc[i].gpio = of_get_named_gpio(dev->nd, "key-gpios", i);
        if(dev->irqkeydesc[i].gpio < 0){
            printk("can't find gpio!\r\n");
            return -1;
        }
        memset(dev->irqkeydesc[i].name, 0, sizeof(dev->irqkeydesc[i].name));
        sprintf(dev->irqkeydesc[i].name, "key-%d", i);
        gpio_request(dev->irqkeydesc[i].gpio, dev->irqkeydesc[i].name);
        gpio_direction_input(dev->irqkeydesc[i].gpio);

        //获取中断号
        dev->irqkeydesc[i].irqnum = gpio_to_irq(dev->irqkeydesc[i].gpio);
        dev->irqkeydesc[i].irqnum = irq_of_parse_and_map(dev->nd, i);

        printk("key%d:gpio=%d iqrnum=%d",i,dev->irqkeydesc[i].gpio,
                                    dev->irqkeydesc[i].irqnum);
        printk("hello world!\r\n");
    }

    //按键中断初始化
    dev->irqkeydesc[0].handler = key0_handler;
    dev->irqkeydesc[0].value = YES;
    


    for(i = 0; i < IRQ_COUNT; i++){
        ret = request_irq(dev->irqkeydesc[i].irqnum,
                        dev->irqkeydesc[i].handler, 
                        IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING,
                        dev->irqkeydesc[i].name, dev);
        if(ret < 0){
            printk("irq %d request fail!\r\n",
                dev->irqkeydesc[i].irqnum);
            return -1;
        }else{
            printk("key%d:gpio=%d iqrnum=%d\r\n",i,dev->irqkeydesc[i].gpio,
                            dev->irqkeydesc[i].irqnum);
        }
    }

    init_timer(&dev->timer);
    dev->timer.function = timer_funcion;
    return 0;
}

static int irq_open(struct inode *inode, struct file *filp)
{
    int ret = 0;
    filp->private_data = &gtq_irq;
    gtq_irq.timerperiod = 1000;
    ret = key_init(&gtq_irq);
    if(ret != 0){
        printk("key_init fail\r\n");
    }else   
        printk("initing success!\r\n");
    return 0;
}

static ssize_t irq_read (struct file* filp, char __user* buf , size_t cnt, loff_t * offt){
    int ret = 0;
    unsigned char keyvalue = 0;
    unsigned char releasekey = 0;
    struct gtq_devices* dev = (struct gtq_devices*)filp->private_data;
    keyvalue = atomic_read(&dev->keyvalue);
    releasekey = atomic_read(&dev->releasekey);
    if(releasekey){
        if(keyvalue & 0x80){
            keyvalue &= ~0x80;
            ret = copy_to_user(buf, &keyvalue, sizeof(keyvalue));
        }else{
            printk("failing!, keyvalue = %#x\r\n", releasekey);
            goto data_error;
        }
    }
    return 0;
data_error:
    return -1;
}

static struct file_operations gtq_devices_operation = {
    .owner = THIS_MODULE,
    .read = irq_read,
    .open = irq_open,
};

static int __init gtq_init(void){
    if(gtq_irq.major){
        gtq_irq.devid = MKDEV(gtq_irq.major, 0);
        register_chrdev_region(gtq_irq.devid, IRQ_COUNT, Name);
        printk("register_chrdev_region: major = %d, minor = %d\r\n", gtq_irq.major, gtq_irq.minor);
    }else{
        alloc_chrdev_region(&gtq_irq.devid, 0, IRQ_COUNT, Name);
        gtq_irq.major = MAJOR(gtq_irq.devid);
        gtq_irq.minor = MINOR(gtq_irq.devid);
        printk("alloc_chrdev_region: major = %d, minor = %d\r\n", gtq_irq.major, gtq_irq.minor);
    }
    

    cdev_init(&gtq_irq.chrdev, &gtq_devices_operation);
    cdev_add(&gtq_irq.chrdev, gtq_irq.devid, IRQ_COUNT);

    gtq_irq.class = class_create(THIS_MODULE, Name);
    if(IS_ERR(gtq_irq.class)){
        return PTR_ERR(gtq_irq.class);
    }

    gtq_irq.gtq_device = device_create(gtq_irq.class, NULL, gtq_irq.devid, NULL, Name);
    if(IS_ERR(gtq_irq.gtq_device)){
        printk("device create fail!\r\n");
        goto device_create_fail;
    }else{
        printk("success!\r\n");
    }

    atomic_set(&gtq_irq.releasekey, 0);
    // key_init(&gtq_irq);
    return 0;
device_create_fail:
    class_destroy(gtq_irq.class);

    return -1;
}

static void __exit gtq_exit(void){
    unsigned int i = 0;
    del_timer_sync(&gtq_irq.timer);//删除定时器

    for(i = 0; i < IRQ_COUNT; i++){
        free_irq(gtq_irq.irqkeydesc[i].irqnum, &gtq_irq.devid);
        gpio_free(gtq_irq.irqkeydesc[i].gpio);
    }

    cdev_del(&gtq_irq.chrdev);
    unregister_chrdev_region(gtq_irq.devid, IRQ_COUNT);
    device_destroy(gtq_irq.class, gtq_irq.devid);
    class_destroy(gtq_irq.class);
}

module_init(gtq_init);
module_exit(gtq_exit);
MODULE_LICENSE("GPL");
