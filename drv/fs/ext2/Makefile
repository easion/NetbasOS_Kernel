CROSSCOMPILER=i386-elf-
CC = $(CROSSCOMPILER)gcc
LD = $(CROSSCOMPILER)ld

RM = rm -f 
EXT2DIR = ./
INCDIR	=$(SDKDIR)/drv/include
CFLAGS	=   -finline-functions -fno-builtin -nostdinc   -fno-PIC\
 -c -O2 -nostdinc -D__DLL__ -I$(INCDIR) -I$(EXT2DIR)

NTFSOBJS = $(EXT2DIR)main.o  $(EXT2DIR)bmap.o\
$(EXT2DIR)desc.o $(EXT2DIR)dir.o \
$(EXT2DIR)file.o $(EXT2DIR)imap.o $(EXT2DIR)inode.o\
$(EXT2DIR)namei.o $(EXT2DIR)super.o 

OBJS = $(NTFSOBJS)

%.o%.c:
	$(CC) $(CFLAGS) -c -o $@ $<

all:	 ext2

ext2: $(OBJS) $(MAKEDEP)
	$(RM) *.gz
	$(LD) -nostdlib -d -r -o $@ $(OBJS)
	$(CROSSCOMPILER)objdump  -r  $@ > reloc.lst
	gzip   $@ 

clean: 

	$(RM) *.o
	$(RM) *.bak
	$(RM) $(OBJS)

	


