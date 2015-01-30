

#include <net/net.h>
#include <jicama/msgport.h>

struct netif *loopback_netif;

//#define USE_THREAD 1
#ifdef USE_THREAD
static int loopback_semid;
static thread_t* loop_thread=NULL ;
static const int loopback_packets=1024; //TCP_SND_QUEUELEN*2;

struct ether_msg
{
  struct pbuf *p;
  struct netif *netif;
};


static err_t loopif_output(struct netif *netif, struct pbuf *p, struct ip_addr *ipaddr)
{
  struct pbuf *q;
  struct ether_msg msg;
  int rc;

  if ((netif->flags & NETIF_UP) == 0) return ENETDOWN;



  q = pbuf_dup(PBUF_RAW, p);
  if (!q) return ENOMEM;


  lock_semaphore(loopback_semid);

  msg.p = q;
  msg.netif = netif;
 
	rc = post_thread_message(loop_thread, &msg, sizeof(msg));

	

	if (rc!=sizeof(msg))
	{
		kprintf("ether: post_thread_message (%d)\n",rc);
		return -1;
	}

  pbuf_free(p);

  return 0;
}

void loopif_dispatcher(void *arg)
{
  struct netif *netif = arg;
  struct pbuf *p;
  int rc;
  struct ether_msg msg;

  trace("entry loopif_dispatcher\n");
  loop_thread = current_thread();
  loopback_semid = create_semaphore("iomux",0,loopback_packets);

  while (1)
  {	 
	  rc = get_thread_message(loop_thread,&msg,sizeof(msg),INFINITE);
	  if (rc<=0)
	  {
		kprintf("loopif_dispatcher called @0x%x error\n", &msg);
		  continue;
	  }

	  p = msg.p;
	 //kprintf("loopif_dispatcher called at 0x%x,len=%d\n",p,p->len);

    rc = netif->input(p, netif);
    if (rc < 0) pbuf_free(p);
	unlock_semaphore(loopback_semid);
  }

  kprintf("OOps: loopif exit ..\n");
}
#else
static err_t loopif_output(struct netif *netif, struct pbuf *p, struct ip_addr *ipaddr)
{
	int err=0;

	err=ether_output(netif,p,ipaddr);
	pbuf_free(p);
	//err=loopif_input(netif,p,ipaddr);

	return err;
}
#endif

void loopif_init()
{
  struct ip_addr loip;
  struct ip_addr logw;
  struct ip_addr lomask;

  loip.addr = htonl(INADDR_LOOPBACK);
  lomask.addr = htonl(0xFF000000);
  logw.addr = htonl(INADDR_ANY);

  loopback_netif = netif_add("loop", &loip, &lomask, &logw);
  if (!loopback_netif) return;
  loopback_netif->mtu_size = 16384-40;

  //loopback_netif->output = ether_output;//loopif_output;
  loopback_netif->output = loopif_output;
  loopback_netif->flags |= NETIF_LOOPBACK | NETIF_UP |
                           NETIF_IP_TX_CHECKSUM_OFFLOAD | NETIF_IP_RX_CHECKSUM_OFFLOAD |
		           NETIF_UDP_RX_CHECKSUM_OFFLOAD | NETIF_UDP_TX_CHECKSUM_OFFLOAD |
		           NETIF_TCP_RX_CHECKSUM_OFFLOAD | NETIF_TCP_TX_CHECKSUM_OFFLOAD;

#ifdef USE_THREAD
	loopback_packets = 0;
  new_kernel_thread("loopback",loopif_dispatcher, loopback_netif );
#endif
}
