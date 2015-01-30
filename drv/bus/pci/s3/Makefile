
CC = gcc

INCDIR	=/drv/include
CFLAGS	=-g -Wall -W -c -O2 -nostdinc -I$(INCDIR) 
OBJS =  s3.o\


%.o%.c:
	$(CC) $(CFLAGS) -c -o $@ $<

all:	s3.dll clean

s3.dll: $(OBJS) $(MAKEDEP)
	$(LD) -d -r -nostdlib -o $@ $(OBJS)
	del *.bak

clean:  
	del *.bak
	


