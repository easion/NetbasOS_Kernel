

#ifndef RAW_H
#define RAW_H

struct raw_pcb 
{
  struct ip_addr local_ip;
  struct ip_addr remote_ip;
  int ttl;
  struct raw_pcb *next;
  unsigned short protocol;
  void *recv_arg;

  err_t (*recv)(void *arg, struct raw_pcb *pcb, struct pbuf *p, struct ip_addr *addr);
};

// Application layer interface to the RAW code

struct raw_pcb *raw_new(unsigned short proto);
void raw_remove(struct raw_pcb *pcb);
err_t raw_bind(struct raw_pcb *pcb, struct ip_addr *ipaddr);
err_t raw_connect(struct raw_pcb *pcb, struct ip_addr *ipaddr);
err_t raw_recv(struct raw_pcb *pcb, err_t (*recv)(void *arg, struct raw_pcb *pcb, struct pbuf *p, struct ip_addr *addr), void *recv_arg);
err_t raw_sendto(struct raw_pcb *pcb, struct pbuf *p, struct ip_addr *ipaddr);
err_t raw_send(struct raw_pcb *pcb, struct pbuf *p);

// Lower layer interface to RAW

err_t raw_input(struct pbuf *p, struct netif *inp);
void raw_init();

#endif
