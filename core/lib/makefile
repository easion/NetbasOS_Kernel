ROOTDIR=../../

include $(ROOTDIR)cpu/$(ARCH)_platform.inc

CFLAGS += -I$(ROOTDIR)sys


OBJS = kprintf.o   conv.o  snprintf.o string.o strtok.o  scanf.o log.o\
misc.o  

all:	lib.o




lib.o:	$(OBJS)
	$(LD) -r -o $@ $(OBJS)

clean:
	$(RM) *.bak
	$(RM)  $(OBJS)