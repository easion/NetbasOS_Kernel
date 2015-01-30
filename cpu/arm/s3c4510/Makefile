ROOTDIR=../../../

include $(ROOTDIR)cpu/$(ARCH)_platform.inc


CFLAGS += -I$(ROOTDIR)sys

SRC	= cpu.c interrupt.c serial.c
OBJ	= $(SRC:.c=.o) #context.o

AFLAGS += -DTEXT_BASE=$(TEXTBASE)

all: s3c4510.o 

s3c4510.o: $(OBJ) start.o
	$(LD)  -r -d -o $@ start.o $(OBJ)


start.o:
	$(CC) $(AFLAGS) -c start.S 

clean :
	$(RM) *.o *~ *.bak
	$(RM) .depend

dep : .depend

include .depend

.depend: $(SRC)
	$(CC) $(CFLAGS) -M $^ > $@
