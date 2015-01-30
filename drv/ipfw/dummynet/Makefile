
CROSSCOMPILER=i386-elf-
CC = $(CROSSCOMPILER)gcc
LD=$(CROSSCOMPILER)ld

INCDIR	=$(SDKDIR)/drv/include

CFLAGS	= -c -fno-PIC -nostdinc -D__NETBAS__ -I$(INCDIR) -I../include -Iinclude

CFLAGS += -DBYTE_ORDER=LITTLE_ENDIAN -DIPFIREWALL_DEFAULT_TO_ACCEPT -D_KERNEL -DNO_LOGGING # -DNO_USE_SOCKETS -DNO_FW_PUNCH
# _BSD_SOURCE enables __FAVOR_BSD (udp/tcp bsd structs instead of posix)
CFLAGS += -D_BSD_SOURCE -D__LITTLE_ENDIAN
CFLAGS += -DKERNEL_MODULE	# build linux kernel module
# the two header trees for empty and override files
CFLAGS += -Iinclude_e -Iinclude
CFLAGS += -include ../glue.h	# headers


OBJS =  in_cksum.o ip_dummynet.o ip_divert.o ip_fw2.o ip_fw_pfil.o\
	 bsd_compat.o ipfw2_mod.o ip_fw_nat.o \
	 alias.o alias_db.o alias_proxy.o alias_ftp.o\
	 alias_nbt.o alias_util.o
	
 # O_TARGET := ipfw_mod.o


%.o%.c:
	$(CC) $(CFLAGS) -c -o $@ $<

all:	ipfw.sys 

ipfw.sys: $(OBJS) $(MAKEDEP)
	$(LD) -d -r -nostdlib -o $@ $(OBJS)
	$(CROSSCOMPILER)objdump  -t  $@ | grep "*UND*" > reloc.lst
	cp $@ $(SDKDIR)/livecd/iso/system/drivers -fr

clean:  
	rm -f  *.bak *.sys
	rm -f *.o *.dll
	


