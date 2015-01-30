
/*netbas port*/

#include <drv/defs.h>
#include <drv/drv.h>
#include <drv/sym.h>
#include <drv/pci.h>
#include <drv/ia.h>
#include <drv/errno.h>
#include <drv/spin.h>
#include <drv/log.h>
#include <net/pkt.h>
#include <assert.h>

#include <sys/queue.h>
#include <sys/compat_bsd.h>
#include <sys/endian.h>

#include "etherdrv.h"
int pcnet32_attach(dev_prvi_t* devfp, struct eth_addr *hwaddr);
int pcnet32_detach(dev_prvi_t* devfp);
int pcnet32_transmit(dev_prvi_t* devfp, struct pbuf *p);


char* get_mac();
int pcnet32_probe();
int pcnet32_ioctl( int fd, 
int nCommand, void* pArgs, int len,bool bFromKernel );
#define RTL8139_MAJOR 0X40

#define ETHTHREAD_NAME "EthernetIF"
#define SIGTTIN           21	/* background process wants to read */


CREATE_SPINLOCK( pcnet32_lock );
void sendsig(void *rp, int signo);
int sigdelset(void *rp, int signo);
int sigrecv(void *rp, char*);

int pcnet32_drv_open(const char *f, int mode,dev_prvi_t* devfp);

int pcnet32_drv_close(int  file);



static const driver_ops_t pcnet32_drv_fops =
{
	d_name:		"eth",
	d_author:	"Thomas Bogendoerfer",
	d_version:	"v2006-01-25",
	d_kver:	CURRENT_KERNEL_VERSION,
	d_index:	RTL8139_MAJOR<<8,
	open:		pcnet32_drv_open,
	close:		pcnet32_drv_close,
	read:		NULL,
	write:		NULL,
	ioctl:		pcnet32_ioctl,		
	attach	:pcnet32_attach,
	detach	:pcnet32_detach,
	transmit	:pcnet32_transmit,
};



struct netdev_link link;


int dll_main()
{

	return 0;
}

int dll_version()
{
	kprintf("JICAMA PCNET32 Fast Ethernet driver!\n");
	return 0;
}

int dll_destroy()
{
	return 0;
}


int pci_pcnet32_attach(device_t self);
int pci_pcnet32_detach(device_t self);
int pci_pcnet32_probe(device_t self);

int pci_pcnet32_attach(device_t self)
{
	int retval;

	syslog(LOG_DEBUG,"pcnet32 start\n");	

	retval=kernel_driver_register(&pcnet32_drv_fops);
	
	 if(retval < 0)
    {
      printk("Could not register pcnet32 device\n");
      return 0;
    }
  	memset(&link,0,sizeof(link));
	strncpy(link.devname, pcnet32_drv_fops.d_name,16);
	ether_interface_register(&link);
	return 0;

}

int pci_pcnet32_detach(device_t self)
{
	kernel_driver_unregister(&pcnet32_drv_fops);
	ether_interface_unregister(&link);
}


static driver_t ne2000_driver =
{
	.name    = "ne2000",
	.methods = (device_method_t [])
	{
	  /* device interface */
	  DEVMETHOD(device_probe, pci_pcnet32_probe),
	  DEVMETHOD(device_attach, pci_pcnet32_attach),
	  DEVMETHOD(device_detach, pci_pcnet32_detach),

	  //DEVMETHOD(device_suspend, pci_pcnet32_suspend),
	  //DEVMETHOD(device_resume, pci_pcnet32_resume),
	  DEVMETHOD(device_shutdown, bus_generic_shutdown),

	  /* Bus interface */
	  //DEVMETHOD(bus_print_child, bus_generic_print_child),
	  {0, 0}
	},
	.size = 0,
};


static devclass_t ne2000_devclass;

DRIVER_MODULE(ne2000, pci, ne2000_driver, ne2000_devclass, 0, 0);





