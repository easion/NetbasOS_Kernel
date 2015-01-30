
CC = gcc

INCDIR	=$(SDKDIR)/drv/include
CFLAGS	=-g -Wall -W -c -O2 -nostdinc -I$(INCDIR) 
OBJS = ramdisk.o stdlib.o

%.o%.c:
	$(CC) $(CFLAGS) -c -o $@ $<

all:	ramdisk.dll clean

ramdisk.dll: $(OBJS) $(MAKEDEP)
	$(LD) -d -r -nostdlib -o $@ $(OBJS)
	del *.bak

clean:  
	del *.bak
	


