/*
**(R)Jicama OS
** USB Devices Implement
** Copyright (C) 2003 DengPingPing
*/

#include <drv/drv.h>
#include <drv/sym.h>
#include <drv/pci.h>

/*uhci or ohci support ?
 * Universal Host Controller Interface driver for USB.
  *learn from: my.execpc.com/~geezer/os
            www.osdev.org
SPOON microkernel: www.djm.co.za/spoon   
*/

typedef struct 
{
	unsigned int start; 
    unsigned short vendor_id;
	unsigned short dev_id;
	unsigned char usb_name[60];
}usb_lookup_t;

static usb_lookup_t usb_lk [8] = {
	{0x7020, 0x8086, 0x00, "Intel 82371SB USB controller"},
	{0x7112, 0x8086, 0x00, "Intel PIIX4 USB controller"},
	{0x7000, 0x8086, 0x00, "Intel 82371SB ISA-PCI controller"},
	{0x7110, 0x8086, 0x00, "Intel PIIX4 ISA-PCI controller"},
	{0x0571, 0x1106, 0x00, "VIA AMD-645 USB controller"},
	{0xa0f8, 0x1045, 0x00, "Opti 82C750 (Vendetta) USB controller"},
	{0xc861, 0x1045, 0x00, "Opti 82C861/871 (Firelink/FireBlast) USB controller"},
	{0x00, 0x00, 0x00, ""},
};


unsigned long usbdev_init()
{
	int i;
	unsigned long dev_addr;
    struct pci_dev *usb_device = NULL;

    for (i=0; i<7; i++)
	 if ((usb_device = pci_lookup(usb_lk[i].start, usb_lk[i].vendor_id, usb_lk[i].dev_id)) != NULL )
	    break;

    if (usb_device){
       printk("USB Controller Found!\n");
	   return 0;
	}  

   usb_device = pci_dev_lookup(12, 3);

   if(usb_device)  pci_dump(usb_device);
	dev_addr = (unsigned long)usb_device;
	
    return dev_addr;
}
