
CROSSCOMPILER=i386-elf-
CC = $(CROSSCOMPILER)gcc
LD=$(CROSSCOMPILER)ld

INCDIR	=$(SDKDIR)/drv/include

CFLAGS	= -c -fno-PIC -nostdinc -D__DLL__ -I$(INCDIR) -I../include
OBJS =  fdc.o util.o


%.o%.c:
	$(CC) $(CFLAGS) -c -o $@ $<

all:	fdc.dll 

fdc.dll: $(OBJS) $(MAKEDEP)
	$(LD) -d -r -nostdlib -o $@ $(OBJS)
	$(CROSSCOMPILER)objdump  -r  $@ > reloc.lst
	cp $@ $(SDKDIR)/livecd/iso/system/drivers -fr

clean:  
	rm -f  *.bak
	rm -f *.o
	


