

#ifndef __NETWORK_PKT_H__
#define __NETWORK_PKT_H__


#define PBUF_POOL_BUFSIZE       256
#define PBUF_POOL_BLOCKS       256
#define PKT_QUEUE_BLOCKS       32

typedef struct mypacket
{
   struct mypacket  *parent;
   struct mypacket  *rest;
   struct mypacket  *next;

   char flags;
   char *base;
   char   *data;
   char   *tail;
   char   *end;
   int len;
   int rlen;
}packet_t;


typedef struct 
{
	char flags;
   packet_t *head;
   packet_t *tail;
   int     count;
}packetqueue_t;


char * pbuf_get_1(int len);
void pbuf_free_1(char *p, int len);

int pkt_size(packet_t *p);
packet_t *new_pkt(void *content, int len, packet_t *parent, packet_t *rest);
packet_t *new_pkt2(int len, packet_t *parent, packet_t *rest);

int new_mark_pkt(int mark, packet_t *p);
void free_pkt(packet_t *p);
packetqueue_t *new_pktque(void);
void queue_pkt_add(packetqueue_t *pq, packet_t *p);
packet_t *queue_pkt_del(packetqueue_t *pq);
void queue_pkt_free(packetqueue_t *pq);
int queue_pkt_empty(packetqueue_t *pq);
void *pkt_data(packet_t *pkt);
void pkt_move(packet_t *pkt, int len);

#endif
