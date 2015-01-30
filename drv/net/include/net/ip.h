//

#ifndef IP_H
#define IP_H


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

#define IP_ADDRESS(a,b,c,d)	((a) | (b) << 8 | (c) << 16 | (d) << 24)

#define NETADDR_TO_IPV4(naddr) (*(ipv4_addr *)(&((&(naddr))->addr[0])))
#define IPV4_DOTADDR_TO_ADDR(a, b, c, d) \
	(((ipv4_addr)(a) << 24) | (((ipv4_addr)(b) & 0xff) << 16) | (((ipv4_addr)(c) & 0xff) << 8) | ((ipv4_addr)(d) & 0xff))



//! IP version 4.
#define IP_V4			4
//! IP version 6.
#define IP_V6			6

//! Maximum IP frame length.
#define IP_FRAME_LEN		65535
//! Minimum IP header length.
#define IP_HEAD_MIN_LEN		20
//! Default TTL (Time To Live).
#define IP_DEFAULT_TTL		64

//! Generic IP packet type.
#define IPPROTO_IP		0
//! ICMP (Internet Control Message Protocol) packet type.
#define IPPROTO_ICMP		1
//! IGMP (Internet Group Message Protocol) packet type.
#define IPPROTO_IGMP		2
//! TCP (Transmition Control Protocol) packet type.
#define IPPROTO_TCP		6
//! UDP (User Datagram Protocol) packet type.
#define IPPROTO_UDP		17

//! Type of service :: Minimum delay.
#define IP_TOS_MIN_DELAY	0x10
//! Type of service :: Maximum throughput.
#define IP_TOS_MAX_THRU		0x08
//! Type of service :: Maximum rely.
#define IP_TOS_MAX_RELY		0x04
//! Type of service :: Minimum cost.
#define IP_TOS_MIN_COST		0x02

// Fragment flags							//
//! More Fragments.
#define IP_FLAG_MF		0x2000
//! Don't Fragment.
#define IP_FLAG_DF		0x4000
//! The CE flag.
#define IP_FLAG_CE		0x8000
//! The flag mask.
#define IP_FLAG_MASK		0x1FFF

/** \ingroup Network
 *  \defgroup NetIP IP (Internet Protocol) layer
 *  The IP (Internet Protocol) layer.
 *  @{
 */



//! Loopback IP address.
#define INADDR_LOOPBACK		IP_ADDRESS(127, 0, 0, 1)
//! Null IP address.
#define INADDR_ANY		IP_ADDRESS(0, 0, 0, 0)
//! Broadcast IP address.
#define INADDR_BROADCAST	IP_ADDRESS(255, 255, 255, 255)

//! IP address type (in binary network format).
typedef u32_t in_addr_t;

//! The IP packet structure.
typedef struct ip
{
#if __BYTE_ORDER__ == __LITTLE_ENDIAN__
	u8_t ip_hdr_len:4;	//!< The header length.
	u8_t ip_version:4;	//!< The IP version.
#else
	u8_t ip_version:4;	//!< The IP version.
	u8_t ip_hdr_len:4;	//!< The IP header length.
#endif
	//! Type of Service.
	u8_t ip_tos;
	//! IP packet length (both data and header).
	u16_t ip_len;

	//! Identification.
	u16_t ip_id;
	//! Fragment offset.
	u16_t ip_off;

	//! Time To Live.
	u8_t ip_ttl;
	//! The type of the upper-level protocol.
	u8_t ip_proto;
	//! IP header checksum.
	u16_t ip_chk;

	//! IP source address (in network format).
	u32_t ip_src;
	//! IP destination address (in network format).
	u32_t ip_dst;
} __attribute__ ((packed)) ip_t;


#endif


