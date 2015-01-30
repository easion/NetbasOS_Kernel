

CROSSCOMPILER=i386-elf-
CC = $(CROSSCOMPILER)gcc
LD=$(CROSSCOMPILER)ld

INCDIR	=$(SDKDIR)/drv/include
CFLAGS	=-g -Wall -W -c -O2 -D__DLL__ -nostdinc -I$(INCDIR) 
OBJS =  sb16.o sb_dma.o\


%.o%.c:
	$(CC) $(CFLAGS) -c -o $@ $<

all:	sb16.dll clean

sb16.dll: $(OBJS) $(MAKEDEP)
	rm -f $@
	$(LD)  -r -o $@ $(OBJS)
	$(CROSSCOMPILER)objdump -a -r  $@ > reloc.lst

clean:  
	rm -f *.bak
	rm -f *.o
	


