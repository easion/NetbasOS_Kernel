
CROSSCOMPILER=i386-elf-
CC = $(CROSSCOMPILER)gcc
LD=$(CROSSCOMPILER)ld
OBJDUMP=$(CROSSCOMPILER)objdump

INCDIR	=$(SDKDIR)/drv/include
CFLAGS	=   -fno-PIC -O2 -nostdinc -D__DLL__ -I$(INCDIR) -I../include
OBJS =  ne2k.o os.o utils.o
#utils.o


%.o%.c:
	$(CC) $(CFLAGS) -c -o $@ $<

all:	ne2000.dll 

ne2000.dll: $(OBJS) $(MAKEDEP)
	$(LD) -d -r -nostdlib -o $@ $(OBJS)
	$(OBJDUMP) -r  $@ > reloc.lst
	cp $@ $(SDKDIR)/livecd/iso/system/drivers -fr

clean:  
	rm -f  *.bak
	rm -f *.o
	


