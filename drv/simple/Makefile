
#CROSSCOMPILER=i386-elf-
CROSSCOMPILER=arm-elf-
CC = $(CROSSCOMPILER)gcc
CXX = $(CROSSCOMPILER)g++
LD=$(CROSSCOMPILER)ld

OBJDUMP = $(CROSSCOMPILER)objdump
INCDIR	=$(SDKDIR)/drv/include
CFLAGS	=  -nostdinc -D__DLL__ -I$(INCDIR) -I../include

CFLAGS   +=   -c -D__ENABLE_DEBUG__ 
#-fno-PIC
AFLAGS   = -c
C_INCLUDE_PATH=$(DRV_INCLUDE_PATH)

OBJS := drv_c.o 
#drv_cpp.o cpplib.o


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




all :	objs drv_c.o $(OBJDIR)/drv_cpp.dll

objs:
	mkdir objs

$(OBJDIR)/drv_cpp.dll: $(OBJS) Makefile
#	$(CC) -kernel $(OBJS) -o $(OBJDIR)/usb_ohci.dll
	$(LD) -d -r -nostdlib -o $@ $(OBJS)
	$(OBJDUMP) -r  $@ > reloc.lst

drv_c.o: $(OBJS)
	$(OBJDUMP) -x -S  ./objs/drv_c.o > drv_c.lst

install: all
#	mkdir -p /system/drivers/dev/hcd
#	cp -f $(OBJDIR)/usb_ohci.dll /system/drivers/dev/hcd/usb_ohci

dist: all
#	mkdir -p $(DIST_DIR)/atheos/sys/drivers/dev/hcd
#	cp -f $(OBJDIR)/usb_ohci.dll $(DIST_DIR)/atheos/sys/drivers/dev/hcd/usb_ohci

clean:
	rm -f $(OBJS) $(OBJDIR)/usb_ohci.dll
	rm -f  *.bak
	rm -f *.o
