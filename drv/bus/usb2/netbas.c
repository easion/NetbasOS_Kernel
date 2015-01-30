#include "netbas.h"

#if 0

int register_bsd_driver(struct driver *driver);
int unregister_bsd_driver(struct driver *driver);
struct driver* find_bsd_driver(const char* name);
devop_t get_bsd_driver_ops(struct driver* driver,const char* ops);

static LIST_HEAD(, driver)	  usb_linux_driver_list;

static init()
{
LIST_INIT(&usb_linux_driver_list);
}

int register_bsd_driver(struct driver *driver)
{
	LIST_INSERT_HEAD(&usb_linux_driver_list, driver, link);
}


int unregister_bsd_driver(struct driver *driver)
{
	LIST_REMOVE( driver, link);
}

static int probe(struct device *dev, char *hcd)
{
	int err;
	struct driver*driver;
	devop_t ops;

	driver = find_bsd_driver(hcd);
	if (driver==NULL)
	{
		return -1;
	}
	
	err = dev_probe(driver,dev);
	return err;
}


struct driver* find_bsd_driver(const char* name)
{
	struct driver *driver;

	if (!LIST_EMPTY(&usb_linux_driver_list))
	{
	}

	LIST_FOREACH(driver, &usb_linux_driver_list, link)
	{
		if (strcmp(driver->name,name)==0)
		{
			return driver;
		}
		printf("  driver=%p\n", driver);
	}
	return NULL;
}

int device_probe_and_attach_1(struct device *dev)
{
	int err;
	int i;
	struct driver *driver;


	LIST_FOREACH(driver, &usb_linux_driver_list, link)
	{
		if (dev_probe(driver,dev)==0)
		{
			return 0;
		}
		printf("  driver=%p\n", driver);
	}
	return -1;	
}

#endif

static pci_pos_t* pci_bus;

static pci_state_t * usb_dev_probe()
{
	int i;
    pci_state_t *usb_device = NULL; 
   usb_device = pci_bus->pci_dev_lookup(PCI_SERIAL_BUS, PCI_USB);
   if(usb_device)  pci_bus->pci_dump(usb_device);	
    return usb_device;
}

extern struct mtx Giant;


int dll_main( )
{
	int i;
	pci_state_t* pci;
	bool found = false;
	struct bus_man *bus;

	usb_init();
	usb_loadsym();
	
	/* Get busmanagers */
	bus =  busman_get( "PCI32" );

	if (!bus)
	{
		printk("PCI32 BUS Not AVAILABLE\n");
		return -1;
	}
	pci_bus = bus->bus_hooks(0);
	if( pci_bus == NULL )
	{
		return( -1 );
	}
	
#if 1
	struct device *pcidev;// = make_device(NULL, "pci", -1);;


	//device_attach(dev);
	printk("device_add_child_usb start:\n");


	//pcidev = pci_get_root_dev();
	pcidev=pci_bus->pci_root_dev();


	bus_generic_attach(pcidev);
	
	printk("device_add_child done:%p\n",pcidev);
	//usb_init(NULL);
	usb_post_init(NULL);
#endif
	return 0;
}

int dll_destroy()
{
	return 0;
}


uint8_t
usb2_transfer_pending(struct usb2_xfer *xfer)
{
}

usbd_set_frame_data(){
}

usbd_copy_out(){
}

usbd_clear_data_toggle(){
}

usbd_set_alt_interface_index()
{
}
usbd_needs_explore_all()
{
}
device_set_usb2_desc()
{
}

usbd_transfer_pending(){
}

usbd_copy_in(){
}

usbd_cv_init(){
}

usbd_cv_destroy(){
}

usbd_cv_signal()
{
}

usbd_cv_wait(){
}

