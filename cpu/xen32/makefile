ROOTDIR=../../

include $(ROOTDIR)cpu/$(ARCH)_platform.inc

CFLAGS += -I$(ROOTDIR)sys

OBJS = xen.o mem.o console.o traps.o


#all:	 cpu.o
all:	asm kernel.bin

asm:
	i386-elf-gcc -Wall -O -finline-functions -fno-builtin -nostdinc -g  -I./include -D__IA32__   -I../../sys   -c -o boot.o boot.S
	i386-elf-gcc -Wall -O -finline-functions -fno-builtin -nostdinc -g  -I./include -D__IA32__   -I../../sys   -E -o xen_ok.c xen.c

cpu.o:	$(OBJS)
	$(LD) -r -o $@ $(OBJS)

kernel.bin:	$(OBJS)
	$(LD) -T link.ld  -s -nostartfiles -nostdlib -Map kernel.map  -o $@ boot.o $(OBJS)

clean:
	$(RM) *.bak
	$(RM)  $(OBJS)
	$(RM)  *.bin

