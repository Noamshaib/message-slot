obj-m := message_slot.o
KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

compile: all
	gcc -O3 -Wall -std=c11 message_sender.c -o sender
	gcc -O3 -Wall -std=c11 message_reader.c -o reader

compile_reader: all
	gcc -O3 -Wall -std=c11 message_reader.c -o reader

compile_sender: all
	gcc -O3 -Wall -std=c11 message_sender.c -o sender
