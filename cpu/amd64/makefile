ROOTDIR=../../

include $(ROOTDIR)cpu/$(ARCH)_platform.inc

CFLAGS += -I$(ROOTDIR)sys

OBJS = boot.o core64.o test.o i8259.o \
 loader.o   isr.o arch.o cons.o\
kernel/process.o traps.o kernel/ia32.o\
kernel/idt.o kernel/elf64.o  kernel/system.o\
mm/cons.o mm/pages.o   syscall.o
#grub/loader.o grub/cons.o grub/arch.o
#vm32.o
all:	cpu.o

cpu.o:	$(OBJS)
	$(LD)   -r -o $@ $(OBJS)

clean:
	$(RM)  *.o
	$(RM)  *.bak
	$(RM)  kernel/*.bak
	$(RM)  mm/*.bak
	$(RM)  $(OBJS)
