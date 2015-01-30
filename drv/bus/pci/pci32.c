/*****************************************************************************
Chris Giese <geezer@execpc.com>, http://www.execpc.com/~geezer

This code was lifted from Linux bios32.c and pci.c
- Linux bios32.c - Copyright 1994 Drew Eckhardt
- Linux pci.c - Copyright 1993-1995 Drew Eckhardt, Frederic Potter,
	David Mosberger-Tang Bugs are due to Giese
*****************************************************************************/
#include "pci32.h"


#define	inpd(P)		inl(P)
#define	outpd(P,V)	outl(P,V)
extern struct pci_dev *pcidec_slot();

int (*pci_read_config_byte)(pci_state_t *state,
		unsigned reg, unsigned char *value);
int (*pci_read_config_word)(pci_state_t *state,
		unsigned reg, unsigned short *value);
int (*pci_read_config_dword)(pci_state_t *state,
		unsigned reg, unsigned long *value);

int (*pci_write_config_byte)(pci_state_t *state,
		unsigned reg, unsigned char value);
int (*pci_write_config_word)(pci_state_t *state,
		unsigned reg, unsigned short value);
int (*pci_write_config_dword)(pci_state_t *state,
		unsigned reg, unsigned long value);

int pci_dev_class(pci_state_t *state);
int pci_add_to_devtree(pci_state_t *s);

/*============================================================================
USING TYPE 1 CONFIG
============================================================================*/
#define	CONFIG1_ADR	0xCF8
#define	CONFIG1_DATA	0xCFC


static int type1_read_config_byte(pci_state_t *state, unsigned reg,
		unsigned char *value)
{
	unsigned long config_cmd;

	config_cmd = 0x80;		/* b31    = enable configuration space mapping */
	config_cmd <<= 8;
	config_cmd |= state->bus;	/* b23:16 = bus number */
	config_cmd <<= 8;
	config_cmd |= state->dev_fn;	/* b15:8  = dev (b15:11) fn (b10:8) */
	config_cmd <<= 8;
	config_cmd |= (reg & ~3);	/* b7:2   = register number */
	outpd(CONFIG1_ADR, config_cmd);
	*value = inb(CONFIG1_DATA + (reg & 3));
	return (OK);
}


static int type1_read_config_word(pci_state_t *state, unsigned reg,
		unsigned short *value)
{
	unsigned long config_cmd;

	config_cmd = 0x80;		/* b31    = enable configuration space mapping */
	config_cmd <<= 8;
	config_cmd |= state->bus;	/* b23:16 = bus number */
	config_cmd <<= 8;
	config_cmd |= state->dev_fn;	/* b15:8  = dev (b15:11) fn (b10:8) */
	config_cmd <<= 8;
	config_cmd |= (reg & ~3);	/* b7:2   = register number */
	outpd(CONFIG1_ADR, config_cmd);
	*value = inw(CONFIG1_DATA + (reg & 2));
	return (OK);
}


static int type1_read_config_dword (pci_state_t *state, unsigned reg,
		unsigned long *value)
{
	unsigned long config_cmd;

	config_cmd = 0x80;		/* b31    = enable configuration space mapping */
	config_cmd <<= 8;
	config_cmd |= state->bus;	/* b23:16 = bus number */
	config_cmd <<= 8;
	config_cmd |= state->dev_fn;	/* b15:8  = dev (b15:11) fn (b10:8) */
	config_cmd <<= 8;
	config_cmd |= (reg & ~3);	/* b7:2   = register number */
	outpd(CONFIG1_ADR, config_cmd);
	*value = inpd(CONFIG1_DATA);
	return (OK);
}


static int type1_write_config_byte(pci_state_t *state, unsigned reg,
		unsigned value)
{
	unsigned long config_cmd;

	config_cmd = 0x80;		/* b31    = enable configuration space mapping */
	config_cmd <<= 8;
	config_cmd |= state->bus;	/* b23:16 = bus number */
	config_cmd <<= 8;
	config_cmd |= state->dev_fn;	/* b15:8  = dev (b15:11) fn (b10:8) */
	config_cmd <<= 8;
	config_cmd |= (reg & ~3);	/* b7:2   = register number */
	outpd(CONFIG1_ADR, config_cmd);
	intel_outb(CONFIG1_DATA + (reg & 3), value);
	return (OK);
}


static int type1_write_config_word(pci_state_t *state, unsigned reg,
		unsigned value)
{
	unsigned long config_cmd;

	config_cmd = 0x80;		/* b31    = enable configuration space mapping */
	config_cmd <<= 8;
	config_cmd |= state->bus;	/* b23:16 = bus number */
	config_cmd <<= 8;
	config_cmd |= state->dev_fn;	/* b15:8  = dev (b15:11) fn (b10:8) */
	config_cmd <<= 8;
	config_cmd |= (reg & ~3);	/* b7:2   = register number */
	outpd(CONFIG1_ADR, config_cmd);
	intel_outw(CONFIG1_DATA + (reg & 2), value);
	return (OK);
}


static int type1_write_config_dword(pci_state_t *state, unsigned reg,
		unsigned long value)
{
	unsigned long config_cmd;

	config_cmd = 0x80;		/* b31    = enable configuration space mapping */
	config_cmd <<= 8;
	config_cmd |= state->bus;	/* b23:16 = bus number */
	config_cmd <<= 8;
	config_cmd |= state->dev_fn;	/* b15:8  = dev (b15:11) fn (b10:8) */
	config_cmd <<= 8;
	config_cmd |= (reg & ~3);	/* b7:2   = register number */
	outpd(CONFIG1_ADR, config_cmd);
	outpd(CONFIG1_DATA, value);
	return (OK);
}


static int type1_detect(void)
{
	unsigned long temp;

	intel_outb(CONFIG1_ADR + 3, 0x01); /* why? */
	temp = inpd(CONFIG1_ADR);
	outpd(CONFIG1_ADR, 0x80000000L);
	if(inpd(CONFIG1_ADR) == 0x80000000L)
	{
		outpd(CONFIG1_ADR, temp);
		pci_read_config_byte = type1_read_config_byte;
		pci_read_config_word = type1_read_config_word;
		pci_read_config_dword = type1_read_config_dword;

		pci_write_config_byte = type1_write_config_byte;
		pci_write_config_word = type1_write_config_word;
		pci_write_config_dword = type1_write_config_dword;
		return (OK);
	}
	return (-1);
}


/*============================================================================
USING TYPE 2 CONFIG
"this configuration mechanism is deprecated as of PCI version 2.1;
 only mechanism 1 should be used for new systems"
============================================================================*/


static int type2_read_config_byte(pci_state_t *state, unsigned reg,
		unsigned char *value)
{
	unsigned x;

	if((state->dev_fn & 0x80) != 0)
		return (-1);
	x = state->dev_fn & 7;		/* function */
	x <<= 1;
	x |= 0xF0;
	intel_outb(0xCF8, x);
	intel_outb(0xCFA, state->bus);
	x = state->dev_fn & 0xF8;	/* device */
	x <<= 5;
	x |= reg;
	x |= 0xC000;
	*value = inb(x);
	intel_outb(0xCF8, 0);
	return (OK);
}


static int type2_read_config_word(pci_state_t *state, unsigned reg,
		unsigned short *value)
{
	unsigned x;

	if((state->dev_fn & 0x80) != 0)
		return (-1);
	x = state->dev_fn & 7;		/* function */
	x <<= 1;
	x |= 0xF0;
	intel_outb(0xCF8, x);
	intel_outb(0xCFA, state->bus);
	x = state->dev_fn & 0xF8;	/* device */
	x <<= 5;
	x |= reg;
	x |= 0xC000;
	*value = inw(x);
	intel_outb(0xCF8, 0);
	return (OK);
}


static int type2_read_config_dword(pci_state_t *state, unsigned reg,
		unsigned long *value)
{
	unsigned x;

	if((state->dev_fn & 0x80) != 0)
		return (-1);
	x = state->dev_fn & 7;		/* function */
	x <<= 1;
	x |= 0xF0;
	intel_outb(0xCF8, x);
	intel_outb(0xCFA, state->bus);
	x = state->dev_fn & 0xF8;	/* device */
	x <<= 5;
	x |= reg;
	x |= 0xC000;
	*value = inpd(x);
	intel_outb(0xCF8, 0);
	return (OK);
}


static int type2_write_config_byte(pci_state_t *state, unsigned reg,
		unsigned value)
{
	unsigned x;

	if((state->dev_fn & 0x80) != 0)
		return (-1);
	x = state->dev_fn & 7;		/* function */
	x <<= 1;
	x |= 0xF0;
	intel_outb(0xCF8, x);
	intel_outb(0xCFA, state->bus);
	x = state->dev_fn & 0xF8;	/* device */
	x <<= 5;
	x |= reg;
	x |= 0xC000;
	intel_outb(x, value);
	intel_outb(0xCF8, 0);
	return (OK);
}


static int type2_write_config_word(pci_state_t *state, unsigned reg,
		unsigned value)
{
	unsigned x;

	if((state->dev_fn & 0x80) != 0)
		return (-1);
	x = state->dev_fn & 7;		/* function */
	x <<= 1;
	x |= 0xF0;
	intel_outb(0xCF8, x);
	intel_outb(0xCFA, state->bus);
	x = state->dev_fn & 0xF8;	/* device */
	x <<= 5;
	x |= reg;
	x |= 0xC000;
	intel_outw(x, value);
	intel_outb(0xCF8, 0);
	return (OK);
}


static int type2_write_config_dword(pci_state_t *state, unsigned reg,
		unsigned long value)
{
	unsigned x;

	if((state->dev_fn & 0x80) != 0)
		return (-1);

	x = state->dev_fn & 7;		/* function */
	x <<= 1;
	x |= 0xF0;
	intel_outb(0xCF8, x);
	intel_outb(0xCFA, state->bus);
	x = state->dev_fn & 0xF8;	/* device */
	x <<= 5;
	x |= reg;
	x |= 0xC000;
	outpd(x, value);
	intel_outb(0xCF8, 0);
	return (OK);
}


static int type2_detect(void)
{
	intel_outb(0xCFB, 0x00);
	intel_outb(0xCF8, 0x00);
	intel_outb(0xCFA, 0x00);

	if(inb(0xCF8) == 0x00 && inb(0xCFB) == 0x00)
	{
		pci_read_config_byte = type2_read_config_byte;
		pci_read_config_word = type2_read_config_word;
		pci_read_config_dword = type2_read_config_dword;

		pci_write_config_byte = type2_write_config_byte;
		pci_write_config_word = type2_write_config_word;
		pci_write_config_dword = type2_write_config_dword;
		return (OK);
	}
	return (-1);
}

#define PCI_VENDOR_ID	0x00		/* (2 byte) vendor id */
#define PCI_DEVICE_ID	0x02		/* (2 byte) device id */
#define PCI_COMMAND	0x04		/* (2 byte) command */
#define PCI_STATUS	0x06		/* (2 byte) status */
#define PCI_REVISION	0x08		/* (1 byte) revision id */
#define PCI_CLASS_API	0x09		/* (1 byte) specific register interface type */
#define PCI_CLASS_SUB	0x0a		/* (1 byte) specific device function */
#define PCI_CLASS_BASE	0x0b		/* (1 byte) device type (display vs network, etc) */
#define PCI_LINE_SIZE	0x0c		/* (1 byte) cache line size in 32 bit words */
#define PCI_LATENCY	0x0d		/* (1 byte) latency timer */
#define PCI_HEADER_TYPE	0x0e		/* (1 byte) header type */
#define PCI_BIST	0x0f		/* (1 byte) built-in self-test */


int do_dev_init(pci_state_t *state);
int pci_dev_class(pci_state_t *state)
{
	unsigned long vdev = 0;
	unsigned char c;

	//printk("\n");

	//printk("%s: state->dev_fn:%d", __FUNCTION__, state->dev_fn);

	if(pci_read_config_byte(state, PCI_CLASS_SUB, &c) != 0)
	return (-1);
	state->sub_class = c;

	if(pci_read_config_byte(state, PCI_CLASS_BASE, &c) != 0)
	return (-1);
	state->base_class = c;

	if(pci_read_config_byte(state, PCI_CLASS_API, &c) != 0)
	return (-1);

	state->interface = c;

	//if(state->base_class == 12 && state->sub_class == 3) /* USB */
	//printk("ÕÒµ½usb¿ØÖÆÆ÷!\n");
	//	if(state->base_class == 0x02 && state->sub_class == 0x00) /* net work */
	//printk("ÕÒµ½Íø¿¨!\n");

	//pci_showclass(state->base_class, state->sub_class);

	return (OK);
}


/*DETECT PCI DEVICES**********************************************/

static int pci_scan_vendor(pci_state_t *state)
{
	pci_state_t *s=0;
	unsigned long vdev = 0;
	unsigned char nRevisionID;
	u16_t vendor,  dev;

	while(1)
	{
		unsigned char rev;
		state->dev_fn++;
		if(state->dev_fn >= 256)
		{
			state->dev_fn = 0;
			state->bus++;
			if(state->bus >= 8)
				return 1;
		}

/* 0E= */

		if(pci_read_config_byte(state,
				0x08, &state->revision_id)){
		}

		if((state->dev_fn & 0x07) == 0)
		{
/* 0E=PCI_HEADER_TYPE */
			if(pci_read_config_byte(state,
				0x0E, &state->hdr_type) != 0)
					return (-1);
		}
/* not a multi-function device */
		else if((state->hdr_type & 0x80) == 0)
			continue;
/* 00=PCI_VENDOR_ID */
		if(pci_read_config_dword(state, 0x00, &vdev) != 0)
			return (-1);

#define PCIR_VENDOR	0x00
#define PCIR_DEVICE	0x02

		if(pci_read_config_word(state, PCIR_VENDOR, &vendor) != 0)
			return (-1);

		if(pci_read_config_word(state, PCIR_DEVICE, &dev) != 0)
			return (-1);

		//kprintf("PCIR_VENDOR = %04x, %04x\n",vendor,dev );


/* some broken boards return 0 if a slot is empty: */
		if(vdev == 0xFFFFFFFFL || vdev == 0)
			state->hdr_type = 0;
		else
			break;
	}
	//vendor = (u16_t)vdev & 0xFFFF;
	//vdev >>= 16;
	//dev = (u16_t)vdev&0x0000ffff;

	s = pcidec_slot();
	if(!s){
		kprintf("%s(): pcidec_slot no memory\n", __FUNCTION__);
		return -1;
	}

	s->dev_fn = state->dev_fn;
	s->bus = state->bus;
	s->vendorid = vendor;
	s->deviceid = dev;
	s->revision_id = state->revision_id;
	s->hdr_type = state->hdr_type;

	//	pci_showvendor( s->vendorid );
	pci_dev_class(s);
	do_dev_init(s);

	pci_add_to_devtree(s);
	//printk("dev_fn=%d,   Vendor:%X Device %X\n",		state->dev_fn, *vendor, *dev);
	return (OK);
}



int do_dev_init(pci_state_t *state)
{
	// Set the power state to unknown				//
	state->current_state = 4;

	// Identify the type of the device				//
	switch(state->hdr_type & 0x7F)
	{
		case PCI_HEADER_TYPE_NORMAL:
		// --- NORMAL DEVICE ---				//
		// Read the IRQ line					//
		pci_read_irq(state);
		// Read the base memory and I/O addresses		//
		pci_read_bases(state, 6, PCI_ROM_ADDRESS);
		// Read subsysem vendor and subsystem device id		//
		  pci_read_config_word(state, PCI_SUBSYSTEM_VENDOR_ID, &state->vendorid);
		 pci_read_config_word(state, PCI_SUBSYSTEM_ID, &state->deviceid);
		break;

		case PCI_HEADER_TYPE_BRIDGE:
		// --- PCI <-> PCI BRIDGE ---				//
		pci_read_bases(state, 2, PCI_ROM_ADDRESS_1);
		break;

		case PCI_HEADER_TYPE_CARDBUS:
		// --- PCI CARDBUS ---					//
		// Read the IRQ line					//
		pci_read_irq(state);
		// Read the base memory and I/O addresses		//
		pci_read_bases(state, 1, 0);
		// Read subsysem vendor and subsystem device id		//
		 pci_read_config_word(state, PCI_CB_SUBSYSTEM_VENDOR_ID, &state->vendorid);
		 pci_read_config_word(state, PCI_CB_SUBSYSTEM_ID, &state->deviceid);
		break;

		default:
		// --- UNKNOW HEADER TYPE ---				//
		break;
	}
			// printk("\nDo you want to enable this device (Y/N)? ");
			pci_enable_device(state);

	return (OK);
}





int detect_pci(void)
{
	if(type1_detect() == OK){
	//printk("Use PCI BIOS type1\n");
		return (OK);
	}

	if(type2_detect() == OK){
	    printk("Use PCI BIOS type2\n");
		return (OK);
	}
	
	printk("No PCI BIOS/devices detected\n");
	return (-1);
}

struct pci_dev *pci_dev_lookup(unsigned long basec, unsigned long subc);
void pci_dump(struct pci_dev *cfg);



#include <sys/queue.h>
#include <sys/compat_bsd.h>
#include <sys/endian.h>


static	struct device *pcidev;// = make_device(NULL, "pci", -1);;
extern pci_pos_t pcibus_ops;

void * pci_get_root_dev()
{
	return pcidev;
}

void pci_devtree_init()
{
	pcidev = device_add_child(device_get_top(), NULL, -1);
	device_set_devclass(pcidev,"pci");
	device_set_ivars(pcidev,NULL);
}

int pci_scan_dev(void)
{
	//unsigned vendor, dev, _class, sub;
	pci_state_t state;

/* detect PCI */
	if(detect_pci() != OK)
		return -1;


  /* list all devices */
	state.bus = -1u;
	state.dev_fn = 256;
	state.hdr_type = 0;

	while(pci_scan_vendor(&state) == OK){
		//pci_dump(&state);	 
	}
	return (OK);
}

int pci_add_to_devtree(pci_state_t *s)
{
	struct device *dev;
	pci_softc_t *sc ;

	//return;

	dev = device_add_child(pcidev, NULL, -1);

	if (!dev)
	{
		return -1;
	}
	

	sc = kmalloc( sizeof(*sc), 0);

	sc->bus = &pcibus_ops;
	sc->dev = s;

	device_set_ivars(dev,sc);
	return 0;
}

static int
pci_do_probe(device_t dev)
{

        device_set_desc(dev, "PCI bus");

        /* Allow other subclasses to override this driver. */
        return (-1000);
}

static int
pci_attach(device_t dev)
{
#if 0
        int busno;

        /*
         * Since there can be multiple independantly numbered PCI
         * busses on some large alpha systems, we can't use the unit
         * number to decide what bus we are probing. We ask the parent
         * pcib what our bus number is.
         */
        busno = pcib_get_bus(dev);
        if (bootverbose)
                device_printf(dev, "physical bus=%d\n", busno);

        pci_add_children(dev, busno, sizeof(struct pci_devinfo));
#endif
        return (bus_generic_attach(dev));
}

int
pci_suspend(device_t dev)
{
#if 0

        int dstate, error, i, numdevs;
        device_t acpi_dev, child, *devlist;
        struct pci_devinfo *dinfo;

        /*
         * Save the PCI configuration space for each child and set the
         * device in the appropriate power state for this sleep state.
         */
        acpi_dev = NULL;
        if (pci_do_powerstate)
                acpi_dev = devclass_get_device(devclass_find("acpi"), 0);
        device_get_children(dev, &devlist, &numdevs);
        for (i = 0; i < numdevs; i++) {
                child = devlist[i];
                dinfo = (struct pci_devinfo *) device_get_ivars(child);
                pci_cfg_save(child, dinfo, 0);
        }

        /* Suspend devices before potentially powering them down. */
        error = bus_generic_suspend(dev);
        if (error) {
                free(devlist, M_TEMP);
                return (error);
        }

        /*
         * Always set the device to D3.  If ACPI suggests a different
         * power state, use it instead.  If ACPI is not present, the
         * firmware is responsible for managing device power.  Skip
         * children who aren't attached since they are powered down
         * separately.  Only manage type 0 devices for now.
         */
        for (i = 0; acpi_dev && i < numdevs; i++) {
                child = devlist[i];
                dinfo = (struct pci_devinfo *) device_get_ivars(child);
                if (device_is_attached(child) && dinfo->cfg.hdrtype == 0) {
                        dstate = PCI_POWERSTATE_D3;
                        ACPI_PWR_FOR_SLEEP(acpi_dev, child, &dstate);
                        pci_set_powerstate(child, dstate);
                }
        }
        free(devlist, M_TEMP);
#endif
        return (0);
}

int
pci_resume(device_t dev)
{
#if 0
        int i, numdevs;
        device_t acpi_dev, child, *devlist;
        struct pci_devinfo *dinfo;

        /*
         * Set each child to D0 and restore its PCI configuration space.
         */
        acpi_dev = NULL;
        if (pci_do_powerstate)
                acpi_dev = devclass_get_device(devclass_find("acpi"), 0);
        device_get_children(dev, &devlist, &numdevs);
        for (i = 0; i < numdevs; i++) {
                /*
                 * Notify ACPI we're going to D0 but ignore the result.  If
                 * ACPI is not present, the firmware is responsible for
                 * managing device power.  Only manage type 0 devices for now.
                 */
                child = devlist[i];
                dinfo = (struct pci_devinfo *) device_get_ivars(child);
                if (acpi_dev && device_is_attached(child) &&
                    dinfo->cfg.hdrtype == 0) {
                        ACPI_PWR_FOR_SLEEP(acpi_dev, child, NULL);
                        pci_set_powerstate(child, PCI_POWERSTATE_D0);
                }

                /* Now the device is powered up, restore its config space. */
                pci_cfg_restore(child, dinfo);
        }
        free(devlist, M_TEMP);
#endif
        return (bus_generic_resume(dev));
}


static int pci_detach(device_t self)
{
	return 0;
}




static driver_t pci_driver =
{
	.name    = "pci",
	.methods = (device_method_t [])
	{
	  /* device interface */
	  DEVMETHOD(device_probe, pci_do_probe),
	  DEVMETHOD(device_attach, pci_attach),
	  DEVMETHOD(device_detach, pci_detach),

	  DEVMETHOD(device_suspend, pci_suspend),
	  DEVMETHOD(device_resume, pci_resume),
	  DEVMETHOD(device_shutdown, bus_generic_shutdown),

	  /* Bus interface */
	  //DEVMETHOD(bus_print_child, bus_generic_print_child),
	  {0, 0}
	},
	.size = 0,
};



static devclass_t pci_devclass;

DRIVER_MODULE(pci, pcib, pci_driver, pci_devclass, 0, 0);
//DRIVER_MODULE(pci, root, pci_driver, pci_devclass, 0, 0);

