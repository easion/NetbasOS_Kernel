ROOTDIR=../../

include $(ROOTDIR)cpu/$(ARCH)_platform.inc
#BOARD = s3c2410
#BOARD = s3c4510
BOARD = atmel

CFLAGS += -I$(ROOTDIR)sys -D$(BOARD)



BOARD_OBJS = $(BOARD)/$(BOARD).o

OBJS = farcalls.o thread.o main.o\
asm.o mmu.o div0.o divsi3.o\
traps.o swi.o syscall.o elf.o\
context.o

OBJS += $(BOARD_OBJS)

all:	cpu.o

$(BOARD_OBJS):
	$(MAKE) -C ./$(BOARD)


cpu.o:	$(OBJS)
	$(LD) -r -o $@ $(OBJS)


clean:
	$(MAKE) -C ./s3c4510 clean
	$(RM) *.bak
	$(RM)  $(OBJS)
	$(RM) kernel/*.bak
	$(RM) mm/*.bak
