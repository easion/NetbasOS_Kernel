
AIREBIN	= ../../bin
AIREENV	= ../../env
AIRELIB	= ../../lib
AIRESYS	= ../../sys
AIRETOOLS	= ../../tools

AR	= ar
AS32	= nasm
AS64	= yasm -m amd64
AF32	= -f win32
AF64	= -f elf
CC32	= gcc -m32 -D_MONE32_
CC64	= xgcc -m64
CF32	= -falign-functions=2  -momit-leaf-frame-pointer -fomit-frame-pointer -mpush-args -Os
CF64	= -z nodefaultlib -mpush-args -falign-functions=2  -Os
CFBASE	= -Wall -mno-red-zone -nostdlib -fno-exceptions -fno-builtin -fno-rtti -c
LD32	= /bin/ld
LD64	= ld
LF32	= -e _mainCRTStartup
LF64	= -e mainCRTStartup -T $(AIREENV)/link64.txt
LFBASE	=
MAKE	= make
BIM2BIN	= $(AIRETOOLS)/bim2bin.exe
ELF2COM	= $(AIRETOOLS)/elf2com.exe
BINDCOM	= $(AIRETOOLS)/bindcom.exe
OPCAT	= $(AIRETOOLS)/opcat.exe
LIB64	= $(AIRELIB)/aire64.a
CRT64	= $(AIRELIB)/crt64.o
LIB32	= $(AIRELIB)/aire32.a
CRT32	= $(AIRELIB)/crt32.o

 #x86_64