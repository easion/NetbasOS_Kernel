
/* 
** Jicama OS Loadable Kernel Modules Test
** 2005-3-5
*/
#include <drv/drv.h>
#include "lwip/ip_addr.h"

#define IP_ADDR_ANY_VALUE 0x00000000UL
#define IP_ADDR_BROADCAST_VALUE 0xffffffffUL

const struct ip_addr ip_addr_any = { IP_ADDR_ANY_VALUE };
const struct ip_addr ip_addr_broadcast = { IP_ADDR_BROADCAST_VALUE };

/*dll entry*/
int dll_main(char **argv)
{
	int i=0;

	while (argv&&argv[i]){
		printk("%s\n", argv[i]);
	i++;
	}

	httpd_init();

	//tcpecho_init();
	//udpecho_init();
	return 0;
}


int dll_destroy()
{
	printk("dll_destroy called!\n");
	return 0;
}

int dll_version()
{
	return 0;
}
