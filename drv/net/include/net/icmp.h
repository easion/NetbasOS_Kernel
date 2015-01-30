//


#ifndef ICMP_H
#define ICMP_H

#define ICMP_ER    0       // Echo reply
#define ICMP_DUR   3       // Destination unreachable
#define ICMP_SQ    4       // Source quench
#define ICMP_RD    5       // Redirect
#define ICMP_ECHO  8       // Echo
#define ICMP_TE   11       // Time exceeded
#define ICMP_PP   12       // Parameter problem
#define ICMP_TS   13       // Timestamp
#define ICMP_TSR  14       // Timestamp reply
#define ICMP_IRQ  15       // Information request
#define ICMP_IR   16       // Information reply

#define ICMP_DUR_NET    0  // Net unreachable
#define ICMP_DUR_HOST   1  // Host unreachable
#define ICMP_DUR_PROTO  2  // Protocol unreachable
#define ICMP_DUR_PORT   3  // Port unreachable
#define ICMP_DUR_FRAG   4  // Fragmentation needed and DF set
#define ICMP_DUR_SR     5  // Source route failed

#define ICMP_TE_TTL     0  // Time to live exceeded in transit
#define ICMP_TE_FRAG    1  // Fragment reassembly time exceeded

#define ICMP_HLEN       8

#define ICMPH_TYPE(hdr) (NTOHS((hdr)->_type_code) >> 8)
#define ICMPH_CODE(hdr) (NTOHS((hdr)->_type_code) & 0xFF)

#define ICMPH_TYPE_SET(hdr, type) ((hdr)->_type_code = HTONS(ICMPH_CODE(hdr) | ((type) << 8)))
#define ICMPH_CODE_SET(hdr, code) ((hdr)->_type_code = HTONS((code) | (ICMPH_TYPE(hdr) << 8)))


///
//! Echo Reply.
#define ICMP_ECHOREPLY		0
//! Destination Unreachable.
#define ICMP_DEST_UNREACH	3
//! Source Quench.
#define ICMP_SOURCE_QUENCH	4
//! Redirect (change route).
#define ICMP_REDIRECT		5
//! Echo Request.
#define ICMP_ECHO		8
//! Time Exceeded.
#define ICMP_TIME_EXCEEDED	11
//! Parameter Problem.
#define ICMP_PARAMETERPROB	12
//! Timestamp Request.
#define ICMP_TIMESTAMP		13
//! Timestamp Reply.
#define ICMP_TIMESTAMPREPLY	14
//! Information Request.
#define ICMP_INFO_REQUEST	15
//! Information Reply.
#define ICMP_INFO_REPLY		16
//! Address Mask Request.
#define ICMP_ADDRESS		17
//! Address Mask Reply.
#define ICMP_ADDRESSREPLY	18

//! Network Unreachable.
#define ICMP_NET_UNREACH	0
//! Host Unreachable.
#define ICMP_HOST_UNREACH	1
//! Protocol Unreachable.
#define ICMP_PROT_UNREACH	2
//! Port Unreachable.
#define ICMP_PORT_UNREACH	3
//! Fragmentation Needed/DF set.
#define ICMP_FRAG_NEEDED	4
//! Source Route failed.
#define ICMP_SR_FAILED		5
//! Network Unknown.
#define ICMP_NET_UNKNOWN	6
//! Host Unknown.
#define ICMP_HOST_UNKNOWN	7
//! Host isolated.
#define ICMP_HOST_ISOLATED	8

#define ICMP_NET_ANO		9
#define ICMP_HOST_ANO		10
#define ICMP_NET_UNR_TOS	11
#define ICMP_HOST_UNR_TOS	12

//! Packet Filtered.
#define ICMP_PKT_FILTERED	13
//! Precedence Violation.
#define ICMP_PREC_VIOLATION	14
//! Precedence Cut Off.
#define ICMP_PREC_CUTOFF	15

//! Redirect Net.
#define ICMP_REDIR_NET		0
//! Redirect Host.
#define ICMP_REDIR_HOST		1
//! Redirect Net for TOS.
#define ICMP_REDIR_NETTOS	2
//! Redirect Host for TOS.
#define ICMP_REDIR_HOSTTOS	3

//! TTL cound exceeded.
#define ICMP_EXC_TTL		0
//! Fragment Reass Time exceeeded.
#define ICMP_EXC_FRAGTIME	1

//! ICMP packet structure.
typedef struct icmp
{
	//! ICMP message type.
	u8_t icmp_type;
	//! ICMP operation code.
	u8_t icmp_code;
	//! ICMP checksum.
	u16_t icmp_chk;
} __attribute__ ((packed)) icmp_t;

//! ICMP::PING packet structure.
typedef struct icmp_ping
{
	//! The ICMP header.
	icmp_t ping_icmp;
	//! The PING id.
	u16_t ping_id;
	//! The PING sequence number.
	u16_t ping_seq;
} __attribute__ ((packed)) icmp_ping_t;



#endif
	  
