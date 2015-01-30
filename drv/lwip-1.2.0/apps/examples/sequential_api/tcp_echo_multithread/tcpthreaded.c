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

/*-----------------------------------------------------------------------------------*/
static void 
serve_echo(void *arg)
{
  struct netconn *newconn;

  /* Grab new connection. */
  newconn = (struct netconn *) arg; //netconn_accept(conn);

  rtl_printf("accepted new connection %p by thread 0x%x\n", newconn,(unsigned int)pthread_self());

  /* Process the new connection. */
  if(newconn != NULL) {
    struct netbuf *buf;
    void *data;
    u16_t len;
    
    while((buf = netconn_recv(newconn)) != NULL) {
      do {
	netbuf_data(buf, &data, &len);
	rtl_printf("Got : %s\n", data);
      } while(netbuf_next(buf) >= 0);
      netbuf_delete(buf);	
    }

    /* Close connection and discard connection identifier. */
    netconn_close(newconn);
    netconn_delete(newconn);
  }
  sys_thread_exit();  
}

/*-----------------------------------------------------------------------------------*/
static void 
tcpecho_thread(void *arg)
{
  struct netconn *conn;

   /* Create a new connection identifier. */
  conn = netconn_new(NETCONN_TCP);

  /* Bind connection to well known port number 7. */
  netconn_bind(conn, NULL, 7);

  /* Tell connection to go into listening mode. */
  netconn_listen(conn);

  while(1) {
    /* Grab new connection. */
    sys_thread_new(serve_echo, netconn_accept(conn), 0);
  }
}

/*-----------------------------------------------------------------------------------*/
int init_module(void){
  printk("\n Multithreaded TCP echo server inserted\n");
  sys_thread_new(tcpecho_thread, NULL, 0);  
  return 0;
}

/*-----------------------------------------------------------------------------------*/
void cleanup_module(void){
  printk("\n Multithreaded TCP echo server removed\n");
}
