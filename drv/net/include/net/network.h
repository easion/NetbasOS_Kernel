

#ifndef NETWORK_H
#define NETWORK_H

typedef struct mypacket
{
   unsigned char   *data;
   int len;
   struct mypacket  *parent;
   struct mypacket  *rest;
   struct mypacket  *next;
}packet_t;

typedef struct 
{
   packet_t *head;
   packet_t *tail;
   int     count;
}packetqueue_t;

u16_t packetChecksum(packet_t *p, int all);
int send_ippacket(in_addr_t dst, packet_t *dp, u8_t type);
packet_t *makeDataPacket(void *data, int len);
packet_t *copyPacket(packet_t *p, packet_t *newParent);
void dumpPacketData(packet_t *p);
int packet_length(packet_t *p);
packet_t *packet_alloc(void *content, int len, packet_t *parent, packet_t *rest);
packet_t *packet_mkplus(int len, packet_t *parent, packet_t *rest);
void packet_hdr_split(int mark, packet_t *p);
void packet_freeslot(packet_t *p);
packetqueue_t *queue_alloc_packet(void);
void queue_add_packet(packetqueue_t *pq, packet_t *p);
packet_t *queue_del_packet(packetqueue_t *pq);
void queue_freeslot(packetqueue_t *pq);



//! Network to host conversion for a word.
	#define ntohs(n) ( (((n) & 0xFF00) >> 8) | (((n) & 0x00FF) << 8) )
	//! Host to network conversion for a word.
	#define htons(n) ( (((n) & 0xFF00) >> 8) | (((n) & 0x00FF) << 8) )
	//! Network to host conversion for a double word.
	#define ntohl(n) ( (((n) & 0xFF000000) >> 24) | (((n) & 0x00FF0000) >> 8) \
		| (((n) & 0x0000FF00) << 8) | (((n) & 0x000000FF) << 24) )
	//! Host to network conversion for a double word.
	#define htonl(n) ( (((n) & 0xFF000000) >> 24) | (((n) & 0x00FF0000) >> 8) \
		| (((n) & 0x0000FF00) << 8) | (((n) & 0x000000FF) << 24) )


#define ARP(_p_) (*(arp_t *)(_p_)->data)
#define ETH(_p_) (*(ethernet_t *)(_p_)->data)
#define ICMP(_p_) (*(icmp_ping_t *)(_p_)->data)
#define IP(_p_) (*(ip_t *)(_p_)->data)
#define TCP(_p_) (*(HeaderTCP *)(_p_)->data)
#define UDP(_p_) (*( udp_t *)(_p_)->data)

#define ARP_DEBUG

//#define setIPAddress(_a_, _w_, _x_, _y_, _z_) \
  // ( (_a_).q[0] = (_w_), (_a_).q[1] = (_x_), (_a_).q[2] = (_y_), (_a_).q[3] = (_z_) )

#define makeIPAddress(_w_, _x_, _y_, _z_) \
   ( (u32_t)((_w_) | (_x_) << 8 | (_y_) << 16 | (_z_) << 24) )


#define setHWAddress(_a_, _u_, _v_, _w_, _x_, _y_, _z_) \
   ( (_a_)[0] = (_u_), (_a_)[1] = (_v_), (_a_)[2] = (_w_), \
	(_a_)[3] = (_x_), (_a_)[4] = (_y_), (_a_)[5] = (_z_) )



#endif
