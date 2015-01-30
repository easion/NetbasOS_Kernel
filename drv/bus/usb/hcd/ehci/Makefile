
CROSSCOMPILER=i386-elf-
CC = $(CROSSCOMPILER)gcc
LD=$(CROSSCOMPILER)ld

OBJDUMP = $(CROSSCOMPILER)objdump
INCDIR	=$(SDKDIR)/drv/include
CFLAGS	=  -nostdinc -D__DLL__ -I$(INCDIR) -I../include

CFLAGS   +=  -fno-PIC -c -D__ENABLE_DEBUG__
AFLAGS   = -c

C_INCLUDE_PATH=$(DRV_INCLUDE_PATH)

OBJS := hcd.o ehci.o gcc34_memcpy.o


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




all :	objs $(OBJDIR)/usb_uhci.dll

objs:
	mkdir objs

$(OBJDIR)/usb_uhci.dll: $(OBJS) Makefile
#	$(CC) -kernel $(OBJS) -o $(OBJDIR)/usb_uhci.dll
	$(LD) -d -r -nostdlib -o $@ $(OBJS)
	$(OBJDUMP) -r  $@ > reloc.lst

install: all
#	mkdir -p /system/drivers/dev/hcd
#	cp -f $(OBJDIR)/usb_uhci.dll /system/drivers/dev/hcd/usb_uhci

dist: all
#	mkdir -p $(DIST_DIR)/atheos/sys/drivers/dev/hcd
#	cp -f $(OBJDIR)/usb_uhci.dll $(DIST_DIR)/atheos/sys/drivers/dev/hcd/usb_uhci

clean:
	rm -f $(OBJS) $(OBJDIR)/usb_uhci.dll
	rm -f  *.bak
	rm -f *.o



