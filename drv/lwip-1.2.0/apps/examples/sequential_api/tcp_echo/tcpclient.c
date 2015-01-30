/******************************************************************************/
/* This file has been written by Sergio Perez Alcañiz <serpeal@upvnet.upv.es> */
/*            Departamento de Informática de Sistemas y Computadores          */
/*            Universidad Politécnica de Valencia                             */
/*            Valencia (Spain)                                                */
/*            Date: April 2003                                                */
/******************************************************************************/

#include <linux/module.h>
#include <linux/kernel.h>
#include "lwip/ip_addr.h"
#include "lwip/sys.h"
#include "lwip/api.h"
#include <rtl_sched.h>

pthread_t rtl_lwip_tcpclient;

/*-----------------------------------------------------------------------------------*/
static void * tcp_client(void *arg){
  static struct netconn *conn;
  struct ip_addr ipaddr;
  char data[6];

  sys_thread_register((void *) pthread_self());
 
  rtl_printf("TCP Client thread : 0x%x\n",pthread_self());
  
  conn = netconn_new(NETCONN_TCP);

  IP4_ADDR(&ipaddr, 158,42,58,141);

  netconn_bind(conn, &ipaddr, 7);

  IP4_ADDR(&ipaddr, 158,42,58,139);

  rtl_printf("Netconn_connect\n");
  netconn_connect(conn, &ipaddr, 10);

  data[0] = 'h';
  data[1] = 'e';
  data[2] = 'l';
  data[3] = 'l';
  data[4] = 'o';
  data[5] = '\0';
  
  rtl_printf("Echo TCP client sending: %s\n",data);
  
  netconn_write(conn,data,6,NETCONN_COPY);
  
  netconn_delete(conn);
  
  return sys_thread_exit();
}


/*-----------------------------------------------------------------------------------*/
int init_module(void){
  pthread_attr_t attr;

  printk("\n\nEcho TCP client module inserted.\n\n"); 

  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  if(pthread_create(&(rtl_lwip_tcpclient),&attr, tcp_client, NULL)) {
    rtl_printf("ERROR: cannot create pthread!\n");
    return -1;
  }

 return 0;
}

/*-----------------------------------------------------------------------------------*/
void cleanup_module(void){
  pthread_delete_np(rtl_lwip_tcpclient);
  printk("\n\nEcho TCP client module removed.\n\n"); 
}
