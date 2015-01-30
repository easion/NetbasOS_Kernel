

#ifndef STATS_H
#define STATS_H

struct stats_nic
{
  unsigned long rx_packets;             // Total packets received
  unsigned long tx_packets;             // Total packets transmitted
  unsigned long rx_bytes;               // Total bytes received
  unsigned long tx_bytes;               // Total bytes transmitted
  unsigned long rx_errors;              // Bad packets received
  unsigned long tx_errors;              // Packet transmit problems
  unsigned long rx_dropped;             // Received packets dropped
  unsigned long tx_dropped;             // Transmitted packets dropped
  unsigned long multicast;              // Multicast packets received
  unsigned long collisions;

  // Detailed rx errors
  unsigned long rx_length_errors;
  unsigned long rx_over_errors;         // Receiver ring buff overflow
  unsigned long rx_crc_errors;          // Recved pkt with crc error
  unsigned long rx_frame_errors;        // Recv'd frame alignment error
  unsigned long rx_fifo_errors;         // Recv'r fifo overrun
  unsigned long rx_missed_errors;       // Receiver missed packet

  // Detailed tx errors
  unsigned long tx_aborted_errors;
  unsigned long tx_carrier_errors;
  unsigned long tx_fifo_errors;
  unsigned long tx_heartbeat_errors;
  unsigned long tx_window_errors;

  // Compression
  unsigned long rx_compressed;
  unsigned long tx_compressed;
};

struct stats_proto 
{
  unsigned long xmit;    // Transmitted packets
  unsigned long rexmit;  // Retransmitted packets
  unsigned long recv;    // Received packets
  unsigned long fw;      // Forwarded packets
  unsigned long drop;    // Dropped packets
  unsigned long chkerr;  // Checksum error
  unsigned long lenerr;  // Invalid length error
  unsigned long memerr;  // Out of memory error
  unsigned long rterr;   // Routing error
  unsigned long proterr; // Protocol error
  unsigned long opterr;  // Error in options
  unsigned long err;     // Misc error
};

struct stats_pbuf 
{
  unsigned long avail;
  unsigned long used;
  unsigned long max;  
  unsigned long err;
  unsigned long reclaimed;

  unsigned long alloc_locked;
  unsigned long refresh_locked;

  unsigned long rwbufs;
};

struct stats_all
{
  struct stats_proto link;
  struct stats_proto ip;
  struct stats_proto icmp;
  struct stats_proto udp;
  struct stats_proto tcp;
  struct stats_proto raw;
  struct stats_pbuf pbuf;
};

extern struct stats_all stats;

void stats_init();

#endif
