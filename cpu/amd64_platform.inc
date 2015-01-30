#
# Jicama OS AMD64 Makefile

CROSSCOMPILER = /jicamasdk/z_tools/
INCL   = ./sys
CFINCLUDE = -I$(INCL)

AR      = $(CROSSCOMPILER)ar
AS      = $(CROSSCOMPILER)as
CC = $(CROSSCOMPILER)xgcc
LD64 = $(CROSSCOMPILER)ld64
LD = $(CROSSCOMPILER)ld64
OBJCOPY = $(CROSSCOMPILER)objcopy
ELF2COM  = $(CROSSCOMPILER)elf2com
BIM2BIN  = $(CROSSCOMPILER)bim2bin

CXXFBASE  = -Wall -mno-red-zone -nostdlib -nostdinc -fno-exceptions -fno-builtin -fno-rtti -c -O3

C64FLAGS=    -m64 -z nodefaultlib -mpush-args -falign-functions=2 
CFBASE  = -Wall -mno-red-zone -nostdlib -nostdinc -fno-exceptions -fno-builtin   -O3

CFLAGS  = $(C64FLAGS) $(CFBASE)  $(CFINCLUDE) -D__AMD64__

LF32    = $(CROSSCOMPILER)ld32 -e ___crtLeaf32
LFLAGS64    =  -nostartfiles -nostdlib  -Ttext 0x400000  -e start -Map kernel.map  --gc-sections
LFLAGS64BIN    = -A elf64-x86-64 -T./sys/amd64/kernel.ld  -nostartfiles   -Map kernel.map 


#-T ../../link64.txt

MAKE    = make64
DELE    = rm -f 
COPY    = cp
CAT     = cat

OBJECTS = $(SOURCES:.cpp=.o)
OBJECTS = $(SOURCES:.asm=.o)
.SUFFIXES: .cpp .o
.SUFFIXES: .asm .o
.SUFFIXES: .S .o

.cpp.o:
	$(xgcc) $(CFLAGS) -c $<

.S.o:
	xgcc $(CFLAGS) -c $<

.asm.o:
	yasm -m amd64 -f elf $<
#	fasm $<

%.o%.c:
	$(CC) $(CFLAGS)  -c -o $@ $<
