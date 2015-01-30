
#ifndef NETIF_H
#define NETIF_H

#define NETIF_UP                      0x00000001   // Interface is up
#define NETIF_DHCP                    0x00000002   // Get IP address for interface via DHCP
#define NETIF_LOOPBACK                0x00000004   // Interface is loopback

#define NETIF_MULTICAST               0x00000010   // Supports multicast
#define NETIF_ALLMULTI                0x00000020   // Receive all multicast packets
#define NETIF_PROMISC                 0x00000040   // Receive all packets
#define NETIF_NO_ARP                 0x00000080

#define NETIF_IP_TX_CHECKSUM_OFFLOAD  0x00010000
#define NETIF_IP_RX_CHECKSUM_OFFLOAD  0x00020000
#define NETIF_UDP_RX_CHECKSUM_OFFLOAD 0x00040000
#define NETIF_UDP_TX_CHECKSUM_OFFLOAD 0x00080000
#define NETIF_TCP_RX_CHECKSUM_OFFLOAD 0x00100000
#define NETIF_TCP_TX_CHECKSUM_OFFLOAD 0x00200000

struct mclist
{
  struct mclist *next;
  struct ip_addr ipaddr;
  struct eth_addr hwaddr;
};


struct netif 
{
  struct netif *next;
  struct ip_addr ipaddr;
  struct ip_addr netmask;
  struct ip_addr gw;
  struct ip_addr broadcast;
  struct eth_addr hwaddr;
  int flags;
  char name[NET_NAME_MAX];

  struct mclist *mclist;
  int mccount;
  int ifindex;

  err_t (*input)(struct pbuf *p, struct netif *inp);
  err_t (*output)(struct netif *netif, struct pbuf *p, struct ip_addr *ipaddr);
  err_t (* linkoutput)(struct netif *netif, struct pbuf *p);
  
  void *state;
  int mtu;
};

// The list of network interfaces.

extern struct netif *netif_list;
extern struct netif *netif_default;

void netif_init();

struct netif *netif_add(char *name, struct ip_addr *ipaddr, struct ip_addr *netmask, struct ip_addr *gw);
struct netif *netif_find(char *name);

void netif_set_default(struct netif *netif);

void netif_set_ipaddr(struct netif *netif, struct ip_addr *ipaddr);
void netif_set_netmask(struct netif *netif, struct ip_addr *netmast);
void netif_set_gw(struct netif *netif, struct ip_addr *gw);

int netif_ioctl_list(void *data, size_t size);
int netif_ioctl_cfg(void *data, size_t size);

#endif
