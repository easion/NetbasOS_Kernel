#
# Jicama OS Makefile
CROSSCOMPILER=arm-elf-

AS = $(CROSSCOMPILER)gcc
CC = $(CROSSCOMPILER)gcc
#CPP = gxx
LD = $(CROSSCOMPILER)ld
RM = rm -f
CP = cp
INCL   = ./include
LD32      = $(CROSSCOMPILER)ld
CPU_DIR=./cpu/arm/
CPU_OBJ=$(CPU_DIR)cpu.o
ACTION=cp -f kernel ./sim

TEXTBASE= 0x30004000
LDFLAGS =-static  -T $(CPU_DIR)/arm.lds \
-nostartfiles -nostdlib -Map kernel.map  

CFLAGS=  -Wall -O -finline-functions -fno-builtin\
-nostdinc -g  -I$(INCL) -D__ARM__  -D__DLL__ 

%.o%.c:
	$(CC) $(CFLAGS) -c -o $@ $<

OBJECTS = $(SOURCES:.cpp=.o)
OBJECTS = $(SOURCES:.asm=.o)
.SUFFIXES: .cpp .o
.SUFFIXES: .asm .o

.cpp.o:
	$(CXX) $(CFLAGS) -c -o $@ $<

%.o%.asm:
	$(AS) -o $@ $<

