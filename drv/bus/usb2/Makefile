
CROSSCOMPILER=i386-elf-

CC = $(CROSSCOMPILER)gcc
LD=$(CROSSCOMPILER)ld
OBJDUMP = $(CROSSCOMPILER)objdump

INCDIR	=$(SDKDIR)/drv/include
CFLAGS	=  -nostdinc -D_KERNEL -D__NETBAS__  -I$(INCDIR) -I./include -I. -DUSB_MODULE #-DUSB_DEBUG

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
	hcd/compat_bsd.o\
	netbas.o



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




all :	objs hcd $(OBJDIR)/usb.sys

objs:
	mkdir objs

hcd:
	make -C ./hcd

$(OBJDIR)/usb.sys: $(OBJS) Makefile
	$(LD) -d -r -nostdlib -o $@ $(OBJS)
	$(OBJDUMP) -t  $@ | grep "*UND*" > reloc.lst
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
	make -C ./hcd clean
