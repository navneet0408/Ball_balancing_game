KDIR:=/home/esp/SDK/sysroots/i586-poky-linux/usr/src/kernel
#PWD:= $(shell pwd)

CC = i586-poky-linux-gcc
ARCH = x86
CROSS_COMPILE = i586-poky-linux-
SROOT=/home/esp/SDK/sysroots/i586-poky-linux/

APP = MPU_UserApp
obj-m:= MPU_Driver.o

all:
	make ARCH=x86 CROSS_COMPILE=i586-poky-linux- -C $(KDIR) M=$(PWD) modules 
	i586-poky-linux-gcc -o $(APP) MPU_UserApp.c --sysroot=$(SROOT) -lm -lpthread
clean:
	rm -f *.ko
	rm -f *.o
	rm -f Module.symvers
	rm -f modules.order
	rm -f *.mod.c
	rm -rf .tmp_versions
	rm -f *.mod.c
	rm -f *.mod.o
	rm -f \.*.cmd
	rm -f $(APP)
	rm -f Module.markers
	rm -f *~

