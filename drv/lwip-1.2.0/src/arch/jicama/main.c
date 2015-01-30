#include "lwip/debug.h"
#include "lwip/opt.h"
#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/sys.h"
#include "lwip/stats.h"
#include "lwip/tcpip.h"
#include "netif/loopif.h"

#include "lwip/ip_addr.h"
#include "lwip/sys.h"
#include "lwip/def.h"

err_t ethernetif_init(struct netif *netif);

static void tcpip_init_done(void* arg) ;

static int myflags = 0;

void dev_init()
{
	printf("eth dev init call\n");
}

int module_init()
{
	int idhcpre,result;
	static sys_sem_t sem;
	struct ip_addr ipaddr, netmask, gw;

	static struct netif _udpnetif;
	static struct netif _netif;
	static struct netif _netif_loop;

	struct netif *udpnetif=&_udpnetif;
	struct netif *netif=&_netif;
	struct netif *netif_loop=&_netif_loop;

	memset(netif, 0, sizeof(struct netif ));
	memset(netif_loop, 0, sizeof(struct netif ));

#ifdef STATS
  stats_init();
#endif /* STATS */

	sys_init();
	mem_init();
	memp_init();
	pbuf_init();

	//tcpdump_init();
	netif_init();

	sem = sys_sem_new(0); 
	tcpip_init(tcpip_init_done, &sem);

	if(myflags == 0)
	sys_sem_wait(sem);

	sys_sem_free(sem);
	printf("TCP/IP initialized.\n");

	dll_args(&ipaddr, &gw, &netmask);

	if (*(u32_t*)&ipaddr == 0)
	{

	//#error dhcp init
    IP4_ADDR(&gw, 0,0,0,0);
    IP4_ADDR(&ipaddr, 0,0,0,0);
    IP4_ADDR(&netmask, 0,0,0,0);

	kprintf("Add DHCP Interface ...\n");
    netif_add(udpnetif, &ipaddr, &netmask, &gw, NULL, ethernetif_init,
		      tcpip_input);


	printf("LWIP:waiting for neif init \n");
	thread_wait(current_thread(), 1500);
	schedule();


  for(idhcpre = 0; idhcpre<4;  idhcpre++ )//dhcp最多重试4遍
  {
	  int icount;
      printf("LWIP:start dhcp request %d/4\n", idhcpre);
      result  = dhcp_start(udpnetif);//广播dhcp请求
      IP4_ADDR(&ipaddr, 0,0,0,0);
      for(icount = 0; (icount < 10) && (ipaddr.addr == 0); icount ++ )
      {
          ipaddr.addr = udpnetif->ip_addr.addr;
           thread_wait(current_thread(), 1000);
		   schedule();
      } // if failed ipaddr = 0.0.0.0 ;timeout = 10 * 1000 ms
           //等待dhcp是否接受到IP了
    

      dhcp_stop(udpnetif); //一次dhcp结束
      if (ipaddr.addr != 0)
          break;
  }  

	gw.addr = udpnetif->gw.addr;
	ipaddr.addr = udpnetif->ip_addr.addr;
	netmask.addr = udpnetif->netmask.addr;

	netif_set_addr(udpnetif, &ipaddr, &netmask, &gw);
	//netif_add(netif, &ipaddr, &netmask, &gw, NULL, ethernetif_init,
	//				  tcpip_input);

	netif_set_up(udpnetif);
	//netif_set_up(netif);
	netif_set_default(udpnetif);		

	}
	else{
    /*netif_add(udpnetif, &ipaddr, &netmask, &gw, NULL, dev_init,
		      udp_input);
	netif_set_up(udpnetif);	*/
	netif_add(netif, &ipaddr, &netmask, &gw, NULL, ethernetif_init,
					  tcpip_input);
	netif_set_up(netif);		
	netif_set_default(netif);
	}

	IP4_ADDR(&gw, 127,0,0,1);
	IP4_ADDR(&ipaddr, 127,0,0,1);
	IP4_ADDR(&netmask, 255,0,0,0);
	kprintf("loopif_init add ...\n");

	netif_add(netif_loop,&ipaddr, &netmask, &gw, NULL, loopif_init,
		tcpip_input);
	netif_set_up(netif_loop);
	netif_set_default(netif_loop);
	return 0;
}

char * inetntoa(u32_t net32)
{
	static char bufs[4][16];
	static unsigned int index = 0;

	char * buf = bufs[index++ & 3];
	u8_t * p = (u8_t *) &net32;
	snprintf(buf,64, "%d.%d.%d.%d", p[0], p[1], p[2], p[3]);
	return buf;
}

struct ip_addr *inetaton(char * str)
{
	int i;
	u8_t tmp[4];
	static struct ip_addr ip;

	//memset(&ip, 0, sizeof(struct ip_addr );

	for ( i = 0; i < 4; i++) {
		tmp[i] = 0;
		while (*str&&(*str<'0' || *str>'9'))
		{
			str++;
		}
		while (*str && (*str >= '0') && (*str <='9')) {
			tmp[i] *= 10;
			tmp[i] += *str - '0';
			str++;
		}
		//printk("%d-", tmp[i]);
		str++;
	}

	 IP4_ADDR(&ip, tmp[0],tmp[1],tmp[2],tmp[3]);
	//u32_t host32 = (tmp[0] << 24) | (tmp[1] << 16) | (tmp[2] << 8) | tmp[3];
	//return htonl(host32);
	return &ip;
} 



static void tcpip_init_done(void* arg) 
{ 
	sys_sem_t *sem; 
	sem = (sys_sem_t *)arg; 
	kprintf("tcpip_init_done: ok \n");
	sys_sem_signal(*sem); 
	myflags = 1;
} 

void dpp_thread(void* arg)
{
	kprintf("dpp_thread init ...\n");
	while (1)
	{
		//thread_wait(current_thread(), 0);
		//schedule();
	}
}





#include <drv/drv.h>
#include <drv/unistd.h>
#include <drv/sym.h>
#include <drv/errno.h>
#include <assert.h>
#include "lwip/tcp.h"
#include "lwip/sockets.h"

static struct _export_table_entry lwip_sym_table []=
{
	EXPORT_PC_SYMBOL(inet_ntoa),
	EXPORT_PC_SYMBOL(inet_aton),
	EXPORT_PC_SYMBOL(inet_addr),

	/*bsd socket style*/
	EXPORT_PC_SYMBOL(lwip_accept),
	EXPORT_PC_SYMBOL(lwip_bind),
	EXPORT_PC_SYMBOL(lwip_bind),
	EXPORT_PC_SYMBOL(lwip_shutdown),
	EXPORT_PC_SYMBOL(lwip_close),
	EXPORT_PC_SYMBOL(lwip_connect),
	EXPORT_PC_SYMBOL(lwip_getsockname),
	EXPORT_PC_SYMBOL(lwip_getpeername),
	EXPORT_PC_SYMBOL(lwip_setsockopt),
	EXPORT_PC_SYMBOL(lwip_getsockopt),
	EXPORT_PC_SYMBOL(lwip_listen),
	EXPORT_PC_SYMBOL(lwip_recv),
	EXPORT_PC_SYMBOL(lwip_read),
	EXPORT_PC_SYMBOL(lwip_recvfrom),
	EXPORT_PC_SYMBOL(lwip_send),
	EXPORT_PC_SYMBOL(lwip_sendto),
	EXPORT_PC_SYMBOL(lwip_socket),
	EXPORT_PC_SYMBOL(lwip_write),
	EXPORT_PC_SYMBOL(lwip_select),
	EXPORT_PC_SYMBOL(lwip_ioctl),


	EXPORT_PC_SYMBOL(netconn_new),
	EXPORT_PC_SYMBOL(netconn_bind),
	EXPORT_PC_SYMBOL(netconn_listen),
	EXPORT_PC_SYMBOL(netconn_accept),
	EXPORT_PC_SYMBOL(netconn_write),
	//EXPORT_PC_SYMBOL(),
	EXPORT_PC_SYMBOL(netconn_recv),
	EXPORT_PC_SYMBOL(netconn_send),
	EXPORT_PC_SYMBOL(netconn_delete),
	EXPORT_PC_SYMBOL(netbuf_copy),
	EXPORT_PC_SYMBOL(netconn_type),
	EXPORT_PC_SYMBOL(netconn_peer),
	EXPORT_PC_SYMBOL(netconn_addr),
	EXPORT_PC_SYMBOL(netconn_disconnect),
	EXPORT_PC_SYMBOL(netconn_connect),
	EXPORT_PC_SYMBOL(netconn_err),
	EXPORT_PC_SYMBOL(netconn_new_with_callback),
	EXPORT_PC_SYMBOL(netconn_new_with_proto_and_callback),

	EXPORT_PC_SYMBOL(netbuf_fromaddr),
	EXPORT_PC_SYMBOL(netbuf_fromport),
	EXPORT_PC_SYMBOL(netbuf_next),
	EXPORT_PC_SYMBOL(netbuf_first),
	EXPORT_PC_SYMBOL(netbuf_data),
	EXPORT_PC_SYMBOL(netbuf_next),
	EXPORT_PC_SYMBOL(netbuf_delete),
	EXPORT_PC_SYMBOL(netbuf_len),
	EXPORT_PC_SYMBOL(netbuf_ref),
	EXPORT_PC_SYMBOL(netbuf_chain),
	EXPORT_PC_SYMBOL(netbuf_alloc),
	EXPORT_PC_SYMBOL(netbuf_free),

	//old style
	EXPORT_PC_SYMBOL(tcp_close),
	EXPORT_PC_SYMBOL(tcp_connect),
	EXPORT_PC_SYMBOL(tcp_arg),
	EXPORT_PC_SYMBOL(tcp_sent),
	EXPORT_PC_SYMBOL(tcp_err),
	EXPORT_PC_SYMBOL(tcp_recv),
	EXPORT_PC_SYMBOL(tcp_new),
	EXPORT_PC_SYMBOL(tcp_bind),
	EXPORT_PC_SYMBOL(tcp_listen),
	EXPORT_PC_SYMBOL(tcp_accept),
	EXPORT_PC_SYMBOL(tcp_poll),
	EXPORT_PC_SYMBOL(tcp_write),
	EXPORT_PC_SYMBOL(tcp_setprio),
	EXPORT_PC_SYMBOL(tcp_abort),
	EXPORT_PC_SYMBOL(tcp_recved),
#if 0
	EXPORT_PC_SYMBOL(mem_malloc),
	EXPORT_PC_SYMBOL(mem_free),
	EXPORT_PC_SYMBOL(mem_realloc),
#endif
	EXPORT_PC_SYMBOL(pbuf_free),
	EXPORT_PC_SYMBOL(pbuf_alloc),
	EXPORT_PC_SYMBOL(pbuf_realloc),
	EXPORT_PC_SYMBOL(pbuf_cat),
	EXPORT_PC_SYMBOL(pbuf_queue),
	EXPORT_PC_SYMBOL(pbuf_dequeue),
	EXPORT_PC_SYMBOL(pbuf_clen),
	EXPORT_PC_SYMBOL(pbuf_ref),
	//EXPORT_PC_SYMBOL(pbuf_ref_chain),
	EXPORT_PC_SYMBOL(pbuf_header),
};

#include <drv/proc_entry.h>

int
tcp_print_pcbs(char* buf, int len);
int
etharp_proc(char *buf, int len);
int
netif_proc(char *buf, int len);

struct proc_entry tcpstat = {
	name: "tcp_stat",
	write_func: tcp_print_pcbs,
	read_func: NULL,
};

struct proc_entry ifstat = {
	name: "netif",
	write_func: netif_proc,
	read_func: NULL,
};

struct proc_entry arpstat = {
	name: "arp",
	write_func: etharp_proc,
	read_func: NULL,
};


void proc_entry_init()
{
	register_proc_entry(&tcpstat);
	register_proc_entry(&arpstat);
	register_proc_entry(&ifstat);
}

#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/snmp.h"

#include "lwip/tcp.h"

char *
tcp_print_state(enum tcp_state s)
{
  //return ("State: ");
  switch (s) {
  case CLOSED:
    return ("CLOSED\n");
    break;
 case LISTEN:
   return ("LISTEN\n");
   break;
  case SYN_SENT:
    return ("SYN_SENT\n");
    break;
  case SYN_RCVD:
    return ("SYN_RCVD\n");
    break;
  case ESTABLISHED:
    return ("ESTABLISHED\n");
    break;
  case FIN_WAIT_1:
    return ("FIN_WAIT_1\n");
    break;
  case FIN_WAIT_2:
    return ("FIN_WAIT_2\n");
    break;
  case CLOSE_WAIT:
    return ("CLOSE_WAIT\n");
    break;
  case CLOSING:
    return ("CLOSING\n");
    break;
  case LAST_ACK:
    return ("LAST_ACK\n");
    break;
  case TIME_WAIT:
    return ("TIME_WAIT\n");
   break;
  }
  return "unknow";
}

char*
tcp_print_flags(u8_t flags)
{
  if (flags & TCP_FIN) {
    return ("FIN ");
  }
  if (flags & TCP_SYN) {
    return ("SYN ");
  }
  if (flags & TCP_RST) {
    return ("RST ");
  }
  if (flags & TCP_PSH) {
    return ("PSH ");
  }
  if (flags & TCP_ACK) {
    return ("ACK ");
  }
  if (flags & TCP_URG) {
    return ("URG ");
  }
  if (flags & TCP_ECE) {
    return ("ECE ");
  }
  if (flags & TCP_CWR) {
    return ("CWR ");
  }
  return "unknow";
}

int
tcp_print_pcbs(char* buf, int len)
{
	int cnt = 0, c;
  struct tcp_pcb *pcb;
  c = sprintf(buf, ("Active PCB states:\n"));
  for(pcb = tcp_active_pcbs; pcb != NULL; pcb = pcb->next) {
	  cnt += c;
    c = sprintf(buf+cnt, ("Local port %d, foreign port %d snd_nxt %d rcv_nxt %d %s",
                       pcb->local_port, pcb->remote_port,
                       pcb->snd_nxt, pcb->rcv_nxt),
						tcp_print_state(pcb->state));
  }

  cnt += c;
  sprintf(buf+cnt, ("Listen PCB states:\n"));
  for(pcb = (struct tcp_pcb *)tcp_listen_pcbs.pcbs; pcb != NULL; pcb = pcb->next) {
    cnt += c;
	c = sprintf(buf+cnt, "Local port %"U16_F", foreign port %"U16_F" snd_nxt %"U32_F" rcv_nxt %"U32_F" %s",
                       pcb->local_port, pcb->remote_port,
                       pcb->snd_nxt, pcb->rcv_nxt,
    tcp_print_state(pcb->state));
  } 
  
  c = sprintf(buf+cnt, ("TIME-WAIT PCB states:\n"));
  for(pcb = tcp_tw_pcbs; pcb != NULL; pcb = pcb->next) {
	  cnt += c;

    c = sprintf(buf+cnt, "Local port %d, foreign port %d snd_nxt %d rcv_nxt %d %s",
                       pcb->local_port, pcb->remote_port,
                       pcb->snd_nxt, pcb->rcv_nxt,  
	tcp_print_state(pcb->state) );

  } 
  cnt += c;
  return cnt;
}

static struct ip_addr netif_dns_addr[DHCP_MAX_DNS]; /* DNS server addresses */

void set_dns_addr(struct ip_addr dns_addr[DHCP_MAX_DNS])
{
	memcpy(netif_dns_addr,dns_addr,sizeof(netif_dns_addr));
}

int
netif_proc(char *buf, int len)
{
	int i;
  struct netif *netif;
  int cnt=0,c;
  

  for(netif = netif_list; netif != NULL; netif = netif->next) {
      c = sprintf(buf, "%c%c: %s,Netmask:%s,GW:%s\n"
	  "\tMac:%02x.%02x.%02x.%02x.%02x.%02x MTU=%d Flag:0x%08x\n", 
		  netif->name[0], netif->name[1],
		  inetntoa(netif->ip_addr.addr),
		  inetntoa(netif->netmask.addr),
		  inetntoa(netif->gw.addr),
		  netif->hwaddr[0],
		  netif->hwaddr[1],
		  netif->hwaddr[2],
		  netif->hwaddr[3],
		  netif->hwaddr[4],
		  netif->hwaddr[5],
		  netif->mtu,
		  netif->flags
		  );
	  cnt+= c;
	  buf+=c;
  }

  for (i=0; i<DHCP_MAX_DNS; i++)
  {
	  sprintf(buf,"dns %d address with %s\n",
			  i, inetntoa(netif_dns_addr[i].addr));
	  cnt+= c;
	  buf+=c;
  }
  return cnt;
}

int export_lwip_symtab()
{
	int num =sizeof(lwip_sym_table)/sizeof(struct _export_table_entry);
	proc_entry_init();
	return install_dll_table("lwip.dll", 1, num, lwip_sym_table);
}

void remove_lwip_symtab()
{
	remove_dll_table("lwip.dll");
}

