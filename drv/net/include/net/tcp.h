
#ifndef TCP_H
#define TCP_H


// Options for tcp_write

#define TCP_WRITE_FLUSH          0
#define TCP_WRITE_NAGLE          1
#define TCP_WRITE_NOFLUSH        2

// Used within the TCP code only


#define TCP_SEQ_LT(a,b)     ((int)((a)-(b)) < 0)
#define TCP_SEQ_LEQ(a,b)    ((int)((a)-(b)) <= 0)
#define TCP_SEQ_GT(a,b)     ((int)((a)-(b)) > 0)
#define TCP_SEQ_GEQ(a,b)    ((int)((a)-(b)) >= 0)

#define TCP_FIN 0x01
#define TCP_SYN 0x02
#define TCP_RST 0x04
#define TCP_PSH 0x08
#define TCP_ACK 0x10
#define TCP_URG 0x20

#define TCP_HLEN 20                 // Length of the TCP header, excluding options 
#define TCP_FAST_INTERVAL      200  // The fine grained timeout in milliseconds
#define TCP_SLOW_INTERVAL      500  // The coarse grained timeout in milliseconds
#define TCP_FIN_WAIT_TIMEOUT 20000  // milliseconds
#define TCP_SYN_RCVD_TIMEOUT 20000  // milliseconds

#define TCP_OOSEQ_TIMEOUT        6  // x RTO



#define TCPH_OFFSET(hdr) (NTOHS((hdr)->_offset_flags) >> 8)
#define TCPH_FLAGS(hdr) (NTOHS((hdr)->_offset_flags) & 0xFF)

#define TCPH_OFFSET_SET(hdr, offset) (hdr)->_offset_flags = HTONS(((offset) << 8) | TCPH_FLAGS(hdr))
#define TCPH_FLAGS_SET(hdr, flags) (hdr)->_offset_flags = HTONS((TCPH_OFFSET(hdr) << 8) | (flags))

#define TCP_TCPLEN(seg) ((seg)->len + ((TCPH_FLAGS((seg)->tcphdr) & TCP_FIN || \
					TCPH_FLAGS((seg)->tcphdr) & TCP_SYN) ? 1: 0))

enum tcp_state 
{
  CLOSED      = 0,
  LISTEN      = 1,
  SYN_SENT    = 2,
  SYN_RCVD    = 3,
  ESTABLISHED = 4,
  FIN_WAIT_1  = 5,
  FIN_WAIT_2  = 6,
  CLOSE_WAIT  = 7,
  CLOSING     = 8,
  LAST_ACK    = 9,
  TIME_WAIT   = 10
};

// TCP protocol control block

#define TF_ACK_DELAY 0x01   // Delayed ACK
#define TF_ACK_NOW   0x02   // Immediate ACK
#define TF_INFR      0x04   // In fast recovery
#define TF_RESET     0x08   // Connection was reset
#define TF_CLOSED    0x10   // Connection was sucessfully closed
#define TF_GOT_FIN   0x20   // Connection was closed by the remote end
#define TF_IN_RECV   0x40   // Connection is processing received segment


#define TCP_REG(pcbs, npcb) do { \
                            npcb->next = *pcbs; \
                            *pcbs = npcb; \
                            } while (0)

#define TCP_RMV(pcbs, npcb) do { \
                            if (*pcbs == npcb) { \
                               *pcbs = (*pcbs)->next; \
                            } else for (tcp_tmp_pcb = *pcbs; tcp_tmp_pcb != NULL; tcp_tmp_pcb = tcp_tmp_pcb->next) { \
                               if (tcp_tmp_pcb->next != NULL && tcp_tmp_pcb->next == npcb) { \
                                  tcp_tmp_pcb->next = npcb->next; \
                                  break; \
                               } \
                            } \
                            npcb->next = NULL; \
                            } while (0)

//! TCP connection closed state.
#define TCP_CLOSED	0
//! TCP SYN received state.
#define TCP_SYN_RCVD	1
//! TCP SYN sent state.
#define TCP_SYN_SENT	2
//! TCP connection established state.
#define TCP_ESTABLISHED	3
//! TCP closing state.
#define TCP_CLOSING	4


struct tcp_hdr 
{
  unsigned short src, dest;
  unsigned long seqno, ackno;
  unsigned short _offset_flags;
  unsigned short wnd;
  unsigned short chksum;
  unsigned short urgp;
};


//! A socket structure.
typedef struct
{
	//! IP source address (in network format).
	in_addr_t ip_src;
	//! Source port.
	unsigned short port_src;
	//! IP destination address (in network format).
	in_addr_t ip_dst;
	//! Destination port.
	unsigned short port_dst;
} socket_t;

//! A TCP connection states machine structure.
typedef struct
{
	// The state of the connection.
	int state;
	//! The socket used in the connection.
	socket_t socket;
	//! The current sequence number.
	int seq_num;
} tcp_state_t;

//! TCP packet structure.
typedef struct tcp
{
	//! Source port.
	unsigned short tcp_src;
	//! Destination port.
	unsigned short tcp_dst;

	//! Sequence number.
	unsigned long tcp_seq_num;
	//! ACK number.
	unsigned long tcp_ack_num;

#if __BYTE_ORDER__ == __LITTLE_ENDIAN__
	//! Reserved (bit 0..3).
	unsigned char tcp_res1:4;
	//! Header length.
	unsigned char tcp_hdr_len:4;
	//! FIN flag.
	unsigned char tcp_fin:1;
	//! SYN flag.
	unsigned char tcp_syn:1;
	//! RST flag.
	unsigned char tcp_rst:1;
	//! PSH flag.
	unsigned char tcp_psh:1;
	//! ACK flag.
	unsigned char tcp_ack:1;
	//! URG flag.
	unsigned char tcp_urg:1;
	//! Reserved (bit 4..6).
	unsigned char tcp_res2:2;
#else
	//! Header length.
	unsigned char tcp_hdr_len:4;
	//! Reserved.
	unsigned char tcp_res:6;
	//! URG flag.
	unsigned char tcp_urg:1;
	//! ACK flag.
	unsigned char tcp_ack:1;
	//! PSH flag.
	unsigned char tcp_psh:1;
	//! RST flag.
	unsigned char tcp_rst:1;
	//! SYN flag.
	unsigned char tcp_syn:1;
	//! FIN flag.
	unsigned char tcp_fin:1;
#endif
	//! Window size.
	unsigned short tcp_win_size;
	//! TCP checksum.
	unsigned short tcp_chk;
	//! Urgent pointer.
	unsigned short tcp_urg_ptr;
	
} __attribute__ ((packed)) tcp_t;
#endif


typedef struct HeaderTCP
{
   unsigned short srcPort;
   unsigned short dstPort;
   unsigned long seq;
   unsigned long ack;
   unsigned char  len;
   unsigned char  flags;
   unsigned short window;
   unsigned short checksum;
   unsigned short urgent;
}
HeaderTCP;
