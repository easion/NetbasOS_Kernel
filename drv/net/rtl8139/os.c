
/*netbas port*/

#include <drv/defs.h>
#include <drv/drv.h>
#include <drv/sym.h>
#include <drv/pci.h>
#include <drv/ia.h>
#include <drv/errno.h>
#include <drv/spin.h>
#include <drv/log.h>
#include <net/pkt.h>
#include <assert.h>

char* get_mac();
int rtl8139_probe();
int rtl8129_ioctl( int fd, 
int nCommand, void* pArgs, bool bFromKernel );
static int rtl8139_drv_read(int minor, off_t  pos, void * buf,int count);
int rtl8129_write( int minor, off_t  pos, const void* pBuffer, size_t nSize );
#define RTL8139_MAJOR 0X40

#define ETHTHREAD_NAME "EthernetIF"
#define SIGTTIN           21	/* background process wants to read */


CREATE_SPINLOCK( rtl8139_lock );
void sendsig(void *rp, int signo);
int sigdelset(void *rp, int signo);
int sigrecv(void *rp, char*);
void *find_thread(const char* tname);


void wake_up_eththread()
{
	void *pthread;	

	pthread = find_thread(ETHTHREAD_NAME);	

	if (pthread){
		//sendsig(pthread, SIGTTIN);
		thread_ready(pthread);
	}
	else{
		syslog(4,"thread %s not found!\n", ETHTHREAD_NAME);
	}
}



int pkt_read( packet_t **p)
{
   unsigned eflags;
	if (!p){
		return -1;
	}
   save_eflags(&eflags);
	*p = queue_pkt_del(get_packetqueue());
   restore_eflags(eflags);
   return 0;
}




static int rtl8139_drv_read(int minor, off_t  pos, void * buf,int count)
 {
	 int  start=0;
	 packet_t *packet ;

	if (!buf){
		//null buffer
		return -1;
	}

	if (count==0){
		return 0;
	}

    spin_lock( &rtl8139_lock );

	while(start<count)
	{
		pkt_read(&packet);

		if (packet == 0)
		{
		printk("not del any\n");
		start = -1;
		break;
		}

		start = packet->rlen;
		//syslog(LOG_DEBUG,__FUNCTION__" packet->rlen = %d\n",packet->rlen);
		memcpy(buf, packet->data, start );	
		//dump_packet(packet);
		free_pkt(packet);
		break;
	 }

	spin_unlock( &rtl8139_lock );	
	//syslog(LOG_DEBUG,__FUNCTION__" read %d bytes ok\n", start);
	return start;
}


static int rtl8139_drv_open(const char *f, int mode)
{


  return RTL8139_MAJOR<<8;
}

static int rtl8139_drv_close(int  file)
{     
	return 0;
}

static int rtl8139_drv_select(int file, int cmd, unsigned *timeout)
{
	int ret;
	packetqueue_t* pq;
	unsigned eflags;

	save_eflags(&eflags);
	pq = get_packetqueue();	
	ret = pq->count;//queue_pkt_empty(get_packetqueue());
	restore_eflags(eflags);

	if (!ret)
	{
		//syslog(LOG_DEBUG,__FUNCTION__" %d packets in here\n", ret);
		return 0;
	}
	return 1;
}

static const driver_ops_t rtl8139_drv_fops =
{
	d_name:		"eth",
	d_author:	"Donald Becker <becker@scyld.com>",
	d_version:	"v2006-01-25",
	d_kver:	CURRENT_KERNEL_VERSION,
	d_index:	RTL8139_MAJOR<<8,
	open:		rtl8139_drv_open,
	close:		rtl8139_drv_close,
	read:		rtl8139_drv_read,
	write:		rtl8129_write,
	ioctl:		rtl8129_ioctl,		
	select:		rtl8139_drv_select,
	//interface:NET_ETH_INTERFACE
};



int dll_main()
{
	int retval;

	syslog(LOG_DEBUG,"rtl8139 start\n");
	retval = rtl8139_probe();

	if (retval!=OK)
	{
		syslog(LOG_DEBUG,"ne2k not found\n");
		return -1;
	}

	retval=kernel_driver_register(&rtl8139_drv_fops);
		
	 if(retval < 0)
    {
      printk("Could not register rtl8139 device\n");
      return 0;
    }
  
	return 0;
}

int dll_version()
{
	kprintf("JICAMA RealTek RTL8129/8139 Fast Ethernet driver!\n");
	return 0;
}

int dll_destroy()
{
	return 0;
}

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




inline unsigned short ntohs(unsigned short x)
{
   __asm ("xchgb %b0,%h0" : "=q"(x) : "0"(x));
   return x;
}


void dump_ip(ip_t *p_ip)
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

void dump_arp(arp_t *p_arp)
{
	   syslog(LOG_DEBUG,"ARP type:%d   src=%d.%d.%d.%d, dst=%d.%d.%d.%d\n",
		   p_arp->arp_hard_type,
	   p_arp->src[0], p_arp->src[1], p_arp->src[2], p_arp->src[3],
	   p_arp->dst[0], p_arp->dst[1], p_arp->dst[2], p_arp->dst[3]);
}

void dump_packet(packet_t *packet)
{
	eth_t *p_eth = (eth_t *)packet->data;
	char *ptr = (ip_t *)(packet->data+sizeof(eth_t));

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

/*************************************************/
