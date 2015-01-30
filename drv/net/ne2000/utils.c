#include <drv/defs.h>
#include <drv/drv.h>
#include <drv/sym.h>
#include <drv/ia.h>
#include <drv/pci.h>
#include <drv/errno.h>
#include <net/pkt.h>
#include "ne2000.h"
#if 0
//////////////////////////////////////////
packet_t packet_pool[PBUF_POOL_BLOCKS];
packetqueue_t packque[PKT_QUEUE_BLOCKS];

char pbuf_bitmap[PBUF_POOL_BLOCKS];
static char *pbuf_pool;


static int packet_used;

int pbuf_init()
{
	int i;
	packet_t *p;

	pbuf_pool = (char *)mm_malloc(PBUF_POOL_BLOCKS * PBUF_POOL_BUFSIZE );

	if (!pbuf_pool)
	{
		kprintf("packet_pool alloc failed\n");
		return -1;
	}

	p = &packet_pool[0];
	for (i = 0; i < PBUF_POOL_BLOCKS; i++)
	{
		pbuf_bitmap[i] = 0;
		p->flags = 0;
		p++;
	}

	for (i=0; i<PKT_QUEUE_BLOCKS; i++)
	{
		packque[i].flags = 0;
	}

	packet_used = 0;
	return 0;
}

static packetqueue_t *alloc_pktque()
{
	int i;
	for (i=0; i<PKT_QUEUE_BLOCKS; i++)
	{
		if (packque[i].flags == 0)
		{
			packque[i].flags = 1;
			return &packque[i];
		}
	}
	printk("alloc_pktque null\n");
	return NULL;
}

void packetque_free(packetqueue_t *pq)
{
	pq->flags = 0;
}

packet_t * packet_get(void *pdata, int len)
{
	int i;
	packet_t *ptmp = &packet_pool[0];

	for (i = 0; i < PBUF_POOL_BLOCKS; i++)
	{
		if(ptmp->flags != 0){
			ptmp++;
			continue;
		}

		ptmp->len = len;

		if (pdata)
		{
		ptmp->flags = 2;
		ptmp->base = pdata;
		ptmp->data = ptmp->base ;
		}
		else
		{
		ptmp->flags = 1;
		}


		packet_used ++;
		return ptmp;
	}
	
	return 0;
}




char * pbuf_get_1(int len)
{
	int i, j;
	int blks = (len+PBUF_POOL_BUFSIZE-1)/PBUF_POOL_BUFSIZE;

	if (blks>1)
	{
		//printk("alloc %d bytes\n", len);
	}

	for (i = 0; i < PBUF_POOL_BLOCKS; i++)
	{
		if (pbuf_bitmap[i] == 0)
		{

			for (j=0; j<blks; j++){
				if (pbuf_bitmap[i+j] != 0)	{
					break;
				}
			}

			if(j==blks){
				for (j=0; j<blks; j++){
					pbuf_bitmap[i+j] =1;
				}
				return ((char*)pbuf_pool + (i*PBUF_POOL_BUFSIZE));
			}

		}
	}
	printk("pbuf_get null\n");
	return NULL;
}

void pbuf_free_1(char *p, int len)
{
	int i, j;
	int blks = (len+PBUF_POOL_BUFSIZE-1)/PBUF_POOL_BUFSIZE;

	for (i = 0; i < PBUF_POOL_BLOCKS; i++)
	{		
			if((pbuf_pool + (i*PBUF_POOL_BUFSIZE)) == p){
				//printk("find and free\n");
				for (j=0; j<blks; j++){
					 pbuf_bitmap[i+j] = 0;
				}
			}	
	}

}

void packet_free(packet_t *p)
{
	if(p->flags==1)
		pbuf_free_1(p->base, p->len);
	p->flags = 0;
	packet_used --;
	return ;
}

int pkt_size(packet_t *p)
{
   int result = 0;
   while ( p )
   {
      result += p->len;
      p = p->rest;
   }
   return result;
}


packet_t *new_pkt(void *content, int len, packet_t *parent, packet_t *rest)
{
   packet_t *p = packet_get(content, len);
   if (p == NULL)
   {
	   return NULL;
   }

   if (content == NULL)
   {
		p->base =p->data  = pbuf_get_1(len);
   }

   p->parent = parent;
   p->rest   = rest;
   return p;
}

packet_t *new_pkt2(int len, packet_t *parent, packet_t *rest)
{
   packet_t *p = packet_get(NULL, len);
   p->base =p->data  = pbuf_get_1(len);
   //p->len    = len;
   if (( p->parent = parent )) parent->rest = p;
   if (( p->rest   = rest   )) rest->parent = p;
   return p;
}

void *pkt_data(packet_t *pkt)
{
	return pkt->data;
}

void pkt_move(packet_t *pkt, int len)
{
	pkt->data += len;
	//pkt->numlen -= len;
}

void new_mark_pkt(int mark, packet_t *p)
{
   if ( p->len > mark )
   {
      packet_t *rest = (packet_t *) packet_get(NULL, 0);
      rest->data   = p->data + mark;
      rest->len    = p->len - mark;
      rest->parent = p;
      rest->rest   = p->rest;
      p->len  = mark;
      p->rest = rest;
   }
}



void free_pkt(packet_t *p)
{
   while ( p->parent ) p = p->parent;
   while ( p )
   {
      packet_t *rest = p->rest;
      packet_free(p);
      p = rest;
   }
}


packetqueue_t *new_pktque(void)
{
   packetqueue_t *pq = alloc_pktque();
   pq->count = 0;
   return pq;
}

void quepkt_add(packetqueue_t *pq, packet_t *p)
{
   p->next = 0;
   if ( pq->count++ )
      pq->tail = pq->tail->next = p;
   else
      pq->head = pq->tail = p;
}

packet_t *quepkt_del(packetqueue_t *pq)
{
   if ( pq->count )
   {
      packet_t *p = pq->head;
      pq->head = pq->head->next;
      pq->count--;
      return p;
   }
   else 
	   return 0;
}

int queue_pkt_empty(packetqueue_t *pq)
{
   if ( pq->count == 0)
   { 
	   //printk(" empty\n");
      return 1;
   }
   else 
	   return 0;
}

void queue_pkt_free(packetqueue_t *pq)
{
   packet_t *p = pq->head;
   while ( p )
   {
      packet_t *next = p->next;
      free_pkt(p);
      p = next;
   }
   packetque_free(pq);
}

int quepkt_count(packetqueue_t *pq)
{
   return ( pq->count )  ;
}

int quepkt_head(packetqueue_t *pq)
{
   return ( pq->head )  ;
}

int quepkt_tail(packetqueue_t *pq)
{
   return ( pq->tail )  ;
}
#endif


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


#define HTONS(_x_) ( (((_x_) >> 8) & 0xFF) | (((_x_) & 0xFF) << 8) )
#define HTONL(_x_) ( (((_x_) >> 24) & 0xFF) | (((_x_) >> 8) & 0xFF00) \
			| (((_x_) & 0xFF00) << 8) | (((_x_) & 0xFF) << 24) )

#define IP_PROTO_ICMP 1
#define IP_PROTO_UDP 17
#define IP_PROTO_UDPLITE 170
#define IP_PROTO_TCP 6

inline unsigned short ntohs(unsigned short x)
{
   __asm__ __volatile__("xchgb %b0,%h0" : "=q"(x) : "0"(x));
   return x;
}


void dump_ip(ip_t *p_ip)
{
	#ifdef SIMPLE
   kprintf("IP version=%d,  tos=0x%02X, len=%d\n",
	   p_ip->verlen >> 4,  p_ip->tos, ntohs(p_ip->len));

   kprintf(" id=0x%04X, frag=0x%04X, ttl=%d, type=%d,",
	 ntohs(p_ip->id),  ntohs(p_ip->frag), p_ip->ttl, p_ip->type,
	   ntohs(p_ip->checksum));
#else
   kprintf("IP Version=%d,  type=%d, id=%d ",
	   p_ip->verlen >> 4,  p_ip->type, ntohs(p_ip->id));
#endif
   kprintf("   src=%d.%d.%d.%d, dst=%d.%d.%d.%d\n",
	   p_ip->src[0], p_ip->src[1], p_ip->src[2], p_ip->src[3],
	   p_ip->dst[0], p_ip->dst[1], p_ip->dst[2], p_ip->dst[3]);
}

void dump_arp(arp_t *p_arp)
{
	   kprintf("ARP type:%d   src=%d.%d.%d.%d, dst=%d.%d.%d.%d\n",
		   p_arp->arp_hard_type,
	   p_arp->src[0], p_arp->src[1], p_arp->src[2], p_arp->src[3],
	   p_arp->dst[0], p_arp->dst[1], p_arp->dst[2], p_arp->dst[3]);
}

