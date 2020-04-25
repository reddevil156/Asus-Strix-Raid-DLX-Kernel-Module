KDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)
TARGET = strix-daemon.c
OUTPUT = strix-daemon
CC ?= gcc

obj-m := strixdlx.o

all:
	make -C $(KDIR) M=$(PWD) modules

daemon:

	$(CC) -I/usr/include/alsa -lasound -lpthread -o $(OUTPUT) $(TARGET)
        
clean:

	make -C $(KDIR) M=$(PWD) clean
	rm -f *.o *.ko *.mod.c Module.symvers modules.order
