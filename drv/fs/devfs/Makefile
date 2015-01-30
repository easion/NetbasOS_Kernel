CROSSCOMPILER=i386-elf-
CC = $(CROSSCOMPILER)gcc
LD = $(CROSSCOMPILER)ld

RM = rm -f 
ROMFSDIR = ./
INCDIR	=$(SDKDIR)/drv/include
CFLAGS	=   -finline-functions -fno-builtin -nostdinc   -fno-PIC\
 -c -O2 -nostdinc -D__DLL__ -D__DEVFS_MODULE__ -I$(INCDIR) -I$(ROMFSDIR)  -I.  

ROMFSOBJS = $(ROMFSDIR)netbas.o $(ROMFSDIR)devfs.o


OBJS = $(ROMFSOBJS)

%.o%.c:
	$(CC) $(CFLAGS) -c -o $@ $<

all:	 devfs

devfs: $(OBJS) $(MAKEDEP)
	$(RM) *.gz
	$(LD) -nostdlib -d -r -o $@ $(OBJS)
	$(CROSSCOMPILER)objdump  -r  $@ > reloc.lst
#	gzip   $@ 
	cp  $@ $(SDKDIR)/livecd/iso/system/servers/$@.sys

clean: 

	$(RM) *.o
	$(RM) *.bak
	$(RM) $(OBJS)

	


