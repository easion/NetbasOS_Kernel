

#include <net/net.h>

extern struct sockops tcpops;
extern struct sockops udpops;
extern struct sockops rawops;
extern struct sockops unixops;
static sem_t ioobj_sem;

struct sockops *sockops[SOCKTYPE_NUM];

void socket_init()
{
  sockops[SOCKTYPE_TCP] = &tcpops;
  sockops[SOCKTYPE_UDP] = &udpops;
  sockops[SOCKTYPE_RAW] = &rawops;
  sockops[SOCKTYPE_UNIX] = &unixops;
}

static void cancel_socket_request(struct sockreq *req)
{
  LOCK_SCHED(ioobj_sem);

  //req->thread = NULL;

  if (req->next) 
	  req->next->prev = req->prev;
  if (req->prev) 
	  req->prev->next = req->next;

  req->next = NULL;
  req->prev = NULL;

  if (req->socket)
  {
    if (req == req->socket->waithead) 
		req->socket->waithead = req->next;
    if (req == req->socket->waittail) 
		req->socket->waittail = req->prev;
  }
  UNLOCK_SCHED(ioobj_sem);
}

void release_socket_request(struct sockreq *req, int rc)
{
	thread_t *thread;

	thread = req->thread;

 cancel_socket_request(req);
  req->rc = rc;
  req->thread=NULL;
  req->socket = NULL;

   if (!thread)
   {
   kprintf("release_socket_request() error called(rc=0x%x)\n",rc);
	   return;
   }

  if(rc<0){
  thread->timeout_signaled = -1;
  }
  else{
	  thread->timeout_signaled = 0;
  }
   //kprintf("release_socket_request() %p called(rc=0x%x)\n",thread,rc);

  thread_ready(thread);
}


err_t submit_socket_request(struct socket *s, struct sockreq *req, int type, struct msghdr *msg, unsigned int timeout)
{
 // struct timer timer;
  int rc;
  thread_t *th = current_thread();

  if (timeout == 0){
	  kprintf("submit_socket_request sock%d error  ....\n", s->type);
	  return ETIMEDOUT;
  }

  LOCK_SCHED(ioobj_sem);

  req->socket = s;
  req->thread = th;
  req->type = type;
  req->msg = msg;
  req->rc = 0;
  req->next = NULL;
  req->prev = s->waittail;

  if (s->waittail){
	  s->waittail->next = req;
  }
  s->waittail = req;
  if (!s->waithead) s->waithead = req;

  UNLOCK_SCHED(ioobj_sem);

  //kprintf("submit_socket_request sock%d go  ....\n", s->type);

  rc = thread_wait(th,INFINITE);
  //kprintf("submit_socket_request sock%d got data\n", s->type);

  if (rc<0)
  {
  kprintf("submit_socket_request  timeout %d rc= %x %p...\n", rc,req->rc,req->thread);
    //cancel_socket_request(req);
    //return ETIME;
  } 

  return req->rc;
}

int accept(struct socket *s, struct sockaddr *addr, int *addrlen, struct socket **retval)
{
  int ntype = s->type;

  if (ntype<0 || ntype>=SOCKTYPE_NUM)  {
	  kprintf("accept() error socket type = %d\n", ntype);
	  return EFAULT;
  }
  return sockops[ntype]->accept(s, addr, addrlen, retval);
}

int bind(struct socket *s, struct sockaddr *name, int namelen)
{
  int ntype = s->type;

  if (ntype<0 || ntype>=SOCKTYPE_NUM)  {
	  kprintf("bind() error socket type = %d\n", ntype);
	  return EFAULT;
  }

  return sockops[ntype]->bind(s, name, namelen);
}

int closesocket(struct socket *s)
{
  int rc;
  int ntype = s->type;

  if (!s)
  {
	   return EFAULT;
  }

  if (ntype<0 || ntype>=SOCKTYPE_NUM)  {
	  kprintf("closesocket() error socket type = %d\n", ntype);
	  return EFAULT;
  }

  rc = sockops[ntype]->close(s);
  detach_ioobject(&s->iob);
  kfree(s);
  return rc;
}

int connect(struct socket *s, struct sockaddr *name, int namelen)
{
  int ntype = s->type;
  if (ntype<0 || ntype>=SOCKTYPE_NUM)  {
	  kprintf("connect() error socket type = %d\n", ntype);
	  return EFAULT;
  }

  return sockops[ntype]->connect(s, name, namelen);
}

int getpeername(struct socket *s, struct sockaddr *name, int *namelen)
{
  int ntype = s->type;
  if (ntype<0 || ntype>=SOCKTYPE_NUM)  {
	  kprintf("getpeername() error socket type = %d\n", ntype);
	  return EFAULT;
  }
  return sockops[ntype]->getpeername(s, name, namelen);
}

int getsockname(struct socket *s, struct sockaddr *name, int *namelen)
{
  int ntype = s->type;
  if (ntype<0 || ntype>=SOCKTYPE_NUM)  {
	  kprintf("getsockname() error socket type = %d\n", ntype);
	  return EFAULT;
  }

  return sockops[ntype]->getsockname(s, name, namelen);
}

int getsockopt(struct socket *s, int level, int optname, char *optval, int *optlen)
{
  int ef,ret;
  int ntype = s->type;
  if (ntype<0 || ntype>=SOCKTYPE_NUM)  {
	  kprintf("getsockopt() error socket type = %d\n", ntype);
	  return EFAULT;
  }

  ef = nf_getsockopt(s, level, optname, optval, optlen,&ret);

  if (ef) {
	  return ret;
  }
  return sockops[ntype]->getsockopt(s, level, optname, optval, optlen);
}

int ioctlsocket(struct socket *s, int cmd, void *data, size_t size)
{
  int ntype = s->type;
  if (cmd == SIOIFLIST)
    return netif_ioctl_list(data, size);
  else if (cmd == SIOIFCFG){
	  //kprintf("netif_ioctl_cfg 111\n");
    return netif_ioctl_cfg(data, size);
  }
  else
    return sockops[ntype]->ioctl(s, cmd, data, size);
}

int listen(struct socket *s, int backlog)
{
  int ntype = s->type;

  if (ntype<0 || ntype>=SOCKTYPE_NUM)  {
	  kprintf("listen() error socket type = %d\n", ntype);
	  return EFAULT;
  }

  return sockops[ntype]->listen(s, backlog);
}

int recv(struct socket *s, void *data, int size, unsigned int flags)
{
  struct msghdr msg;
  struct iovec iov;
  int rc;
  int ntype = s->type;



  if (ntype<0 || ntype>=SOCKTYPE_NUM) {
	  kprintf("recv() error socket type = %d\n", ntype);
	  return EFAULT;
  }

  //kprintf("recv() 111 = %d\n", size);
  if (!data) return EFAULT;
  if (size < 0) return EINVAL;

  msg.msg_name = NULL;
  msg.msg_namelen = 0;
  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;
  iov.iov_base = data;
  iov.iov_len = size;

  rc = sockops[ntype]->recvmsg(s, &msg, flags);

  return rc;
}

int recvmsg(struct socket *s, struct msghdr *msg, unsigned int flags)
{
  struct msghdr m;
  int rc;
  int ntype = s->type;

  if (ntype<0 || ntype>=SOCKTYPE_NUM) {
	  kprintf("recvmsg() error socket type = %d\n", ntype);
	  return EFAULT;
  }


  if (!msg) return EINVAL;
  m.msg_name = msg->msg_name;
  m.msg_namelen = msg->msg_namelen;
  m.msg_iov = dup_iovec(msg->msg_iov, msg->msg_iovlen);
  m.msg_iovlen = msg->msg_iovlen;
  if (!m.msg_iov) return ENOMEM;

  rc = sockops[ntype]->recvmsg(s, &m, flags);
  msg->msg_namelen = m.msg_namelen;
  kfree(m.msg_iov);

  return rc;
}

int recvv(struct socket *s, struct iovec *iov, int count)
{
  struct msghdr msg;
  int rc;
  int ntype = s->type;

  if (ntype<0 || ntype>=SOCKTYPE_NUM) {
	  kprintf("recvv() error socket type = %d\n", ntype);
	  return EFAULT;
  }

  msg.msg_name = NULL;
  msg.msg_namelen = 0;
  msg.msg_iov = dup_iovec(iov, count);
  msg.msg_iovlen = count;
  if (!msg.msg_iov) return ENOMEM;

  rc = sockops[ntype]->recvmsg(s, &msg, 0);
  kfree(msg.msg_iov);

  return rc;
}

int recvfrom(struct socket *s, void *data, int size, unsigned int flags,
	struct sockaddr *from, int *fromlen)
{
  struct msghdr msg;
  struct iovec iov;
  int rc;
  int ntype = s->type;

  if (ntype<0 || ntype>=SOCKTYPE_NUM) {
	  kprintf("recvfrom() error socket type = %d\n", ntype);
	  return EFAULT;
  }

  if (!data) return EFAULT;
  if (size < 0) return EINVAL;

  msg.msg_name = from;
  msg.msg_namelen = fromlen ? *fromlen : 0;
  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;
  iov.iov_base = data;
  iov.iov_len = size;

  rc = sockops[ntype]->recvmsg(s, &msg, flags);
  if (fromlen) *fromlen = msg.msg_namelen;

  return rc;
}

int send(struct socket *s, void *data, int size, unsigned int flags)
{
  struct msghdr msg;
  struct iovec iov;
  int rc;
  int ntype = s->type;

  if (ntype<0 || ntype>=SOCKTYPE_NUM) {
	  kprintf("send() error socket type = %d\n", ntype);
	  return EFAULT;
  }

  if (!data) return EFAULT;
  if (size < 0) return EINVAL;

  msg.msg_name = NULL;
  msg.msg_namelen = 0;
  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;
  iov.iov_base = data;
  iov.iov_len = size;

  rc = sockops[ntype]->sendmsg(s, &msg, flags);

  return rc;
}

int sendmsg(struct socket *s, struct msghdr *msg, unsigned int flags)
{
  struct msghdr m;
  int rc;
  int ntype = s->type;

  if (ntype<0 || ntype>=SOCKTYPE_NUM) {
	  kprintf("sendmsg() error socket type = %d\n", ntype);
	  return EFAULT;
  }

  if (!msg) return EINVAL;
  m.msg_name = msg->msg_name;
  m.msg_namelen = msg->msg_namelen;
  m.msg_iov = dup_iovec(msg->msg_iov, msg->msg_iovlen);
  m.msg_iovlen = msg->msg_iovlen;
  if (!m.msg_iov) return ENOMEM;

  rc = sockops[ntype]->sendmsg(s, &m, flags);
  kfree(m.msg_iov);

  return rc;
}

int sendto(struct socket *s, void *data, int size, unsigned int flags, struct sockaddr *to, int tolen)
{
  struct msghdr msg;
  struct iovec iov;
  int rc;
  int ntype = s->type;

  if (ntype<0 || ntype>=SOCKTYPE_NUM) {
	  kprintf("sendto() error socket type = %d\n", ntype);
	  return EFAULT;
  }

  if (!data) return EFAULT;
  if (size < 0) return EINVAL;

  msg.msg_name = to;
  msg.msg_namelen = tolen;
  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;
  iov.iov_base = data;
  iov.iov_len = size;

  rc = sockops[ntype]->sendmsg(s, &msg, flags);

  return rc;
}

int sendv(struct socket *s, struct iovec *iov, int count)
{
  struct msghdr msg;
  int rc;
  int ntype = s->type;

  if (ntype<0 || ntype>=SOCKTYPE_NUM) {
	  kprintf("sendv() error socket type = %d\n", ntype);
	  return EFAULT;
  }

  msg.msg_name = NULL;
  msg.msg_namelen = 0;
  msg.msg_iov = dup_iovec(iov, count);
  msg.msg_iovlen = count;
  if (!msg.msg_iov) return ENOMEM;

  rc = sockops[ntype]->sendmsg(s, &msg, 0);
  kfree(msg.msg_iov);

  return rc;
}

int setsockopt(struct socket *s, int level, int optname, const char *optval, int optlen)
{
	int ef,ret;
  int ntype = s->type;

  if (ntype<0 || ntype>=SOCKTYPE_NUM) {
	  kprintf("setsockopt() error socket type = %d\n", ntype);
	  return EFAULT;
  }

  ef = nf_setsockopt(s, level, optname, optval, optlen,&ret);

  if (ef) {
	  return ret;
  }

  return sockops[ntype]->setsockopt(s, level, optname, optval, optlen);
}

int shutdown(struct socket *s, int how)
{
  int ntype = s->type;

  if (ntype<0 || ntype>=SOCKTYPE_NUM) {
	  kprintf("shutdown() error socket type = %d\n", ntype);
	  return EFAULT;
  }

  return sockops[ntype]->shutdown(s, how);
}



int new_socket(int domain, int type, int protocol, struct socket **retval)
{
  struct socket *s;
  int socktype=-1;
  int rc;

  if (domain != AF_INET&&domain != AF_UNIX) 
	  return EAFNOSUPPORT;

  switch (type)
  {
    case SOCK_STREAM:
	if(domain == AF_UNIX)
		socktype=SOCKTYPE_UNIX; //unix domain socket
    else if (protocol == IPPROTO_IP)
	socktype = SOCKTYPE_TCP;
      else if (protocol == IPPROTO_TCP)
	socktype = SOCKTYPE_TCP;	  
      else
	return EPROTONOSUPPORT;
      break;

    case SOCK_DGRAM:
      if (protocol == IPPROTO_IP)
	socktype = SOCKTYPE_UDP;
      else if (protocol == IPPROTO_UDP)
	socktype = SOCKTYPE_UDP;
      else
	return EPROTONOSUPPORT;
      break;

    case SOCK_RAW:
      socktype = SOCKTYPE_RAW;
      break;

    default:
      return EPROTONOSUPPORT;
  }

  s = (struct socket *) kmalloc(sizeof(struct socket),0);
  if (!s) return ENOMEM;
  memset(s, 0, sizeof(struct socket));

  init_ioobject(&s->iob,"NET");
  s->type = socktype;
  s->state = SOCKSTATE_UNBOUND;
  s->sndtimeo = INFINITE;
  s->rcvtimeo = INFINITE;
  s->sock_sem = NULL;
  //s->sock_sem = create_semaphore("sock_sem",0,loopback_packets);

  ASSERT(socktype!=-1);

  //kprintf("sockops type %d\n",socktype);

  rc = sockops[socktype]->socket(s, domain, type, protocol);
  if (rc < 0) return rc;

  *retval = s;
  return 0;
}
