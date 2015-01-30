/******************************************************************************/
/* This file has been written by Sergio Perez Alcañiz <serpeal@upvnet.upv.es> */
/*            Departamento de Informática de Sistemas y Computadores          */
/*            Universidad Politécnica de Valencia                             */
/*            Valencia (Spain)                                                */
/*            Date: April 2003                                                */
/******************************************************************************/

#include <linux/module.h>
#include <linux/kernel.h>
#include "netdb.h"
#include "ares.h"
#include "ares_dns.h"
#include "memcopy.h"
#include "lwip/sys.h"
#include "lwip/ip_addr.h"
#include "lwip/inet.h"
#include "lwip/sockets.h"
#include <rtl_printf.h>

#ifndef INADDR_NONE
#define	INADDR_NONE 0xffffffff
#endif

#define QUESTION "cybeles.disca.upv.es"
#define DNS_SERVER_ADDR "158.42.249.8"

static void dns_client_callback(void *arg, int status, struct hostent *host)
{
  struct in_addr addr;
  char *mem, **p;

  if (status != ARES_SUCCESS)
    {
      rtl_printf("%s: %s\n", (char *) arg, ares_strerror(status, &mem));
      ares_free_errmem(mem);
      return;
    }

  for (p = host->h_addr_list; *p; p++)
    {
      char *address = "                                  ";
      memcopy(&addr, *p, sizeof(struct in_addr));
      address = inet_ntoa(addr);
      rtl_printf("%s: %s\n", host->h_name, address);
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
      rtl_printf("ares_init: %s\n", ares_strerror(status, &errmem));      
      ares_free_errmem(errmem);
      sys_thread_exit();
    }
  
  
  addr.s_addr = inet_addr(QUESTION);
  if (addr.s_addr == INADDR_NONE){
    ares_gethostbyname(channel, QUESTION, AF_INET, dns_client_callback, QUESTION);
  }else{
      ares_gethostbyaddr(channel, &addr, sizeof(addr), AF_INET, dns_client_callback,
			 QUESTION);
  }

  /* Wait for all queries to complete. */
  while (1)
    {
      FD_ZERO(&read_fds);
      FD_ZERO(&write_fds);
      nfds = ares_fds(channel, &read_fds, &write_fds);
      if (nfds == 0)
	break;
      tvp = ares_timeout(channel, NULL, &tv);
      select(nfds, &read_fds, &write_fds, NULL, tvp);
      ares_process(channel, &read_fds, &write_fds);
    }
  
  ares_destroy(channel);
}

/*-----------------------------------------------------------------------------------*/
int init_module(void){
  printk("\n\nDNS client module inserted.\n\n"); 
  sys_thread_new(dns_client, NULL, 0);
  return 0;
}

/*-----------------------------------------------------------------------------------*/
void cleanup_module(void){
  printk("\n\nDNS client module removed.\n\n"); 
}
