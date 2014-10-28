ccflags-y := -std=gnu99
ifneq ($(KERNELRELEASE),)
	TARGET = shuttle_module

	shuttle_module-objs := shuttle_service.o syscall_handler.o module.o	

	obj-m := $(TARGET).o
else
	KERNELDIR ?= /lib/modules/`uname -r`/build/
	PWD := `pwd`
default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
endif

