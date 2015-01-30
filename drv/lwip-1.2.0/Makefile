
CROSSCOMPILER=i386-elf-
#CROSSCOMPILER=arm-elf-

CC = $(CROSSCOMPILER)gcc
LD=$(CROSSCOMPILER)ld
OBJDUMP = $(CROSSCOMPILER)objdump

LWIPDIR= src
LWIPARCH=jicama

INCDIR	=$(SDKDIR)/drv/include 

CFLAGS	=    -Wall -W -c -O2 -nostdinc -I$(INCDIR) \
-I./ -D__DLL__   -DLWIP_DEBUG

CFLAGS:=$(CFLAGS) \
	-I$(LWIPDIR)/include -I$(LWIPDIR)/arch/$(LWIPARCH)/include -I$(LWIPDIR)/include/ipv4 \

%.o%.o:
	$(CC) $(CFLAGS) -c  $@ $<


COREFILES =  $(LWIPDIR)/core/dhcp.o  $(LWIPDIR)/core/raw.o\
	$(LWIPDIR)/core/mem.o $(LWIPDIR)/core/memp.o $(LWIPDIR)/core/netif.o \
	$(LWIPDIR)/core/pbuf.o $(LWIPDIR)/core/stats.o $(LWIPDIR)/core/sys.o \
        $(LWIPDIR)/core/tcp.o $(LWIPDIR)/core/tcp_in.o \
        $(LWIPDIR)/core/tcp_out.o $(LWIPDIR)/core/udp.o 

CORE4FILES =$(LWIPDIR)/core/ipv4/icmp.o $(LWIPDIR)/core/ipv4/ip.o \
	$(LWIPDIR)/core/inet.o $(LWIPDIR)/core/ipv4/ip_addr.o $(LWIPDIR)/core/ipv4/ip_frag.o

SNMPFILES =$(LWIPDIR)/core/snmp/msg_in.o $(LWIPDIR)/core/snmp/msg_out.o \
	$(LWIPDIR)/core/snmp/asn1_enc.o $(LWIPDIR)/core/snmp/asn1_dec.o $(LWIPDIR)/core/snmp/mib2.o $(LWIPDIR)/core/snmp/mib_structs.o


NETIFFILES =$(LWIPDIR)/netif/loopif.o \
	$(LWIPDIR)/netif/etharp.o $(LWIPDIR)/netif/ethernetif.o

ARCHFILES = $(LWIPDIR)/arch/$(LWIPARCH)/sys_jos.o\
$(LWIPDIR)/arch/$(LWIPARCH)/netif/unixif.o\
$(LWIPDIR)/arch/$(LWIPARCH)/lib_arch.o\
$(LWIPDIR)/arch/$(LWIPARCH)/main.o\
$(LWIPDIR)/arch/$(LWIPARCH)/module.o\
$(LWIPDIR)/arch/$(LWIPARCH)/sockets.o


APIFILES = $(LWIPDIR)/api/err.o   $(LWIPDIR)/api/sockets.o   $(LWIPDIR)/api/tcpip.o  \
 $(LWIPDIR)/api/api_lib.o   $(LWIPDIR)/api/api_msg.o  

APPDIR = ./apps/




#IGMPFILES =$(LWIPDIR)/igmp/igmp.o 


all:	lwip # httpd

lwip: $(COREFILES) $(CORE4FILES)  $(NETIFFILES) $(ARCHFILES)   $(APIFILES)  $(SNMPFILES)  $(APPFILES)  $(IGMPFILES)
	$(LD)   -d -r -nostdlib -o $@ $(COREFILES)  $(CORE4FILES)  $(NETIFFILES) $(ARCHFILES) $(APPFILES)    $(APIFILES)    $(IGMPFILES)
	$(OBJDUMP) -a -r  $@ > reloc.lst
	gzip -f  -S.gz $@
	cp $@.gz $(SDKDIR)/livecd/iso/system/servers -fr

INC_BASE = $(SDKDIR)/bin
STRIP = $(ARCH_C)strip
CRT0 = $(INC_BASE)/lib/crt0.o
CRTI = $(INC_BASE)/lib/crti.o
//CFLAGS += -nostdlib -nostdinc  -I$(SDKDIR)/bin/include/
LCFLAG =  -nostdlib -nostdinc   -Ttext 0x1000  
LINKLIB = -L$(INC_BASE)/lib  -lfreetype    -lstdc -lgcc


clean:  
	rm  -f *.bak *.dll  $(APPFILES)
	rm -f *.o $(COREFILES) $(CORE4FILES) $(SNMPFILES) $(NETIFFILES) $(ARCHFILES)   $(APIFILES)  
	


