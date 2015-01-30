

#ifndef ETH_H
#define ETH_H


//! IP packet type.
#define ETH_FRAME_IP		0x0800
//! ARP packet type.
#define ETH_FRAME_ARP		0x0806
//! Size of an ethernet address (a.k.a. MAC).
#define ETH_ADDR_LEN		6
//! Size of the ethernet header.
#define ETH_HEAD_LEN		14
//! Minimum ethernet packet size.
#define ETH_MIN_LEN		60
//! Maximum ethernet packet size.
#define ETH_FRAME_LEN		1514
//! Ethernet MTU (Maximum transfer unit).
#define ETH_MTU			(ETH_FRAME_LEN - ETH_HEAD_LEN)

//! Send and receive buffers size.
#define ETH_RECV_BUF_DIM	10

//! Physical packet structure.
typedef struct phys_packet
{
	//! The packet size.
	size_t length;
	//! The packet content.
	uint8_t data[ETH_FRAME_LEN];
} phys_packet_t;

//! The ethernet buffer structure.
typedef struct eth_buf
{
	//! The packets pointers.
	phys_packet_t packet[ETH_RECV_BUF_DIM];
	//! The position of the next packet received.
	int read;
	//! The position of the next packet to be sent.
	int write;
	//! The amount of packets in the buffer.
	int count;
} eth_buf_t;





typedef struct 
{
    u8_t  dst[ETH_ADDR_LEN];
    u8_t  src[ETH_ADDR_LEN];    
    uint16_t type;    
	uint8_t data[1];
} eth_t;




extern void (*netcard_send)(const void *packet, size_t len);
int send_eth_packet(const u8_t *to, const void *data, size_t len, u16_t type);
u8_t* get_eth_mac_addr();


#endif
