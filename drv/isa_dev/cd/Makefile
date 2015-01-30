
CC = gcc

INCDIR	=$(SDKDIR)/drv/include
CFLAGS	=-g -Wall -W -c -O2 -nostdinc -I$(INCDIR) 
OBJS =  atapi.o dll.o\


%.o%.c:
	$(CC) $(CFLAGS) -c -o $@ $<

all:	atapi.dll clean

atapi.dll: $(OBJS) $(MAKEDEP)
	del $@
	$(LD) -d -r -nostdlib -o $@ $(OBJS)
	objdump -a -r  $@ > reloc.lst

clean:  
	del *.bak
	


