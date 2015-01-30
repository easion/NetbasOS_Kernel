/******************************************************************************/
/* This file has been written by Sergio Perez Alcañiz <serpeal@upvnet.upv.es> */
/*            Departamento de Informática de Sistemas y Computadores          */
/*            Universidad Politécnica de Valencia                             */
/*            Valencia (Spain)                                                */
/******************************************************************************/

#include <linux/module.h>
#include <linux/kernel.h>
#include "lwip/ip_addr.h"
#include "lwip/api.h"
#include "lwip/sys.h"
#include <rtl_sched.h>

void *status;
static struct netconn *conn;
static struct netbuf *buf;
struct ip_addr ipaddr;
char buffer[5];

static void udpclient(void *arg){

  IP4_ADDR(&ipaddr, 158,42,58,139);
  
  conn = netconn_new(NETCONN_UDP);
  netconn_connect(conn, &ipaddr, 7);

  buffer[0]='h';
  buffer[1]='o';
  buffer[2]='l';
  buffer[3]='a';
  buffer[4]='\0';

  buf=netbuf_new();
  
  netbuf_ref(buf,&buffer,sizeof(buffer));
  netconn_send(conn,buf);

  rtl_printf(" UDP CLient sending %s\n", buffer);

  sys_thread_exit();
}


/*-----------------------------------------------------------------------------------*/
int init_module(void){
  printk("\n\n UDP echo client module inserted\n\n"); 
  sys_thread_new(udpclient, NULL, 0);
 return 0;
}

/*-----------------------------------------------------------------------------------*/
void cleanup_module(void){
  printk("\n UDP echo client module removed\n");
}
