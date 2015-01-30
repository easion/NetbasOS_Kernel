/******************************************************************************/
/* This file has been written by Sergio Perez Alcañiz <serpeal@upvnet.upv.es> */
/*            Departamento de Informática de Sistemas y Computadores          */
/*            Universidad Politécnica de Valencia                             */
/*            Valencia (Spain)                                                */
/*            Date: April 2003                                                */
/******************************************************************************/

#include <linux/module.h>
#include <linux/kernel.h>
#include "lwip/sys.h"
#include "lwip/api.h"
#include <rtl_sched.h>
#include <rtl_debug.h>

struct netconn *conn, *newconn;
pthread_t sequential_tcpserver_thread;

/*-----------------------------------------------------------------------------------*/
static void 
tcpecho_thread(void *arg)
{

  rtl_printf("\n\nEcho TCP server module inserted 0x%x.\n\n", (unsigned int) pthread_self()); 

  /* Create a new connection identifier. */
  conn = netconn_new(NETCONN_TCP);
  
  /* Bind connection to well known port number 7. */
  netconn_bind(conn, NULL, 7);

  /* Tell connection to go into listening mode. */
  netconn_listen(conn);

  while(1) {

    /* Grab new connection. */
    newconn = netconn_accept(conn);

    /* Process the new connection. */
    if(newconn != NULL) {
      struct netbuf *buf;
      void *data;
      u16_t len;

      while((buf = netconn_recv(newconn)) != NULL) {
	do {
	  netbuf_data(buf, &data, &len);
	  rtl_printf("\n\n\n\n\n---------------Echo TCP server receiving: %s --------------------\n\n\n\n\n",data);
	} while(netbuf_next(buf) >= 0);
	netbuf_delete(buf);	
      }
    }
  }
}




/*-----------------------------------------------------------------------------------*/
int init_module(void){
  printk("\n\nEcho TCP server module being inserted.\n\n"); 
  sequential_tcpserver_thread = (pthread_t) sys_thread_new(tcpecho_thread, NULL, 0);
  return 0;
}

/*-----------------------------------------------------------------------------------*/
void cleanup_module(void){
  sys_thread_delete((void *) sequential_tcpserver_thread);
  netconn_delete(conn);
  printk("\n\nEcho TCP server module removed.\n\n"); 
}

