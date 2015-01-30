

/*
 * $Id: glue.h 2986 2009-07-13 19:13:07Z marta $
 *
 * glue code to adapt the FreeBSD version to linux and windows,
 * userland and kernel.
 * This is included before any other headers, so we do not have
 * a chance to override any #define that should appear in other
 * headers.
 */
 
#ifndef _GLUE_H
#define	_GLUE_H

/*
 * common definitions to allow portability
 */
#ifndef __FBSDID
#define __FBSDID(x)
#endif  /* FBSDID */

/*
 * emulation of FreeBSD's sockopt and thread
 * This was in sockopt.h
 */
enum sopt_dir { SOPT_GET, SOPT_SET };

#ifndef KERNEL_MODULE	/* Userland part */

#include <stdint.h>		/* linux needs this in addition to sys/types.h */

#include <sys/types.h>		/* for size_t */
#include <sys/ioctl.h>
//#include <sys/queue.h>
#include <time.h>

#include <netinet/ether.h>

#else /* KERNEL_MODULE, kernel part */

#ifndef _WIN32
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

#include <assert.h>



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

#include "netbas.h"
#  define       bzero(a,b)      memset(a,0,b)
#  define       bcmp            memcmp
#  define       bcopy(a,b,c)    memmove(b,a,c)

//#include <net/dhcp.h>
#define	IFNAMSIZ	16

typedef void *usb_malloc_type;
typedef void *caddr_t;
typedef u8_t u_int8_t;
typedef u16_t u_int16_t;
typedef u32_t u_int32_t;
typedef u64_t u_int64_t;

typedef	unsigned char	u_char;
typedef	unsigned short	u_short;
typedef	unsigned int	u_int;
typedef	unsigned long	u_long;
typedef	unsigned short	ushort;		/* Sys V compatibility */

#define ifnet		netif	/* remap */
#define	_KERNEL		# make kernel structure visible
#define	KLD_MODULE	# add the module glue
#define	INET		# want inet support
/*
 * Structure returned by gettimeofday(2) system call,
 * and used in other calls.
 */
struct timeval {
	long	tv_sec;		/* seconds */
	long	tv_usec;	/* and microseconds */
};

#ifndef _TIMESPEC_DECLARED
#define _TIMESPEC_DECLARED
struct timespec {
	time_t	tv_sec;		/* seconds */
	long	tv_nsec;	/* and nanoseconds */
};
#endif

#define	TIMEVAL_TO_TIMESPEC(tv, ts)					\
	do {								\
		(ts)->tv_sec = (tv)->tv_sec;				\
		(ts)->tv_nsec = (tv)->tv_usec * 1000;			\
	} while (0)
#define	TIMESPEC_TO_TIMEVAL(tv, ts)					\
	do {								\
		(tv)->tv_sec = (ts)->tv_sec;				\
		(tv)->tv_usec = (ts)->tv_nsec / 1000;			\
	} while (0)

struct timezone {
	int	tz_minuteswest;	/* minutes west of Greenwich */
	int	tz_dsttime;	/* type of dst correction */
};




struct in6_addr {
        union {
                u_int8_t   __u6_addr8[16];
                u_int16_t  __u6_addr16[8];
                u_int32_t  __u6_addr32[4];
        } __u6_addr;                    /* 128-bit IP6 address */
};

 # define SIN6_LEN
   

   struct sockaddr_in6 {
          u_char           sin6_len;      /* length of this structure */
          u_char           sin6_family;   /* AF_INET6                 */
          u_int16_t       sin6_port;     /* Transport layer port #   */
          u_int32_t       sin6_flowinfo; /* IPv6 flow information    */
          struct in6_addr  sin6_addr;     /* IPv6 address             */
   };

typedef unsigned short int sa_family_t;

#define INT_MAX		((int)(~0U>>1))
#define INT_MIN		(-INT_MAX - 1)
/*
 * LIST_HEAD in queue.h conflict with linux/list.h
 * some previous linux include need list.h definition
 */
#undef LIST_HEAD

#define         IF_NAMESIZE     16
typedef uint32_t      in_addr_t;

//#define printf(fmt, arg...) printk(KERN_ERR fmt, ##arg)

#endif	/* !_WIN32 */
#endif /* KERNEL_MODULE */

/*
 * In windows, we need to emulate the sockopt interface
 * so also the userland needs to have the struct sockopt defined.
 * No need to declare struct thread on linux, but we need on windows.
 */

struct thread {
        void *sopt_td;
        void *td_ucred;
};

struct  sockopt {
        enum    sopt_dir sopt_dir; /* is this a get or a set? */
        int     sopt_level;     /* second arg of [gs]etsockopt */
        int     sopt_name;      /* third arg of [gs]etsockopt */
        void   *sopt_val;       /* fourth arg of [gs]etsockopt */
        size_t  sopt_valsize;   /* (almost) fifth arg of [gs]etsockopt */
        struct  thread *sopt_td; /* calling thread or null if kernel */
};

typedef struct 
{
	void *head;
	void *tail;
}rwlock_t;

/* This must be included here after list.h */
//#include <sys/queue.h>		/* both the kernel side and nat.c needs this */

#ifndef KERNEL_MODULE

/* define internals for struct in6_addr netinet/in6.h on FreeBSD */
#define __u6_addr in6_u
#define __u6_addr32 u6_addr32
/* define missing type for ipv6 (linux 2.6.28) */
#define in6_u __in6_u

/* missing in linux netinet/ip.h */
#define IPTOS_ECN_ECT0          0x02    /* ECN-capable transport (0) */
#define IPTOS_ECN_CE            0x03    /* congestion experienced */

/* defined in freebsd netinet/icmp6.h */
#define ICMP6_MAXTYPE                   201

/* on freebsd sys/socket.h pf specific */
#define NET_RT_IFLIST   3               /* survey interface list */

/* on freebsd net/if.h XXX used */
struct if_data {

	/* ... */
        u_long ifi_mtu;                /* maximum transmission unit */   
};

/*
 * Message format for use in obtaining information about interfaces
 * from getkerninfo and the routing socket.
 * This is used in nat.c
 */
struct if_msghdr {
        u_short ifm_msglen;     /* to skip over non-understood messages */
        u_char  ifm_version;    /* future binary compatibility */
        u_char  ifm_type;       /* message type */
        int     ifm_addrs;      /* like rtm_addrs */
        int     ifm_flags;      /* value of if_flags */
        u_short ifm_index;      /* index for associated ifp */
        struct  if_data ifm_data;/* statistics and other data about if */
};

/*
 * Message format for use in obtaining information about interface addresses
 * from getkerninfo and the routing socket
 */
struct ifa_msghdr {
        u_short ifam_msglen;    /* to skip over non-understood messages */
        u_char  ifam_version;   /* future binary compatibility */
        u_char  ifam_type;      /* message type */
        int     ifam_addrs;     /* like rtm_addrs */
        int     ifam_flags;     /* value of ifa_flags */
        u_short ifam_index;     /* index for associated ifp */
        int     ifam_metric;    /* value of ifa_metric */
};

#ifndef NO_RTM	/* conflicting with netlink */
/* missing in net/route.h */
#define RTM_VERSION     5       /* Up the ante and ignore older versions */
#define RTM_IFINFO      0xe     /* iface going up/down etc. */
#define RTM_NEWADDR     0xc     /* address being added to iface */
#define RTA_IFA         0x20    /* interface addr sockaddr present */
#endif	/* NO_RTM */

/* SA_SIZE is used in the userland nat.c modified */
#define SA_SIZE(sa)                                             \
    (  (!(sa) ) ?      \
        sizeof(long)            :                               \
        1 + ( (sizeof(struct sockaddr) - 1) | (sizeof(long) - 1) ) )

/* sys/time.h */
/*
 * Getkerninfo clock information structure
 */
struct clockinfo {
        int     hz;             /* clock frequency */
        int     tick;           /* micro-seconds per hz tick */
        int     spare;
        int     stathz;         /* statistics clock frequency */
        int     profhz;         /* profiling clock frequency */
};


/*
 * linux does not have heapsort
 */
#define heapsort(_a, _b, _c, _d)	qsort(_a, _b, _c, _d)

#define setprogname(x)	/* not present in linux */

extern int optreset;	/* not present in linux */

size_t strlcpy(char * dst, const char * src, size_t siz);
long long int
strtonum(const char *nptr, long long minval, long long maxval,
         const char **errstr);
 
int
sysctlbyname(const char *name, void *oldp, size_t *oldlenp, void *newp,
         size_t newlen);
 
#else /* KERNEL_MODULE */

/* linux and windows kernel do not have bcopy ? */
#define bcopy(_s, _d, _l)	memcpy(_d, _s, _l)



/* definitions useful for the kernel side */

struct route_in6 { };

#endif	/* KERNEL_MODULE */

/*
 * List of values used for set/getsockopt options.
 * The base value on FreeBSD is defined as a macro,
 * if not available we will use our own enum.
 * The TABLE_BASE value is used in the kernel.
 */
#ifndef IP_FW_TABLE_ADD
#define _IPFW_SOCKOPT_BASE	100	/* 40 on freebsd */
enum ipfw_msg_type {
	IP_FW_TABLE_ADD		= _IPFW_SOCKOPT_BASE,
	IP_FW_TABLE_DEL,
	IP_FW_TABLE_FLUSH,
	IP_FW_TABLE_GETSIZE,
	IP_FW_TABLE_LIST,

	IP_FW_ADD		= _IPFW_SOCKOPT_BASE + 10,
	IP_FW_DEL,
	IP_FW_FLUSH,
	IP_FW_ZERO,
	IP_FW_GET,
	IP_FW_RESETLOG,

	IP_FW_NAT_CFG,
	IP_FW_NAT_DEL,
	IP_FW_NAT_GET_CONFIG,
	IP_FW_NAT_GET_LOG,

	IP_DUMMYNET_CONFIGURE,
	IP_DUMMYNET_DEL	,
	IP_DUMMYNET_FLUSH,
	/* 63 is missing */
	IP_DUMMYNET_GET		= _IPFW_SOCKOPT_BASE + 24,
	_IPFW_SOCKOPT_END
};
#endif /* IP_FW_TABLE_ADD */


#endif /* !_GLUE_H */
