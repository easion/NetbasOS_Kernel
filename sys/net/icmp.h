
#ifndef ICMP_H
#define ICMP_H

#define ICMP_ER    0       // Echo reply
#define ICMP_DUR   3       // Destination unreachable
#define ICMP_SQ    4       // Source quench
#define ICMP_REDIRECT     5       // Redirect
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

err_t icmp_input(struct pbuf *p, struct netif *inp);

void icmp_dest_unreach(struct pbuf *p, int t);
void icmp_time_exceeded(struct pbuf *p, int t);

//#pragma pack(push)
//#pragma pack(1)

struct icmp_echo_hdr 
{
  u16_t _type_code;
  u16_t chksum;
  u16_t id;
  u16_t seqno;
}__attr_packet;

struct icmp_dur_hdr 
{
  u16_t _type_code;
  u16_t chksum;
  u32_t unused;
}__attr_packet;

struct icmp_te_hdr 
{
  u16_t _type_code;
  u16_t chksum;
  u32_t unused;
}__attr_packet;

//#pragma pack(pop)

#define ICMPH_TYPE(hdr) (NTOHS((hdr)->_type_code) >> 8)
#define ICMPH_CODE(hdr) (NTOHS((hdr)->_type_code) & 0xFF)

#define ICMPH_TYPE_SET(hdr, type) ((hdr)->_type_code = HTONS(ICMPH_CODE(hdr) | ((type) << 8)))
#define ICMPH_CODE_SET(hdr, code) ((hdr)->_type_code = HTONS((code) | (ICMPH_TYPE(hdr) << 8)))

#endif
	  
