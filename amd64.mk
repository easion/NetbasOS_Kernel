#
#Makefile for Jicama OS
#
include amd64.inc

OBJS =  cpu/amd64/cpu.o devices/driver.o core/kernel.o   lib/libc.o 
#core/kernel.o 

all:   clean kern64

cpu/amd64/cpu.o:
	$(MAKE) -C cpu/amd64

lib/libc.o:
	$(MAKE) -C ./lib

fs/fs.o:
	$(MAKE) -C ./fs

net/net.o:
	$(MAKE) -C ./net

devices/driver.o:
	$(MAKE) -C ./devices

core/kernel.o:
	$(MAKE) -C ./core

vin/vin.o:
	$(MAKE) -C ./vin

clean:  
	$(RM)   cpu\amd64\cpu.o
	$(RM)   core\kernel.o
	$(RM)   devices\driver.o
	$(RM)  kernel.gz
	$(RM)  *.bak

BFD_NAME = elf64-x86-64
BFD_ARCH = i386:x86-64
BFD = binary

kern64: $(OBJS)  $(LIBS)
	$(RM) $@.gz
	$(LD)  $(LFLAGS64BIN) -o $@  $(OBJS)
	 sys/gzip $@
	

image: ./jicama.bin
           #rdsk.exe -o kernel.bin ./jicama.bin 
           #objdump ./jicama.bin --disassemble > kernel.asm
            #objcopy -R .note -R .comment -S -O binary jicama.o sh.bin
