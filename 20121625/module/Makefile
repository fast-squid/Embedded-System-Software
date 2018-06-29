#Makefile for a basic kernel module

obj-m   := dev_driver.o

KDIR    :=/work/achroimx_kernel
PWD     :=$(shell pwd)

all: driver
#all: driver 

driver:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules ARCH=arm

#app:
#	arm-none-linux-gnueabi-gcc -static -o dev_driver dev_driver.c


install:
	cp -a dev_driver.ko /nfsroot
	cp -a dev_driver /nfsroot
clean:
	rm -rf *.ko
	rm -rf *.mod.*
	rm -rf *.o
	rm -rf fpga_test_led
	rm -rf Module.symvers
	rm -rf modules.order
	rm -rf .led*
	rm -rf .tmp*
