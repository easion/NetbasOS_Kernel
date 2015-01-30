
CROSSCOMPILER=i386-elf-
CC = $(CROSSCOMPILER)gcc
LD=$(CROSSCOMPILER)ld

INCDIR	=$(SDKDIR)/drv/include

CFLAGS	= -c -fno-PIC -nostdinc -D__DLL__ -I$(INCDIR) -I../include

OBJS = atapi.o util.o hd.o partition.o

%.o%.c:
	$(CC) $(CFLAGS) -c -o $@ $<

all:	 ide  


ide: $(OBJS) $(MAKEDEP)
	$(RM) *.gz
	$(LD) -d -r -nostdlib -o $@ $(OBJS)
	$(CROSSCOMPILER)objdump  -r  $@ > reloc.lst
	gzip $@
	cp *.gz $(SDKDIR)/livecd/iso/system/drivers

clean: 
	$(RM) *.o *.gz *.lst
	$(RM) *.bak
	


