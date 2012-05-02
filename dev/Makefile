ARCH=arm
CC=/home/netlab/jclseema/year3/buildroot/output/host/usr/bin/arm-unknown-linux-uclibcgnueabi-gcc
obj-m := usbPioKeypad.o
KDIR := /home/netlab/jclseema/year3/buildroot/output/build/linux-

all:


clean:
	  rm -f *.o

ins: usbPioKeypad.ko
          sudo insmod usbPioKeypad.ko

rmm: usbPioKeypad.ko
          sudo rmmod usbPioKeypad.ko