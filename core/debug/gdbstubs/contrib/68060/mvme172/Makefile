###############################################################################
# Tools, Rules, and Environment
###############################################################################

CC	= m68k-elf-gcc
AS	= m68k-elf-as
LD	= m68k-elf-ld
OBJCOPY	= m68k-elf-objcopy
RM	= rm -f
ECHO	= echo

MACHINE	 = -m68060
CFLAGS	 = -v -nostdlib -Wall $(MACHINE)

################################################################################
# Targets
###############################################################################

PROG	= \
		stub \
		$(FRG)
	
####### CPU specific, SCC specific and re-usable stub code
OBJ0	= \
	crt0.o \
	main.o \
	gdb.o  \
	gdbints.o \
	gdbcpu32.o \
	z8530lib.o \
	gdb-ser-z8530.o \
	$(FRG)

###############################################################################
# Compile
###############################################################################

all:	$(OBJ0) $(OBJ1) $(OBJ2)

gdb.o: ../gdb.c
	$(CC) -c $(CFLAGS) -I.. ../gdb.c

gdbints.o: ../68k/gdbints.c
	$(CC) -c $(CFLAGS) -I. -I../68k ../68k/gdbints.c

gdbcpu32.o: ../68k/gdbcpu32.c
	$(CC) -c $(CFLAGS) -I. -I.. -I../68k ../68k/gdbcpu32.c

main.o: ../68k/main.c
	$(CC) -c $(CFLAGS) -I. -I.. -I../68k ../68k/main.c

z8530lib.o: ../z8530/z8530lib.c
	$(CC) -c $(CFLAGS) -I. -I../z8530 ../z8530/z8530lib.c

gdb-ser-z8530.o: ../z8530/gdb-ser-z8530.c
	$(CC) -c $(CFLAGS) -I. -I.. -I../z8530 ../z8530/gdb-ser-z8530.c

crt0.o: ../cstart/crt0.S
	$(CC) -c $(CFLAGS) -I../cstart ../cstart/crt0.S -Wa,-ahlms=asm.lst


###############################################################################
# Link
###############################################################################

stub: $(OBJ0)
	$(CC) $(CFLAGS) -O1 -o stub $(OBJ0) -Wl,-Tbootstub.ld,-Map,stub.map
	$(OBJCOPY) -O binary stub.exe stub.abs

###############################################################################
# Cleanup
###############################################################################

clean:
	$(RM) $(OBJ0) $(PROG)
	$(RM) *.abs
	$(RM) *.exe
	$(RM) *.map
	$(RM) *.lst

clobber: clean
	$(RM) *~

