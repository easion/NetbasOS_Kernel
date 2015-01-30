
#ifndef SOCKETS_H
#define SOCKETS_H


#define OBJECT_THREAD     0
#define OBJECT_EVENT      1
#define OBJECT_TIMER      2
#define OBJECT_MUTEX      3
#define OBJECT_SEMAPHORE  4
#define OBJECT_FILE       5
#define OBJECT_SOCKET     6
#define OBJECT_IOMUX      7




#define INTRES(x) ((char *)((unsigned long)((unsigned short)(x))))

struct in_addr 
{
  union 
  {
    struct { unsigned char s_b1, s_b2, s_b3, s_b4; } s_un_b;
    struct { unsigned short s_w1, s_w2; } s_un_w;
    unsigned long s_addr;
  };
};

struct sockaddr_in
{
  unsigned char sin_len;
  unsigned char sin_family;
  unsigned short sin_port;
  struct in_addr sin_addr;
  char sin_zero[8];
};

struct sockaddr 
{
  unsigned char sa_len;
  unsigned char sa_family;
  char sa_data[14];
};

#define IFCFG_UP         1
#define IFCFG_DHCP       2
#define IFCFG_DEFAULT    4
#define IFCFG_LOOPBACK   8

#define NET_NAME_MAX 16

struct ifcfg
{
  char name[NET_NAME_MAX];
  int flags;
  char hwaddr[12];
  struct sockaddr addr;
  struct sockaddr gw;
  struct sockaddr netmask;
  struct sockaddr broadcast;
};

struct linger
{
  unsigned short l_onoff;
  unsigned short l_linger;
};

struct msghdr
{
  struct sockaddr *name;
  int namelen;
  struct iovec *iov;
  int iovlen;
};

struct hostent 
{
  char *h_name;        // Official name of host
  char **h_aliases;    // Alias list
  short h_addrtype;    // Host address type
  short h_length;      // Length of address
  char **h_addr_list;  // List of addresses
};

struct protoent
{
  char *p_name;
  char **p_aliases;
  short p_proto;
};

struct servent 
{
  char *s_name;
  char **s_aliases;
  short s_port;
  char *s_proto;
};

#define SOCK_STREAM      1
#define SOCK_DGRAM       2
#define SOCK_RAW         3

#define AF_UNSPEC        0
#define AF_INET          2

#define PF_UNSPEC        AF_UNSPEC
#define PF_INET          AF_INET

#define IPPROTO_IP       0
#define IPPROTO_ICMP     1
#define IPPROTO_TCP      6
#define IPPROTO_UDP      17

#define INADDR_ANY       0
#define INADDR_BROADCAST 0xffffffff
#define INADDR_LOOPBACK  0x7f000001
#define INADDR_NONE      0xffffffff

#define SOL_SOCKET      0xffff

#define SO_REUSEADDR    0x0004
#define SO_BROADCAST    0x0020
#define SO_SNDTIMEO     0x1005
#define SO_RCVTIMEO     0x1006
#define SO_LINGER       0x0080

#define SO_DONTLINGER   ((unsigned int) (~SO_LINGER))

#define TCP_NODELAY     0x0001

#define SD_RECEIVE      0x00
#define SD_SEND         0x01
#define SD_BOTH         0x02


//
// Socket types
//

#define SOCKTYPE_TCP          0
#define SOCKTYPE_UDP          1

#define SOCKTYPE_NUM          2

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



struct tcpsocket
{
  struct tcp_pcb *pcb;

  int backlog;
  int numpending;
  unsigned int lingertime;
  struct socket **pending;

  //struct pbuf *recvhead;
  //struct pbuf *recvtail;
};

struct udpsocket
{
  struct udp_pcb *pcb;
  struct pbuf *recvhead;
  struct pbuf *recvtail;
};

struct socket
{
  //struct ioobject iob;

  int type;
  int state;
  int flags;

  struct sockreq *waithead;
  struct sockreq *waittail;

  unsigned int sndtimeo;
  unsigned int rcvtimeo;

  union 
  {
    struct tcpsocket tcp;
    struct udpsocket udp;
  };
};

struct sockreq
{
  struct sockreq *next;
  struct sockreq *prev;
  struct socket *socket;
  //struct thread *thread;
  int type;
  int rc;
  //struct msghdr *msg;
  struct socket *newsock;
};

struct sockreq;

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

#endif
