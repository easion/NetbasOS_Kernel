
CROSSCOMPILER=i386-elf-

CC = $(CROSSCOMPILER)gcc
LD=$(CROSSCOMPILER)ld
OBJDUMP = $(CROSSCOMPILER)objdump

INCDIR	=$(SDKDIR)/drv/include
CFLAGS	=  -nostdinc -D_KERNEL -D__NETBAS__ -DUSB_DEBUG -I$(INCDIR) -I../include -I../scsi/

CFLAGS   +=  -fno-PIC -c
AFLAGS   = -c

C_INCLUDE_PATH=$(DRV_INCLUDE_PATH)

OBJS := bbb.o \
	cbi.o \
	core.o  \
	umass2.o \
	cam.o \



OBJDIR	:= objs
OBJS	:= $(addprefix $(OBJDIR)/,$(OBJS))


#
#	rules
#

$(OBJDIR)/%.o : %.c
	$(CC) $(CFLAGS) $< -o $@

$(OBJDIR)/%.o : %.cpp
	$(CXX) $(CFLAGS) $< -o $@

$(OBJDIR)/%.o : %.s
	$(CC) $(AFLAGS) -x assembler-with-cpp $< -o $@

$(OBJDIR)/%.o : %.S
	$(CC) $(CFLAGS) -x assembler-with-cpp $< -o $@




all :	objs $(OBJDIR)/umass.sys

objs:
	mkdir objs

$(OBJDIR)/umass.sys: $(OBJS) Makefile
	$(LD) -d -r -nostdlib -o $@ $(OBJS)
	$(OBJDUMP) -t  $@ | grep "*UND*" > reloc.lst
	cp  $@ $(SDKDIR)/livecd/iso/system/drivers


clean:
	rm -f $(OBJS) $(OBJDIR)/usb.sys
	rm -f  *.bak
	rm -f *.o
