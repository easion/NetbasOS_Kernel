/*
 * Copyright (c) 2001, Swedish Institute of Computer Science.
 * All rights reserved. 
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met: 
 * 1. Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright 
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the distribution. 
 * 3. Neither the name of the Institute nor the names of its contributors 
 *    may be used to endorse or promote products derived from this software 
 *    without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND 
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE 
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS 
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY 
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
 * SUCH DAMAGE. 
 *
 * udpserver.c
 *                     
 * Author : Adam Dunkels <adam@sics.se>                               
 *
 * CHANGELOG: this file has been modified by Sergio Perez Alcañiz <serpeal@upvnet.upv.es> 
 *            Departamento de Informática de Sistemas y Computadores          
 *            Universidad Politécnica de Valencia                             
 *            Valencia (Spain)    
 *            Date: March 2003                                          
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include "lwip/api.h"
#include "lwip/sys.h"
#include "rtl_lwipopts.h"

/*-----------------------------------------------------------------------------------*/
void 
udpecho_thread(void *arg)
{
  static struct netconn *conn;
  static struct netbuf *buf;
  char buffer[4096];
  struct ip_addr ipaddr;

  string2ip_addr(&ipaddr,RTETH0_IP, sizeof(RTETH0_IP)); 

  conn = netconn_new(NETCONN_UDP);

  netconn_bind(conn, &ipaddr, 8);

  while(1) {
    buf = netconn_recv(conn);
    netbuf_copy(buf, buffer, sizeof(buffer));
    rtl_printf("------------------- UDP echo server got: %s -----------------\n", buffer);
    netbuf_delete(buf);
  }
}

/*-----------------------------------------------------------------------------------*/
int init_module(void){
  printk("\n\n UDP echo server module inserted\n\n"); 
  sys_thread_new(udpecho_thread, NULL, 0);
 return 0;
}

/*-----------------------------------------------------------------------------------*/
void cleanup_module(void){
  printk("\n UDP echo server module removed\n");

}
