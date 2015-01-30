

#ifndef IP_H
#define IP_H

/*
 * Option flags per-socket. These are the same like SO_XXX.
 */
#define SOF_DEBUG       (u16_t)0x0001U    /* turn on debugging info recording */
#define SOF_ACCEPTCONN  (u16_t)0x0002U    /* socket has had listen() */
#define SOF_REUSEADDR   (u16_t)0x0004U    /* allow local address reuse */
#define SOF_KEEPALIVE   (u16_t)0x0008U    /* keep connections alive */
#define SOF_DONTROUTE   (u16_t)0x0010U    /* just use interface addresses */
#define SOF_BROADCAST   (u16_t)0x0020U    /* permit sending of broadcast msgs */
#define SOF_USELOOPBACK (u16_t)0x0040U    /* bypass hardware when possible */
#define SOF_LINGER      (u16_t)0x0080U    /* linger on close if data present */
#define SOF_OOBINLINE   (u16_t)0x0100U    /* leave received OOB data in line */
#define SOF_REUSEPORT   (u16_t)0x0200U    /* allow local address & port reuse */


#define IP_PCB_ADDRHINT
/* This is the common part of all PCB types. It needs to be at the
   beginning of a PCB type definition. It is located here so that
   changes to this common part are made in one location instead of
   having to change all PCB structs. */
#define IP_PCB \
  /* ip addresses in network byte order */ \
  struct ip_addr local_ip; \
  struct ip_addr remote_ip; \
   /* Socket options */  \
  u16_t so_options;      \
   /* Type Of Service */ \
  u8_t tos;              \
  /* Time To Live */     \
  u8_t ttl               \
  /* link layer address resolution hint */ \
  IP_PCB_ADDRHINT

struct ip_pcb {
/* Common members of all PCB types */
  IP_PCB;
};
void ip_init(void);
int ip_ownaddr(struct ip_addr *addr);
struct netif *ip_route(struct ip_addr *dest);
err_t ip_input(struct pbuf *p, struct netif *inp);
err_t ip_input_dur(int code, struct pbuf *p);
err_t ip_output(struct pbuf *p, struct ip_addr *src, struct ip_addr *dest, int ttl, int proto);
err_t ip_output_if(struct pbuf *p, struct ip_addr *src, struct ip_addr *dest, int ttl, int proto, struct netif *netif);

#define IP_HLEN 20

#define IP_PROTO_ICMP    1
#define IP_PROTO_UDP     17
#define IP_PROTO_TCP     6

// This is passed as the destination address to ip_output_if (not
// to ip_output), meaning that an IP header already is constructed
// in the pbuf.

#define IP_HDRINCL  NULL

#define IP_RF      0x8000        // Reserved fragment flag
#define IP_DF      0x4000        // Dont fragment flag
#define IP_MF      0x2000        // More fragments flag
#define IP_OFFMASK 0x1fff        // Mask for fragmenting bits

//#pragma pack(push)
//#pragma pack(1)

struct ip_hdr 
{
  u16_t _v_hl_tos;      // Version / header length / type of service
  u16_t _len;           // Total length
  u16_t _id;            // Identification
  u16_t _offset;        // Fragment offset field
  u16_t _ttl_proto;     // Time to live / protocol
  u16_t _chksum;        // Checksum
  struct ip_addr src, dest;      // Source and destination IP addresses
}__attr_packet;

//#pragma pack(pop)

#define IPH_V(hdr)  (NTOHS((hdr)->_v_hl_tos) >> 12)
#define IPH_HL(hdr) ((NTOHS((hdr)->_v_hl_tos) >> 8) & 0x0F)
#define IPH_TOS(hdr) HTONS((NTOHS((hdr)->_v_hl_tos) & 0xFF))
#define IPH_LEN(hdr) ((hdr)->_len)
#define IPH_ID(hdr) ((hdr)->_id)
#define IPH_OFFSET(hdr) ((hdr)->_offset)
#define IPH_TTL(hdr) (NTOHS((hdr)->_ttl_proto) >> 8)
#define IPH_PROTO(hdr) (NTOHS((hdr)->_ttl_proto) & 0xFF)
#define IPH_CHKSUM(hdr) ((hdr)->_chksum)

#define IPH_VHLTOS_SET(hdr, v, hl, tos) (hdr)->_v_hl_tos = HTONS(((v) << 12) | ((hl) << 8) | (tos))
#define IPH_LEN_SET(hdr, len) (hdr)->_len = (len)
#define IPH_ID_SET(hdr, id) (hdr)->_id = (id)
#define IPH_OFFSET_SET(hdr, off) (hdr)->_offset = (off)
#define IPH_TTL_SET(hdr, ttl) (hdr)->_ttl_proto = HTONS(IPH_PROTO(hdr) | ((ttl) << 8))
#define IPH_PROTO_SET(hdr, proto) (hdr)->_ttl_proto = HTONS((proto) | (IPH_TTL(hdr) << 8))
#define IPH_CHKSUM_SET(hdr, chksum) (hdr)->_chksum = (chksum)


//typedef struct  _Route  Route_s;
struct  route
{
	/* Locked by the routing list mutex */
	//Route_s*        rt_psNext;
	TAILQ_ENTRY(route) next;
	/* bigtime_t       rt_nExpireTime; */

	/* Locked by rt_hMutex */
	int          rt_hMutex;
	struct ip_addr        rt_anNetAddr;
	struct ip_addr        rt_anNetMask;
	struct ip_addr        rt_anGatewayAddr;
	int             rt_nMaskBits, rt_nMetric;
	u32_t         rt_nFlags;
	struct netif* rt_psInterface;
};


typedef struct route_table
{
   // Route_s*  rtb_psFirstRoute;
	TAILQ_HEAD(,_route) route_head;
    struct _route*  route_default;
} RouteTable_s;

#endif
