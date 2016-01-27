#obj-$(CONFIG_WEBAD) += webad.o
#webad-y := main.o capture.o mnetlink.o mstring.o http_session.o plug.o plug_extern.o

obj-m := webad.o
webad-objs := main.o capture.o mnetlink.o mstring.o http_session.o plug.o plug_extern.o
KERNELDIR :=/lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)
all:
	make -C $(KERNELDIR) M=$(PWD) modules
clean:
	make -C $(KERNELDIR) M=$(PWD) clean
	-rm -f config.h
