
#include <jicama/system.h>
#include "pkt.h"
#define pbuf_free_1 mm_free
#define pbuf_get_1 mm_malloc

//////////////////////////////////////////
__local packet_t packet_pool[PBUF_POOL_BLOCKS];
__local packetqueue_t packque[PKT_QUEUE_BLOCKS];

__local int packet_used;

enum{PKT_STAT_FREE,PKT_STAT_SYS,PKT_STAT_USER};

int pkt_buf_init(void)
{
	int i,err;
	packet_t *my_pkt;


	my_pkt = &packet_pool[0];
	for (i = 0; i < PBUF_POOL_BLOCKS; i++,my_pkt++)
	{
		my_pkt->flags = PKT_STAT_FREE;
	}

	for (i=0; i<PKT_QUEUE_BLOCKS; i++)
	{
		packque[i].flags = 0;
	}

	packet_used = 0;
	return 0;
}

/*******************************************/

__local void pkt_init(packet_t *my_pkt, int len, long flags)
{
   my_pkt->data  = my_pkt->base;
   my_pkt->tail  = my_pkt->base;
   my_pkt->end  = my_pkt->base+len;
   my_pkt->len  = len;
   my_pkt->rlen  = 0;
   my_pkt->flags = flags;
   my_pkt->parent = NULL;
   my_pkt->rest   = NULL;
}

__local packetqueue_t *alloc_pktque()
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
	kprintf("alloc_pktque null\n");
	return NULL;
}

void packetque_free(packetqueue_t *pq)
{
	pq->flags = 0;
}

packet_t * packet_get(void *pdata, int len)
{
	int i;
	packet_t *my_pkt = &packet_pool[0];

	for (i = 0; i < PBUF_POOL_BLOCKS; i++)
	{
		if(my_pkt->flags != PKT_STAT_FREE){
			my_pkt++;
			continue;
		}

		my_pkt->len = len;

		if (pdata != NULL)
		{
			my_pkt->base = pdata;
			pkt_init(my_pkt,len,PKT_STAT_USER);
		}
		else
		{
			my_pkt->base = pbuf_get_1(len);
			pkt_init(my_pkt,len,PKT_STAT_SYS);
		}


		packet_used ++;
		return my_pkt;
	}
	
	return NULL;
}



void packet_free(packet_t *my_pkt)
{
	if(my_pkt->flags==PKT_STAT_SYS){
		//syslog(LOG_DEBUG,__FUNCTION__" FREE PKT\n");
		pbuf_free_1(my_pkt->base, my_pkt->len);
	}
	my_pkt->flags = PKT_STAT_FREE;
	packet_used --;
	return ;
}

int pkt_size(packet_t *my_pkt)
{
   int result = 0;
   while ( my_pkt )
   {
      result += my_pkt->len;
      my_pkt = my_pkt->rest;
   }
   return result;
}


packet_t *new_pkt(void *content, int len, packet_t *parent, packet_t *rest)
{
   packet_t *my_pkt = packet_get(content, len);
   if (my_pkt == NULL)
   {
	   return NULL;
   }


   if (!my_pkt->base)
   {
	   free_pkt(my_pkt);
	   return NULL;
   }

   //pkt_init(my_pkt,len,PKT_STAT_SYS);
   my_pkt->parent = parent;
   my_pkt->rest   = rest;
   return my_pkt;
}



packet_t *new_pkt2(int len, packet_t *parent, packet_t *rest)
{
   packet_t *my_pkt = packet_get(NULL, len);

   if (!my_pkt)
   {
	   return NULL;
   }
   //my_pkt->base = pbuf_get_1(len);

   if (!my_pkt->base)
   {
	   free_pkt(my_pkt);
	   return NULL;
   }

   //pkt_init(my_pkt, len, PKT_STAT_SYS);

   if (( my_pkt->parent = parent ))
	   parent->rest = my_pkt;

   if (( my_pkt->rest   = rest   ))
	   rest->parent = my_pkt;

   return my_pkt;
}

void *pkt_data(packet_t *pkt)
{
	return pkt->data;
}

//在数据包前面追加 len  个字节
void *pkt_put(packet_t *pkt, int len)
{
	void *old_tail= pkt->tail;

	pkt->tail+=len;
	pkt->rlen+=len;

	return old_tail;
}

//在数据包前面删除 len  个字节
void *pkt_reserve(packet_t *pkt, int len)
{
	void *old_tail= pkt->data;

	pkt->data+=len;
	pkt->rlen-=len;

	return old_tail;
}


void pkt_move(packet_t *pkt, int len)
{
	void *old_tail= pkt->data;

	pkt->data+=len;
	pkt->rlen-=len;

	return old_tail;
}

int new_mark_pkt(int mark, packet_t *pkt)
{
	packet_t *rest;
	char *new_base;

	if( pkt->len < mark || pkt->rlen > mark )
	{
	   return -1;
	}

	new_base = pkt->base + mark;
	rest = new_pkt(new_base, pkt->len - mark, pkt,pkt->rest);

	if(!rest)
		return -1;

	pkt->len  = mark;
	pkt->end  = new_base;
	pkt->rest = rest;
	return 0;

}



void free_pkt(packet_t *my_pkt)
{
   while ( my_pkt->parent )
	   my_pkt = my_pkt->parent;
   while ( my_pkt )
   {
      packet_t *rest = my_pkt->rest;
      packet_free(my_pkt);
      my_pkt = rest;
   }
}

/*******************************************/
packetqueue_t *new_pktque(void)
{
   packetqueue_t *pq = alloc_pktque();
   pq->count = 0;
   return pq;
}

void queue_pkt_add(packetqueue_t *pq, packet_t *my_pkt)
{
   my_pkt->next = 0;
   if ( pq->count++ )
      pq->tail = pq->tail->next = my_pkt;
   else
      pq->head = pq->tail = my_pkt;
}

packet_t *queue_pkt_del(packetqueue_t *pq)
{
   if ( pq->count )
   {
      packet_t *my_pkt = pq->head;
      pq->head = pq->head->next;
      pq->count--;
      return my_pkt;
   }
   else 
	   return NULL;
}

int queue_pkt_empty(packetqueue_t *pq)
{
   if ( pq->count == 0)
   { 
	   //kprintf(" empty\n");
      return 1;
   }
   else 
	   return 0;
}

void queue_pkt_free(packetqueue_t *pq)
{
   packet_t *my_pkt = pq->head;
   while ( my_pkt )
   {
      packet_t *next = my_pkt->next;
      free_pkt(my_pkt);
      my_pkt = next;
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

/*******************************************/

#include <jicama/process.h>
#include <jicama/module.h>



__local struct _export_table_entry pkt_symtab []=
{
	/*file system*/
	EXPORT_PC_SYMBOL(queue_pkt_empty),
	EXPORT_PC_SYMBOL(queue_pkt_add),
	EXPORT_PC_SYMBOL(queue_pkt_del),
	EXPORT_PC_SYMBOL(queue_pkt_free),
	EXPORT_PC_SYMBOL(new_pktque),
	/*EXPORT_PC_SYMBOL(new_pkt),
	EXPORT_PC_SYMBOL(new_pkt2),
	EXPORT_PC_SYMBOL(free_pkt),
	EXPORT_PC_SYMBOL(pkt_size),
	EXPORT_PC_SYMBOL(new_mark_pkt),
	EXPORT_PC_SYMBOL(pkt_data),
	EXPORT_PC_SYMBOL(pkt_move),
	EXPORT_PC_SYMBOL(pkt_put),
	EXPORT_PC_SYMBOL(pkt_reserve),*/
	NULL
};

int queue_pkt_sym_setup()
{
	int num =sizeof(pkt_symtab)/sizeof(struct _export_table_entry);
	return install_dll_table("pkt.net", 1, num, pkt_symtab);
	return 0;
}

