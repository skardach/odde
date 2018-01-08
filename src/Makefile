#
# Fujitsu Esprimo Optical Disc Drive Eject button driver
#
obj-m += odde.o

KDIR ?= /lib/modules/`uname -r`/build

all:
	@$(MAKE) -C $(KDIR) M=$$PWD modules

clean:
	@$(MAKE) -C $(KDIR) M=$$PWD clean

install:
	sudo $(MAKE) -C $(KDIR) M=$$PWD modules_install
	depmod
