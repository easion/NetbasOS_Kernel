

#ifndef NET_H
#define NET_H

//#ifndef KRNL_H

extern time_t volatile ticks;
#define TICKS_PER_SEC   HZ


#define KERN_EMERG       "<0>"       // System is unusable
#define KERN_ALERT       "<1>"       // Action must be taken immediately
#define KERN_CRIT        "<2>"       // Critical conditions
#define KERN_ERR         "<3>"       // Error conditions
#define KERN_WARNING     "<4>"       // Warning conditions
#define KERN_NOTICE      "<5>"       // Normal but significant condition
#define KERN_INFO        "<6>"       // Informational
#define KERN_DEBUG       "<7>"       // Debug-level messages

#define NET_NAME_MAX 16



#define FIONREAD	0x541B
#define FIONBIO		0x5421

#define SIOWAITRECV   _IOCW('s', 120, unsigned long)

#define SIOIFLIST     _IOCRW('i', 20, void *)           // Get netif list
#define SIOIFCFG      _IOCRW('i', 21, void *)           // Configure netif

#define INADDR_LOOPBACK  0x7f000001
#define INADDR_ANY       0
#define INADDR_BROADCAST 0xffffffff

#define SOCK_STREAM      1
#define SOCK_DGRAM       2
#define SOCK_RAW         3

#define AF_UNIX		1	/* Unix domain sockets 		*/
#define AF_LOCAL	1	/* POSIX name for AF_UNIX	*/

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

#define SOL_SOCKET	1

#define SO_DEBUG	1
#define SO_REUSEADDR	2
#define SO_TYPE		3
#define SO_ERROR	4
#define SO_DONTROUTE	5
#define SO_BROADCAST	6
#define SO_SNDBUF	7
#define SO_RCVBUF	8
#define SO_SNDBUFFORCE	32
#define SO_RCVBUFFORCE	33
#define SO_KEEPALIVE	9
#define SO_OOBINLINE	10
#define SO_NO_CHECK	11
#define SO_PRIORITY	12
#define SO_LINGER	13
#define SO_BSDCOMPAT	14
/* To add :#define SO_REUSEPORT 15 */
#define SO_PASSCRED	16
#define SO_PEERCRED	17
#define SO_RCVLOWAT	18
#define SO_SNDLOWAT	19
#define SO_RCVTIMEO	20
#define SO_SNDTIMEO	21

/* Security levels - as per NRL IPv6 - don't actually do anything */
#define SO_SECURITY_AUTHENTICATION		22
#define SO_SECURITY_ENCRYPTION_TRANSPORT	23
#define SO_SECURITY_ENCRYPTION_NETWORK		24

#define SO_BINDTODEVICE	25

/* Socket filtering */
#define SO_ATTACH_FILTER        26
#define SO_DETACH_FILTER        27

#define SO_PEERNAME		28
#define SO_TIMESTAMP		29
#define SCM_TIMESTAMP		SO_TIMESTAMP

#define SO_ACCEPTCONN		30

#define SO_PEERSEC		31
#define SO_PASSSEC		34
#define SO_TIMESTAMPNS		35
#define SCM_TIMESTAMPNS		SO_TIMESTAMPNS


#define SO_DONTLINGER   ((unsigned int) (~SO_LINGER))

#define TCP_NODELAY     0x0001

#define SHUT_RD         0x00
#define SHUT_WR         0x01
#define SHUT_RDWR       0x02

#define IFCFG_UP         1
#define IFCFG_DHCP       2
#define IFCFG_DEFAULT    4
#define IFCFG_LOOPBACK   8

struct linger
{
  unsigned short l_onoff;
  unsigned short l_linger;
};


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



#endif
