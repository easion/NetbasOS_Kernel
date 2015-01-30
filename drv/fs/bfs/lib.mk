AFLAGS   = -c


ARCH=i386-elf-
CC = $(ARCH)gcc
LD = $(ARCH)ld
AR = $(ARCH)ar

RM = rm -f 
NTFSDIR = ./
INCDIR	=$(SDKDIR)/uclibc/include
CFLAGS	=   -finline-functions -fno-builtin -nostdinc   \
 -c -O2 -nostdinc -D__USER_MODE__ -I$(INCDIR) -I$(NTFSDIR)  -I.   -D_NO_INLINE_ASM \
 -DUNSAFE_GET_VNODE  -fno-strict-aliasing -Wno-multichar -fno-exceptions

CXXFLAGS=$(CFLAGS)



OBJS := BlockAllocator.o BPlusTree.o BufferPool.o Debug.o Index.o Inode.o \
		Journal.o Utility.o Volume.o user.o 

%.o%.c:
	$(CC) $(CFLAGS) -c -o $@ $<

all:	 libbfs.a

libbfs.a: $(OBJS) $(MAKEDEP)
	$(AR) rcs $@ $(OBJS)


clean: 

	$(RM) *.o
	$(RM) *.bak
	$(RM) $(OBJS)

	


