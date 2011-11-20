obj-m := mod.o

KDIR  := /lib/modules/$(shell uname -r)/build
PWD   := $(shell pwd)

default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	rm -f modules.order mod.ko .mod.ko.cmd .mod.mod.o.cmd .mod.o.cmd Module.symvers mod.mod.c mod.mod.o mod.o
	rm -rdf .tmp_versions

ins: mod.ko
	sudo insmod mod.ko

rmm: mod.ko
	sudo rmmod mod.ko cdc_acm

ls:
	@ls -l /dev/ | grep dio[0-9]*$
