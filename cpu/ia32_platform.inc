#
# Jicama OS Makefile
#CROSSCOMPILER=i386-elf-

AS = yasm -f elf
CC = $(CROSSCOMPILER)gcc
#CPP = gxx
LD = $(CROSSCOMPILER)ld -g
STRIP = $(CROSSCOMPILER)strip
NM = $(CROSSCOMPILER)nm
OBJDUMP = $(CROSSCOMPILER)objdump
RM = rm -f
CP = cp
INCL   = ./include

CPU_DIR=cpu/i386/
CPU_OBJ=$(CPU_DIR)/cpu.o
ACTION=gzip kernel
LD32      = $(CROSSCOMPILER)ld

LDFLAGS = -T $(CPU_DIR)/kernel.lds -s -nostartfiles -nostdlib -Map kernel.map 
MYLDFLAGS = -T $(CPU_DIR)/kernel.lds  -nostdlib -Map kernel.map 
CFLAGS= -march=i386 -Wall -O -finline-functions -fno-builtin -nostdinc -g -I$(INCL) -D__IA32__  -D__ENABLE_ASSERT__ -DENABLE_GDB #-D__ENABLE_TRACE__

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


ifeq ($(MACHINE),athlon-xp)
	CFLAGS += -march=athlon-xp -mmmx -msse -m3dnow
	DEFS += -DCONFIG_FENCES_P3
	CONFIG_SMP = n
	CONFIG_HT = n
endif

ifeq ($(MACHINE),athlon-mp)
	CFLAGS += -march=athlon-mp -mmmx -msse -m3dnow
	DEFS += -DCONFIG_FENCES_P3
	CONFIG_HT = n
endif

ifeq ($(MACHINE),pentium3)
	CFLAGS += -march=pentium3 -mmmx -msse
	DEFS += -DCONFIG_FENCES_P3
	CONFIG_HT = n
endif

ifeq ($(MACHINE),prescott)
	CFLAGS += -march=pentium4 -mfpmath=sse -mmmx -msse -msse2 -msse3
	DEFS += -DCONFIG_FENCES_P4
endif

ifeq ($(MACHINE),pentium4)
	CFLAGS += -march=pentium4 -mfpmath=sse -mmmx -msse -msse2
	DEFS += -DCONFIG_FENCES_P4
endif

CFLAGS += -s -Werror-implicit-function-declaration -Wundef -Wstrict-prototypes -Wno-trigraphs   -msoft-float

CFLAGS1 +=    -fomit-frame-pointer     

#CFLAGS +=-fno-strict-aliasing -fno-common    -maccumulate-outgoing-args  -Os  -mregparm=3 -march=i586 -ffreestanding  -freg-struct-return -mpreferred-stack-boundary=2
#CFLAGS += -mtune=generic -mtune=generic -pipe -Wno-pointer-sign -Wdeclaration-after-statement -fno-stack-protector

