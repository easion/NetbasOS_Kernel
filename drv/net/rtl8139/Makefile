
CROSSCOMPILER=i386-elf-
CC = $(CROSSCOMPILER)gcc
LD=$(CROSSCOMPILER)ld
OBJDUMP=$(CROSSCOMPILER)objdump

INCDIR	=$(SDKDIR)/drv/include
#CFLAGS	=-g -Wall -W -c -O2 -nostdinc -D__DLL__ -I$(INCDIR) -I../include
CFLAGS	= -c -fno-PIC  -Wall -Wstrict-prototypes -O6 -nostdinc -D__DLL__ -I$(INCDIR) -I../include
OBJS =  rtl8139.o os.o\



%.o%.c:
	$(CC) $(CFLAGS) -c -o $@ $<

all:	rtl8139.dll 

rtl8139.dll: $(OBJS) $(MAKEDEP)
	$(LD) -d -r -nostdlib -o $@ $(OBJS)
	$(OBJDUMP) -a -r  $@ > reloc.lst
	cp $@ $(SDKDIR)/livecd/iso/system/drivers

clean:  
	rm -f  *.bak
	rm -f *.dll
	rm -f *.lst
	rm -f *.o
	


