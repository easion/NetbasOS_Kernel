#ifndef _ALIAS_H_
#define _ALIAS_H_
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/kernel.h>
#include <sys/queue.h>
#include <sys/mbuf.h>

#include <netinet/ip.h>
//#include <netinet/ip_var.h>
#include <netinet/ip_icmp.h>
#include <netinet/ip_fw.h>
#include <netinet/ip_divert.h>
#include <netinet/ip_dummynet.h>
//#include <netinet/ip_carp.h>
//#include <netinet/pim.h>
#include <netinet/tcp_var.h>
#include <netinet/udp.h>
//#include <netinet/udp_var.h>
//#include <netinet/sctp.h>
#include <netgraph/ng_ipfw.h>
#define	LIST_HEAD(name, type)						\
struct name {								\
	struct type *lh_first;	/* first element */			\
}


#define LIBALIAS_BUF_SIZE 128
#ifdef  _KERNEL
/*
 * The kernel version of libalias does not support these features...
 */
#define NO_FW_PUNCH
#define NO_USE_SOCKETS

#endif

/*
 * The external interface to libalias, the packet aliasing engine.
 *
 * There are two sets of functions:
 *
 * PacketAlias*() the old API which doesn't take an instance pointer
 * and therefore can only have one packet engine at a time.
 *
 * LibAlias*() the new API which takes as first argument a pointer to
 * the instance of the packet aliasing engine.
 *
 * The functions otherwise correspond to each other one for one, except
 * for the LibAliasUnaliasOut()/PacketUnaliasOut() function which were
 * were misnamed in the old API.
 */

/*
 * The instance structure
 */
struct libalias;

/*
 * An anonymous structure, a pointer to which is returned from
 * PacketAliasRedirectAddr(), PacketAliasRedirectPort() or
 * PacketAliasRedirectProto(), passed to PacketAliasAddServer(),
 * and freed by PacketAliasRedirectDelete().
 */
struct alias_link;


/* OLD API */

/* Initialization and control functions. */
void            PacketAliasInit(void);
void            PacketAliasSetAddress(struct in_addr _addr);
void            PacketAliasSetFWBase(unsigned int _base, unsigned int _num);
void            PacketAliasSetSkinnyPort(unsigned int _port);
unsigned int
                PacketAliasSetMode(unsigned int _flags, unsigned int _mask);
void            PacketAliasUninit(void);

/* Packet Handling functions. */
int             PacketAliasIn(char *_ptr, int _maxpacketsize);
int             PacketAliasOut(char *_ptr, int _maxpacketsize);
int             PacketUnaliasOut(char *_ptr, int _maxpacketsize);

/* Port and address redirection functions. */


int
PacketAliasAddServer(struct alias_link *_lnk,
    struct in_addr _addr, unsigned short _port);
struct alias_link *
PacketAliasRedirectAddr(struct in_addr _src_addr,
    struct in_addr _alias_addr);
int             PacketAliasRedirectDynamic(struct alias_link *_lnk);
void            PacketAliasRedirectDelete(struct alias_link *_lnk);
struct alias_link *
PacketAliasRedirectPort(struct in_addr _src_addr,
    unsigned short _src_port, struct in_addr _dst_addr,
    unsigned short _dst_port, struct in_addr _alias_addr,
    unsigned short _alias_port, unsigned char _proto);
struct alias_link *
PacketAliasRedirectProto(struct in_addr _src_addr,
    struct in_addr _dst_addr, struct in_addr _alias_addr,
    unsigned char _proto);

/* Fragment Handling functions. */
void            PacketAliasFragmentIn(char *_ptr, char *_ptr_fragment);
char           *PacketAliasGetFragment(char *_ptr);
int             PacketAliasSaveFragment(char *_ptr);

/* Miscellaneous functions. */
int             PacketAliasCheckNewLink(void);
unsigned short
                PacketAliasInternetChecksum(unsigned short *_ptr, int _nbytes);
void            PacketAliasSetTarget(struct in_addr _target_addr);

/* Transparent proxying routines. */
int             PacketAliasProxyRule(const char *_cmd);

/* NEW API */

/* Initialization and control functions. */
struct libalias *LibAliasInit(struct libalias *);
void            LibAliasSetAddress(struct libalias *, struct in_addr _addr);
void            LibAliasSetFWBase(struct libalias *, unsigned int _base, unsigned int _num);
void            LibAliasSetSkinnyPort(struct libalias *, unsigned int _port);
unsigned int
                LibAliasSetMode(struct libalias *, unsigned int _flags, unsigned int _mask);
void            LibAliasUninit(struct libalias *);

/* Packet Handling functions. */
int             LibAliasIn (struct libalias *, char *_ptr, int _maxpacketsize);
int             LibAliasOut(struct libalias *, char *_ptr, int _maxpacketsize);
int             LibAliasOutTry(struct libalias *, char *_ptr, int _maxpacketsize, int _create);
int             LibAliasUnaliasOut(struct libalias *, char *_ptr, int _maxpacketsize);

/* Port and address redirection functions. */

int
LibAliasAddServer(struct libalias *, struct alias_link *_lnk,
    struct in_addr _addr, unsigned short _port);
struct alias_link *
LibAliasRedirectAddr(struct libalias *, struct in_addr _src_addr,
    struct in_addr _alias_addr);
int             LibAliasRedirectDynamic(struct libalias *, struct alias_link *_lnk);
void            LibAliasRedirectDelete(struct libalias *, struct alias_link *_lnk);
struct alias_link *
LibAliasRedirectPort(struct libalias *, struct in_addr _src_addr,
    unsigned short _src_port, struct in_addr _dst_addr,
    unsigned short _dst_port, struct in_addr _alias_addr,
    unsigned short _alias_port, unsigned char _proto);
struct alias_link *
LibAliasRedirectProto(struct libalias *, struct in_addr _src_addr,
    struct in_addr _dst_addr, struct in_addr _alias_addr,
    unsigned char _proto);

/* Fragment Handling functions. */
void            LibAliasFragmentIn(struct libalias *, char *_ptr, char *_ptr_fragment);
char           *LibAliasGetFragment(struct libalias *, char *_ptr);
int             LibAliasSaveFragment(struct libalias *, char *_ptr);

/* Miscellaneous functions. */
int             LibAliasCheckNewLink(struct libalias *);
unsigned short
                LibAliasInternetChecksum(struct libalias *, unsigned short *_ptr, int _nbytes);
void            LibAliasSetTarget(struct libalias *, struct in_addr _target_addr);

/* Transparent proxying routines. */
int             LibAliasProxyRule(struct libalias *, const char *_cmd);

/* Module handling API */
int             LibAliasLoadModule(char *);
int             LibAliasUnLoadAllModule(void);
int             LibAliasRefreshModules(void);

/*
 * Mode flags and other constants.
 */

/* Mode flags, set using PacketAliasSetMode() */

/*
 * If PKT_ALIAS_LOG is set, a message will be printed to /var/log/alias.log
 * every time a link is created or deleted.  This is useful for debugging.
 */
#define PKT_ALIAS_LOG                   0x01

/*
 * If PKT_ALIAS_DENY_INCOMING is set, then incoming connections (e.g. to ftp,
 * telnet or web servers will be prevented by the aliasing mechanism.
 */
#define PKT_ALIAS_DENY_INCOMING         0x02

/*
 * If PKT_ALIAS_SAME_PORTS is set, packets will be attempted sent from the
 * same port as they originated on.  This allows e.g. rsh to work *99% of the
 * time*, but _not_ 100% (it will be slightly flakey instead of not working
 * at all).  This mode bit is set by PacketAliasInit(), so it is a default
 * mode of operation.
 */
#define PKT_ALIAS_SAME_PORTS            0x04

/*
 * If PKT_ALIAS_USE_SOCKETS is set, then when partially specified links (e.g.
 * destination port and/or address is zero), the packet aliasing engine will
 * attempt to allocate a socket for the aliasing port it chooses.  This will
 * avoid interference with the host machine.  Fully specified links do not
 * require this.  This bit is set after a call to PacketAliasInit(), so it is
 * a default mode of operation.
 */
#ifndef NO_USE_SOCKETS
#define PKT_ALIAS_USE_SOCKETS           0x08
#endif
/*-
 * If PKT_ALIAS_UNREGISTERED_ONLY is set, then only packets with
 * unregistered source addresses will be aliased.  Private
 * addresses are those in the following ranges:
 *
 *              10.0.0.0     ->   10.255.255.255
 *              172.16.0.0   ->   172.31.255.255
 *              192.168.0.0  ->   192.168.255.255
 */
#define PKT_ALIAS_UNREGISTERED_ONLY     0x10

/*
 * If PKT_ALIAS_RESET_ON_ADDR_CHANGE is set, then the table of dynamic
 * aliasing links will be reset whenever PacketAliasSetAddress() changes the
 * default aliasing address.  If the default aliasing address is left
 * unchanged by this function call, then the table of dynamic aliasing links
 * will be left intact.  This bit is set after a call to PacketAliasInit().
 */
#define PKT_ALIAS_RESET_ON_ADDR_CHANGE  0x20

#ifndef NO_FW_PUNCH
/*
 * If PKT_ALIAS_PUNCH_FW is set, active FTP and IRC DCC connections will
 * create a 'hole' in the firewall to allow the transfers to work.  The
 * ipfw rule number that the hole is created with is controlled by
 * PacketAliasSetFWBase().  The hole will be attached to that
 * particular alias_link, so when the link goes away the hole is deleted.
 */
#define PKT_ALIAS_PUNCH_FW              0x100
#endif

/*
 * If PKT_ALIAS_PROXY_ONLY is set, then NAT will be disabled and only
 * transparent proxying is performed.
 */
#define PKT_ALIAS_PROXY_ONLY            0x40

/*
 * If PKT_ALIAS_REVERSE is set, the actions of PacketAliasIn() and
 * PacketAliasOut() are reversed.
 */
#define PKT_ALIAS_REVERSE               0x80

/* Function return codes. */
#define PKT_ALIAS_ERROR                 -1
#define PKT_ALIAS_OK                    1
#define PKT_ALIAS_IGNORED               2
#define PKT_ALIAS_UNRESOLVED_FRAGMENT   3
#define PKT_ALIAS_FOUND_HEADER_FRAGMENT 4



#define M_IPFW 0
#ifndef MCLSHIFT
#define MCLSHIFT        11              /* convert bytes to m_buf clusters */
#endif  /* MCLSHIFT */
#define MCLBYTES        (1 << MCLSHIFT) /* size of an m_buf cluster */
#define MCLOFSET        (MCLBYTES - 1)  /* offset within an m_buf cluster */


#define LIBALIAS_BUF_SIZE 128
/* Function return codes. */
#define PKT_ALIAS_ERROR                 -1
#define PKT_ALIAS_OK                    1
#define PKT_ALIAS_IGNORED               2
#define PKT_ALIAS_UNRESOLVED_FRAGMENT   3
#define PKT_ALIAS_FOUND_HEADER_FRAGMENT 4

/* Generic priority levels */
#define EVENTHANDLER_PRI_FIRST  0
#define EVENTHANDLER_PRI_ANY    10000
#define EVENTHANDLER_PRI_LAST   20000
#if 0
struct eventhandler_list {
        char                            *el_name;
        int                             el_flags;
#define EHL_INITTED     (1<<0)
        u_int                           el_runcount;
        //struct mtx                      el_lock;
        //TAILQ_ENTRY(eventhandler_list)  el_link;
        //TAILQ_HEAD(,eventhandler_entry) el_entries;
};

typedef struct eventhandler_entry       *eventhandler_tag;
#endif

/*
 * Flags indicating hw checksum support and sw checksum requirements.
 */
#define CSUM_IP                 0x0001          /* will csum IP */
#define CSUM_TCP                0x0002          /* will csum TCP */
#define CSUM_UDP                0x0004          /* will csum UDP */
#define CSUM_IP_FRAGS           0x0008          /* will csum IP fragments */
#define CSUM_FRAGMENT           0x0010          /* will do IP fragmentation */

#define CSUM_IP_CHECKED         0x0100          /* did csum IP */
#define CSUM_IP_VALID           0x0200          /*   ... the csum is valid */
#define CSUM_DATA_VALID         0x0400          /* csum_data field is valid */
#define CSUM_PSEUDO_HDR         0x0800          /* csum_data has pseudo hdr */

#define CSUM_DELAY_DATA         (CSUM_TCP | CSUM_UDP)
#define CSUM_DELAY_IP           (CSUM_IP)       /* XXX add ipv6 here too? */



/* XXX: LibAliasSetTarget() uses this constant. */
#ifdef _KERNEL
#define INADDR_NONE     0xffffffff
#endif

#define ICMP_ECHOREPLY          0               /* echo reply */
#define ICMP_UNREACH            3               /* dest unreachable, codes: */
#define ICMP_UNREACH_NET                0       /* bad net */
#define ICMP_UNREACH_HOST               1       /* bad host */
#define ICMP_UNREACH_PROTOCOL           2       /* bad protocol */
#define ICMP_UNREACH_PORT               3       /* bad port */
#define ICMP_UNREACH_NEEDFRAG           4       /* IP_DF caused drop */
#define ICMP_UNREACH_SRCFAIL            5       /* src route failed */
#define ICMP_UNREACH_NET_UNKNOWN        6       /* unknown net */
#define ICMP_UNREACH_HOST_UNKNOWN       7       /* unknown host */
#define ICMP_UNREACH_ISOLATED           8       /* src host isolated */
#define ICMP_UNREACH_NET_PROHIB         9       /* for crypto devs */
#define ICMP_UNREACH_HOST_PROHIB        10      /* ditto */
#define ICMP_UNREACH_TOSNET             11      /* bad tos for net */
#define ICMP_UNREACH_TOSHOST            12      /* bad tos for host */
#define ICMP_UNREACH_FILTER_PROHIB      13      /* prohibited access */
#define ICMP_UNREACH_HOST_PRECEDENCE    14      /* precedence violat'n*/
#define ICMP_UNREACH_PRECEDENCE_CUTOFF  15      /* precedence cutoff */
#define ICMP_SOURCEQUENCH       4               /* packet lost, slow down */
#define ICMP_REDIRECT           5               /* shorter route, codes: */
#define ICMP_REDIRECT_NET       0               /* for network */
#define ICMP_REDIRECT_HOST      1               /* for host */
#define ICMP_REDIRECT_TOSNET    2               /* for tos and net */
#define ICMP_REDIRECT_TOSHOST   3               /* for tos and host */
#define ICMP_ALTHOSTADDR        6               /* alternate host address */
#define ICMP_ECHO               8               /* echo service */
#define ICMP_ROUTERADVERT       9               /* router advertisement */
#define ICMP_ROUTERADVERT_NORMAL                0       /* normal advertisement */
#define ICMP_ROUTERADVERT_NOROUTE_COMMON        16      /* selective routing */
#define ICMP_ROUTERSOLICIT      10              /* router solicitation */
#define ICMP_TIMXCEED           11              /* time exceeded, code: */
#define ICMP_TIMXCEED_INTRANS   0               /* ttl==0 in transit */
#define ICMP_TIMXCEED_REASS     1               /* ttl==0 in reass */
#define ICMP_PARAMPROB          12              /* ip header bad */
#define ICMP_PARAMPROB_ERRATPTR 0               /* req. opt. absent */
#define ICMP_PARAMPROB_OPTABSENT 1              /* req. opt. absent */
#define ICMP_PARAMPROB_LENGTH   2               /* bad length */
#define ICMP_TSTAMP             13              /* timestamp request */
#define ICMP_TSTAMPREPLY        14              /* timestamp reply */
#define ICMP_IREQ               15              /* information request */
#define ICMP_IREQREPLY          16              /* information reply */
#define ICMP_MASKREQ            17              /* address mask request */
#define ICMP_MASKREPLY          18              /* address mask reply */
#define ICMP_TRACEROUTE         30              /* traceroute */
#define ICMP_DATACONVERR        31              /* data conversion error */
#define ICMP_MOBILE_REDIRECT    32              /* mobile host redirect */
#define ICMP_IPV6_WHEREAREYOU   33              /* IPv6 where-are-you */
#define ICMP_IPV6_IAMHERE       34              /* IPv6 i-am-here */
#define ICMP_MOBILE_REGREQUEST  35              /* mobile registration req */
#define ICMP_MOBILE_REGREPLY    36              /* mobile registration reply */
#define ICMP_SKIP               39              /* SKIP */
#define ICMP_PHOTURIS           40              /* Photuris */
#define ICMP_PHOTURIS_UNKNOWN_INDEX     1       /* unknown sec index */
#define ICMP_PHOTURIS_AUTH_FAILED       2       /* auth failed */
#define ICMP_PHOTURIS_DECRYPT_FAILED    3       /* decrypt failed */






/* Macros */

/*
 * The following macro is used to update an
 * internet checksum.  "delta" is a 32-bit
 * accumulation of all the changes to the
 * checksum (adding in new 16-bit words and
 * subtracting out old words), and "cksum"
 * is the checksum value to be updated.
 */
#define ADJUST_CHECKSUM(acc, cksum) \
        do { \
                acc += cksum; \
                if (acc < 0) { \
                        acc = -acc; \
                        acc = (acc >> 16) + (acc & 0xffff); \
                        acc += acc >> 16; \
                        cksum = (u_short) ~acc; \
                } else { \
                        acc = (acc >> 16) + (acc & 0xffff); \
                        acc += acc >> 16; \
                        cksum = (u_short) acc; \
                } \
        } while (0)


/* Prototypes */

/*
 * We do not calculate TCP checksums when libalias is a kernel
 * module, since it has no idea about checksum offloading.
 * If TCP data has changed, then we just set checksum to zero,
 * and caller must recalculate it himself.
 * In case if libalias will edit UDP data, the same approach
 * should be used.
 */
#ifndef _KERNEL
u_short         IpChecksum(struct ip *_pip);
u_short         TcpChecksum(struct ip *_pip);
#endif
void
DifferentialChecksum(u_short * _cksum, void * _new, void * _old, int _n);

/* Internal data access */
struct alias_link *
FindIcmpIn(struct libalias *la, struct in_addr _dst_addr, struct in_addr _alias_addr,
    u_short _id_alias, int _create);
struct alias_link *
FindIcmpOut(struct libalias *la, struct in_addr _src_addr, struct in_addr _dst_addr,
    u_short _id, int _create);
struct alias_link *
FindFragmentIn1(struct libalias *la, struct in_addr _dst_addr, struct in_addr _alias_addr,
    u_short _ip_id);
struct alias_link *
FindFragmentIn2(struct libalias *la, struct in_addr _dst_addr, struct in_addr _alias_addr,
    u_short _ip_id);
struct alias_link *
                AddFragmentPtrLink(struct libalias *la, struct in_addr _dst_addr, u_short _ip_id);
struct alias_link *
                FindFragmentPtr(struct libalias *la, struct in_addr _dst_addr, u_short _ip_id);
struct alias_link *
FindProtoIn(struct libalias *la, struct in_addr _dst_addr, struct in_addr _alias_addr,
    u_char _proto);
struct alias_link *
FindProtoOut(struct libalias *la, struct in_addr _src_addr, struct in_addr _dst_addr,
    u_char _proto);
struct alias_link *
FindUdpTcpIn(struct libalias *la, struct in_addr _dst_addr, struct in_addr _alias_addr,
    u_short _dst_port, u_short _alias_port, u_char _proto, int _create);
struct alias_link *
FindUdpTcpOut(struct libalias *la, struct in_addr _src_addr, struct in_addr _dst_addr,
    u_short _src_port, u_short _dst_port, u_char _proto, int _create);
struct alias_link *
AddPptp(struct libalias *la, struct in_addr _src_addr, struct in_addr _dst_addr,
    struct in_addr _alias_addr, u_int16_t _src_call_id);
struct alias_link *
FindPptpOutByCallId(struct libalias *la, struct in_addr _src_addr,
    struct in_addr _dst_addr, u_int16_t _src_call_id);
struct alias_link *
FindPptpInByCallId(struct libalias *la, struct in_addr _dst_addr,
    struct in_addr _alias_addr, u_int16_t _dst_call_id);
struct alias_link *
FindPptpOutByPeerCallId(struct libalias *la, struct in_addr _src_addr,
    struct in_addr _dst_addr, u_int16_t _dst_call_id);
struct alias_link *
FindPptpInByPeerCallId(struct libalias *la, struct in_addr _dst_addr,
    struct in_addr _alias_addr, u_int16_t _alias_call_id);
struct alias_link *
FindRtspOut(struct libalias *la, struct in_addr _src_addr, struct in_addr _dst_addr,
    u_short _src_port, u_short _alias_port, u_char _proto);
struct in_addr
                FindOriginalAddress(struct libalias *la, struct in_addr _alias_addr);
struct in_addr
                FindAliasAddress(struct libalias *la, struct in_addr _original_addr);

/* External data access/modification */
int
FindNewPortGroup(struct libalias *la, struct in_addr _dst_addr, struct in_addr _alias_addr,
    u_short _src_port, u_short _dst_port, u_short _port_count,
    u_char _proto, u_char _align);
void            GetFragmentAddr(struct alias_link *_lnk, struct in_addr *_src_addr);
void            SetFragmentAddr(struct alias_link *_lnk, struct in_addr _src_addr);
void            GetFragmentPtr(struct alias_link *_lnk, char **_fptr);
void            SetFragmentPtr(struct alias_link *_lnk, char *fptr);
void            SetStateIn(struct alias_link *_lnk, int _state);
void            SetStateOut(struct alias_link *_lnk, int _state);
int             GetStateIn (struct alias_link *_lnk);
int             GetStateOut(struct alias_link *_lnk);
struct in_addr
                GetOriginalAddress(struct alias_link *_lnk);
struct in_addr
                GetDestAddress(struct alias_link *_lnk);
struct in_addr
                GetAliasAddress(struct alias_link *_lnk);
struct in_addr
                GetDefaultAliasAddress(struct libalias *la);
void            SetDefaultAliasAddress(struct libalias *la, struct in_addr _alias_addr);
u_short         GetOriginalPort(struct alias_link *_lnk);
u_short         GetAliasPort(struct alias_link *_lnk);
struct in_addr
                GetProxyAddress(struct alias_link *_lnk);
void            SetProxyAddress(struct alias_link *_lnk, struct in_addr _addr);
u_short         GetProxyPort(struct alias_link *_lnk);
void            SetProxyPort(struct alias_link *_lnk, u_short _port);
void            SetAckModified(struct alias_link *_lnk);
int             GetAckModified(struct alias_link *_lnk);
int             GetDeltaAckIn(struct ip *_pip, struct alias_link *_lnk);
int             GetDeltaSeqOut(struct ip *_pip, struct alias_link *_lnk);
void            AddSeq    (struct ip *_pip, struct alias_link *_lnk, int _delta);
void            SetExpire (struct alias_link *_lnk, int _expire);
void            ClearCheckNewLink(struct libalias *la);
void            SetProtocolFlags(struct alias_link *_lnk, int _pflags);
int             GetProtocolFlags(struct alias_link *_lnk);
void            SetDestCallId(struct alias_link *_lnk, u_int16_t _cid);

#ifndef NO_FW_PUNCH
void            PunchFWHole(struct alias_link *_lnk);

#endif

/* Housekeeping function */
void            HouseKeeping(struct libalias *);

/* Tcp specfic routines */
/* lint -save -library Suppress flexelint warnings */

/* Transparent proxy routines */
int
ProxyCheck(struct libalias *la, struct ip *_pip, struct in_addr *_proxy_server_addr,
    u_short * _proxy_server_port);
void
ProxyModify(struct libalias *la, struct alias_link *_lnk, struct ip *_pip,
    int _maxpacketsize, int _proxy_type);

enum alias_tcp_state {
        ALIAS_TCP_STATE_NOT_CONNECTED,
        ALIAS_TCP_STATE_CONNECTED,
        ALIAS_TCP_STATE_DISCONNECTED
};

#if defined(_NETINET_IP_H_)
static __inline void *
ip_next(struct ip *iphdr)
{
        char *p = (char *)iphdr;
        return (&p[iphdr->ip_hl * 4]);
}
#endif

#if defined(_NETINET_TCP_H_)
static __inline void *
tcp_next(struct tcphdr *tcphdr)
{
        char *p = (char *)tcphdr;
        return (&p[tcphdr->th_off * 4]);
}
#endif

#if defined(_NETINET_UDP_H_)
static __inline void *
udp_next(struct udphdr *udphdr)
{
        return ((void *)(udphdr + 1));
}
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

//typedef	u32_t	tcp_seq;

struct	ipfudphdr	{
	u16_t	uh_sport;
	u16_t	uh_dport;
	u16_t	uh_ulen;
	u16_t	uh_sum;
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




//#define udphdr ipfudphdr
//#define tcphdr ipftcphdr


typedef	struct	ipftcphdr	tcphdr_t;
typedef	struct	ipfudphdr	udphdr_t;
struct libalias {
        LIST_ENTRY(libalias) instancelist;

        int             packetAliasMode;        /* Mode flags                      */
        /* - documented in alias.h  */

        struct in_addr  aliasAddress;   /* Address written onto source     */
        /* field of IP packet.           */

        struct in_addr  targetAddress;  /* IP address incoming packets     */
        /* are sent to if no aliasing    */
        /* link already exists           */

        struct in_addr  nullAddress;    /* Used as a dummy parameter for   */
        /* some function calls           */

                        LIST_HEAD     (, alias_link) linkTableOut[LINK_TABLE_OUT_SIZE];
        /* Lookup table of pointers to     */
        /* chains of link records. Each  */

                        LIST_HEAD     (, alias_link) linkTableIn[LINK_TABLE_IN_SIZE];
        /* link record is doubly indexed */
        /* into input and output lookup  */
        /* tables.                       */

        /* Link statistics                 */
        int             icmpLinkCount;
        int             udpLinkCount;
        int             tcpLinkCount;
        int             pptpLinkCount;
        int             protoLinkCount;
        int             fragmentIdLinkCount;
        int             fragmentPtrLinkCount;
        int             sockCount;

        int             cleanupIndex;   /* Index to chain of link table    */
        /* being inspected for old links   */

        int             timeStamp;      /* System time in seconds for      */
        /* current packet                  */

        int             lastCleanupTime;        /* Last time
                                                 * IncrementalCleanup()  */
        /* was called                      */

        int             houseKeepingResidual;   /* used by HouseKeeping()          */

        int             deleteAllLinks; /* If equal to zero, DeleteLink()  */
        /* will not remove permanent links */

        /* log descriptor        */
#ifdef  _KERNEL
        char           *logDesc;
#else
        FILE           *logDesc;
#endif
        /* statistics monitoring */

        int             newDefaultLink; /* Indicates if a new aliasing     */
        /* link has been created after a   */
        /* call to PacketAliasIn/Out().    */

#ifndef NO_FW_PUNCH
        int             fireWallFD;     /* File descriptor to be able to   */
        /* control firewall.  Opened by    */
        /* PacketAliasSetMode on first     */
        /* setting the PKT_ALIAS_PUNCH_FW  */
        /* flag.                           */
        int             fireWallBaseNum;        /* The first firewall entry
                                                 * free for our use */
        int             fireWallNumNums;        /* How many entries can we
                                                 * use? */
        int             fireWallActiveNum;      /* Which entry did we last
                                                 * use? */
        char           *fireWallField;  /* bool array for entries */
#endif

        unsigned int    skinnyPort;     /* TCP port used by the Skinny     */
        /* protocol.                       */

        struct proxy_entry *proxyList;

        struct in_addr  true_addr;      /* in network byte order. */
        u_short         true_port;      /* in host byte order. */

};

#define IPPORT_RESERVED         1024

extern time_t	time_uptime;

#define _NETBAS_JUSTFTP
#define _NETBAS_DISABLE_GRE
#   define ULONG_MAX	4294967295UL

#ifndef USE_PREV_ALLOC
#undef malloc
#undef free
#undef calloc
#define malloc(n) kmalloc((n),0)
#define calloc(n,z) kcalloc((n)*(z))
#define free(n) kfree((n))
#endif

#endif

