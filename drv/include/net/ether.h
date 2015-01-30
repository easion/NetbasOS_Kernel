

#ifndef ETHER_H
#define ETHER_H

//#pragma pack(push)
//#pragma pack(1)

#define ETHER_HLEN 14

#define ETHER_ADDR_LEN 6
#define ETHTYPE_ARP 0x0806
#define ETHTYPE_IP  0x0800
struct eth_addr 
{
  unsigned char addr[ETHER_ADDR_LEN];
}__attr_packet;
  
struct eth_hdr 
{
  struct eth_addr dest;
  struct eth_addr src;
  unsigned short type;
}__attr_packet;

//#pragma pack(pop)

__inline static  int eth_addr_isbroadcast(struct eth_addr *addr)
{
  int n;
  for (n = 0; n < ETHER_ADDR_LEN; n++) if (addr->addr[n] != 0xFF) return 0;
  return 1;
}

 char *ether2str(struct eth_addr *hwaddr, char *s);
 unsigned long ether_crc(int length, unsigned char *data);

 struct netif *ether_netif_add(char *name, char *devname,
	struct ip_addr *theipaddr, struct ip_addr *netmask, struct ip_addr *gw);

 err_t ether_input(struct netif *netif, struct pbuf *p);
 err_t ether_output(struct netif *netif, struct pbuf *p, struct ip_addr *ipaddr);

void ether_init();

#endif
