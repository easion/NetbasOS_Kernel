
ROOTDIR=../../

include $(ROOTDIR)cpu/$(ARCH)_platform.inc

CFLAGS += -I$(ROOTDIR)sys

OBJS =termios.o kfs.o clist.o\
devclass.o\
common.o terminal.o sysdev.o\
ramdisk.o ioport.o\
irq.o bus/bus.o\
char/console.o \
char/vt100.o\
char/tty.o char/timer.o\
char/alarm.o\
char/mem.o 

all:	driver.o


driver.o:	$(OBJS)
	$(LD) -r -o $@ $(OBJS)


clean:
	$(RM) -f *.o
	$(RM) -f *.bak
	$(RM) char/*.bak
	$(RM) bus/*.bak
	$(RM)  $(OBJS)


