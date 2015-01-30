
/* 
** Jicama OS Loadable Kernel Modules Test
** 2005-3-5
*/
#include <drv/drv.h>
#include "lwip/sockets.h"
#include "netdb.h"
#include "ares.h"
#include "ares_dns.h"
#include "lwip/sys.h"
#include "lwip/ip_addr.h"
#include "lwip/inet.h"

int errno;

#define IP_ADDR_ANY_VALUE 0x00000000UL
#define IP_ADDR_BROADCAST_VALUE 0xffffffffUL

const struct ip_addr ip_addr_any = { IP_ADDR_ANY_VALUE };
const struct ip_addr ip_addr_broadcast = { IP_ADDR_BROADCAST_VALUE };

#ifndef INADDR_NONE
#define	INADDR_NONE 0xffffffff
#endif

//#define QUESTION "cybeles.disca.upv.es"
//#define DNS_SERVER_ADDR "158.42.249.8"

#define QUESTION "netbas.cn"
#define DNS_SERVER_ADDR "202.96.134.133"

static void dns_client_callback(void *arg, int status, struct hostent *host)
{
  struct in_addr addr;
  char *mem, **p;

  kprintf("dns_client_callback caled\n");

  if (status != ARES_SUCCESS)
    {
      syslog(4,"%s: %s\n", (char *) arg, ares_strerror(status, &mem));
      ares_free_errmem(mem);
      return;
    }

  for (p = host->h_addr_list; *p; p++)
    {
      char *address = "                                  ";
      memcpy(&addr, *p, sizeof(struct in_addr));
      address = inet_ntoa(addr);
      syslog(4,"%s: %s\n", host->h_name, address);
    }
}


/*-----------------------------------------------------------------------------------*/
static void dns_client(void *arg){
  ares_channel channel;
  int status, nfds;
  fd_set read_fds, write_fds;
  struct timeval *tvp, tv;
  char *errmem;
  struct in_addr addr;
  struct in_addr ns1;
  struct ares_options options;

  inet_aton(DNS_SERVER_ADDR,&ns1);

  options.servers = &ns1;
  options.nservers = 1;
  options.flags = ARES_FLAG_USEVC;

  status = ares_init_options(&channel, &options, ARES_OPT_SERVERS | ARES_OPT_FLAGS);

  if (status != ARES_SUCCESS)
    {
      syslog(4,"ares_init: %s\n", ares_strerror(status, &errmem));      
      ares_free_errmem(errmem);
      sys_thread_exit();
    }

	syslog(4,"start get hostby name %s\n", QUESTION);
  
  
  addr.s_addr = inet_addr(QUESTION);
  if (addr.s_addr == INADDR_NONE){
	  syslog(4,"start query ...\n");
    ares_gethostbyname(channel, QUESTION, AF_INET, dns_client_callback, QUESTION);
  }else{
      ares_gethostbyaddr(channel, &addr, sizeof(addr), AF_INET, dns_client_callback,
			 QUESTION);
  }
  syslog(4,"wait data ...\n");

  /* Wait for all queries to complete. */
  while (1)
    {
      FD_ZERO(&read_fds);
      FD_ZERO(&write_fds);
      nfds = ares_fds(channel, &read_fds, &write_fds);
      if (nfds == 0)
	break;
      tvp = ares_timeout(channel, NULL, &tv);
      lwip_select(nfds, &read_fds, &write_fds, NULL, tvp);
      ares_process(channel, &read_fds, &write_fds);
    }
	syslog(4,"call  ares_destroy...\n");
  
  ares_destroy(channel);
}

void sys_thread_exit()
{
	int ret = 0;
	thread_exit(current_thread(), NULL);
	return;
}



/*dll entry*/
int dll_main()
{
	new_kernel_thread("DNS", dns_client, NULL );
	return 0;
}


int dll_destroy()
{
	printk("dll_destroy called!\n");
	return 0;
}

int dll_version()
{
	return 0;
}
