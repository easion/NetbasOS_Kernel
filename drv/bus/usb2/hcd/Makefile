
CROSSCOMPILER=i386-elf-

CC = $(CROSSCOMPILER)gcc
LD=$(CROSSCOMPILER)ld
OBJDUMP = $(CROSSCOMPILER)objdump

INCDIR	=$(SDKDIR)/drv/include
CFLAGS	=  -nostdinc -D_KERNEL -D__NETBAS__ -DUSB_DEBUG -I$(INCDIR) -I../include -I.. -DUSB_MODULE

CFLAGS   +=  -fno-PIC -c
AFLAGS   = -c

C_INCLUDE_PATH=$(DRV_INCLUDE_PATH)

OBJS :=  _usb_hid.o  \
	_uhub.o \
	_usb_requests.o \
	_usb_transfer.o \
	_usb_subr.o  \
	_usb.o  \
	compat_linux.o\
	compat_bsd.o\
	netbas.o

#OBJS+=	_uhci_pci.o _uhci.o 
#OBJS+=	_ohci_pci.o _ohci.o 
#OBJS+=	_ehci_pci.o _ehci.o 

UHCI_OBJS=	_uhci_pci.o _uhci.o compat_bsd.o
OHCI_OBJS=	_ohci_pci.o _ohci.o compat_bsd.o
EHCI_OBJS=	_ehci_pci.o _ehci.o compat_bsd.o


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




all :	objs $(OBJDIR)/uhci.sys\
	objs $(OBJDIR)/ohci.sys\
	objs $(OBJDIR)/ehci.sys

objs:
	mkdir objs




$(OBJDIR)/uhci.sys: $(UHCI_OBJS) Makefile
	$(LD) -d -r -nostdlib -o $@ $(UHCI_OBJS)
	$(OBJDUMP) -t  $@ | grep "*UND*" > u_reloc.lst
	cp  $@ $(SDKDIR)/livecd/iso/system/drivers

$(OBJDIR)/ohci.sys: $(OHCI_OBJS) Makefile
	$(LD) -d -r -nostdlib -o $@ $(OHCI_OBJS)
	$(OBJDUMP) -t  $@ | grep "*UND*" > o_reloc.lst
	cp  $@ $(SDKDIR)/livecd/iso/system/drivers

$(OBJDIR)/ehci.sys: $(EHCI_OBJS) Makefile
	$(LD) -d -r -nostdlib -o $@ $(EHCI_OBJS)
	$(OBJDUMP) -t  $@ | grep "*UND*" > e_reloc.lst
	cp  $@ $(SDKDIR)/livecd/iso/system/drivers

install: all
#	mkdir -p /system/drivers/dev/hcd
#	cp -f $(OBJDIR)/usb.sys /system/drivers/dev/hcd/usb

dist: all
#	mkdir -p $(DIST_DIR)/atheos/sys/drivers/dev/hcd
#	cp -f $(OBJDIR)/usb.sys $(DIST_DIR)/atheos/sys/drivers/dev/hcd/usb

clean:
	rm -f $(OBJS) $(OBJDIR)/usb.sys
	rm -f  *.bak
	rm -f *.o
