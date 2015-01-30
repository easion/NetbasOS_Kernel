ROOTDIR=../../../

include $(ROOTDIR)cpu/$(ARCH)_platform.inc


CFLAGS += -I$(ROOTDIR)sys

SRC	= uart.c at91_init.c clock.c aic.c cpu.c serial.c interrupts.c at45.c
OBJ	= $(SRC:.c=.o) 

TEXTBASE= 0x8000
AFLAGS += -DTEXT_BASE=$(TEXTBASE)

all: atmel.o 

atmel.o: $(OBJ) start.o
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
