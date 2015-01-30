
CROSSCOMPILER=i386-elf-
CC = $(CROSSCOMPILER)gcc
CXX = $(CROSSCOMPILER)g++
LD=$(CROSSCOMPILER)ld
AS = yasm -f elf

OBJDUMP = $(CROSSCOMPILER)objdump
INCDIR	=$(SDKDIR)/drv/include
CFLAGS	=  -nostdinc -D__netbas__ -I$(INCDIR) -I../include -I./include

CFLAGS   +=  -fno-PIC -c -D__ENABLE_DEBUG__
AFLAGS   = -c



C_INCLUDE_PATH=$(DRV_INCLUDE_PATH)

OBJS := biosdisk.o  bios.o \
	utils.o entry.o
	#cpplib.o





OBJDIR	:= objs
OBJS	:= $(addprefix $(OBJDIR)/,$(OBJS))
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




all :	objs $(OBJDIR)/biosdisk.sys

objs:
	mkdir objs

$(OBJDIR)/biosdisk.sys: $(OBJS) Makefile
#	$(CC) -kernel $(OBJS) -o $(OBJDIR)/usb_ohci.dll
	$(LD) -d -r -nostdlib -o $@ $(OBJS)
	$(OBJDUMP) -r  $@ > reloc.lst
	rm -f  $@.gz
#	gzip $@
	cp -fr  $@  $(SDKDIR)/livecd/iso/system/servers/biosdisk.sys


clean:
	rm -f *.bak
	rm -f objs/*.o
	rm -f objs/*.sys
