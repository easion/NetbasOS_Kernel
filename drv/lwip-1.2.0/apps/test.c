

#include <stdio.h>
#include <time.h>
#include <string.h>

#include "lwip/debug.h"
#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/sys.h"

#include "lwip/stats.h"

#include "lwip/tcpip.h"

#include "netif/loopif.h"
#include "netif/tcpdump.h"

#include "arch/perf.h"

#include "httpd.h"
#include "fs.h"

void ethernetif_init(struct netif *netif);
void pktif_update(void);
void pktif_release(void);

static err_t netio_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
	if (err == ERR_OK && p != NULL)
	{
		tcp_recved(pcb, p->tot_len);
		pbuf_free(p);
	}
	else
		pbuf_free(p);

	if (err == ERR_OK && p == NULL)
	{
		tcp_arg(pcb, NULL);
		tcp_sent(pcb, NULL);
		tcp_recv(pcb, NULL);
		tcp_close(pcb);
	}

	return ERR_OK;
}

static err_t netio_accept(void *arg, struct tcp_pcb *pcb, err_t err)
{
	tcp_arg(pcb, NULL);
	tcp_sent(pcb, NULL);
	tcp_recv(pcb, netio_recv);
	return ERR_OK;
}

void netio_init(void)
{
	struct tcp_pcb *pcb;

	pcb = tcp_new();
	tcp_bind(pcb, IP_ADDR_ANY, 18767);
	pcb = tcp_listen(pcb);
	tcp_accept(pcb, netio_accept);
}

void main_loop()
{
	struct ip_addr ipaddr, netmask, gw;
	int last_time;
	int timer1;
	int timer2;
	int done;
	
	IP4_ADDR(&gw, 192,168,2,201);
	IP4_ADDR(&ipaddr, 192,168,2,200);
	IP4_ADDR(&netmask, 255,255,255,0);
	
	netif_set_default(netif_add(&ipaddr, &netmask, &gw, ethernetif_init,
		ip_input));

	/*
	IP4_ADDR(&gw, 127,0,0,1);
	IP4_ADDR(&ipaddr, 127,0,0,1);
	IP4_ADDR(&netmask, 255,0,0,0);
	
	netif_add(&ipaddr, &netmask, &gw, loopif_init,
		ip_input);
	*/

	tcp_init();
	udp_init();
	ip_init();

	httpd_init();
	netio_init();

	last_time=clock();
	timer1=0;
	timer2=0;
	done=0;
	
	while(!done)
	{
		int cur_time;
		int time_diff;

		cur_time=clock();
		time_diff=cur_time-last_time;
		if (time_diff>0)
		{
			last_time=cur_time;
			timer1+=time_diff;
			timer2+=time_diff;
		}

		if (timer1>10)
		{
			tcp_fasttmr();
			timer1=0;
		}

		if (timer2>45)
		{
			tcp_slowtmr();
			timer2=0;
			done=kbhit();
		}

		pktif_update();
	}

	pktif_release();
}

void bcopy(const void *src, void *dest, int len)
{
  memcpy(dest,src,len);
}

void bzero(void *data, int n)
{
  memset(data,0,n);
}

int main(void)
{
	setvbuf(stdout,NULL,_IONBF,0);
#ifdef PERF
	perf_init("/tmp/lwip.perf");
#endif /* PERF */
#ifdef STATS
	stats_init();
#endif /* STATS */
	sys_init();
	mem_init();
	memp_init();
	pbuf_init();

	tcpdump_init();

	printf("System initialized.\n");

	main_loop();

	return 0;
}

