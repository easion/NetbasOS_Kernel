
CROSSCOMPILER=i386-elf-
CC = $(CROSSCOMPILER)gcc
LD=$(CROSSCOMPILER)ld

INCDIR	=$(SDKDIR)/drv/include

CFLAGS	= -c -fno-PIC -nostdinc -D__NETBAS__ -I$(INCDIR) -I../include -Iinclude -D__netbas__=1




CFLAGS += -DKERNEL_MODULE	# build linux kernel module

#CFLAGS += -Iinclude_e -Iinclude
#CFLAGS += -include ../glue.h	# headers



OBJS=ip_sync.o \
	ip_state.o \
	ip_frag.o \
	ip_auth.o \
	ip_htable.o \
	ip_log.o \
	ip_lookup.o \
	ip_nat.o \
	ip_pool.o \
	ip_scan.o \
	fil.o\
	main.o \
	ip_fil_netbas.o 


	



%.o%.c:
	$(CC) $(CFLAGS) -c -o $@ $<

all:	ipfilter.sys 

ipfilter.sys: $(OBJS) $(MAKEDEP)
	$(LD) -d -r -nostdlib -o $@ $(OBJS)
	$(CROSSCOMPILER)objdump  -t  $@ | grep "*UND*" > reloc.lst
	cp $@ $(SDKDIR)/livecd/iso/system/drivers -fr

clean:  
	rm -f  *.bak *.lst
	rm -f *.o *.dll
	


