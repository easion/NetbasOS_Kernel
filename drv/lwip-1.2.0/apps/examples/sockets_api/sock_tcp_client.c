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
#include "lwip/sockets.h"
#include "lwip/inet.h"

//#define htons HTONS
//#define htonl HTONL

#define LOCAL_PORT 10
#define REMOTE_PORT 7
#define REMOTE_SERVER "158.42.58.139"
#define SEND_PHRASE "MOOOOLA"

/*-----------------------------------------------------------------------------------*/
static void sock_udpclient(void *arg){
  int s, error;
  struct sockaddr_in localAddr, servAddr;
  struct in_addr addr;

  inet_aton(REMOTE_SERVER,&addr);

  s = socket(AF_INET, SOCK_STREAM, 0);

  /* bind any port number */
  localAddr.sin_family = AF_INET;
  localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  localAddr.sin_port = htons(LOCAL_PORT);

  error = bind(s, (struct sockaddr *) &localAddr, sizeof(localAddr));
  if(error<0)
    rtl_printf("ERROR: cannot bind port TCP %u\n",LOCAL_PORT);

  servAddr.sin_family = AF_INET;
  servAddr.sin_addr.s_addr = addr.s_addr;
  servAddr.sin_port = htons(REMOTE_PORT);

  /* connect to server */
  error = connect(s, (struct sockaddr *) &servAddr, sizeof(servAddr));
  if(error<0)
    rtl_printf("cannot connect ");

  rtl_printf("Socket TCP echo client sending: %s\n",SEND_PHRASE); 
  error = send(s,SEND_PHRASE , sizeof(SEND_PHRASE), 0);
    
  if(error<0) {
    rtl_printf("cannot send data ");
  }

  close(s);

  sys_thread_exit();
}
/*-----------------------------------------------------------------------------------*/
int init_module(void){
  printk("\n\n Socket TCP echo client module inserted\n\n"); 
  sys_thread_new(sock_udpclient, NULL, 1000);
 return 0;
}

/*-----------------------------------------------------------------------------------*/
void cleanup_module(void){
  printk("\n Socket TCP echo client module removed\n");
}
