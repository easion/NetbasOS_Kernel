
CROSSCOMPILER=i386-elf-
CC = $(CROSSCOMPILER)gcc
LD=$(CROSSCOMPILER)ld

INCDIR	=$(SDKDIR)/drv/include
#CFLAGS	=-g -Wall -W -c -O2 -nostdinc -D__DLL__ -I$(INCDIR) -I../include
CFLAGS	= -c -fno-PIC -nostdinc -D__DRV__ -I$(INCDIR) -I../include
OBJS =  tunif.o


%.o%.c:
	$(CC) $(CFLAGS) -c -o $@ $<

all:	tunif.dll 

tunif.dll: $(OBJS) $(MAKEDEP)
	$(LD) -d -r -nostdlib -o $@ $(OBJS)
	$(CROSSCOMPILER)objdump  -t  $@ | grep "*UND*" > reloc.lst
	cp $@ $(SDKDIR)/livecd/iso/system/drivers -fr

clean:  
	rm -f  *.bak *.lst
	rm -f *.o *.dll
	


