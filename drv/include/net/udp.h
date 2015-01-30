
#ifndef UDP_H
#define UDP_H

#define UDP_HLEN 8

//#pragma pack(push)
//#pragma pack(1)

struct udp_hdr 
{
  unsigned short src;          // Source port number 
  unsigned short dest;         // Destination port number
  unsigned short len;          // Length
  unsigned short chksum;       // Checksum
}__attr_packet;

//#pragma pack(pop)

#define UDP_FLAGS_NOCHKSUM  0x01
#define UDP_FLAGS_BROADCAST 0x02
#define UDP_FLAGS_CONNECTED 0x04

struct udp_pcb 
{
  struct udp_pcb *next;

  struct ip_addr local_ip, remote_ip;
  unsigned short local_port, remote_port;
  
  int flags;
  
  err_t (*recv)(void *arg, struct udp_pcb *pcb, struct pbuf *p, struct ip_addr *addr, unsigned short port);
  void *recv_arg;  
};

// Socket layer interface to the UDP code

struct udp_pcb *udp_new();
void udp_remove (struct udp_pcb *pcb);
err_t udp_bind(struct udp_pcb *pcb, struct ip_addr *ipaddr, unsigned short port);
err_t udp_connect(struct udp_pcb *pcb, struct ip_addr *ipaddr, unsigned short port);
void udp_recv(struct udp_pcb *pcb, err_t (*recv)(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, unsigned short port), void *recv_arg);
err_t udp_send(struct udp_pcb *pcb, struct pbuf *p, struct ip_addr *dst_ip, unsigned short dst_port, struct netif *netif);

#define udp_flags(pcb) ((pcb)->flags)
#define udp_setflags(pcb, f) ((pcb)->flags = (f))

// Lower layer interface to UDP

err_t udp_input(struct pbuf *p, struct netif *inp);
void udp_init();

#endif
