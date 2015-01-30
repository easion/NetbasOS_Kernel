
/*
 * Definitions for options.
 */
#define IPOPT_COPIED(o)         ((o)&0x80)
#define IPOPT_CLASS(o)          ((o)&0x60)
#define IPOPT_NUMBER(o)         ((o)&0x1f)

#define IPOPT_CONTROL           0x00
#define IPOPT_RESERVED1         0x20
#define IPOPT_DEBMEAS           0x40
#define IPOPT_RESERVED2         0x60

#define IPOPT_EOL               0               /* end of option list */
#define IPOPT_NOP               1               /* no operation */

#define IPOPT_RR                7               /* record packet route */
#define IPOPT_TS                68              /* timestamp */
#define IPOPT_SECURITY          130             /* provide s,c,h,tcc */
#define IPOPT_LSRR              131             /* loose source route */
#define IPOPT_SATID             136             /* satnet id */
#define IPOPT_SSRR              137             /* strict source route */

/*
 * Offsets to fields in options other than EOL and NOP.
 */
#define IPOPT_OPTVAL            0               /* option ID */
#define IPOPT_OLEN              1               /* option length */
#define IPOPT_OFFSET            2               /* offset within option */
#define IPOPT_MINOFF            4               /* min value of above */

#define TCPOPT_EOL              0
#define TCPOPT_NOP              1
#define TCPOPT_MAXSEG           2
#define    TCPOLEN_MAXSEG               4
#define TCPOPT_WINDOW           3
#define    TCPOLEN_WINDOW               3
#define TCPOPT_SACK_PERMITTED   4               /* Experimental */
#define    TCPOLEN_SACK_PERMITTED       2
#define TCPOPT_SACK             5               /* Experimental */
#define TCPOPT_TIMESTAMP        8
#define    TCPOLEN_TIMESTAMP            10
#define    TCPOLEN_TSTAMP_APPA          (TCPOLEN_TIMESTAMP+2) /* appendix A */

#define TCPOPT_TSTAMP_HDR       \
    (TCPOPT_NOP<<24|TCPOPT_NOP<<16|TCPOPT_TIMESTAMP<<8|TCPOLEN_TIMESTAMP)

#define IN_CLASSD(a)            ((((long int) (a)) & 0xf0000000) == 0xe0000000)
#define IN_MULTICAST(a)         IN_CLASSD(a)
#define IN_MULTICAST_NET        0xF0000000

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))


/* Standard well-defined IP protocols.  */
enum
  {
    IPPROTO_IP = 0,        /* Dummy protocol for TCP.  */
#define IPPROTO_IP              IPPROTO_IP
    IPPROTO_HOPOPTS = 0,   /* IPv6 Hop-by-Hop options.  */
#define IPPROTO_HOPOPTS         IPPROTO_HOPOPTS
    IPPROTO_ICMP = 1,      /* Internet Control Message Protocol.  */
#define IPPROTO_ICMP            IPPROTO_ICMP
    IPPROTO_IGMP = 2,      /* Internet Group Management Protocol. */
#define IPPROTO_IGMP            IPPROTO_IGMP
    IPPROTO_IPIP = 4,      /* IPIP tunnels (older KA9Q tunnels use 94).  */
#define IPPROTO_IPIP            IPPROTO_IPIP
    IPPROTO_TCP = 6,       /* Transmission Control Protocol.  */
#define IPPROTO_TCP             IPPROTO_TCP
    IPPROTO_EGP = 8,       /* Exterior Gateway Protocol.  */
#define IPPROTO_EGP             IPPROTO_EGP
    IPPROTO_PUP = 12,      /* PUP protocol.  */
#define IPPROTO_PUP             IPPROTO_PUP
    IPPROTO_UDP = 17,      /* User Datagram Protocol.  */
#define IPPROTO_UDP             IPPROTO_UDP
    IPPROTO_IDP = 22,      /* XNS IDP protocol.  */
#define IPPROTO_IDP             IPPROTO_IDP
    IPPROTO_TP = 29,       /* SO Transport Protocol Class 4.  */
#define IPPROTO_TP              IPPROTO_TP
    IPPROTO_IPV6 = 41,     /* IPv6 header.  */
#define IPPROTO_IPV6            IPPROTO_IPV6
    IPPROTO_ROUTING = 43,  /* IPv6 routing header.  */
#define IPPROTO_ROUTING         IPPROTO_ROUTING
    IPPROTO_FRAGMENT = 44, /* IPv6 fragmentation header.  */
#define IPPROTO_FRAGMENT        IPPROTO_FRAGMENT
    IPPROTO_RSVP = 46,     /* Reservation Protocol.  */
#define IPPROTO_RSVP            IPPROTO_RSVP
    IPPROTO_GRE = 47,      /* General Routing Encapsulation.  */
#define IPPROTO_GRE             IPPROTO_GRE
    IPPROTO_ESP = 50,      /* encapsulating security payload.  */
#define IPPROTO_ESP             IPPROTO_ESP
    IPPROTO_AH = 51,       /* authentication header.  */
#define IPPROTO_AH              IPPROTO_AH
    IPPROTO_ICMPV6 = 58,   /* ICMPv6.  */
#define IPPROTO_ICMPV6          IPPROTO_ICMPV6
    IPPROTO_NONE = 59,     /* IPv6 no next header.  */
#define IPPROTO_NONE            IPPROTO_NONE
    IPPROTO_DSTOPTS = 60,  /* IPv6 destination options.  */
#define IPPROTO_DSTOPTS         IPPROTO_DSTOPTS
    IPPROTO_MTP = 92,      /* Multicast Transport Protocol.  */
#define IPPROTO_MTP             IPPROTO_MTP
    IPPROTO_ENCAP = 98,    /* Encapsulation Header.  */
#define IPPROTO_ENCAP           IPPROTO_ENCAP
    IPPROTO_PIM = 103,     /* Protocol Independent Multicast.  */
#define IPPROTO_PIM             IPPROTO_PIM
    IPPROTO_COMP = 108,    /* Compression Header Protocol.  */
#define IPPROTO_COMP            IPPROTO_COMP
    IPPROTO_SCTP = 132,    /* Stream Control Transmission Protocol.  */
#define IPPROTO_SCTP            IPPROTO_SCTP
    IPPROTO_RAW = 255,     /* Raw IP packets.  */
#define IPPROTO_RAW             IPPROTO_RAW
    IPPROTO_MAX
  };

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

struct icmphdr {
  u8_t          type;
  u8_t          code;
  u16_t         checksum;
  union {
        struct {
                u16_t   id;
                u16_t   sequence;
        } echo;
        u32_t   gateway;
        struct {
                u16_t   __unused;
                u16_t   mtu;
        } frag;
  } un;
};



