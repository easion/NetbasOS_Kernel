
#include <net/net.h>
#define IP_ADDR_ANY_VALUE 0x00000000UL
#define IP_ADDR_BROADCAST_VALUE 0xffffffffUL
struct ip_addr ip_addr_any       = {0x00000000};
struct ip_addr ip_addr_broadcast = {0xFFFFFFFF};


char * inetntoa(u32_t net32)
{
	static char bufs[4][16];
	static unsigned int index = 0;

	char * buf = bufs[index++ & 3];
	u8_t * p = (u8_t *) &net32;
	snprintf(buf,64, "%d.%d.%d.%d", p[0], p[1], p[2], p[3]);
	return buf;
}

void dump_ipaddr(char *label,struct ip_addr *ipaddr)
{
  kprintf("%s: (%d.%d.%d.%d)\n",
  label,
	     ntohl(ipaddr->addr) >> 24 & 0xFF,
	     ntohl(ipaddr->addr) >> 16 & 0xFF,
	     ntohl(ipaddr->addr) >> 8 & 0xFF,
	     ntohl(ipaddr->addr) & 0xFF);
}

struct ip_addr *inetaton(char * str)
{
	int i;
	u8_t tmp[4];
	static struct ip_addr ip;

	//memset(&ip, 0, sizeof(struct ip_addr );

	for ( i = 0; i < 4; i++) {
		tmp[i] = 0;
		while (*str&&(*str<'0' || *str>'9'))
		{
			str++;
		}
		while (*str && (*str >= '0') && (*str <='9')) {
			tmp[i] *= 10;
			tmp[i] += *str - '0';
			str++;
		}
		//printk("%d-", tmp[i]);
		str++;
	}

	 IP4_ADDR(&ip, tmp[0],tmp[1],tmp[2],tmp[3]);
	//u32_t host32 = (tmp[0] << 24) | (tmp[1] << 16) | (tmp[2] << 8) | tmp[3];
	//return htonl(host32);
	return &ip;
} 



