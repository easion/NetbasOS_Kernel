AFLAGS   = -c


CROSSCOMPILER=i386-elf-
CC = $(CROSSCOMPILER)gcc
LD = $(CROSSCOMPILER)ld

RM = rm -f 
NTFSDIR = ./
INCDIR	=$(SDKDIR)/drv/include
CFLAGS	=   -finline-functions -fno-builtin -nostdinc   -fno-PIC\
 -c -O2 -nostdinc -D__NETBAS__ -I$(INCDIR) -I$(NTFSDIR)  -I.   -D_NO_INLINE_ASM \
 -DUNSAFE_GET_VNODE  -fno-strict-aliasing -Wno-multichar -fno-exceptions

CXXFLAGS=$(CFLAGS)



OBJS := BlockAllocator.o BPlusTree.o BufferPool.o Debug.o Index.o Inode.o \
		Journal.o Utility.o Volume.o netbas.o kernel_interface.o

%.o%.c:
	$(CC) $(CFLAGS) -c -o $@ $<

all:	 bfs

bfs: $(OBJS) $(MAKEDEP)
	$(RM) *.gz
	$(LD) -nostdlib -d -r -o $@ $(OBJS)
	$(CROSSCOMPILER)objdump  -t  $@ | grep "*UND*" > reloc.lst
	gzip   $@ 
	cp *.gz $(SDKDIR)/livecd/iso/system/servers

clean: 

	$(RM) *.o
	$(RM) *.bak
	$(RM) $(OBJS)

	


