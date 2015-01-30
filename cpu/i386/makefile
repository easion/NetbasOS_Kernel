ROOTDIR=../../

include $(ROOTDIR)cpu/$(ARCH)_platform.inc

CFLAGS += -I$(ROOTDIR)sys

OBJS = startup.o  fpu.o \
mm/page.o mm/init.o mm/mmap.o mm/utils.o mm/mem.o\
 entry.o isr.o traps.o i8259.o doscall.o setvec86.o\
protect.o main.o segment.o cpuid.o asm.o\
exception.o   x86-64.o debug.o\
kernel/elf_dll.o kernel/coff_dll.o  kernel/coffexec.o kernel/elf.o\
kernel/process.o kernel/task.o kernel/fork.o\
kernel/signal.o kernel/vm86.o \
kernel/int86.o kernel/syscall.o \
kernel/linux.o  kernel/ustack.o\
kernel/exec.o kernel/kernel_sym.o kernel/modules.o\
kernel/multiboot.o kernel/pe_exec.o kernel/pecoff_dll.o\
kernel/thread.o kernel/msgport.o\
dev/sound.o dev/keyboard.o dev/kbdfuc.o\
dev/vga03h.o dev/dma.o dev/memory.o dev/equip.o dev/isa.o\
dev/serial.o dev/rtc.o  

all:	cpu.o

cpu.o:	$(OBJS)
	$(LD) -r -o $@ $(OBJS)


clean:
	$(RM) *.bak
	$(RM)  $(OBJS)

	$(RM) kernel/*.bak

	$(RM) mm/*.bak
