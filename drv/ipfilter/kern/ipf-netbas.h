#ifndef __IPF_LINUX_H__
#define __IPF_LINUX_H__

#if defined(KERNEL) || defined(_KERNEL)


#include <drv/drv.h>
#include <drv/defs.h>
#include <drv/drv.h>
#include <drv/sym.h>
#include <drv/pci.h>
#include <drv/ia.h>
#include <drv/timer.h>
#include <drv/errno.h>
#include <drv/spin.h>
#include <drv/log.h>
#include <net/pkt.h>



#include <net/ipaddr.h>
#include <net/stats.h>
#include <net/opt.h>
#include <net/pbuf.h>
#include <net/ether.h>
#include <net/inet.h>
#include <net/netif.h>
#include <net/arp.h>
#include <net/icmp.h>
#include <net/ip.h>
#include <net/raw.h>
#include <net/un.h>
#include <net/udp.h>
#include <net/tcp.h>
#include <net/socket.h>
#include <net/net.h>
#include <drv/nf_hook.h>


#include <drv/ioctl.h>
#include <drv/console.h>



#define SOCK_STREAM      1
#define SOCK_DGRAM       2
#define SOCK_RAW         3

/* Supported address families. */
#define AF_UNSPEC       0
#define AF_UNIX         1       /* Unix domain sockets          */
#define AF_LOCAL        1       /* POSIX name for AF_UNIX       */
#define AF_INET         2       /* Internet IP Protocol         */
#define AF_AX25         3       /* Amateur Radio AX.25          */
#define AF_IPX          4       /* Novell IPX                   */
#define AF_APPLETALK    5       /* AppleTalk DDP                */
#define AF_NETROM       6       /* Amateur Radio NET/ROM        */
#define AF_BRIDGE       7       /* Multiprotocol bridge         */
#define AF_ATMPVC       8       /* ATM PVCs                     */
#define AF_X25          9       /* Reserved for X.25 project    */
#define AF_INET6        10      /* IP version 6                 */
#define AF_ROSE         11      /* Amateur Radio X.25 PLP       */
#define AF_DECnet       12      /* Reserved for DECnet project  */
#define AF_NETBEUI      13      /* Reserved for 802.2LLC project*/
#define AF_SECURITY     14      /* Security callback pseudo AF */
#define AF_KEY   15      /* PF_KEY key management API */
#define AF_NETLINK      16
#define AF_ROUTE        AF_NETLINK /* Alias to emulate 4.4BSD */
#define AF_PACKET       17      /* Packet family                */
#define AF_ASH          18      /* Ash                          */
#define AF_ECONET       19      /* Acorn Econet                 */
#define AF_ATMSVC       20      /* ATM SVCs                     */
#define AF_SNA          22      /* Linux SNA Project (nutters!) */
#define AF_IRDA         23      /* IRDA sockets                 */
#define AF_MAX          32      /* For now.. */


#define INADDR_ANY       0

#define KERN_EMERG       "<0>"       // System is unusable
#define KERN_ALERT       "<1>"       // Action must be taken immediately
#define KERN_CRIT        "<2>"       // Critical conditions
#define KERN_ERR         "<3>"       // Error conditions
#define KERN_WARNING     "<4>"       // Warning conditions
#define KERN_NOTICE      "<5>"       // Normal but significant condition
#define KERN_INFO        "<6>"       // Informational
#define KERN_DEBUG       "<7>"       // Debug-level messages


typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned long dword;
typedef unsigned long u_long;
typedef unsigned short u_short;
typedef unsigned char u_char;
typedef unsigned int u_int;

typedef void *usb_malloc_type;
typedef void *caddr_t;
typedef u8_t u_int8_t;
typedef u16_t u_int16_t;
typedef u32_t u_int32_t;

typedef struct 
{
	int lock;
}rwlock_t;

#endif

#if defined(KERNEL) || defined(_KERNEL)
# undef KERNEL
# undef _KERNEL
# define        KERNEL	1
# define        _KERNEL	1
#endif



typedef unsigned int* uintptr_t;
typedef int* intptr_t;


struct	ipftcphdr	{
	u16_t	th_sport;
	u16_t	th_dport;
	u32_t	th_seq;
	u32_t	th_ack;
# if defined(__i386__) || defined(__MIPSEL__) || defined(__alpha__) ||\
	defined(__vax__)  || defined(__x86_64__)
	u8_t	th_res:4;
	u8_t	th_off:4;
#else
	u8_t	th_off:4;
	u8_t	th_res:4;
#endif
	u8_t	th_flags;
	u16_t	th_win;
	u16_t	th_sum;
	u16_t	th_urp;
};

typedef	u32_t	tcp_seq;

struct	ipfudphdr	{
	u16_t	uh_sport;
	u16_t	uh_dport;
	u16_t	uh_ulen;
	u16_t	uh_sum;
};

struct	ip	{
# if defined(__i386__) || defined(__MIPSEL__) || defined(__alpha__) ||\
	defined(__vax__)
	u8_t	ip_hl:4;
	u8_t	ip_v:4;
# else
	u8_t	ip_v:4;
	u8_t	ip_hl:4;
# endif
	u8_t	ip_tos;
	u16_t	ip_len;
	u16_t	ip_id;
	u16_t	ip_off;
	u8_t	ip_ttl;
	u8_t	ip_p;
	u16_t	ip_sum;
	struct	in_addr ip_src;
	struct	in_addr ip_dst;
};

/*
 * Structure of an icmp header.
 */
struct icmp {
	u8_t	icmp_type;		/* type of message, see below */
	u8_t	icmp_code;		/* type sub code */
	u16_t	icmp_cksum;		/* ones complement cksum of struct */
	union {
		u8_t	ih_pptr;		/* ICMP_PARAMPROB */
		struct	in_addr ih_gwaddr;	/* ICMP_REDIRECT */
		struct	ih_idseq {
			u16_t	icd_id;
			u16_t	icd_seq;
		} ih_idseq;
		u32_t	 ih_void;
		struct	ih_pmtu	{
			u16_t	ipm_void;
			u16_t	ipm_nextmtu;
		} ih_pmtu;
	} icmp_hun;
# define	icmp_pptr	icmp_hun.ih_pptr
# define	icmp_gwaddr	icmp_hun.ih_gwaddr
# define	icmp_id		icmp_hun.ih_idseq.icd_id
# define	icmp_seq	icmp_hun.ih_idseq.icd_seq
# define	icmp_void	icmp_hun.ih_void
# define	icmp_nextmtu	icmp_hun.ih_pmtu.ipm_nextmtu
	union {
		struct id_ts {
			u32_t	its_otime;
			u32_t	its_rtime;
			u32_t	its_ttime;
		} id_ts;
		struct id_ip	{
			struct	ip	idi_ip;
			/* options and then 64 bits of data */
		} id_ip;
		u_long	id_mask;
		char	id_data[1];
	} icmp_dun;
# define	icmp_otime	icmp_dun.id_ts.its_otime
# define	icmp_rtime	icmp_dun.id_ts.its_rtime
# define	icmp_ttime	icmp_dun.id_ts.its_ttime
# define	icmp_ip	 icmp_dun.id_ip.idi_ip
# define	icmp_mask	icmp_dun.id_mask
# define	icmp_data	icmp_dun.id_data
};

# ifndef LINUX_IPOVLY
#	define LINUX_IPOVLY
struct ipovly {
	caddr_t ih_next, ih_prev;	/* for protocol sequence q's */
	u_char	ih_x1;			/* (unused) */
	u_char	ih_pr;			/* protocol */
	short	ih_len;		 /* protocol length */
	struct	in_addr ih_src;	 /* source internet address */
	struct	in_addr ih_dst;	 /* destination internet address */
};
# endif

struct	ether_header	{
	u8_t	ether_dhost[6];
	u8_t	ether_shost[6];
	u16_t	ether_type;
};

#define udphdr ipfudphdr
#define tcphdr ipftcphdr


typedef	struct	ipftcphdr	tcphdr_t;
typedef	struct	ipfudphdr	udphdr_t;

#define IPFILTER_LOOKUP 1

#include "ip_compat.h"

#include "ip_fil.h"
#include "ip_auth.h"
#include "ip_state.h"
#include "ip_nat.h"
#include "ip_proxy.h"
#include "ip_frag.h"
#ifdef	IPFILTER_LOOKUP
# include "ip_lookup.h"
# include "ip_pool.h"
# include "ip_htable.h"
#endif
#ifdef  IPFILTER_SYNC
# include "netinet/ip_sync.h"
#endif
#ifdef  IPFILTER_SCAN
# include "ip_scan.h"
#endif
#ifdef IPFILTER_COMPILED
# include "ip_rules.h"
#endif
#include "ipl.h"


#endif /* __IPF_LINUX_H__ */
