KERNELDIR := /home/gtq2/linux/origin-linux/linux-imx-rel_imx_4.1.15_2.1.0_ga_alientek
CURRENT_PATH := $(shell pwd)

obj-m := input_dri.o
# mudulename-objs := syncsignal_gtq.c
# EXTAR_CFLAGS=-g o0

build: kernel_modules

kernel_modules:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) modules
	# arm-linux-gnueabihf-gcc asyncnotiApp.c -o asyncnotiApp

clean:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) clean