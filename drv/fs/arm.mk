
CROSSCOMPILER=arm-elf-

CC = $(CROSSCOMPILER)gcc
LD=$(CROSSCOMPILER)ld
OBJDUMP = $(CROSSCOMPILER)objdump


RM = rm -f 
NTFSDIR = ./ntfs/
INCDIR	=$(SDKDIR)/drv/include
CFLAGS	=   -finline-functions -fno-builtin -nostdinc   -fno-PIC\
 -c -O2 -nostdinc -D__DLL__ -D__ARM__ -I$(INCDIR) -I$(NTFSDIR)


OBJS = vfs/fs_server.o vfs/process.o vfs/abort.o vfs/devops.o\
vfs/node.o vfs/buffer.o vfs/fcall.o vfs/fcb.o vfs/fcntl.o\
unicode/case.o unicode/convers.o unicode/utf8.o unicode/utf16.o\
vfs/link.o vfs/open.o vfs/path.o vfs/stdio.o vfs/pipe.o vfs/dup.o

FATOBJS = fat/attrib.o fat/fat12.o fat/fat16.o fat/fat32.o fat/fcntl.o fat/super.o\
fat/create.o fat/truncate.o fat/fdt.o\
fat/lfn.o

CDFSOBJS =cdfs/iso9660.o cdfs/init.o

NTFSOBJS =  $(NTFSDIR)readdir.o $(NTFSDIR)ntfs.o $(NTFSDIR)util.o \
$(NTFSDIR)inode.o $(NTFSDIR)attr.o $(NTFSDIR)unistr.o\
$(NTFSDIR)support.o $(NTFSDIR)super.o $(NTFSDIR)dir.o

OBJS += $(FATOBJS)
OBJS += $(CDFSOBJS)
#OBJS += $(NTFSOBJS)

%.o%.c:
	$(CC) $(CFLAGS) -c -o $@ $<

all:	 armfs

armfs: $(OBJS) $(MAKEDEP)
	$(RM) *.gz
	$(LD) -nostdlib -d -r -o $@ $(OBJS)
	$(CROSSCOMPILER)objdump  -r  $@ > reloc.lst
	gzip   $@ 

clean:  
	$(RM) ntfs/*.o
	$(RM) cdfs/*.bak
	$(RM) ntfs/*.bak
	$(RM) vfs/*.o
	$(RM) vfs/*.bak
	$(RM) fat/*.o
	$(RM) fat/*.bak
	$(RM) *.o
	$(RM) *.bak
	$(RM) $(OBJS)
	$(RM) ext2/*.o
	$(RM) ext2/*.bak	
	$(RM) unicode/*.bak	
	$(RM) unicode/*.bak	
	


