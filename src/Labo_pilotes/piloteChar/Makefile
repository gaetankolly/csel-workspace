export PATH := /buildroot/output/host/usr/sbin$\
	:/buildroot/output/host/usr/bin/$\
	:/buildroot/output/host/sbin$\
	:/buildroot/output/host/bin/$\
	:$(PATH)

# Part executed when called from kernel build system:
ifneq ($(KERNELRELEASE),)
obj-m += mymodule.o		## name of the generated module

mymodule-objs := skeleton.o	## list of objects needed for that module
CFLAGS_skeleton.o := -DDEBUG

# Part executed when called from standard make in module source directory:
else
include ../../kernel_settings
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=$(CPU) CROSS_COMPILE=$(TOOLS) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

install:
	$(MAKE) -C $(KDIR) M=$(PWD) INSTALL_MOD_PATH=$(MODPATH) modules_install

endif