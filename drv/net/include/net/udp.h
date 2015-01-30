

#ifndef UDP_H
#define UDP_H

#define UDP_HLEN 8


#define UDP_FLAGS_NOCHKSUM  0x01
#define UDP_FLAGS_BROADCAST 0x02
#define UDP_FLAGS_CONNECTED 0x04


typedef struct udp
{
	//! The source port.
	u16_t udp_src;
	//! The destination port.
	u16_t udp_dst;

	//! The packet length.
	u16_t udp_len;
	//! The UDP checksum.
	u16_t udp_chk;

} __attribute__ ((packed)) udp_t;

#endif
