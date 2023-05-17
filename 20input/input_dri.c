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
#include <linux/input.h>
#include <linux/semaphore.h>
#include <linux/timer.h>
#include <linux/of_irq.h>
#include <linux/irq.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#define NAME        "gtq-input"
struct irq_keydesc {
	int gpio;								/* gpio */
	int irqnum;								/* 中断号     */
	unsigned char value;					/* 按键对应的键值 */
	char name[10];							/* 名字 */
	irqreturn_t (*handler)(int, void *);	/* 中断服务函数 */
};

struct input_devices
{
    struct cdev cdev;
    int major;
    int minor;
    struct class *class;
    struct device *device;
    struct device_node *nd;
    struct irq_keydesc irqkeydesc;	/* 按键描述数组 */
    struct timer_list timer;/* 定义一个定时器*/
    struct input_dev  *inputdev;
};



struct input_devices gtq_input_device;

void timer_function(unsigned long arg)
{
	unsigned char value;

	struct irq_keydesc *keydesc;
	struct input_devices *dev = (struct input_devices *)arg;


	keydesc = &dev->irqkeydesc;
	value = gpio_get_value(keydesc->gpio); 	/* 读取IO值 */
	if(value == 0){ 						/* 按下按键 */
		/* 上报按键值 */
		//input_event(dev->inputdev, EV_KEY, keydesc->value, 1);
		input_report_key(dev->inputdev, keydesc->value, 1);/* 最后一个参数表示按下还是松开，1为按下，0为松开 */
		input_sync(dev->inputdev);
	} else { 									/* 按键松开 */
		//input_event(dev->inputdev, EV_KEY, keydesc->value, 0);
		input_report_key(dev->inputdev, keydesc->value, 0);
		input_sync(dev->inputdev);
	}	
}

static irqreturn_t key0_handler(int irq, void *dev_id)
{
	struct input_devices *dev = (struct input_devices *)dev_id;
	dev->timer.data = (volatile long)dev_id;
	mod_timer(&dev->timer, jiffies + msecs_to_jiffies(10));	/* 10ms定时 */
	return IRQ_RETVAL(IRQ_HANDLED);
}

static int keyio_init(void)
{
	unsigned char i = 0;
	char name[10];
	int ret = 0;
	
	gtq_input_device.nd = of_find_node_by_path("/key");
	if (gtq_input_device.nd== NULL){
		printk("key node not find!\r\n");
		return -EINVAL;
	} 

	/* 提取GPIO */
	
    gtq_input_device.irqkeydesc.gpio = of_get_named_gpio(gtq_input_device.nd ,"key-gpios", i);
    if (gtq_input_device.irqkeydesc.gpio < 0) {
        printk("can't get key%d\r\n", i);
    }
	
	printk("gpio: %d\r\n\r\n", gtq_input_device.irqkeydesc.gpio);
	/* 初始化key所使用的IO，并且设置成中断模式 */

    memset(gtq_input_device.irqkeydesc.name, 0, sizeof(name));	/* 缓冲区清零 */
    sprintf(gtq_input_device.irqkeydesc.name, "KEY");		/* 组合名字 */
    gpio_request(gtq_input_device.irqkeydesc.gpio, name);
    gpio_direction_input(gtq_input_device.irqkeydesc.gpio);	
    gtq_input_device.irqkeydesc.irqnum = irq_of_parse_and_map(gtq_input_device.nd, 0);

	/* 申请中断 */
	gtq_input_device.irqkeydesc.handler = key0_handler;
	gtq_input_device.irqkeydesc.value = KEY_0;
	

    ret = request_irq(gtq_input_device.irqkeydesc.irqnum, gtq_input_device.irqkeydesc.handler, 
                        IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING, gtq_input_device.irqkeydesc.name, &gtq_input_device);
    if(ret < 0){
        printk("irq %d request failed!\r\n", gtq_input_device.irqkeydesc.irqnum);
        return -EFAULT;
    }


	/* 创建定时器 */
	init_timer(&gtq_input_device.timer);
	gtq_input_device.timer.function = timer_function;

	/* 申请input_dev */
	gtq_input_device.inputdev = input_allocate_device();
	gtq_input_device.inputdev->name = NAME;
#if 0
	/* 初始化input_dev，设置产生哪些事件 */
	__set_bit(EV_KEY, gtq_input_device.inputdev->evbit);	/* 设置产生按键事件          */
	__set_bit(EV_REP, gtq_input_device.inputdev->evbit);	/* 重复事件，比如按下去不放开，就会一直输出信息 		 */

	/* 初始化input_dev，设置产生哪些按键 */
	__set_bit(KEY_0, gtq_input_device.inputdev->keybit);	
#endif

#if 0
	gtq_input_device.inputdev->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_REP);
	gtq_input_device.inputdev->keybit[BIT_WORD(KEY_0)] |= BIT_MASK(KEY_0);
#endif

	gtq_input_device.inputdev->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_REP);
	input_set_capability(gtq_input_device.inputdev, EV_KEY, KEY_0);

	/* 注册输入设备 */
	ret = input_register_device(gtq_input_device.inputdev);
	if (ret) {
		printk("register input device failed!\r\n");
		return ret;
	}
	return 0;
}

static int __init input_init(void)
{
    keyio_init(); 
    return 0;
}
static void __exit input_exit(void)
{
	/* 删除定时器 */
	del_timer_sync(&gtq_input_device.timer);	/* 删除定时器 */
		
	/* 释放中断 */

	free_irq(gtq_input_device.irqkeydesc.irqnum, &gtq_input_device);

	/* 释放input_dev */
	input_unregister_device(gtq_input_device.inputdev);
	input_free_device(gtq_input_device.inputdev);
}
module_init(input_init);
module_exit(input_exit);
MODULE_LICENSE("GPL v2");