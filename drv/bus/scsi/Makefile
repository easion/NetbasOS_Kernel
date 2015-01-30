
CROSSCOMPILER=i386-elf-

CC = $(CROSSCOMPILER)gcc
LD=$(CROSSCOMPILER)ld
OBJDUMP = $(CROSSCOMPILER)objdump

INCDIR	=$(SDKDIR)/drv/include
CFLAGS	=  -nostdinc -D__DLL__ -I$(INCDIR) -I../include

CFLAGS   +=  -fno-PIC -c -I.
AFLAGS   = -c

C_INCLUDE_PATH=$(DRV_INCLUDE_PATH)

OBJS := scsi.o scsi_common.o scsi_generic.o scsi_disk.o scsi_cdrom.o


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




all :	objs $(OBJDIR)/scsi.dll

objs:
	mkdir objs

$(OBJDIR)/scsi.dll: $(OBJS) Makefile
#	$(CC) -kernel $(OBJS) -o $(OBJDIR)/scsi.dll
	$(LD) -d -r -nostdlib -o $@ $(OBJS)
	$(OBJDUMP) -a -r  $@ > reloc.lst
	cp $@ $(SDKDIR)/livecd/iso/system/drivers -fr

install: all
#	mkdir -p /system/drivers/dev/hcd
#	cp -f $(OBJDIR)/scsi.dll /system/drivers/dev/hcd/scsi

dist: all
#	mkdir -p $(DIST_DIR)/atheos/sys/drivers/dev/hcd
#	cp -f $(OBJDIR)/scsi.dll $(DIST_DIR)/atheos/sys/drivers/dev/hcd/scsi

clean:
	rm -f $(OBJS) $(OBJDIR)/scsi.dll
	rm -f  *.bak
	rm -f *.o
