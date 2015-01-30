
#config
SUPPORT_FAT=y
SUPPORT_BFS=n
SUPPORT_CDFS=y
SUPPORT_NTFS=n
SUPPORT_MINIXFS=n
SUPPORT_PROCFS=y
SUPPORT_RAMFS=y
SUPPORT_ROMFS=n



#CROSSCOMPILER=i386-elf-
CC = $(CROSSCOMPILER)gcc
LD = $(CROSSCOMPILER)ld
OBJDUMP = $(CROSSCOMPILER)objdump

RM = rm -f 

INCDIR	=$(SDKDIR)/drv/include
CFLAGS	=   -finline-functions -fno-builtin -nostdinc   -fno-PIC\
 -c -O2 -nostdinc -D__DLL__ -I$(INCDIR) -I$(NTFSDIR)\
 -Werror-implicit-function-declaration -Wundef -Wstrict-prototypes -Wno-trigraphs   -msoft-float


OBJS = vfs/fs_server.o vfs/process.o vfs/abort.o vfs/devops.o\
vfs/node.o vfs/buffer.o vfs/fcall.o vfs/fcb.o vfs/fcntl.o\
unicode/case.o unicode/convers.o unicode/utf8.o unicode/utf16.o\
vfs/link.o vfs/open.o vfs/path.o vfs/stdio.o\
vfs/pipe.o vfs/socket.o vfs/dup.o vfs/proc.o


ifeq ($(SUPPORT_OLD_FAT), y)
	CFLAGS += -D__ENABLE_FAT__
	FATOBJS = fat/attrib.o fat/abstract.o fat/fattable.o fat/lower.o fat/fcntl.o fat/super.o\
	fat/create.o fat/truncate.o fat/fdt.o\
	fat/lfn.o fat/readdir.o
endif

ifeq ($(SUPPORT_FAT), y)
	CFLAGS += -D__ENABLE_FAT__ -Wno-multichar -fno-strict-aliasing -nostdinc -I./dosfs/
	FATOBJS = dosfs/dir.o dosfs/dlist.o dosfs/fat.o dosfs/iter.o dosfs/util.o\
	dosfs/encodings.o dosfs/EncodingConversions.o dosfs/UnicodeMappings.o dosfs/version.o dosfs/attr.o\
	dosfs/file.o dosfs/vcache.o dosfs/dosfs.o
endif


ifeq ($(SUPPORT_CDFS), y)
	CFLAGS += -D__ENABLE_CDFS__
	CDFSOBJS =cdfs/iso9660.o cdfs/init.o
endif


ifeq ($(SUPPORT_RAMFS), y)
	CFLAGS += -D__ENABLE_RAMFS__
	CDFSOBJS +=ramfs/ramfs.o
endif


ifeq ($(SUPPORT_BFS), y)
	CFLAGS += -D__ENABLE_BFS__ -I./bfs/
	CDFSOBJS =bfs/BlockAllocator.o bfs/BPlusTree.o bfs/BufferPool.o bfs/Debug.o bfs/Index.o bfs/Inode.o \
		bfs/Journal.o bfs/Utility.o bfs/Volume.o bfs/netbas.o bfs/kernel_interface.o
endif



ifeq ($(SUPPORT_NTFS), y)
	CFLAGS += -D__ENABLE_NTFS__
	NTFSDIR = ./ntfs/
	NTFSOBJS =  $(NTFSDIR)readdir.o $(NTFSDIR)ntfs.o $(NTFSDIR)util.o \
	$(NTFSDIR)inode.o $(NTFSDIR)attr.o $(NTFSDIR)unistr.o\
	$(NTFSDIR)support.o $(NTFSDIR)super.o $(NTFSDIR)dir.o
endif


ifeq ($(SUPPORT_MINIXFS), y)
	CFLAGS += -D__ENABLE_MINIXFS__
	MINIXFSDIR = ./minixfs/

	MINIXFSOBJS =  $(MINIXFSDIR)minixfs.o  $(MINIXFSDIR)imap.o \
	$(MINIXFSDIR)zmap.o $(MINIXFSDIR)write.o $(MINIXFSDIR)dirent.o\
	$(MINIXFSDIR)access.o
endif



ifeq ($(SUPPORT_ROMFS), y)
	CFLAGS += -D__ENABLE_ROMFS__
	ROMFSOBJS=romfs/romfs.o
endif

PROCFSOBJS =procfs/procfs.o procfs/netbas.o
DEVFSOBJS=devfs/devfs.o devfs/netbas.o

OBJS += $(FATOBJS)
OBJS += $(CDFSOBJS)
OBJS += $(ROMFSOBJS)
OBJS += $(DEVFSOBJS)
OBJS += $(PROCFSOBJS)
OBJS += $(MINIXFSOBJS)
#OBJS += $(NTFSOBJS)
CXXFLAGS = $(CFLAGS)

%.o%.c:
	$(CC) $(CFLAGS) -c -o $@ $<

%.o%.cpp:
	$(CXX) $(CXXFLAGS) -c -o $@ $<

all:	 fs

fs: $(OBJS) $(MAKEDEP)
	$(RM) *.gz
	$(LD) -nostdlib -d -r -o $@ $(OBJS)
	$(OBJDUMP) -t  $@ | grep "*UND*" > reloc.lst
	gzip   $@ 
	cp *.gz $(SDKDIR)/livecd/iso/system/servers

clean:  
	$(RM) ntfs/*.o
	$(RM) cdfs/*.bak
	$(RM) ntfs/*.bak
	$(RM) vfs/*.o
	$(RM) vfs/*.bak
	$(RM) fat/*.o
	$(RM) fat/*.bak
	$(RM) minixfs/*.bak
	$(RM) *.o *.gz
	$(RM) *.bak
	$(RM) $(OBJS)
	$(RM) ext2/*.o
	$(RM) ext2/*.bak	
	$(RM) unicode/*.o	
	$(RM) unicode/*.bak	
	$(RM) romfs/*.o	
	$(RM) romfs/*.bak		
	$(RM) romfs/*.gz		
	$(RM) romfs/*.lst		


