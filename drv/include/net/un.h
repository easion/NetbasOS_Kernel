#ifndef _NETBAS_UN_H_
#define _NETBAS_UN_H_

struct sockaddr_un {
	unsigned short sun_family;	/* AF_UNIX */
	char sun_path[108];		/* pathname */
};

struct unix_pcb 
{
	char local_path[108];
	char remote_path[108];
  //struct ip_addr local_ip;
 // struct ip_addr remote_ip;
  //int ttl;
  struct unix_pcb *remote_pcb;
  struct unix_pcb *next;
  //unsigned short protocol;
  //struct pbuf *pbuf_next; //Êý¾ÝÇø
  //int pbuf_count;

  void *recv_arg; //which socket
  err_t (*recv)(void *arg, struct unix_pcb *pcb, struct pbuf *p);
};


struct unixsocket
{
	struct unix_pcb *pcb;		/* pathname */
	struct pbuf *recvhead;
	struct pbuf *recvtail;

  int backlog;
  int numpending;
  //unsigned int lingertime;
  struct socket **pending;
};
#endif /* _NETBAS_UN_H_ */
