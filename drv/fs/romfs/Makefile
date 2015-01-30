CROSSCOMPILER=i386-elf-
CC = $(CROSSCOMPILER)gcc
LD = $(CROSSCOMPILER)ld

RM = rm -f 
ROMFSDIR = ./
INCDIR	=$(SDKDIR)/drv/include
CFLAGS	=   -finline-functions -fno-builtin -nostdinc   -fno-PIC\
 -c -O2 -nostdinc -D__DLL__ -I$(INCDIR) -I$(ROMFSDIR)  -I.  

ROMFSOBJS = $(ROMFSDIR)netbas.o $(ROMFSDIR)romfs.o


OBJS = $(ROMFSOBJS)

%.o%.c:
	$(CC) $(CFLAGS) -c -o $@ $<

all:	 romfs

romfs: $(OBJS) $(MAKEDEP)
	$(RM) *.gz
	$(LD) -nostdlib -d -r -o $@ $(OBJS)
	$(CROSSCOMPILER)objdump  -r  $@ > reloc.lst
	gzip   $@ 
	cp *.gz $(SDKDIR)/livecd/iso/system/servers

clean: 

	$(RM) *.o
	$(RM) *.bak
	$(RM) $(OBJS)

	


