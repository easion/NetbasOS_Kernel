

#include <net/net.h>

static err_t recv_tcp_cb(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err);
static err_t sent_tcp_cb(void *arg, struct tcp_pcb *pcb, unsigned short len);
static void error_tcp_cb(void *arg, err_t err);

static int fill_sndbuf(struct socket *s, struct iovec *iov, int iovlen)
{
  int left;
  int bytes;
  int len;
  int rc;

  left = tcp_sndbuf(s->tcp.pcb);

  if (iovlen == 1)
  {
    if (iov->iov_len == 0) return 0;

    len = iov->iov_len;
    if (len > left) len = left;

    rc = tcp_write(s->tcp.pcb, iov->iov_base, len, (s->flags & SOCK_NODELAY) ? TCP_WRITE_FLUSH : TCP_WRITE_NAGLE);
    if (rc < 0) return rc;

     iov->iov_base =(char *) iov->iov_base + len;
    iov->iov_len -= len;

    return len;
  }
  else
  {
    bytes = 0;
    while (left > 0 && iovlen > 0)
    {
      if (iov->iov_len > 0)
      {
	len = iov->iov_len;
	if (len > left) len = left;

	rc = tcp_write(s->tcp.pcb, iov->iov_base, len, TCP_WRITE_NOFLUSH);
	if (rc < 0) return rc;

	 iov->iov_base = (char *) iov->iov_base+len;
	iov->iov_len -= len;

	left -= len;
	bytes += len;
      }

      iov++;
      iovlen--;
    }

    rc = tcp_write(s->tcp.pcb, NULL, 0, (s->flags & SOCK_NODELAY) ? TCP_WRITE_FLUSH : TCP_WRITE_NAGLE);
    if (rc < 0) return rc;

    return bytes;
  }
}

static int tcp_fetch_rcvbuf(struct socket *s, struct iovec *iov, int iovlen)
{
  int left;
  int recved;
  int rc;
  struct pbuf *p;
  
  left = get_iovec_size(iov, iovlen);
  recved = 0;
  LOCK_SCHED(s->sock_sem);
  while (s->tcp.recvhead && left > 0)
  {
    p = s->tcp.recvhead;

    if (left < p->len)
    {
      rc = write_iovec(iov, iovlen, p->payload, left);
      if (rc < 0){
		  UNLOCK_SCHED(s->sock_sem);
		  return rc;
	  }

      recved += rc;
      left -= rc;

      pbuf_header(p, -rc);
    }
    else
    {
      rc = write_iovec(iov, iovlen, p->payload, p->len);
      if (rc < 0){
		  UNLOCK_SCHED(s->sock_sem);
		  return rc;
	  }

      recved += rc;
      left -= rc;

      s->tcp.recvhead = pbuf_dechain(p);
      if (!s->tcp.recvhead) s->tcp.recvtail = NULL;
      pbuf_free(p);
    }
  }

  UNLOCK_SCHED(s->sock_sem);

  return recved;
}

static int tcp_recv_ready(struct socket *s)
{
  struct pbuf *p;
  int bytes = 0;

  LOCK_SCHED(s->sock_sem);
	p = s->tcp.recvhead;
  while (p)
  {
    bytes += p->len;
    p = p->next;
  }

  UNLOCK_SCHED(s->sock_sem);

  return bytes;
}

static struct socket *accept_tcp_connection(struct tcp_pcb *pcb)
{
  struct socket *s;

  if (new_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP, &s) < 0) return NULL;

  s->state = SOCKSTATE_CONNECTED;
  s->tcp.pcb = pcb;
  tcp_arg(pcb, s);
  tcp_recv(pcb, recv_tcp_cb);
  tcp_sent(pcb, sent_tcp_cb);
  tcp_err(pcb, error_tcp_cb);

  set_io_event(&s->iob, IOEVT_WRITE);

  return s;
}

static err_t accept_tcp(void *arg, struct tcp_pcb *newpcb, err_t err)
{
  struct socket *s = arg;
  struct socket *newsock;
  struct sockreq *req;
  
  req = s->waithead;

  while (req)
  {
    if (req->type == SOCKREQ_ACCEPT)
    {
      if (err < 0)
      {
	release_socket_request(req, err);
	return 0;
      }

      req->newsock = accept_tcp_connection(newpcb);
      release_socket_request(req, err);
      return 0;
    }

    req = req->next;
  }

  if (s->tcp.numpending < s->tcp.backlog)
  {
    if (err < 0) return err;
    newsock = accept_tcp_connection(newpcb);
    if (!newsock) return ENOMEM;

    s->tcp.pending[s->tcp.numpending++] = newsock;
    set_io_event(&s->iob, IOEVT_ACCEPT);

    return 0;
  }

  return EFAULT;
}

static err_t connected_tcp(void *arg, struct tcp_pcb *pcb, err_t err)
{
  struct socket *s = arg;
  struct sockreq *req;

  LOCK_SCHED(s->sock_sem);
  
  req = s->waithead;

  while (req)
  {
    if (req->type == SOCKREQ_CONNECT)
    {
      s->state = SOCKSTATE_CONNECTED;
      set_io_event(&s->iob, IOEVT_WRITE);
      release_socket_request(req, err);
	  UNLOCK_SCHED(s->sock_sem);
      return 0;
    }

    req = req->next;
  }

  s->state = SOCKSTATE_CONNECTED;
  set_io_event(&s->iob, IOEVT_CONNECT | IOEVT_WRITE);
  UNLOCK_SCHED(s->sock_sem);
  
  return 0;
}

static err_t recv_tcp_cb(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
  struct socket *s = arg;
  struct sockreq *req;
  struct sockreq *next;
  int bytes;
  int waitrecv;
  int bytesrecv = 0;

  LOCK_SCHED(s->sock_sem);

  if (p)
  {
    if (s->tcp.recvtail)
    {
      pbuf_chain(s->tcp.recvtail, p);
      s->tcp.recvtail = p;
    }
    else
    {
      s->tcp.recvhead = p;
      s->tcp.recvtail = p;
    }
  }
  else
  {
    s->state = SOCKSTATE_CLOSING;
    set_io_event(&s->iob, IOEVT_CLOSE);
  }

  if (s->state == SOCKSTATE_CLOSING && s->tcp.recvhead == NULL)
  {
    req = s->waithead;
    while (req)
    {
      next = req->next;
      if (req->type == SOCKREQ_RECV || req->type == SOCKREQ_WAITRECV) 
		  release_socket_request(req, 0);
      req = next;
    }

	UNLOCK_SCHED(s->sock_sem);

    return 0;
  }
 

  while (1)
  {
	  unsigned pgdir;

    if (!s->tcp.recvhead) break;

    req = s->waithead;
    waitrecv = 0;

    while (req)
    {
      if (req->type == SOCKREQ_RECV) break;
      if (req->type == SOCKREQ_WAITRECV) 
		  waitrecv++;
      req = req->next;
    }

    if (!req) 
    {
      if (waitrecv)
      {
	req = s->waithead;
	while (req)
	{
	  next = req->next;
	  if (req->type == SOCKREQ_WAITRECV) 
		  release_socket_request(req, EAGAIN);
	  req = next;
	}
      }

      break;
    }

	pgdir = save_pagedir(req->thread->plink);

    bytes = tcp_fetch_rcvbuf(s, req->msg->msg_iov, req->msg->msg_iovlen);

	restore_pagedir(pgdir);

    if (bytes > 0)
    {
      bytesrecv += bytes;
      req->rc += bytes;
      release_socket_request(req, req->rc);
    }
  }

  if (bytesrecv) 
	  tcp_recved(pcb, bytesrecv);
  
  if (s->tcp.recvhead) 
    set_io_event(&s->iob, IOEVT_READ);
  else
    clear_io_event(&s->iob, IOEVT_READ);

  UNLOCK_SCHED(s->sock_sem);

  return 0;
}

static err_t poll_tcp(void *arg, struct tcp_pcb *pcb)
{
  struct socket *s = arg;
  return 0;
}

static err_t sent_tcp_cb(void *arg, struct tcp_pcb *pcb, unsigned short len)
{
  struct socket *s = arg;
  struct sockreq *req;
  int rc;
  unsigned pgdir;


  while (1)
  {
    if (tcp_sndbuf(pcb) == 0) break;

    req = s->waithead;
    while (req)
    {
      if (req->type == SOCKREQ_SEND) break;
      req = req->next;
    }

    if (!req) break;

	pgdir = save_pagedir(req->thread->plink);
    rc = fill_sndbuf(s, req->msg->msg_iov, req->msg->msg_iovlen);
	restore_pagedir(pgdir);

    if (rc < 0)
    {
      release_socket_request(req, rc);
      return rc;
    }

    req->rc += rc;

    if (get_iovec_size(req->msg->msg_iov, req->msg->msg_iovlen) == 0) 
		release_socket_request(req, req->rc);
  }

  if (tcp_sndbuf(pcb) > 0)
    set_io_event(&s->iob, IOEVT_WRITE);
  else
    clear_io_event(&s->iob, IOEVT_WRITE);

  return 0;
}

static void error_tcp_cb(void *arg, err_t err)
{
  struct socket *s = arg;
  struct sockreq *req = s->waithead;
  struct sockreq *next;

  s->tcp.pcb = NULL;
  while (req)
  {
    next = req->next;
    release_socket_request(req, err);
    req = next;
  }

  set_io_event(&s->iob, err == ECONNRESET ? IOEVT_CLOSE : IOEVT_ERROR);
}

static err_t alloc_sock_pcb(struct socket *s)
{
  s->tcp.pcb = tcp_new();
  if (!s->tcp.pcb) return ENOMEM;
  
  tcp_arg(s->tcp.pcb, s);
  tcp_recv(s->tcp.pcb, recv_tcp_cb);
  tcp_sent(s->tcp.pcb, sent_tcp_cb);
  tcp_err(s->tcp.pcb, error_tcp_cb);

  return 0;
}

static int tcpsock_accept(struct socket *s, struct sockaddr *addr, int *addrlen, struct socket **retval)
{
  int rc;
  struct sockreq req;
  struct tcp_pcb *pcb;
  struct socket *newsock;
  struct sockaddr_in *sin;

  if (s->state != SOCKSTATE_LISTENING) 
	  return EINVAL;

  if (s->tcp.numpending == 0)
  {
    if (s->flags & SOCK_NBIO) 
		return EAGAIN;

    rc = submit_socket_request(s, &req, SOCKREQ_ACCEPT, NULL, INFINITE);
    if (rc < 0) return rc;
    newsock = req.newsock;
  }
  else
  {
    newsock = s->tcp.pending[0];
    if (--s->tcp.numpending > 0) 
    {
      memmove(s->tcp.pending, s->tcp.pending + 1, 
		  s->tcp.numpending * sizeof(struct socket *));
    }

    if (s->tcp.numpending == 0) 
		clear_io_event(&s->iob, IOEVT_ACCEPT);
  }

  if (addr) 
  {
    pcb = newsock->tcp.pcb;

    sin = (struct sockaddr_in *) addr;
    sin->sin_family = AF_INET;
    sin->sin_port = htons(pcb->remote_port);
    sin->sin_addr.s_addr = pcb->remote_ip.addr;
  }

  if (addrlen) *addrlen = sizeof(struct sockaddr_in);
  *retval = newsock;
  return 0;
}

static int tcpsock_bind(struct socket *s, struct sockaddr *name, int namelen)
{
  int rc;
  struct sockaddr_in *sin;
  //kprintf("%s() called 1\n",__FUNCTION__);

  if (!name) return EFAULT;
  if (namelen < sizeof(struct sockaddr_in)) return EFAULT;
  sin = (struct sockaddr_in *) name;
  if (sin->sin_family != AF_INET && sin->sin_family != AF_UNSPEC) return EAFNOSUPPORT;

  if (s->state != SOCKSTATE_UNBOUND) return EINVAL;

  if (!s->tcp.pcb)
  {
    rc = alloc_sock_pcb(s);
    if (rc < 0) return rc;
  }

  //kprintf("%s() called tcp_bind\n",__FUNCTION__);

  rc = tcp_bind(s->tcp.pcb, (struct ip_addr *) &sin->sin_addr, ntohs(sin->sin_port));
  if (rc < 0) return rc;

  s->state = SOCKSTATE_BOUND;
  //kprintf("%s() called OK\n",__FUNCTION__);
  return 0;
}

static int tcpsock_close(struct socket *s)
{
  int n;
  struct sockreq *req;
  struct sockreq *next;

  s->state = SOCKSTATE_CLOSED;
  req = s->waithead;
  while (req)
  {
    next = req->next;
    release_socket_request(req, EFAULT);
    req = next;
  }
  
  set_io_event(&s->iob, IOEVT_CLOSE);

  if (s->tcp.pcb)
  {
    tcp_arg(s->tcp.pcb, NULL);
    tcp_sent(s->tcp.pcb, NULL);
    tcp_recv(s->tcp.pcb, NULL);
    tcp_accept(s->tcp.pcb, NULL);
    tcp_err(s->tcp.pcb, NULL);

    if (s->tcp.pcb->state == LISTEN)
    {
      tcp_close(s->tcp.pcb);
      for (n = 0; n < s->tcp.numpending; n++) tcpsock_close(s->tcp.pending[n]); 
      kfree(s->tcp.pending);
    }
    else
    {
      if (tcp_close(s->tcp.pcb) < 0)
      {
	tcp_abort(s->tcp.pcb);
      }
    }
  }

  if (s->tcp.recvhead) pbuf_free(s->tcp.recvhead);

  return 0;
}

static int tcpsock_connect(struct socket *s, struct sockaddr *name, int namelen)
{
  int rc;
  struct sockaddr_in *sin;
  struct sockreq req;

  if (!name) return EFAULT;
  if (namelen < sizeof(struct sockaddr_in)) return EFAULT;
  sin = (struct sockaddr_in *) name;
  if (sin->sin_family != AF_INET && sin->sin_family != AF_UNSPEC) return EAFNOSUPPORT;

  if (s->state == SOCKSTATE_CONNECTED) return EISCONN;
  if (s->state == SOCKSTATE_CONNECTING) return EALREADY;
  if (s->state != SOCKSTATE_BOUND && s->state != SOCKSTATE_UNBOUND) return EINVAL;

  if (!s->tcp.pcb)
  {
    rc = alloc_sock_pcb(s);
    if (rc < 0) return rc;
  }

  s->state = SOCKSTATE_CONNECTING;

  rc = tcp_connect(s->tcp.pcb, (struct ip_addr *) &sin->sin_addr, ntohs(sin->sin_port), connected_tcp);
  if (rc < 0) return rc;

  if (s->state != SOCKSTATE_CONNECTED)
  {
    if (s->flags & SOCK_NBIO) return EAGAIN;
  
    rc = submit_socket_request(s, &req, SOCKREQ_CONNECT, NULL, INFINITE);
    if (rc < 0) return rc;
  }

  return 0;
}

static int tcpsock_getpeername(struct socket *s, struct sockaddr *name, int *namelen)
{
  struct sockaddr_in *sin;

  if (!namelen) return EFAULT;
  if (*namelen < sizeof(struct sockaddr_in)) return EFAULT;
  if (s->state != SOCKSTATE_CONNECTED) return ENOTCONN;
  if (!s->tcp.pcb) return ENOTCONN;

  sin = (struct sockaddr_in *) name;
  sin->sin_family = AF_INET;
  sin->sin_port = htons(s->tcp.pcb->remote_port);
  sin->sin_addr.s_addr = s->tcp.pcb->remote_ip.addr;

  *namelen = sizeof(struct sockaddr_in);
  return 0;
}

static int tcpsock_getsockname(struct socket *s, struct sockaddr *name, int *namelen)
{
  struct sockaddr_in *sin;


  if (!namelen) return EFAULT;

  if (*namelen < sizeof(struct sockaddr_in)) return EFAULT;
#if 0 //fixme
  if (s->state != SOCKSTATE_CONNECTED && s->state != SOCKSTATE_BOUND) return ENOTCONN;
#endif

  if (!s->tcp.pcb) return ENOTCONN;


  sin = (struct sockaddr_in *) name;
  sin->sin_family = AF_INET;
  sin->sin_port = htons(s->tcp.pcb->local_port);
  sin->sin_addr.s_addr = s->tcp.pcb->local_ip.addr;

  *namelen = sizeof(struct sockaddr_in);
  return 0;
}

static int tcpsock_getsockopt(struct socket *s, int level, int optname, char *optval, int *optlen)
{
  return ENOSYS;
}

static int tcpsock_ioctl(struct socket *s, int cmd, void *data, size_t size)
{
  unsigned int timeout;
  int rc;
  struct sockreq req;
  
  switch (cmd)
  {
    case SIOWAITRECV:
      if (!data ) return EFAULT;
      timeout = *(unsigned int *) data;
      if (s->state != SOCKSTATE_CONNECTED) return ENOTCONN;
      if (!s->tcp.pcb) return ENOTCONN;
      if (s->tcp.recvhead != NULL) return 0;

      rc = submit_socket_request(s, &req, SOCKREQ_WAITRECV, NULL, timeout);
      if (rc < 0) return rc;
	  if (rc==0)  return EAGAIN;
      break;

    case FIONREAD:
      if (!data ) return EFAULT;
      *(int *) data = tcp_recv_ready(s);
      break;

    case FIONBIO:
      if (!data ) return EFAULT;
      if (*(int *) data)
	s->flags |= SOCK_NBIO;
      else
	s->flags &= ~SOCK_NBIO;
      break;

    default:
	  kprintf("tcpsock_ioctl unknow %x\n",cmd);
      return ENOSYS;
  }

  return 0;
}

static int tcpsock_listen(struct socket *s, int backlog)
{
  if (s->state == SOCKSTATE_CONNECTED) return EISCONN;
  if (s->state != SOCKSTATE_BOUND) return EINVAL;

  s->tcp.backlog = backlog;
  s->tcp.pending = kmalloc(sizeof(struct socket *) * backlog,0);
  if (!s->tcp.pending) return ENOMEM;

  s->tcp.pcb = tcp_listen(s->tcp.pcb);
  if (!s->tcp.pcb) 
  {
    kfree(s->tcp.pending);
    return ENOMEM;
  }
  
  tcp_accept(s->tcp.pcb, accept_tcp);
  s->state = SOCKSTATE_LISTENING;

  //tcp_debug_print_pcbs();

  return 0;
}

static int tcpsock_recvmsg(struct socket *s, struct msghdr *msg, unsigned int flags)
{
  int rc;
  int size;
  struct sockaddr_in *sin;
  struct sockreq req;

  if (s->state != SOCKSTATE_CONNECTED && s->state != SOCKSTATE_CLOSING) return ENOTCONN;
  if (!s->tcp.pcb) return ECONNRESET;
  //kprintf("%s got line %d\n",__FUNCTION__,__LINE__);

  LOCK_SCHED(s->sock_sem);

  if (s->tcp.pcb && msg->msg_name)
  {
    if (msg->msg_namelen < sizeof(struct sockaddr_in)) return EFAULT;
    sin = (struct sockaddr_in *) msg->msg_name;
    sin->sin_family = AF_INET;
    sin->sin_port = htons(s->tcp.pcb->remote_port);
    sin->sin_addr.s_addr = s->tcp.pcb->remote_ip.addr;
  }
  msg->msg_namelen = sizeof(struct sockaddr_in);
  UNLOCK_SCHED(s->sock_sem);
  //kprintf("%s got line %d\n",__FUNCTION__,__LINE__);

  size = get_iovec_size(msg->msg_iov, msg->msg_iovlen);
  if (size < 0) return EINVAL;
  if (size == 0) return 0;

  rc = tcp_fetch_rcvbuf(s, msg->msg_iov, msg->msg_iovlen);
  if (rc < 0){
	  return rc;
  }

  LOCK_SCHED(s->sock_sem);

  if (!s->tcp.recvhead) 
	  clear_io_event(&s->iob, IOEVT_READ);
  UNLOCK_SCHED(s->sock_sem);
  //kprintf("%s got line %d\n",__FUNCTION__,__LINE__);

  if (rc > 0)
  {
    tcp_recved(s->tcp.pcb, rc);
    return rc;
  }
 
  if (s->state == SOCKSTATE_CLOSING) return 0;
  if (s->flags & SOCK_NBIO) return EAGAIN;

  //kprintf("%s got line %d\n",__FUNCTION__,__LINE__);
  rc = submit_socket_request(s, &req, SOCKREQ_RECV, msg, s->rcvtimeo);
  //kprintf("%s got line %d\n",__FUNCTION__,__LINE__);
  if (rc < 0) return rc;

  if (s->state == SOCKSTATE_CLOSING) return 0;

  if (rc==0)
  {
	kprintf("%s got line %d,%d\n",__FUNCTION__,__LINE__,rc);
	  return EAGAIN;
  }

  return rc; 
}

static int tcpsock_sendmsg(struct socket *s, struct msghdr *msg, unsigned int flags)
{
  int rc;
  int size;
  struct sockreq req;
  int bytes;


  if (s->state != SOCKSTATE_CONNECTED) return ENOTCONN;
  if (!s->tcp.pcb) return ECONNRESET;

  size = get_iovec_size(msg->msg_iov, msg->msg_iovlen);
  if (size == 0){
	  kprintf("%s got line %d\n",__FUNCTION__,__LINE__);
	  return 0;
  }


  rc = fill_sndbuf(s, msg->msg_iov, msg->msg_iovlen);
  if (rc < 0){
	  kprintf("%s got line %d\n",__FUNCTION__,__LINE__);
	  return rc;
  }

  bytes = rc;


  if (tcp_sndbuf(s->tcp.pcb) == 0) 
	  clear_io_event(&s->iob, IOEVT_WRITE);

  if (bytes < size && (s->flags & SOCK_NBIO) == 0)
  {
	  tcp_fasttimer_needed();

  //kprintf("%s got line %d\n",__FUNCTION__,__LINE__);
    rc = submit_socket_request(s, &req, SOCKREQ_SEND, msg, s->sndtimeo);
    if (rc < 0) return rc;

	if (rc==0){
		kprintf("%s got line %d\n",__FUNCTION__,__LINE__);
		return EAGAIN;
	}

    bytes += rc;
  }
  return bytes;
}

static int tcpsock_setsockopt(struct socket *s, int level, int optname, const char *optval, int optlen)
{
  if (level == SOL_SOCKET)
  {
    struct linger *l;

    switch (optname)
    {
      case SO_LINGER:
	if (!optval ) return EFAULT;
	l = (struct linger *) optval;
	s->tcp.lingertime = l->l_linger * HZ;
	if (l->l_onoff)
	  s->flags |= SOCK_LINGER;
	else
	  s->flags &= ~SOCK_LINGER;
	break;

      case SO_DONTLINGER:
	if (!optval ) return EFAULT;
	if (*(int *) optval)
	  s->flags &= ~SOCK_LINGER;
	else
	  s->flags |= SOCK_LINGER;
	break;

      case SO_SNDTIMEO:
	if (optlen != 4) return EINVAL;
	s->sndtimeo = *(unsigned int *) optval;
	break;

	case SO_REUSEADDR:
		if (optlen != 4) return EINVAL;
		s->flags |= *(unsigned int *) optval;
		break;

      case SO_RCVTIMEO:
	//if (optlen != 4) return EINVAL;
	s->rcvtimeo = *(unsigned int *) optval;
	break;

	case SO_ERROR:
		//kprintf("tcpsock_setsockopt SOL_SOCKET cmd=SO_ERROR\n");
		*(int *) optval = 0;
		break;

		case SO_KEEPALIVE:
			if ( *(int*)optval ) {
        s->tcp.pcb->so_options |= SOF_KEEPALIVE;
      } else {
        s->tcp.pcb->so_options &= ~SOF_KEEPALIVE;
      }
      //LWIP_DEBUGF(SOCKETS_DEBUG, ("lwip_setsockopt(%d, SOL_SOCKET, optname=0x%x, ..) -> %s\n", s, optname, (*(int*)optval?"on":"off")));
			kprintf("tcpsock_setsockopt test optname=SO_KEEPALIVE\n");
			break;

      default:
		  kprintf("tcpsock_setsockopt error optname=%d\n",optname);
        return ENOPROTOOPT;
    }
  }
  else if (level == IPPROTO_TCP)
  {
    switch (optname)
    {
      case TCP_NODELAY:
	if (!optval ) return EFAULT;
	if (*(int *) optval)
	  s->flags |= SOCK_NODELAY;
	else
	  s->flags &= ~SOCK_NODELAY;
	break;

      default:
		  kprintf("tcpsock_setsockopt IPPROTO_TCP error optname=%d\n",optname);
        return ENOPROTOOPT;
    }
  }
  else{
	  kprintf("tcpsock_setsockopt error level=%d\n",level);
    return ENOPROTOOPT;
  }

  return 0;
}

static int tcpsock_shutdown(struct socket *s, int how)
{
  return ENOSYS;
}

static int tcpsock_socket(struct socket *s, int domain, int type, int protocol)
{
  return 0;
}

struct sockops tcpops =
{
  tcpsock_accept,
  tcpsock_bind,
  tcpsock_close,
  tcpsock_connect,
  tcpsock_getpeername,
  tcpsock_getsockname,
  tcpsock_getsockopt,
  tcpsock_ioctl,
  tcpsock_listen,
  tcpsock_recvmsg,
  tcpsock_sendmsg,
  tcpsock_setsockopt,
  tcpsock_shutdown,
  tcpsock_socket,
};
