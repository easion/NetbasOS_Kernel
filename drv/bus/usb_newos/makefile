
CROSSCOMPILER=i386-elf-
CC = $(CROSSCOMPILER)gcc
CXX = $(CROSSCOMPILER)g++
LD=$(CROSSCOMPILER)ld
AS = yasm -f elf

OBJDUMP = $(CROSSCOMPILER)objdump
INCDIR	=$(SDKDIR)/drv/include
CFLAGS	=  -nostdinc -D__KERNEL__=1 -I$(INCDIR) -I../include -I./include -Wall 

CFLAGS   += -I./ -fno-PIC -c -D__ENABLE_DEBUG__
AFLAGS   = -c

C_INCLUDE_PATH=$(DRV_INCLUDE_PATH)

MY_OBJS := \
	usb.o \
	usb_device.o \
	usb_enum.o \
	usb_hub.o \
	usb_pipe.o

#MY_OBJS += ohci/ohci.o 

MY_OBJS += uhci/uhci.o 


OBJECTS = $(SOURCES:.asm=.o)

OBJDIR	:= objs
MY_OBJS	:= $(addprefix $(OBJDIR)/,$(MY_OBJS))
OBJECTS = $(SOURCES:.asm=.o)


#
#	rules
#

$(OBJDIR)/%.o%.asm:
	$(AS) -o $@ $<

$(OBJDIR)/%.o : %.c
	$(CC) $(CFLAGS) $< -o $@

$(OBJDIR)/%.o : %.cpp
	$(CXX) $(CFLAGS) $< -o $@

$(OBJDIR)/%.o : %.s
	$(CC) $(AFLAGS) -x assembler-with-cpp $< -o $@

$(OBJDIR)/%.o : %.S
	$(AS) -o $@ $<


all :	objs $(OBJDIR)/usb.sys

objs:
	mkdir objs

$(OBJDIR)/usb.sys: $(MY_OBJS) Makefile
	$(LD) -d -r -nostdlib -o $@ $(MY_OBJS)
	$(OBJDUMP) -t  $@ | grep "UND"> reloc.lst
	cp -fr  $@  $(SDKDIR)/livecd/iso/system/servers/usb.sys



clean:	
	rm -f $(MY_OBJS)
	rm -f *.bak
	rm -f *.sys
	rm -f *.o

