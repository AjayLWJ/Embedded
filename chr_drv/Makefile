ROOTFS_DIR = /opt/4412/rootfs

ifeq ($(KERNELRELEASE),)

KERNERL_DIR = /home/lwj/linux_4412/kernel/linux-3.14
CUR_DIR = $(shell pwd)

all:
	make -C $(KERNERL_DIR) M=$(CUR_DIR) modules

clean:
	make -C $(KERNERL_DIR) M=$(CUR_DIR) clean

install:
	cp -raf *.ko

else

obj-m += chr_drv.o

endif