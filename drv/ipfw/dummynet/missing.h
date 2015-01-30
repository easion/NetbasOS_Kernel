/*
 * Copyright (C) 2009 Luigi Rizzo, Marta Carbone, Universita` di Pisa
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * $Id: missing.h 2835 2009-06-19 17:10:12Z luigi $
 *
 * Header for kernel variables and functions that are not available in
 * userland.
 */

#ifndef _MISSING_H_
#define _MISSING_H_

#ifdef _WIN32

#ifndef DEFINE_SPINLOCK
#define DEFINE_SPINLOCK(x)	FAST_MUTEX x
#endif
/* spinlock --> Guarded Mutex KGUARDED_MUTEX */
/* http://www.reactos.org/wiki/index.php/Guarded_Mutex */
#define spin_lock_init(_l)
#define spin_lock_bh(_l)
#define spin_unlock_bh(_l)

#include <sys/socket.h>		/* bsd-compat.c */
#include <netinet/in.h>		/* bsd-compat.c */
#include <netinet/ip.h>		/* local version */

#else	/* __NETBAS__ */

//#include <linux/time.h>		/* do_gettimeofday */
#include <netinet/ip.h>		/* local version */
struct inpcb;

/*
 * Kernel locking support.
 * FreeBSD uses mtx in dummynet.c, and rwlocks in ipfw.c
 *
 * In linux we use spinlock_bh to implement both.
 */

#ifndef DEFINE_SPINLOCK	/* this is for linux 2.4 */
#define DEFINE_SPINLOCK(x)   spinlock_t x =  { SPINLOCK_UNLOCKED, }
#endif

#endif	/* __NETBAS__ */

#define rw_assert(a, b)
#define rw_destroy(_l)
#define rw_init(_l, msg)	spin_lock_init(_l)
#define rw_rlock(_l)		spin_lock(_l)
#define rw_runlock(_l)		spin_unlock(_l)
#define rw_wlock(_l)		spin_lock(_l)
#define rw_wunlock(_l)		spin_unlock(_l)

#define mtx_assert(a, b)
#define	mtx_destroy(m)
#define mtx_init(m, a,b,c) 	spin_lock_init(m)
#define mtx_lock(_l)		spin_lock(_l)
#define mtx_unlock(_l)		spin_unlock(_l)

/* end of locking support */

/* ethernet stuff */
#define	ETHERTYPE_IP		0x0800	/* IP protocol */
#define	ETHER_ADDR_LEN		6	/* length of an Ethernet address */
struct ether_header {
        u_char  ether_dhost[ETHER_ADDR_LEN];
        u_char  ether_shost[ETHER_ADDR_LEN];
        u_short ether_type;
};

#define ETHER_ADDR_LEN          6       /* length of an Ethernet address */
#define ETHER_TYPE_LEN          2       /* length of the Ethernet type field */
#define ETHER_HDR_LEN           (ETHER_ADDR_LEN*2+ETHER_TYPE_LEN)

/* ip_dummynet.c */
#define __FreeBSD_version 500035

#ifdef __NETBAS__
struct moduledata;
int my_mod_register(struct moduledata *mod, const char *name, int order);

/* define some macro for ip_dummynet */

struct malloc_type {
};

#define MALLOC_DEFINE(type, shortdesc, longdesc) 	\
	struct malloc_type type[1]; void *md_dummy_ ## type = type

#define CTASSERT(x)

#define log(_level, fmt, arg...)  printk(KERN_ERR fmt, ##arg)

/*
 * gettimeofday would be in sys/time.h but it is not
 * visible if _KERNEL is defined
 */
int gettimeofday(struct timeval *, struct timezone *);

#else  /* _WIN32 */
#define MALLOC_DEFINE(a,b,c)
#endif /* _WIN32 */

extern int	hz;
extern long	tick;		/* exists in 2.4 but not in 2.6 */
extern int	bootverbose;
extern time_t	time_uptime;
extern struct timeval boottime;

extern int	max_linkhdr;
extern int	ip_defttl;
extern u_long	in_ifaddrhmask;                         /* mask for hash table */
extern struct in_ifaddrhashhead *in_ifaddrhashtbl;    /* inet addr hash table  */

/*-------------------------------------------------*/

/* define, includes and functions missing in linux */
/* include and define */
//#include <arpa/inet.h>		/* inet_ntoa */

struct mbuf;

/* used by ip_dummynet.c */
void reinject_drop(struct mbuf* m);

//#include <linux/errno.h>	/* error define */
//#include <linux/if.h>		/* IFNAMESIZ */

/*
 * some network structure can be defined in the bsd way
 * by using the _FAVOR_BSD definition. This is not true
 * for icmp structure.
 * XXX struct icmp contains bsd names in 
 * /usr/include/netinet/ip_icmp.h
 */
#ifdef __NETBAS__
#define icmp_code code
#define icmp_type type

/* linux in6_addr has no member __u6_addr
 * replace the whole structure ?
 */
//#define __u6_addr       in6_u
//#define __u6_addr32     u6_addr32
#endif /* __NETBAS__ */

/* defined in linux/sctp.h with no bsd definition */
struct sctphdr {
        uint16_t src_port;      /* source port */
        uint16_t dest_port;     /* destination port */
        uint32_t v_tag;         /* verification tag of packet */
        uint32_t checksum;      /* Adler32 C-Sum */
        /* chunks follow... */
};

/* missing definition */
#define TH_FIN  0x01
#define TH_SYN  0x02
#define TH_RST  0x04
#define TH_ACK  0x10

#define RTF_CLONING	0x100		/* generate new routes on use */

#define IPPROTO_OSPFIGP         89              /* OSPFIGP */
#define IPPROTO_CARP            112             /* CARP */
#ifndef _WIN32
#define IPPROTO_IPV4            IPPROTO_IPIP    /* for compatibility */
#endif

#define	CARP_VERSION		2
#define	CARP_ADVERTISEMENT	0x01

#define PRIV_NETINET_IPFW       491     /* Administer IPFW firewall. */

#define IP_FORWARDING           0x1             /* most of ip header exists */

#define NETISR_IP       2               /* same as AF_INET */

#define PRIV_NETINET_DUMMYNET   494     /* Administer DUMMYNET. */

extern int securelevel;

struct carp_header {
#if BYTE_ORDER == LITTLE_ENDIAN
        u_int8_t        carp_type:4,
                        carp_version:4;
#endif
#if BYTE_ORDER == BIG_ENDIAN
        u_int8_t        carp_version:4,
                        carp_type:4;
#endif
};

struct pim {
	int dummy;      /* windows compiler does not like empty definition */
};

struct route {
	struct  rtentry *ro_rt;
	struct  sockaddr ro_dst;
};

struct ifaltq {
	void *ifq_head;
};

/*
 * ifnet->if_snd is used in ip_dummynet.c to take the transmission
 * clock.
 */
#if defined( __NETBAS__)
#define	if_xname	name
#define	if_snd		XXX
#elif defined( _WIN32 )
/* used in ip_dummynet.c */
struct ifnet {
	char    if_xname[IFNAMSIZ];     /* external name (name + unit) */
//        struct ifaltq if_snd;          /* output queue (includes altq) */
};

struct net_device {
	char    if_xname[IFNAMSIZ];     /* external name (name + unit) */
};
#endif

/* involves mbufs */
int in_cksum(struct mbuf *m, int len);
#define divert_cookie(mtag) 0
#define divert_info(mtag) 0
#define INADDR_TO_IFP(a, b) b = NULL
#define pf_find_mtag(a) NULL
#define pf_get_mtag(a) NULL
#ifndef _WIN32
#define AF_LINK AF_ASH	/* ? our sys/socket.h */
#endif

struct pf_mtag {
	void            *hdr;           /* saved hdr pos in mbuf, for ECN */
	sa_family_t      af;            /* for ECN */
        u_int32_t        qid;           /* queue id */
};

/* radix related */

struct radix_node {
	caddr_t rn_key;         /* object of search */
	caddr_t rn_mask;        /* netmask, if present */
};

/* missing kernel functions */
char *inet_ntoa(struct in_addr ina);
int random(void);

/*
 * Return the risult of a/b
 *
 * this is used in linux kernel space,
 * since the 64bit division needs to
 * be done using a macro
 */
int64_t
div64(int64_t a, int64_t b);

char *
inet_ntoa_r(struct in_addr ina, char *buf);

/* from bsd sys/queue.h */
#define TAILQ_FOREACH_SAFE(var, head, field, tvar)                      \
        for ((var) = TAILQ_FIRST((head));                               \
            (var) && ((tvar) = TAILQ_NEXT((var), field), 1);            \
            (var) = (tvar))

#define SLIST_FOREACH_SAFE(var, head, field, tvar)                      \
        for ((var) = SLIST_FIRST((head));                               \
            (var) && ((tvar) = SLIST_NEXT((var), field), 1);            \
            (var) = (tvar))

/* depending of linux version */
#ifndef ETHERTYPE_IPV6
#define ETHERTYPE_IPV6          0x86dd          /* IP protocol version 6 */
#endif

/*-------------------------------------------------*/
#define RT_NUMFIBS 1
extern u_int rt_numfibs;

/* involves kernel locking function */
#ifdef RTFREE
#undef RTFREE
#define RTFREE(a) fprintf(stderr, "RTFREE: commented out locks\n");
#endif

void getmicrouptime(struct timeval *tv);

/* from sys/netinet/ip_output.c */
struct ip_moptions;
struct route;
struct ip;

struct mbuf *ip_reass(struct mbuf *);
u_short in_cksum_hdr(struct ip *);
int ip_output(struct mbuf *m, struct mbuf *opt, struct route *ro, int flags,
    struct ip_moptions *imo, struct inpcb *inp);

/* from net/netisr.c */
void netisr_dispatch(int num, struct mbuf *m);

/* definition moved in missing.c */
int sooptcopyout(struct sockopt *sopt, const void *buf, size_t len);

int sooptcopyin(struct sockopt *sopt, void *buf, size_t len, size_t minlen);

/* defined in session.c */
int priv_check(struct thread *td, int priv);
struct ucred
{
	int dummy;
};
int securelevel_ge(struct ucred *cr, int level);

struct sysctl_oid;
struct sysctl_req;

/*
 * sysctl are mapped into /sys/module/ipfw_mod parameters
 */
#define CTLFLAG_RD		1
#define CTLFLAG_RW		2
#define CTLFLAG_SECURE3		0 // unsupported

//#ifdef _WIN32
#define module_param_named(_name, _var, _ty, _perm)
//#else
//#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,17)
//#define module_param_named(_name, _var, _ty, _perm)	\
	//module_param(_name, _ty, 0644)
//#endif
//#endif /* __NETBAS__ */

#define SYSCTL_DECL(_1)
#define SYSCTL_NODE(_1, _2, _3, _4, _5, _6)
#define _SYSCTL_BASE(_name, _var, _ty, _perm)		\
	module_param_named(_name, *(_var), _ty, 	\
		( (_perm) == CTLFLAG_RD) ? 0444: 0644 )
#define SYSCTL_PROC(_base, _oid, _name, _mode, _var, _val, _desc, _a, _b)

#define SYSCTL_INT(_base, _oid, _name, _mode, _var, _val, _desc)	\
	_SYSCTL_BASE(_name, _var, int, _mode)

#define SYSCTL_LONG(_base, _oid, _name, _mode, _var, _val, _desc)	\
	_SYSCTL_BASE(_name, _var, long, _mode)

#define SYSCTL_ULONG(_base, _oid, _name, _mode, _var, _val, _desc)	\
	_SYSCTL_BASE(_name, _var, ulong, _mode)

#define SYSCTL_UINT(_base, _oid, _name, _mode, _var, _val, _desc)	\
	// _SYSCTL_BASE(_name, _var, uint, _mode)

#define SYSCTL_HANDLER_ARGS 		\
	struct sysctl_oid *oidp, void *arg1, int arg2, struct sysctl_req *req
int sysctl_handle_int(SYSCTL_HANDLER_ARGS);
int sysctl_handle_long(SYSCTL_HANDLER_ARGS); 

void ether_demux(struct ifnet *ifp, struct mbuf *m);

int ether_output_frame(struct ifnet *ifp, struct mbuf *m);

void in_rtalloc_ign(struct route *ro, u_long ignflags, u_int fibnum);

void icmp_error(struct mbuf *n, int type, int code, uint32_t dest, int mtu);

void rtfree(struct rtentry *rt);

u_short in_cksum_skip(struct mbuf *m, int len, int skip);

#ifdef INP_LOCK_ASSERT
#undef INP_LOCK_ASSERT
#define INP_LOCK_ASSERT(a)
#endif

int rn_inithead(void **head, int off);

int jailed(struct ucred *cred);

/*
* Return 1 if an internet address is for a ``local'' host
* (one to which we have a connection).  If subnetsarelocal
* is true, this includes other subnets of the local net.
* Otherwise, it includes only the directly-connected (sub)nets.
*/
int in_localaddr(struct in_addr in);

/* the prototype is already in the headers */
//int ipfw_chg_hook(SYSCTL_HANDLER_ARGS); 

int fnmatch(const char *pattern, const char *string, int flags);

#endif /* !_MISSING_H_ */
