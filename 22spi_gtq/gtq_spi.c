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
#include <linux/of_gpio.h>
#include <linux/semaphore.h>
#include <linux/timer.h>
#include <linux/i2c.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/spi/spi.h>
#include "icm20608reg.h"

#define COUNT    1
#define NAME     "gtq_spi"

struct icm20608_dev{
    struct cdev cdev;
    struct class* class;
    struct device* pdev;
    struct device_node* pnd;
    int major;
    int minor;
    dev_t devid;//设备号
    void* private_data; //用于存放spi对应的
    int cs_gpio;        //片选的GPIO编号
};
static struct icm20608_dev icm20608_gtq;

static int icm20608_read_regs(struct icm20608_dev* dev, u8 reg, void *buf, int len)
{
    int ret = 0;
    u8 txdata[1];   //数据发送
    u8 *rxdata;     //数据接受
	struct spi_message m;
	struct spi_transfer *t;
	struct spi_device *spi = (struct spi_device *)dev->private_data;

    t = kzalloc(sizeof(struct spi_transfer), GFP_KERNEL);
    if(!t)
        return -ENOMEN;
    
    rxdata = kzalloc(sizeof(char) * len, GFP_KERNEL);
    if(!rxdata)
        goto out1;

    txdata[0] = reg | 0x80;
    t->tx_buf = txdata;
    t->rx_buf = rxdata;
    t->len = len + 1;

    spi_message_init(&m);
    spi_message_add_tail(t, &m);
    ret = spi_sync(spi, &m);
    if(ret)
        goto out2;
    memcpy(buf, rxdata+1, len);
    
    return 0;
out2:
    kfree(rxdata);
out1:
    kfree(t);
    return -1;
}

static s32 icm20608_write_regs(struct icm20608_dev *dev, u8 reg, u8 *buf, u8 len)
{
    int ret = -1;
    
}

void icm20608_gtq_reginit(void)
{
    u8 value = 0;
    icm20608_write_onereg(&icm20608dev, ICM20_PWR_MGMT_1, 0x80);
}

int	icm20608_probe(struct spi_device *spi)
{
    if(icm20608_gtq.major){
        icm20608_gtq.devid = MKDEV(icm20608_gtq.major, 0);
        icm20608_gtq.minor = 0;
        register_chrdev_region(icm20608.devid, COUNT, NAME);
    }else{
        alloc_chrdev_region(&icm20608_gtq.devid, 0, COUNT, NAME);
        icm20608_gtq.major = MAJOR(icm20608_gtq.devid);
        icm20608_gtq.minor = MINOR(icm20608_gtq.minor);
    }

    cdev_init(&icm20608_gtq.cdev, &icm20608_filp);
    cdev_add(&icm20608_gtq.cdev, icm20608_gtq.devid, COUNT);

    icm20608_gtq.class = class_create(THIS_MODULE, NAME);
    icm20608_gtq.pdev = device_create(icm20608_gtq.class, NULL, icm20608_gtq.devid, NULL, NAME);

    spi->mode = SPI_MODE_0;
    spi_setup(spi);
    icm20608_gtq.private_data = spi;

    icm20608_gtq_reginit();//寄存器初始化
    return 0;
}

//用于注销驱动
int	icm20608_remove(struct spi_device *spi)
{
    //删除设备
    cdev_del(&icm20608_gtq.cdev);
    unregister_chrdev_region(icm20608_gtq.devid, COUNT);
    //注销类和设备
    device_destroy(icm20608_gtq.class, icm20608_gtq.devid);
    class_destroy(icm20608_gtq.class);
}

static struct file_operations icm20608_filp = {
    .open = icm20608_open(),
};

//读写寄存器的操作

static struct spi_driver icm20608_driver = {
    .probe = icm20608_probe,
    // .id_table = icm20608_of_match,
    .remove = icm20608_remove,
    .driver = {
        .owner = THIS_MODULE,
        .of_match_table = icm20608_of_match,
        .name = "icm20608",
    },
};

static const struct of_device_id icm20608_of_match[] = {
    {.compatible = "alientek,icm20608"},
    {},
};

static int spi_init(void)
{
    return spi_register_driver(&icm20608_driver);
}

static void spi_exit(void)
{
    spi_unregister_driver(&icm20608_driver);
}

module_init(spi_init);
module_exit(spi_exit);
MODULE_LICENSE("GPL v2");