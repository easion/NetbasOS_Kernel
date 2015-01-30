
CC = gcc

INCDIR	=/drv/include
CFLAGS	=-g -Wall -W -c -O2 -nostdinc -D__DLL__ -I$(INCDIR) 
OBJS =  fdc.o fd.o\


%.o%.c:
	$(CC) $(CFLAGS) -c -o $@ $<

all:	fdc.dll clean

fdc.dll: $(OBJS) $(MAKEDEP)
	del $@
	$(LD) -d -r -nostdlib -o $@ $(OBJS)
	objdump -a -r  $@ > reloc.lst
	del *.o

clean:  
	del *.bak
	


