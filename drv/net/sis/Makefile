
CROSSCOMPILER=i386-elf-
CC = $(CROSSCOMPILER)gcc
LD=$(CROSSCOMPILER)ld

INCDIR	=$(SDKDIR)/drv/include
#CFLAGS	=-g -Wall -W -c -O2 -nostdinc -D__DLL__ -I$(INCDIR) -I../include
CFLAGS	= -c -fno-PIC -nostdinc -D__DLL__ -I$(INCDIR) -I../include
OBJS =  sis900.o 


%.o%.c:
	$(CC) $(CFLAGS) -c -o $@ $<

all:	sis900.dll 

sis900.dll: $(OBJS) $(MAKEDEP)
	$(LD) -d -r -nostdlib -o $@ $(OBJS)
	$(CROSSCOMPILER)objdump -a -r  $@ > reloc.lst
	cp $@ $(SDKDIR)/livecd/iso/system/drivers

clean:  
	rm -f  *.bak
	rm -f *.o
	


