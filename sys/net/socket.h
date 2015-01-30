

#ifndef SOCKETS_H
#define SOCKETS_H

#include <jicama/thread.h>
//
// Socket types
//

#define SOCKTYPE_TCP          0
#define SOCKTYPE_UDP          1
#define SOCKTYPE_RAW          2
#define SOCKTYPE_UNIX          3

#define SOCKTYPE_NUM          4

//
// Socket flags
//

#define SOCK_NBIO             1
#define SOCK_NODELAY          2
#define SOCK_BCAST            4
#define SOCK_LINGER           8

//
// Socket state
//

#define SOCKSTATE_UNBOUND     0
#define SOCKSTATE_BOUND       1
#define SOCKSTATE_CLOSING     2
#define SOCKSTATE_CLOSED      3
#define SOCKSTATE_CONNECTING  4
#define SOCKSTATE_CONNECTED   5
#define SOCKSTATE_LISTENING   6

//
// Socket request
//

#define SOCKREQ_ACCEPT        0
#define SOCKREQ_CONNECT       1
#define SOCKREQ_RECV          2
#define SOCKREQ_RECVFROM      3
#define SOCKREQ_SEND          4
#define SOCKREQ_SENDTO        5
#define SOCKREQ_CLOSE         6
#define SOCKREQ_WAITRECV      7

struct sockaddr 
{
  unsigned char sa_len;
  unsigned char sa_family;
  char sa_data[14];
};

/*struct iovec 
{ 
  size_t iov_len;
  void *iov_base;
};
*/
#define UIO_MAXIOV	255


/* Structure for scatter/gather I/O.  */
struct iovec
  {
    void *iov_base;	/* Pointer to data.  */
    size_t iov_len;	/* Length of data.  */
  };


struct tcpsocket
{
  struct tcp_pcb *pcb;

  int backlog;
  int numpending;
  unsigned int lingertime;
  struct socket **pending;

  struct pbuf *recvhead;
  struct pbuf *recvtail;
};

struct udpsocket
{
  struct udp_pcb *pcb;
  struct pbuf *recvhead;
  struct pbuf *recvtail;
};

struct rawsocket
{
  struct raw_pcb *pcb;
  struct pbuf *recvhead;
  struct pbuf *recvtail;
};

//struct unixsocket;



struct unixsocket
{
	struct unix_pcb *pcb;		/* pathname */
	struct pbuf *recvhead;
	struct pbuf *recvtail;

	int recv_count;
	thread_wait_t recv_wait;
	int backlog;
	int numpending;
	struct socket **pending;
};

struct socket
{
  struct ioobject iob;

  int type;
  int state;
  int flags;

  struct sockreq *waithead;
  struct sockreq *waittail;

  unsigned int sndtimeo;
  unsigned int rcvtimeo;
  sem_t sock_sem;

  union 
  {
    struct tcpsocket tcp;
    struct unixsocket un;
    struct udpsocket udp;
    struct rawsocket raw;
  };
};

struct msghdr
{
  struct sockaddr *msg_name;
  int msg_namelen;
  struct iovec *msg_iov;
  int msg_iovlen;
};

struct sockreq
{
  struct sockreq *next;
  struct sockreq *prev;
  struct socket *socket;
  thread_t *thread;
  int type;
  err_t rc;
  struct msghdr *msg;
  struct socket *newsock;
};


struct sockops
{
  int (*accept)(struct socket *s, struct sockaddr *addr, int *addrlen, struct socket **retval);
  int (*bind)(struct socket *s, struct sockaddr *name, int namelen);
  int (*close)(struct socket *s);
  int (*connect)(struct socket *s, struct sockaddr *name, int namelen);
  int (*getpeername)(struct socket *s, struct sockaddr *name, int *namelen);
  int (*getsockname)(struct socket *s, struct sockaddr *name, int *namelen);
  int (*getsockopt)(struct socket *s, int level, int optname, char *optval, int *optlen);
  int (*ioctl)(struct socket *s, int cmd, void *data, size_t size);
  int (*listen)(struct socket *s, int backlog);
  int (*recvmsg)(struct socket *s, struct msghdr *msg, unsigned int flags);
  int (*sendmsg)(struct socket *s, struct msghdr *msg, unsigned int flags);
  int (*setsockopt)(struct socket *s, int level, int optname, const char *optval, int optlen);
  int (*shutdown)(struct socket *s, int how);
  int (*socket)(struct socket *s, int domain, int type, int protocol);
};

err_t submit_socket_request(struct socket *s, struct sockreq *req, int type, struct msghdr *msg, unsigned int timeout);
void release_socket_request(struct sockreq *req, int rc);
void socket_init(void);


struct nf_sockopt_ops
{
	TAILQ_ENTRY(nf_sockopt_ops) next;

	int pf;

	/* Non-inclusive ranges: use 0/0/NULL to never get called. */
	int set_optmin;
	int set_optmax;
	int (*set)(struct socket *sk, int optval, void *user, unsigned int len);

	int get_optmin;
	int get_optmax;
	int (*get)(struct socket *sk, int optval, void *user, int *len);

	/* Number of users inside set() or get(). */
	unsigned int use;
       // struct task_struct *cleanup_task;
};

void nf_unregister_sockopt(struct nf_sockopt_ops *reg);
int nf_register_sockopt(struct nf_sockopt_ops *reg);
int nf_setsockopt(struct socket *s, int level, int optname, const char *optval, int optlen, int *ret);
int nf_getsockopt(struct socket *s, int level, int optname, char *optval, int *optlen, int *ret);

int accept(struct socket *s, struct sockaddr *addr, int *addrlen, struct socket **retval);
int bind(struct socket *s, struct sockaddr *name, int namelen);
int closesocket(struct socket *s);
int connect(struct socket *s, struct sockaddr *name, int namelen);
int getpeername(struct socket *s, struct sockaddr *name, int *namelen);
int getsockname(struct socket *s, struct sockaddr *name, int *namelen);
int getsockopt(struct socket *s, int level, int optname, char *optval, int *optlen);
int ioctlsocket(struct socket *s, int cmd, void *data, size_t size);
int listen(struct socket *s, int backlog);
int recv(struct socket *s, void *data, int size, unsigned int flags);
int recvfrom(struct socket *s, void *data, int size, unsigned int flags, struct sockaddr *from, int *fromlen);
int recvmsg(struct socket *s, struct msghdr *msg, unsigned int flags);
int recvv(struct socket *s, struct iovec *iov, int count);
int send(struct socket *s, void *data, int size, unsigned int flags);
int sendmsg(struct socket *s, struct msghdr *msg, unsigned int flags);
int sendto(struct socket *s, void *data, int size, unsigned int flags, struct sockaddr *to, int tolen);
int sendv(struct socket *s, struct iovec *iov, int count);
int setsockopt(struct socket *s, int level, int optname, const char *optval, int optlen);
int shutdown(struct socket *s, int how);
int new_socket(int domain, int type, int protocol, struct socket **retval);



int write_iovec(struct iovec *iov, int iovlen, char *buf, size_t count);
size_t get_iovec_size(struct iovec *iov, int iovlen);
int read_iovec(struct iovec *iov, int iovlen, char *buf, size_t count);
struct iovec *dup_iovec(struct iovec *iov, int iovlen);




#endif
