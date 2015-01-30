
#ifndef __ETH_DRV__H__
#define __ETH_DRV__H__
#include <drv/log.h>
#define ETHER_ADDR_LEN 6
struct eth_addr 
{
  unsigned char addr[ETHER_ADDR_LEN];
};
enum{
	IF_STOP,
	IF_RUNNING
};

struct netdev_link
{
	int flags;
	char devname[16];
	struct netdev_link *next_link;
};


/*************************************************/
#define my_htons(__N) ( (((__N) >> 8) & 0xFF) | (((__N) & 0xFF) << 8) )
#define my_htonl(__N) ( (((__N) >> 24) & 0xFF) | (((__N) >> 8) & 0xFF00) \
			| (((__N) & 0xFF00) << 8) | (((__N) & 0xFF) << 24) )

#define IP_PROTO_ICMP 1
#define IP_PROTO_UDP 17
#define IP_PROTO_UDPLITE 170
#define IP_PROTO_TCP 6

typedef struct 
{
    u8_t  dst[6];
    u8_t  src[6];    
    u16_t type;    
} eth_t;

typedef struct 
{
   u8_t     verlen;
   u8_t     tos;
   u16_t    len;
   u16_t    id;
   u16_t    frag;
   u8_t     ttl;
   u8_t     type;
   u16_t    checksum;
   u8_t src[4];
   u8_t dst[4];
}ip_t;

typedef struct 
{
	u16_t arp_hard_type; //Format of hardware address.
	u16_t arp_proto_type; //Format of protocol address.
	u8_t  arp_hard_size; //Length of hardware address.
	u8_t  arp_proto_size; //Length of protocol address.

	u16_t arp_op; //ARP operation code (command).
	u8_t  arp_eth_source[6]; //Hardware source address.
	u8_t src[4]; //IP source address.
	u8_t  arp_eth_dest[6];  //Hardware destination address.
	u8_t dst[4];  // IP destination address.
} __attribute__ ((packed)) arp_t;




static inline  unsigned short ntohs(unsigned short x)
{
   __asm ("xchgb %b0,%h0" : "=q"(x) : "0"(x));
   return x;
}


static inline void dump_ip(ip_t *p_ip)
{
#ifdef SIMPLE
   syslog(LOG_DEBUG,"IP version=%d,  tos=0x%02X, len=%d\n",
	   p_ip->verlen >> 4,  p_ip->tos, ntohs(p_ip->len));
   syslog(LOG_DEBUG," id=0x%04X, frag=0x%04X, ttl=%d, type=%d,",
	 ntohs(p_ip->id),  ntohs(p_ip->frag), p_ip->ttl, p_ip->type,
	   ntohs(p_ip->checksum));
#else
   syslog(LOG_DEBUG,"IP Version=%d,  type=%d, id=%d ",
	   p_ip->verlen >> 4,  p_ip->type, ntohs(p_ip->id));
#endif
   syslog(LOG_DEBUG,"   src=%d.%d.%d.%d, dst=%d.%d.%d.%d\n",
	   p_ip->src[0], p_ip->src[1], p_ip->src[2], p_ip->src[3],
	   p_ip->dst[0], p_ip->dst[1], p_ip->dst[2], p_ip->dst[3]);
}

static inline void dump_arp(arp_t *p_arp)
{
	   syslog(LOG_DEBUG,"ARP type:%d   src=%d.%d.%d.%d, dst=%d.%d.%d.%d\n",
		   p_arp->arp_hard_type,
	   p_arp->src[0], p_arp->src[1], p_arp->src[2], p_arp->src[3],
	   p_arp->dst[0], p_arp->dst[1], p_arp->dst[2], p_arp->dst[3]);
}

static inline void dump_packet(char *packet)
{
	eth_t *p_eth = (eth_t *)packet;
	char *ptr = (ip_t *)(packet+sizeof(eth_t));

	if (p_eth->type == my_htons(0x800)){
		dump_ip((ip_t*) ptr);
	}
	else	if (p_eth->type == my_htons(0x806)){
		dump_arp((arp_t*) ptr);
	}
	else
	{
		printk("dump type is %x\n", my_htons(p_eth->type));
		return ;
	}

}

/*
static inline unsigned char inp8(unsigned long port) {

    unsigned char ret;
    asm volatile ("inb %%dx, %%al": "=a"(ret): "d"(port));
    return ret;
}

static inline void outp8(unsigned long port, unsigned char value) {
   asm volatile ("outb %%al, %%dx": :"d" (port), "a" (value));
}

static inline unsigned short inpw(unsigned long port) {

    unsigned short ret;
    asm volatile ("inw %%dx, %%ax": "=a"(ret): "d"(port));
    return ret;
}

static inline void outpw(unsigned long port, unsigned short value) {
   asm volatile ("outw %%ax, %%dx": :"d" (port), "a" (value));
}

static inline unsigned long inpd(unsigned long port) {

    unsigned long ret;
    asm volatile ("inl %%dx, %%eax": "=a"(ret): "d"(port));
    return ret;
}

static inline void outpd(unsigned long port, unsigned long value) {
   asm volatile ("outl %%eax, %%dx": :"d" (port), "a" (value));
}*/

void sendsig(void *rp, int signo);
int sigdelset(void *rp, int signo);
int sigrecv(void *rp, char*);


#endif
