CROSSCOMPILER=i386-elf-
CC = $(CROSSCOMPILER)gcc
CXX = $(CROSSCOMPILER)g++
LD = $(CROSSCOMPILER)ld

RM = rm -f 
ROMFSDIR = ./
INCDIR	=$(SDKDIR)/drv/include
CFLAGS	=  -Wno-multichar -fno-strict-aliasing -nostdinc    -c -O2 -nostdinc -D__DLL__ -I$(INCDIR)   -I. 

CXXFLAGS = $(CFLAGS)

OBJS := dir.o dlist.o fat.o iter.o util.o encodings.o EncodingConversions.o UnicodeMappings.o version.o attr.o\
	file.o vcache.o dosfs.o




%.o%.c:
	$(CC) $(CFLAGS) -c -o $@ $<


%.o%.cpp:
	$(CXX) $(CXXFLAGS) -c -o $@ $<

all:	 dosfs.sys

dosfs.sys: $(OBJS) $(MAKEDEP)
	$(LD) -nostdlib -d -r -o $@ $(OBJS)
	$(CROSSCOMPILER)objdump  -t  $@ | grep "*UND*" > reloc.lst
	cp $@ $(SDKDIR)/livecd/iso/system/drivers -fr


clean: 

	$(RM) *.o
	$(RM) *.bak
	$(RM) $(OBJS)

	


