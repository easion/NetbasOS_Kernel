#include <drv/defs.h>
#include <drv/drv.h>
#include <drv/sym.h>
#include <drv/pci.h>
#include <drv/ia.h>
#include <drv/errno.h>
#include <drv/user.h>
#include <drv/spin.h>
#include <drv/proc_entry.h>
//#include <net/if.h>
#include <drv/pbuf.h>
#include <net/sockios.h>
#include "etherdrv.h"
#include "ne2000.h"
#include <sys/queue.h>
#include <sys/compat_bsd.h>
#include <sys/endian.h>

static	ne2000_t ne_2000;
u8_t *get_mac_addr();
ne2000_t* get_ne2000_device();

struct netdev_link link;

int pci_ne2000_attach(device_t self);
int pci_ne2000_detach(device_t self);
int pci_ne2000_suspend(device_t self);
int pci_ne2000_resume(device_t self);
int pci_ne2000_probe(device_t self);



static int ne2k_drv_open(const char *f, int mode,void *ptr);
int ne2000_drv_attach(void* dev, struct eth_addr *hwaddr);
int ne2000_drv_detach(void* dev);
int ne2000_drv_transmit(void* dev, struct pbuf *p);



ne2000_t* get_ne2000_device()
{
	return &ne_2000;
}

u8_t *get_mac_addr()
{
	ne2000_t* nic=&ne_2000;

#if 0
	printk("NE2000: MAC (station address) is %X-%X-%X-%X-%X-%X\n",
	nic->station_address[0], nic->station_address[1], nic->station_address[2], 
	nic->station_address[3], nic->station_address[4], nic->station_address[5]);
#endif
		return(nic->station_address);
}

int
netif_proc(char *buf, int len,void *pf)
{
  ne2000_t *netif= get_ne2000_device();
  int cnt=0,c;  

  pprintf(pf, "ne2000 irq %d, iobase=0x%x, word_mode=%d \n"
  "\tMac:%02x.%02x.%02x.%02x.%02x. tx %d bytes rx %d bytes T&R:%d-%d\n", 
	  netif->irq,
	  netif->iobase,
	  netif->station_address[0],
	  netif->station_address[1],
	  netif->station_address[2],
	  netif->station_address[3],
	  netif->station_address[4],
	  netif->station_address[5],
	  netif->txsize,
	  netif->rxsize,

	  netif->tx_packets,
	  netif->rx_packets
	  );
  return 0;
}


struct proc_entry ne2000stat = {
	name: "ne2000",
	write_func: netif_proc,
	read_func: NULL,
};






static int ne2k_drv_ioctl(int  file, int cmd, void* arg, int len,int from_kernel)
{
	int ret = 0;
	//void __user * argp = (void*)cur_addr_insystem((size_t)arg);
     ne2000_t *nic= get_ne2000_device();


  switch (cmd)
    {
      
    case SIOC_ETH_START:
		break;
      
    case SIOC_ETH_STOP:
      break;
	case SIOCGIFHWADDR:
	   if (!from_kernel)
	   {
    		memcpy_to_user(arg, get_mac_addr(),6);
	   }
		else{
		//ÄÚºË¿Õ¼ä¸´ÖÆmac
			memcpy(arg, get_mac_addr(),6);
		}
		//panic("ioctl(): get mac addr\n");
       return 0;    	

    default:
      return -1;	
    }
	return ret;
}



static int ne2k_drv_close(void*  file)
{
     ne2000_t *nic= get_ne2000_device();
	return 0;
}


int ne2000_drv_attach(void* dev, struct eth_addr *hwaddr)
{
  ne2000_t *netif= get_ne2000_device();
  memcpy(hwaddr->addr,netif->station_address,6);
  //*hwaddr = pcnet32->hwaddr;

  return 0;
}

int ne2000_drv_detach(void* dev)
{
  return 0;
}


static const driver_ops_t ne2k_drv_fops =
{
	d_name:		"eth",
	d_author:	"Esaion",
	d_version:	"v2006-01-25",
	d_kver:	CURRENT_KERNEL_VERSION,
	d_index:	ALLOC_MAJOR_NUMBER,
	open:		ne2k_drv_open,
	close:		ne2k_drv_close,
	//read:		ne2k_drv_read,
	//write:		ne2k_drv_write,
	ioctl:		ne2k_drv_ioctl,		
	attach	:ne2000_drv_attach,
	detach	:ne2000_drv_detach,
	transmit	:ne2000_drv_transmit
};

static int ne2k_drv_open(const char *f, int mode, void* ptr)
{
	int ret ;

	ret = ne2000_install();


	if (ret!=OK){
	kprintf("ne2k not found\n");
	return -1;
	}

	ne_2000.fd = ptr;
	return 0;
}

void load_ne2000()
{
	int pcin;
	int retval;

	/*if (ne2000_probe())
	{
		kprintf("no ne2000 netcard\n");
		return -1;
	}*/

	retval = kernel_driver_register(&ne2k_drv_fops);
		
	 if(retval < 0)
    {
      printk("Could not register ne2000 device\n");
      return 0;
    }
  
	register_proc_entry(&ne2000stat);
  	memset(&link,0,sizeof(link));
	strncpy(link.devname, ne2k_drv_fops.d_name,16);
	ether_interface_register(&link);


}


int
pci_ne2000_detach(device_t self)
{
	pci_softc_t *sc = device_get_softc(self);

	ether_interface_unregister(&link);
	kernel_driver_unregister(&ne2k_drv_fops);

	return 0;
}

int dll_main()
{

	return 0;
}

int dll_version()
{
	kprintf("JICAMA NE2000 NetCard Driver Version 0.01!\n");
	return 0;
}

int dll_destroy()
{
	ne2000_t *nic = get_ne2000_device();
	if (nic && nic->irq>0)
	{
		free_irq_handler(nic->irq,nic->irq_hld);
	}
	return 0;
}



static driver_t ne2000_driver =
{
	.name    = "ne2000",
	.methods = (device_method_t [])
	{
	  /* device interface */
	  DEVMETHOD(device_probe, pci_ne2000_probe),
	  DEVMETHOD(device_attach, pci_ne2000_attach),
	  DEVMETHOD(device_detach, pci_ne2000_detach),

	  //DEVMETHOD(device_suspend, pci_ne2000_suspend),
	  //DEVMETHOD(device_resume, pci_ne2000_resume),
	  DEVMETHOD(device_shutdown, bus_generic_shutdown),

	  /* Bus interface */
	  //DEVMETHOD(bus_print_child, bus_generic_print_child),
	  {0, 0}
	},
	.size = 0,
};


static devclass_t ne2000_devclass;

DRIVER_MODULE(ne2000, pci, ne2000_driver, ne2000_devclass, 0, 0);



