CROSSCOMPILER=i386-elf-
CC = $(CROSSCOMPILER)gcc
LD = $(CROSSCOMPILER)ld

RM = rm -f 
NTFSDIR = ./
INCDIR	=$(SDKDIR)/drv/include
CFLAGS	=   -finline-functions -fno-builtin -nostdinc   -fno-PIC\
 -c -O2 -nostdinc -D__DLL__ -I$(INCDIR) -I$(NTFSDIR)  -I.  

NTFSOBJS = $(NTFSDIR)netbas.o $(NTFSDIR)fs.o\
 $(NTFSDIR)util.o  $(NTFSDIR)dir.o\
$(NTFSDIR)inode.o $(NTFSDIR)attr.o $(NTFSDIR)unistr.o\
$(NTFSDIR)support.o $(NTFSDIR)super.o

OBJS = $(NTFSOBJS)

%.o%.c:
	$(CC) $(CFLAGS) -c -o $@ $<

all:	 ntfs

ntfs: $(OBJS) $(MAKEDEP)
	$(RM) *.gz
	$(LD) -nostdlib -d -r -o $@ $(OBJS)
	$(CROSSCOMPILER)objdump  -r  $@ > reloc.lst
	gzip   $@ 
	cp *.gz $(SDKDIR)/livecd/iso/system/servers

clean: 

	$(RM) *.o
	$(RM) *.bak
	$(RM) $(OBJS)

	


