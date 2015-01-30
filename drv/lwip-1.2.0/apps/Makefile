
OBJDUMP = i386-elf-objdump
CC = i386-elf-gcc
LD = i386-elf-ld
LWIPDIR= ../src
LWIPARCH=jicama

INCDIR	=$(SDKDIR)/drv/include 

CFLAGS	=-g    -Wall -W -c -O2 -nostdinc -I$(INCDIR) \
-I./ -D__DLL__   -DLWIP_DEBUG


ARESDIR=./ares-1.1.1-lwip
APPDIR = ./

CFLAGS:=$(CFLAGS) \
	-I$(LWIPDIR)/include -I$(LWIPDIR)/arch/$(LWIPARCH)/include -I$(LWIPDIR)/include/ipv4 \
	-I. -I. -I./httpd -I$(ARESDIR)

%.o%.o:
	$(CC) $(CFLAGS) -c  $@ $<

APPFILES = 	./udpecho.o ./tcpecho.o \
	./httpd/fsdata.o\
	./httpd/fs.o ./httpd/httpd.o\
	./main.o 

DNSFILES = dns.o 

ARESFILES=$(ARESDIR)/ares__close_sockets.o $(ARESDIR)/ares_compat.o $(ARESDIR)/ares_fds.o \
	$(ARESDIR)/ares_gethostbyaddr.o $(ARESDIR)/ares_gethostbyname.o $(ARESDIR)/ares_init.o \
	$(ARESDIR)/ares_process.o $(ARESDIR)/ares_search.o $(ARESDIR)/ares_strerror.o $(ARESDIR)/ares_timeout.o \
	$(ARESDIR)/ares_expand_name.o $(ARESDIR)/netdb.o $(ARESDIR)/ares_query.o	$(ARESDIR)/ares_parse_a_reply.o \
	$(ARESDIR)/ares_destroy.o $(ARESDIR)/ares_send.o $(ARESDIR)/ares_parse_ptr_reply.o $(ARESDIR)/ares_free_hostent.o \
	$(ARESDIR)/ares_mkquery.o $(ARESDIR)/ares_free_string.o $(ARESDIR)/ares_free_errmem.o

all:	 khttpd dns

dns: $(DNSFILES) $(ARESFILES)
	$(LD)  -r -o $@     $(DNSFILES) $(ARESFILES) 
	gzip -f  -S.gz $@
	cp $@.gz $(SDKDIR)/livecd/iso/system/servers -fr

khttpd: $(APPFILES)
	$(LD)  -r -o $@     $(APPFILES) 
	gzip -f  -S.gz $@
	cp $@.gz $(SDKDIR)/livecd/iso/system/servers -fr

clean:  
	rm  -f *.bak *.dll  $(APPFILES)
	rm -f *.o $(COREFILES) $(CORE4FILES) $(SNMPFILES) $(NETIFFILES) $(ARCHFILES)   $(APIFILES)  
	


