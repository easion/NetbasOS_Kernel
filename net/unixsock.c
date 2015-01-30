

#include <net/net.h>

static struct unix_pcb *unix_pcbs = NULL;
static err_t recv_unix_cb(void *arg, struct unix_pcb *pcb, struct pbuf *p);
static struct unix_pcb * unix_find_pcb(const struct sockaddr_un *sun);
static err_t unix_recv(struct unix_pcb *pcb, int (*recv)(void *arg, struct unix_pcb *upcb, struct pbuf *p), void *recv_arg);
static err_t unix_bind(struct unix_pcb *pcb, struct sockaddr_un *sun);
static struct unix_pcb *unix_new(void) ;


//创建双向管道
int socketpair(int domain, int type, int protocol, struct socket *retval[2])
{
  struct socket *s;
  struct sockaddr_un un;
  struct unix_pcb *pcb = kmalloc(sizeof(struct unix_pcb), 0); //dup it

  if (!pcb)
  {
	  return ENOMEM;
  }


  //memcpy(pcb,rpcb,sizeof(struct unix_pcb));

  if (new_socket(AF_UNIX, SOCK_STREAM, 0, &s) < 0) return NULL;

  s->state = SOCKSTATE_CONNECTED;
  s->type = SOCKTYPE_UNIX;
  s->un.pcb = pcb;
  //pcb->remote_pcb = lpcb;
  //strcpy(pcb->remote_path,lpcb->local_path);
  //unix_recv(pcb, recv_unix_cb,s);

  //strcpy(un.sun_path,pcb->local_path);
  //unix_bind(pcb,&un);
  //lpcb->remote_pcb= pcb;

  set_io_event(&s->iob, IOEVT_WRITE);
  return 0;
}

static int unix_fetch_rcvbuf(struct socket *s, struct iovec *iov, int iovlen)
{
  int left;
  int recved;
  int rc;
  struct pbuf *p;
  
  left = get_iovec_size(iov, iovlen);
  recved = 0;
  LOCK_SCHED(s->sock_sem);
  while (s->un.recvhead && left > 0)
  {
    p = s->un.recvhead;

    if (left < p->len)
    {
      rc = write_iovec(iov, iovlen, p->payload, left);
      if (rc < 0){
		  UNLOCK_SCHED(s->sock_sem);
		  return rc;
	  }

	  s->un.recv_count -= rc;

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

	  s->un.recv_count -= rc;
      recved += rc;
      left -= rc;

      s->un.recvhead = pbuf_dechain(p);
      if (!s->un.recvhead) s->un.recvtail = NULL;
      pbuf_free(p);
    }
  }

  UNLOCK_SCHED(s->sock_sem);

  return recved;
}


static struct socket *accept_uds_connection(struct unix_pcb *rpcb,struct unix_pcb *lpcb)
{
  struct socket *s;
  struct sockaddr_un un;
  struct unix_pcb *pcb = kmalloc(sizeof(struct unix_pcb), 0); //dup it

  if (!pcb)
  {
	  return ENOMEM;
  }

  memcpy(pcb,rpcb,sizeof(struct unix_pcb));


  if (new_socket(AF_UNIX, SOCK_STREAM, 0, &s) < 0) return NULL;

  s->state = SOCKSTATE_CONNECTED;
  s->type = SOCKTYPE_UNIX;
  s->un.recv_count= 0;
  thread_waitq_init(&s->un.recv_wait);
  s->un.pcb = pcb;
  pcb->remote_pcb = lpcb;
  strcpy(pcb->remote_path,lpcb->local_path);
  unix_recv(pcb, recv_unix_cb,s);

  strcpy(un.sun_path,pcb->local_path);
  unix_bind(pcb,&un);
  lpcb->remote_pcb= pcb;

  set_io_event(&s->iob, IOEVT_WRITE);
  return s;
}

static err_t accept_unix(void *arg, struct unix_pcb *newpcb,struct unix_pcb *pcb, err_t err)
{
  struct socket *s = arg;
  struct socket *newsock;
  struct sockreq *req = s->waithead;

  ASSERT(s);
  ASSERT(newpcb);

  //trace("accept_unix req->type=%d called\n",req->type);

  while (req)
  {
    if (req->type == SOCKREQ_ACCEPT)
    {
      if (err < 0)
      {
		  trace("accept_unix() 111 error ...\n");
	release_socket_request(req, err);
	return 0;
      }

      req->newsock = accept_uds_connection(newpcb,pcb);
      release_socket_request(req, err);
      return 0;
    }

    req = req->next;
  }

  if (s->un.numpending < s->un.backlog)
  {
    if (err < 0) return err;
    newsock = accept_uds_connection(newpcb,pcb);
    if (!newsock) return ENOMEM;

    s->un.pending[s->un.numpending++] = newsock;
    set_io_event(&s->iob, IOEVT_ACCEPT);

    return 0;
  }

  return EFAULT;
}


//等待socket
static int unixsock_accept(struct socket *s, struct sockaddr *addr, int *addrlen, struct socket **retval)
{
  int rc;
  struct sockreq req;
  struct unix_pcb *ipcb;
  struct socket *newsock;
  struct sockaddr_un *sun;
  int len=0;

  ASSERT(s);

  if (s->state != SOCKSTATE_LISTENING) 
	  return EINVAL;

  if (s->un.numpending == 0)
  {
    if (s->flags & SOCK_NBIO) 
		return EAGAIN;

	memset(&req,0,sizeof(req));

    rc = submit_socket_request(s, &req, SOCKREQ_ACCEPT, NULL, INFINITE);
	//trace("accept  wakeup %x...\n",req.rc);
    if (rc < 0) return rc;
    newsock = req.newsock;
  }
  else
  {
	  //trace("s->un.numpending ok\n");
    newsock = s->un.pending[0];
    if (--s->un.numpending > 0) 
    {
      memmove(s->un.pending, s->un.pending + 1, 
		  s->un.numpending * sizeof(struct socket *));
    }

    if (s->un.numpending == 0) 
		clear_io_event(&s->iob, IOEVT_ACCEPT);
  }

  if (addr) 
  {
    ipcb = newsock->un.pcb;

    sun = (struct sockaddr_un *) addr;
    sun->sun_family = AF_UNIX;
	len=strlen(sun->sun_path)+1;
    //sun->sin_addr.s_addr = pcb->remote_ip.addr;
  }

  if (addrlen) *addrlen = offsetof(struct sockaddr_un,sun_family)+len;
  *retval = newsock;

  return 0;
}

static err_t unix_bind(struct unix_pcb *pcb, struct sockaddr_un *sun)
{
  unsigned flags;
  struct unix_pcb *ipcb;

  ASSERT(pcb);

  save_eflags(&flags);
  // Insert UDP PCB into the list of active UDP PCBs
  for (ipcb = unix_pcbs; ipcb != NULL; ipcb = ipcb->next) 
  {
    // If it is already on the list, just return
    if (pcb == ipcb) {
		restore_eflags(flags);
		return 0;
	}
	//察看有没有出现重复
    //if (stricmp(sun->sun_path,ipcb->local_path)==0) return 1;//exist
  }

  // We need to place the PCB on the list
  strcpy(pcb->local_path,sun->sun_path);
  pcb->next = unix_pcbs;
  unix_pcbs = pcb;
  restore_eflags(flags);

  return 0;
}

static struct unix_pcb * unix_find_pcb(const struct sockaddr_un *sun)
{
  struct unix_pcb *ipcb;
  unsigned flags;
  

  save_eflags(&flags);
  if (sun->sun_family != AF_UNIX)
	  return NULL;

  // Insert UDP PCB into the list of active UDP PCBs
  for (ipcb = unix_pcbs; ipcb != NULL; ipcb = ipcb->next) 
  {
    // If it is already on the list, just return
    if (!ipcb->remote_pcb && stricmp(sun->sun_path,ipcb->local_path)==0){
		restore_eflags(flags);
		return ipcb;//found
	}
  }
  restore_eflags(flags);
 
	return NULL;
}

//邦定socket的fifo文件
static int unixsock_bind(struct socket *s, struct sockaddr *name, int namelen)
{
  int rc;
  struct sockaddr_un *sun;

  if (!name) return EFAULT;
  if (namelen < offsetof(struct sockaddr_un,sun_family)) return EFAULT;
  sun = (struct sockaddr_un *) name;
  if (sun->sun_family != AF_UNIX && sun->sun_family != AF_UNSPEC) return EAFNOSUPPORT;
  trace("unixsock_bind() %s\n", sun->sun_path);

  if (!s->un.pcb)
  {
    s->un.pcb = unix_new();
    if (!s->un.pcb) return ENOMEM;
	unix_recv(s->un.pcb, recv_unix_cb, s);
  }

  rc = unix_bind(s->un.pcb,  sun );
  if (rc < 0) return rc;
  else if(rc>0){
	  kfree(s->un.pcb);
	  return EFAULT;
  }

  s->state = SOCKSTATE_BOUND;
  return 0;
}

void unix_remove(struct unix_pcb *pcb)
{
  struct unix_pcb *pcb2;
  unsigned flags;
  

  save_eflags(&flags);

  
  if (unix_pcbs == pcb)
  {
    unix_pcbs = unix_pcbs->next;
  } 
  else 
  {
    for (pcb2 = unix_pcbs; pcb2 != NULL; pcb2 = pcb2->next) 
    {
      if (pcb2->next != NULL && pcb2->next == pcb) 
      {
	pcb2->next = pcb->next;
	break;
      }
    }
  }
  restore_eflags(flags);

  kfree(pcb);
}

static int unixsock_close(struct socket *s)
{
  struct sockreq *req;
  struct sockreq *next;
  struct unix_pcb *ipcb;
  struct socket *peer=NULL;

  ipcb = s->un.pcb->remote_pcb;
 

  LOCK_SCHED(s->sock_sem);

  kprintf("unixsock_close called on %p,%d\n",s,current_thread_id(NULL));

  s->state = SOCKSTATE_CLOSED;
  req = s->waithead;
  while (req)
  {
    next = req->next;
    release_socket_request(req, EHOSTDOWN);
    req = next;
  }

  if(ipcb){
   peer = ipcb->recv_arg;
    req = peer->waithead;
  }
  else{
	  req = NULL;
  }

    while (req)
    {
      next = req->next;
      if (req->type == SOCKREQ_RECV ) 
		  release_socket_request(req, EHOSTDOWN);
      req = next;
    }


  if (s->un.pcb)
  {
    s->un.pcb->recv_arg = NULL;
    unix_remove(s->un.pcb);
  }

  struct pbuf *p;


  while (s->un.recvhead )
  {
    p = s->un.recvhead;    

	  s->un.recv_count -= p->len;      

      s->un.recvhead = pbuf_dechain(p);
      if (!s->un.recvhead) s->un.recvtail = NULL;
      pbuf_free(p);
  }


  
  UNLOCK_SCHED(s->sock_sem);

  return 0;
}

static struct unix_pcb *unix_new(void) 
{
  struct unix_pcb *pcb;

  pcb = (struct unix_pcb *) kmalloc(sizeof(struct unix_pcb),0);
  if (pcb != NULL) 
  {
    memset(pcb, 0, sizeof(struct unix_pcb));
    return pcb;
  }

  return NULL;
}

static int unixstat_proc(void *arg, int len,struct proc_entry *pf)
{
  struct unix_pcb *pcb;

  pprintf(pf, "local path      remote path                       \n");
  pprintf(pf, "----------- ----------- --------------- ---------------\n");

  for (pcb = unix_pcbs; pcb != NULL; pcb = pcb->next) 
  {
    pprintf(pf, "%-16s    %-16s   \n", pcb->local_path, pcb->remote_path);
  }

  return 0;
}

//
// unix_init
//
static struct proc_entry e_unixstat_proc = {
	name: "udsstat",
	write_func: unixstat_proc,
	read_func: NULL,
};

void uds_init(void)
{
  unix_pcbs =  NULL;
  register_proc_entry(&e_unixstat_proc);
}

err_t unix_connect(struct unix_pcb *pcb, struct sockaddr_un *sun)
{
  struct unix_pcb *ipcb = unix_find_pcb(sun);

  if (!ipcb)
	  return EINVAL;

  strcpy(pcb->remote_path,sun->sun_path);
  //pcb->remote_pcb= ipcb;

	//唤醒等待的任务
  accept_unix(ipcb->recv_arg, ipcb,pcb,0);
  trace("unix_connect() %s ok\n", sun->sun_path);
  return 0;
}

static err_t unix_recv(struct unix_pcb *pcb, int (*recv)(void *arg, struct unix_pcb *upcb, struct pbuf *p), void *recv_arg)
{
  // Remember recv() callback and user data
  pcb->recv = recv;
  pcb->recv_arg = recv_arg;
  return 0;
}


//连接socket的fifo文件
static int unixsock_connect(struct socket *s, struct sockaddr *name, int namelen)
{
  int rc=0;
  struct sockaddr_un *sun;

  if (!name) return EFAULT;
  if (namelen < offsetof(struct sockaddr_un,sun_family)) return EFAULT;
  sun = (struct sockaddr_un *) name;
  if (sun->sun_family != AF_UNIX && sun->sun_family != AF_UNSPEC) return EAFNOSUPPORT;
  if (s->state == SOCKSTATE_CLOSED) return EINVAL;
  trace("unixsock_connect() %s\n", sun->sun_path);

  if (!s->un.pcb)
  {
    s->un.pcb = unix_new();
    if (!s->un.pcb) return ENOMEM;
	 unix_recv(s->un.pcb, recv_unix_cb, s);
  }

  rc = unix_connect(s->un.pcb, sun);
  if (rc < 0) return rc;

  s->state = SOCKSTATE_CONNECTED;
  return 0;
}

static int unixsock_getpeername(struct socket *s, struct sockaddr *name, int *namelen)
{
  struct sockaddr_un *sun;

  if (!namelen) return EFAULT;
  if (*namelen < offsetof(struct sockaddr_un,sun_family)) return EFAULT;
  if (s->state != SOCKSTATE_CONNECTED) return EINVAL;

  sun = (struct sockaddr_un *) name;
  sun->sun_family = AF_UNIX;
  strcpy(sun->sun_path , s->un.pcb->remote_path);

  *namelen = offsetof(struct sockaddr_un,sun_family)+strlen(sun->sun_path)+1;
  return 0;
}

static int unixsock_getsockname(struct socket *s, struct sockaddr *name, int *namelen)
{
  struct sockaddr_un *sun;

  if (!namelen) return EFAULT;
  if (*namelen < offsetof(struct sockaddr_un,sun_family)) return EFAULT;
  if (s->state != SOCKSTATE_BOUND && s->state != SOCKSTATE_CONNECTED) return EINVAL;

  sun = (struct sockaddr_un *) name;
  sun->sun_family = AF_UNIX;
  //sun->sin_addr.s_addr = s->un.pcb->local_ip.addr;
  strcpy(sun->sun_path , s->un.pcb->local_path);

  *namelen = offsetof(struct sockaddr_un,sun_family)+strlen(sun->sun_path)+1;
  return 0;
}

static int unixsock_getsockopt(struct socket *s, int level, int optname, char *optval, int *optlen)
{
  return ENOSYS;
}

static int unix_recv_ready(struct socket *s)
{
  struct pbuf *p = NULL;
  int bytes = 0;

  LOCK_SCHED(s->sock_sem);

  p = s->un.recvhead;

  while (p)
  {
    bytes += p->len;
    p = p->next;
  }

  UNLOCK_SCHED(s->sock_sem);

  return bytes;
}



static int unixsock_ioctl(struct socket *s, int cmd, void *data, size_t size)
{
  switch (cmd)
  {
    case FIONBIO:
      //if (!data || size != 4) return EFAULT;
      if (*(int *) data)
	s->flags |= SOCK_NBIO;
      else
	s->flags &= ~SOCK_NBIO;
      break;

	  case FIONREAD:
      if (!data ) return EFAULT;
      *(int *) data = unix_recv_ready(s);
      break;


    default:
		kprintf("unixsock_ioctl unknow %x\n",cmd);
      return ENOSYS;
  }

  return 0;
}

static int unixsock_listen(struct socket *s, int backlog)
{
  if (s->state == SOCKSTATE_CONNECTED) return EISCONN;
  if (s->state != SOCKSTATE_BOUND) return EINVAL;

  trace("unixsock_listen() %d\n", backlog);

  s->un.backlog = backlog;
  s->un.pending = kmalloc(sizeof(struct socket *) * backlog,0);
  if (!s->un.pending) return ENOMEM;

  /*s->un.pcb = unix_listen(s->un.pcb);
  if (!s->un.pcb) 
  {
    kfree(s->un.pending);
    return ENOMEM;
  }
  
  unix_accept(s->un.pcb, accept_unix);*/
  s->state = SOCKSTATE_LISTENING;
}

/*fix bug,recv,or store*/
static int unixsock_recvmsg(struct socket *s, struct msghdr *msg, unsigned int flags)
{
  struct pbuf *p;
  char *data;
  int rc;
  struct sockreq req;
  struct sockaddr_un *sun;
  int size, len=0;
  struct unix_pcb *ipcb;
  struct socket *peer=NULL;


  ipcb = s->un.pcb->remote_pcb;

  if (ipcb)
  {
   peer = ipcb->recv_arg;
  }
 

  //struct unix_pcb *ipcb;
  //ipcb = s->un.pcb->remote_pcb;
	
 

  if (msg->msg_name)
    {
      if (msg->msg_namelen < offsetof(struct sockaddr_un,sun_family)) {	
		  kprintf("unixsock_recvmsg error uds name\n");
		  return EFAULT;
	  }
      sun = (struct sockaddr_un *) msg->msg_name;
      sun->sun_family = AF_UNIX;
	  len = strlen(sun->sun_path)+1;
      //sun->sin_addr.s_addr = iphdr->src.addr;
    }
    msg->msg_namelen = offsetof(struct sockaddr_un,sun_family)+len;



	size = get_iovec_size(msg->msg_iov, msg->msg_iovlen);
  if (size < 0){
	  kprintf("unixsock_recvmsg oops size\n");
	  return EINVAL;
  }
  if (size == 0) {
	  kprintf("unixsock_recvmsg error size\n");
	  return 0;
  }

  rc = unix_fetch_rcvbuf(s, msg->msg_iov, msg->msg_iovlen);



	if (rc < 0){
		kprintf("unixsock_recvmsg unix_fetch_rcvbuf error\n");
		return rc;
	}

  //LOCK_SCHED(s->sock_sem);
    if (!s->un.recvhead) {
	//唤醒正在睡眠的接收进程
		clear_io_event(&s->iob, IOEVT_READ);
		 thread_wakeup_all(&s->un.recv_wait,-1);	
	}
  //UNLOCK_SCHED(s->sock_sem);

	if (rc>0)
	{
		//thread_wakeup(&s->un.recv_wait);
		return rc;
	}
 
	 thread_wakeup_all(&s->un.recv_wait,-1);	

	 if (!peer)
	 {
		 kprintf("unixsock_recvmsg sock closeed\n");
		 return 0;
	 }
	 else if (peer->state == SOCKSTATE_CLOSING||peer->state == SOCKSTATE_CLOSED){
	 kprintf("unixsock_recvmsg SOCKSTATE_CLOSING = %d\n",rc);
		 return rc;
	 }
	 else{
	 }

	 if (s->flags & SOCK_NBIO)
	 {
		 kprintf("non block mode\n");
		 return EAGAIN;

	 }


	  //等待数据
    rc = submit_socket_request(s, &req, SOCKREQ_RECV, msg, s->rcvtimeo);

	if (rc>0)
	{
		return rc;
	}
	//else if (rc==0)  return EAGAIN;
	else{
	}

	 //kprintf("unixsock_recvmsg wait %d done\n",current_thread_id(NULL));
	if (peer->state == SOCKSTATE_CLOSING)
	{
	kprintf("unixsock_recvmsg rc= %d\n",rc);
		 rc = unix_fetch_rcvbuf(s, msg->msg_iov, msg->msg_iovlen);
		     if (!s->un.recvhead) {
	//唤醒正在睡眠的接收进程
		clear_io_event(&s->iob, IOEVT_READ);
		 thread_wakeup_all(&s->un.recv_wait,-1);	
	}
	}
	else if(peer->state == SOCKSTATE_CLOSED){
		kprintf("unixsock_recvmsg closeed ??? rc= %d,%d\n",rc,current_thread_id(NULL));
		rc = unix_fetch_rcvbuf(s, msg->msg_iov, msg->msg_iovlen);
		    if (!s->un.recvhead) {
	//唤醒正在睡眠的接收进程
		clear_io_event(&s->iob, IOEVT_READ);
		 thread_wakeup_all(&s->un.recv_wait,-1);	
	}
	}

  return rc;
}

//发送接受数据
static err_t recv_unix_cb(void *arg, struct unix_pcb *pcb, struct pbuf *p)
{
  struct socket *s = arg;
  struct sockreq *req = NULL;
  //struct sockaddr_un *sun;
  int rc;
  int bytes=0;
  struct sockreq *next;

  if (!s || !pcb)
  {
	  return EINVAL;
  }

  ASSERT(s);
  ASSERT(pcb);

  LOCK_SCHED(s->sock_sem);

  if (p)
  {
    if (s->un.recvtail)
    {
      pbuf_chain(s->un.recvtail, p);
      s->un.recvtail = p;
    }
    else
    {
      s->un.recvhead = p;
      s->un.recvtail = p;
    }
  }
  else
  {
    s->state = SOCKSTATE_CLOSED;
    set_io_event(&s->iob, IOEVT_CLOSE);
  }

  if (s->state == SOCKSTATE_CLOSED && s->un.recvhead == NULL)
  {
    req = s->waithead;
    while (req)
    {
      next = req->next;
      if (req->type == SOCKREQ_RECV ) 
		  release_socket_request(req, EAGAIN);
      req = next;
    }

	UNLOCK_SCHED(s->sock_sem);

    return 0;
  }

   while (1)
  {
	   unsigned pgdir;

    if (!s->un.recvhead) break;

    req = s->waithead;  

    if (!req) 
    {   
      break;
    }

	pgdir = save_pagedir(req->thread->plink);

    bytes = unix_fetch_rcvbuf(s, req->msg->msg_iov, req->msg->msg_iovlen);

	restore_pagedir(pgdir);

    if (bytes > 0)
    {
      req->rc += bytes;
      release_socket_request(req, req->rc);
    }
  }
  
  if (s->un.recvhead) 
    set_io_event(&s->iob, IOEVT_READ);
  else
    clear_io_event(&s->iob, IOEVT_READ);

  UNLOCK_SCHED(s->sock_sem);

  //trace("recv_unix_cb() ok called\n");
  return 1;
}


int unix_read_iovec(struct socket *s,struct socket *peer,struct iovec *iov, int iovlen)
{
  size_t read = 0;
  size_t len;
  struct pbuf *p;
  int try_again=0;
  int rc;
  struct unix_pcb *ipcb;
  ipcb = s->un.pcb->remote_pcb;

  while ( iovlen > 0)
  {
	  try_again=0;

    if (iov->iov_len > 0)
    {
      len = iov->iov_len;

	  if (len>UDS_RECV_BYTES)
	  {
		  try_again=1;
	  //kprintf("unixsock_sendmsg: tr size=%d\n",len);
		  len = UDS_RECV_BYTES;
	  }
	  else{
		  try_again=0;
	  }

	    p = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_RW);
		if (!p) return ENOMEM;

      //if (count < iov->iov_len) len = count;

      memcpy(p->payload, iov->iov_base, len);

	  rc = s->un.pcb->recv(ipcb->recv_arg,ipcb, p);
	  if (rc < 0)
	  {
		pbuf_free(p);
		return rc;
	  }

      read += len;
	  peer->un.recv_count += len;

       iov->iov_base =(char *) iov->iov_base +len;
      iov->iov_len -= len;

	  //等待接收完成
	  if (peer->un.recv_count>=UDS_RECV_MAX)
	  {	  
		 // kprintf("unixsock_sendmsg: %d,%d too big go sleep\n",peer->un.recv_count,rc);
		thread_sleep_on(&peer->un.recv_wait);
	  }  

	  if (try_again)
	  {
		  continue;
	  }

      //buf += len;
      //count -= len;
    }

    iov++;
    iovlen--;
  }

  return read;
}

static int unixsock_sendmsg(struct socket *s, struct msghdr *msg, unsigned int flags)
{
  int size;
  int rc;
  struct socket *peer;
  struct unix_pcb *ipcb;
  ipcb = s->un.pcb->remote_pcb;

  if (!ipcb)
  {
	  kprintf("unixsock_sendmsg close called on %p,%d\n",s,current_thread_id(NULL));

	  //socket closed
	  return 0;
  }

  peer = ipcb->recv_arg;
  if (!peer)
  {
	  kprintf("unixsock_sendmsg close2 called on %p\n",s);
	  //socket closed
	  return -1;
  }

	//取得尺寸
  size = get_iovec_size(msg->msg_iov, msg->msg_iovlen);

  

  if (msg->msg_name)
  {
    rc = unixsock_connect(s, msg->msg_name, msg->msg_namelen);
    if (rc < 0) {
		kprintf("unixsock_sendmsg connect error\n");
		return rc;
	}
  }

  if (s->state != SOCKSTATE_CONNECTED) return ENOTCONN;

  

  rc = unix_read_iovec(s,peer,msg->msg_iov, msg->msg_iovlen);
  if (rc < 0){
	  kprintf("unixsock_sendmsg error\n");
  return rc;
  }


  return rc;

}

static int unixsock_setsockopt(struct socket *s, int level, int optname, const char *optval, int optlen)
{

  if (level == SOL_SOCKET)
  {
    switch (optname)
    {
      case SO_SNDTIMEO:
	if (optlen != 4) {
		  kprintf("unixsock_setsockopt SO_SNDTIMEO error\n");
		  return EINVAL;
	  }
	s->sndtimeo = *(unsigned int *) optval;
	break;

      case SO_RCVTIMEO:
	if (optlen != 4){
		  kprintf("unixsock_setsockopt SO_RCVTIMEO error\n");
		  return EINVAL;
	  }
	s->rcvtimeo = *(unsigned int *) optval;
	break;

	case SO_REUSEADDR:
		break;

      default:
		  kprintf("unixsock_setsockopt optname=%d error\n", optname);
        return ENOPROTOOPT;
    }
  }
  else{
	kprintf("unixsock_setsockopt level=%d error\n",level);
    return ENOPROTOOPT;
  }

  return 0;
}

static int unixsock_shutdown(struct socket *s, int how)
{

  struct unix_pcb *ipcb;
  struct socket *peer=NULL;
  struct sockreq *req;
  struct sockreq *next;

  kprintf("unixsock_shutdown called\n");

  s->state = SOCKSTATE_CLOSING;
  ipcb = s->un.pcb->remote_pcb;
  if(ipcb){
   peer = ipcb->recv_arg;
    req = peer->waithead;
  }
  else{
	  req = NULL;
  }

    while (req)
    {
      next = req->next;
      if (req->type == SOCKREQ_RECV ) 
		  release_socket_request(req, EHOSTDOWN);
      req = next;
    }


	//thread_wakeup_all(&s->un.recv_wait,-1);	
 

  //fixme!
  return 0;
}

static int unixsock_socket(struct socket *s, int domain, int type, int protocol)
{
	//trace("unixsock_socket called\n");
  set_io_event(&s->iob, IOEVT_WRITE);
  s->un.recv_count= 0;
  thread_waitq_init(&s->un.recv_wait);

  return 0;
}

struct sockops unixops =
{
  unixsock_accept,
  unixsock_bind,
  unixsock_close,
  unixsock_connect,
  unixsock_getpeername,
  unixsock_getsockname,
  unixsock_getsockopt,
  unixsock_ioctl,
  unixsock_listen,
  unixsock_recvmsg,
  unixsock_sendmsg,
  unixsock_setsockopt,
  unixsock_shutdown,
  unixsock_socket,
};
