ROOTDIR=../

include $(ROOTDIR)cpu/$(ARCH)_platform.inc

CFLAGS += -I$(ROOTDIR)sys 

#CFLAGS +=  -DPPP_SUPPORT=1

PPPOBJ=\
	ppp/auth.o\
	ppp/chap.o\
	ppp/chpms.o\
	ppp/fsm.o\
	ppp/ipcp.o\
	ppp/lcp.o\
	ppp/magic.o\
	ppp/md5.o\
	ppp/pap.o\
	ppp/ppp.o\
	ppp/randm.o\
	ppp/vj.o



OBJS+=$(PPPOBJ)

OBJS += netmain.o ip.o dhcp.o dns.o ether.o arp.o  icmp.o\
	inet.o ipaddr.o loopif.o netif.o pbuf.o  misc.o\
	stats.o tcp.o tcp_input.o tcp_output.o tcpsock.o\
	udp.o udpsock.o raw.o rawsock.o unixsock.o socket.o\
	nf_hook.o nf_sockopt.o route.o



all:	net.o 



net.o:	$(OBJS) 
	$(LD) -r -o $@ $(OBJS)


clean:
	$(RM) *.bak *.o
	$(RM) ipf/*.bak 
	$(RM)  $(OBJS)$(LIBS)

