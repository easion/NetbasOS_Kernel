
CROSSCOMPILER=i386-elf-

CC = $(CROSSCOMPILER)gcc
LD=$(CROSSCOMPILER)ld
OBJDUMP = $(CROSSCOMPILER)objdump

INCDIR	=$(SDKDIR)/drv/include
CFLAGS	=  -nostdinc -D_KERNEL -D__NETBAS__ -DUSB_DEBUG -I$(INCDIR) -I../include -I.

CFLAGS   +=  -fno-PIC -c
AFLAGS   = -c

C_INCLUDE_PATH=$(DRV_INCLUDE_PATH)


OBJS+=scsi.o
OBJS+=scsi_generic.o
OBJS+=scsi_common.o
OBJS+=scsi_disk.o



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




all :	objs $(OBJDIR)/scsi.sys

objs:
	mkdir objs

$(OBJDIR)/scsi.sys: $(OBJS) Makefile
#	$(CC) -kernel $(OBJS) -o $(OBJDIR)/scsi.sys
	$(LD) -d -r -nostdlib -o $@ $(OBJS)
	$(OBJDUMP) -t  $@ | grep "*UND*" > reloc.lst
	cp  $@ $(SDKDIR)/livecd/iso/system/drivers


clean:
	rm -f $(OBJS) $(OBJDIR)/scsi.sys
	rm -f  *.bak
	rm -f *.o
