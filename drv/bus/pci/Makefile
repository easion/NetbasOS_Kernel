
CROSSCOMPILER=i386-elf-
CC = $(CROSSCOMPILER)gcc
LD=$(CROSSCOMPILER)ld

OBJDUMP = $(CROSSCOMPILER)objdump
INCDIR	=$(SDKDIR)/drv/include
CFLAGS	=  -nostdinc -D__DLL__ -I$(INCDIR) -I../include

CFLAGS   +=  -fno-PIC -c
AFLAGS   = -c

C_INCLUDE_PATH=$(DRV_INCLUDE_PATH)

OBJS := bios.o init.o\
pci32.o pci.o pci_core.o 


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




all :	objs $(OBJDIR)/pci32.dll

objs:
	mkdir objs

$(OBJDIR)/pci32.dll: $(OBJS) Makefile
#	$(CC) -kernel $(OBJS) -o $(OBJDIR)/pci32.dll
	$(LD) -d -r -nostdlib -o $@ $(OBJS)
	$(OBJDUMP)  -r  $@ > reloc.lst
	cp -fr $@ $(SDKDIR)/livecd/iso/system/drivers/

install: all
#	mkdir -p /system/drivers/dev/hcd
#	cp -f $(OBJDIR)/pci32.dll /system/drivers/dev/hcd/usb

dist: all
#	mkdir -p $(DIST_DIR)/atheos/sys/drivers/dev/hcd
#	cp -f $(OBJDIR)/pci32.dll $(DIST_DIR)/atheos/sys/drivers/dev/hcd/usb

clean:
	rm -f $(OBJS) $(OBJDIR)/pci32.dll
	rm -f  *.bak
	rm -f *.o
