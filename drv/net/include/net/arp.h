//
// arp.h
//

#ifndef ARP_H
#define ARP_H

#define ETHTYPE_ARP 0x0806
#define ETHTYPE_IP  0x0800


enum {
	ARP_OP_REQUEST = 1,
	ARP_OP_REPLY,
	ARP_OP_RARP_REQUEST,
	ARP_OP_RARP_REPLY,
	ARP_OP_DL_REQUEST,
	ARP_OP_DL_REPLY
};

enum {
	ARPHRD_ETHER = 1
};

// The ARP packet structure.
typedef struct arp
{
	//! Format of hardware address.
	u16_t arp_hard_type;
	//! Format of protocol address.
	u16_t arp_proto_type;
	//! Length of hardware address.
	u8_t  arp_hard_size;
	//! Length of protocol address.
	u8_t  arp_proto_size;
	//! ARP operation code (command).
	u16_t arp_op;
	//! Hardware source address.
	u8_t  arp_eth_source[6];
	//! IP source address.
	u32_t arp_ip_source;
	//! Hardware destination address.
	u8_t  arp_eth_dest[6];
	//! IP destination address.
	u32_t arp_ip_dest;
} __attribute__ ((packed)) arp_t;

#endif
