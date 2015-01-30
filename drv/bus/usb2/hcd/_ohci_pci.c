/*
 * Copyright (c) 1998 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Lennart Augustsson (augustss@carlstedt.se) at
 * Carlstedt Research & Technology.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *        This product includes software developed by the NetBSD
 *        Foundation, Inc. and its contributors.
 * 4. Neither the name of The NetBSD Foundation nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * USB Open Host Controller driver.
 *
 * OHCI spec: http://www.intel.com/design/usb/ohci11d.pdf
 */

/* The low level controller code for OHCI has been split into
 * PCI probes and OHCI specific code. This was done to facilitate the
 * sharing of code between *BSD's
 */


#include "netbas.h"

#define INCLUDE_PCIXXX_H

#include <usb_port.h>
#include <usb.h>
#include <usb_subr.h>
#include <ohci.h> 
#include "freebsd_pcireg.h"

__FBSDID("$FreeBSD: src/sys/dev/usb2/ohci_pci.c,v 1.40 2004/08/02 15:37:35 iedowse Exp $");

#define PCI_OHCI_VENDORID_ACERLABS	0x10b9
#define PCI_OHCI_VENDORID_AMD		0x1022
#define PCI_OHCI_VENDORID_APPLE		0x106b
#define PCI_OHCI_VENDORID_CMDTECH	0x1095
#define PCI_OHCI_VENDORID_NEC		0x1033
#define PCI_OHCI_VENDORID_NVIDIA	0x12D2
#define PCI_OHCI_VENDORID_NVIDIA2	0x10DE
#define PCI_OHCI_VENDORID_OPTI		0x1045
#define PCI_OHCI_VENDORID_SIS		0x1039

#define PCI_OHCI_BASE_REG	0x10

static int ohci_pci_attach(device_t self);
static int ohci_pci_detach(device_t self);
static int ohci_pci_suspend(device_t self);
static int ohci_pci_resume(device_t self);

static int
ohci_pci_suspend(device_t self)
{
	ohci_softc_t *sc = device_get_softc(self);
	int err;

	err = bus_generic_suspend(self);
	if(err)
	{
		return err;
	}
	ohci_suspend(sc);
	return 0;
}

static int
ohci_pci_resume(device_t self)
{
	ohci_softc_t *sc = device_get_softc(self);
	u_int32_t reg, int_line;

	if(pci_get_powerstate(self) != PCI_POWERSTATE_D0)
	{
                device_printf(self, "chip is in D%d mode "
                        "-- setting to D0\n", pci_get_powerstate(self));
                reg = pci_read_config(self, PCI_CBMEM, 4);
                int_line = pci_read_config(self, PCIR_INTLINE, 4);
                pci_set_powerstate(self, PCI_POWERSTATE_D0);
                pci_write_config(self, PCI_CBMEM, reg, 4);
                pci_write_config(self, PCIR_INTLINE, int_line, 4);
	}

	ohci_resume(sc);

	bus_generic_resume(self);
	return 0;
}

static const char *
ohci_pci_match(device_t self)
{
	u_int32_t device_id = pci_get_devid(self);

	if(device_id == 0x523710b9)
	  { return ("AcerLabs M5237 (Aladdin-V) USB controller"); }
        if(device_id == 0x740c1022)
	  { return ("AMD-756 USB Controller"); }
        if(device_id == 0x74141022)
	  { return ("AMD-766 USB Controller"); }
        if(device_id == 0x06701095)
	  { return ("CMD Tech 670 (USB0670) USB controller"); }
        if(device_id == 0x06731095)
	  { return ("CMD Tech 673 (USB0673) USB controller"); }
        if(device_id == 0xc8611045)
	  { return ("OPTi 82C861 (FireLink) USB controller"); }
        if(device_id == 0x00351033)
	  { return ("NEC uPD 9210 USB controller"); }
        if(device_id == 0x00d710de)
	  { return ("nVidia nForce3 USB Controller"); }
        if(device_id == 0x70011039)
	  { return ("SiS 5571 USB controller"); }
        if(device_id == 0x0019106b)
	  { return ("Apple KeyLargo USB controller"); }

	if ((pci_get_class(self) == PCIC_SERIALBUS) &&
	    (pci_get_subclass(self) == PCIS_SERIALBUS_USB) &&
	    (pci_get_progif(self) == PCI_INTERFACE_OHCI))
	{
		return ("OHCI (generic) USB controller");
	}
	return NULL;
}

static int
ohci_pci_probe(device_t self)
{
	const char *desc = ohci_pci_match(self);

	if (desc)
	{
		device_set_desc(self, desc);
		return 0;
	}
	else
	{
		return ENXIO;
	}
}

static int
ohci_pci_attach(device_t self)
{
	ohci_softc_t *sc;
	int rid;
	int err;

	sc = usb_alloc_mem(device_get_dma_tag(self),
			   sizeof(*sc), LOG2(OHCI_HCCA_ALIGN));

	if(sc == NULL)
	{
		device_printf(self, "Could not allocate sc\n");
		return ENXIO;
	}

#if 1
	bzero(sc, sizeof(*sc));
#endif

	minit(&sc->sc_bus.bus_wait_q);
	minit(&sc->sc_bus.wait_explore_q);

	sc->sc_physaddr = usb_vtophys(sc, sizeof(*sc)); /* physical address of sc */

	mtx_init(&sc->sc_bus.mtx, "usb lock",
		 NULL, MTX_DEF|MTX_RECURSE);

	device_set_softc(self, sc);
	sc->sc_dev = self;

	pci_enable_busmaster(self);

	rid = PCI_CBMEM;
	/*sc->io_res = bus_alloc_resource_any(self, SYS_RES_MEMORY, &rid,
					    RF_ACTIVE);
	if(!sc->io_res)
	{
		device_printf(self, "Could not map memory\n");
		goto error;
	}
	sc->iot = rman_get_bustag(sc->io_res);
	sc->ioh = rman_get_bushandle(sc->io_res);

	rid = 0;
	sc->irq_res = bus_alloc_resource_any(self, SYS_RES_IRQ, &rid,
					     RF_SHAREABLE | RF_ACTIVE);
	if(sc->irq_res == NULL)
	{
		device_printf(self, "Could not allocate irq\n");
		goto error;
	}*/
	sc->iot = rman_get_bustag(self);
	sc->ioh = pci_get_base_addr(self,0);

	printf("iomem =  %x\n",sc->ioh);

	sc->sc_bus.bdev = device_add_child(self, "usb", -1);
	if(!sc->sc_bus.bdev)
	{
		device_printf(self, "Could not add USB device\n");
		goto error;
	}

	device_set_ivars(sc->sc_bus.bdev, &sc->sc_bus);
	device_set_softc(sc->sc_bus.bdev, &sc->sc_bus);

	/* ohci_pci_match will never return NULL if ohci_pci_probe succeeded */
	device_set_desc(sc->sc_bus.bdev, ohci_pci_match(self));
	switch (pci_get_vendor(self)) {
	case PCI_OHCI_VENDORID_ACERLABS:
		sprintf(sc->sc_vendor, "AcerLabs");
		break;
	case PCI_OHCI_VENDORID_AMD:
		sprintf(sc->sc_vendor, "AMD");
		break;
	case PCI_OHCI_VENDORID_APPLE:
		sprintf(sc->sc_vendor, "Apple");
		break;
	case PCI_OHCI_VENDORID_CMDTECH:
		sprintf(sc->sc_vendor, "CMDTECH");
		break;
	case PCI_OHCI_VENDORID_NEC:
		sprintf(sc->sc_vendor, "NEC");
		break;
	case PCI_OHCI_VENDORID_NVIDIA:
	case PCI_OHCI_VENDORID_NVIDIA2:
		sprintf(sc->sc_vendor, "nVidia");
		break;
	case PCI_OHCI_VENDORID_OPTI:
		sprintf(sc->sc_vendor, "OPTi");
		break;
	case PCI_OHCI_VENDORID_SIS:
		sprintf(sc->sc_vendor, "SiS");
		break;
	default:
		if(bootverbose)
		{
			device_printf(self, "(New OHCI DeviceId=0x%08x)\n",
				      pci_get_devid(self));
		}
		sprintf(sc->sc_vendor, "(0x%04x)", pci_get_vendor(self));
	}

	/* sc->sc_bus.usbrev; set by ohci_init() */

	/*err = bus_setup_intr(self, sc->irq_res, INTR_TYPE_BIO|INTR_MPSAFE,
			     (void *)(void *)ohci_interrupt, sc, &sc->ih);
	if(err)
	{
		device_printf(self, "Could not setup irq, %d\n", err);
		sc->ih = NULL;
		goto error;
	}*/
	int irq = pci_get_irq(self);

	printf("ohci on irq %d\n", irq);

	put_irq_handler(irq,(void *)ohci_interrupt, sc,"ohci");


	err = ohci_init(sc);
	if(!err)
	{
		err = device_probe_and_attach(sc->sc_bus.bdev);
	}

	if(err)
	{
		device_printf(self, "USB init failed\n");
		goto error;
	}
	bus_generic_attach(sc->sc_bus.bdev);
	return 0;

 error:
	ohci_pci_detach(self);
	return ENXIO;
}

static int
ohci_pci_detach(device_t self)
{
	ohci_softc_t *sc = device_get_softc(self);

	if(sc->sc_bus.bdev)
	{
		device_delete_child(self, sc->sc_bus.bdev);
		sc->sc_bus.bdev = NULL;
	}

	pci_disable_busmaster(self);

	if(sc->irq_res && sc->ih)
	{
		/* only call ohci_detach()
		 * after ohci_init()
		 */
		ohci_detach(sc);

		int err = bus_teardown_intr(self, sc->irq_res, sc->ih);

		if(err)
		{
			/* XXX or should we panic? */
			device_printf(self, "Could not tear down irq, %d\n",
				      err);
		}
		sc->ih = NULL;
	}
	if(sc->irq_res)
	{
		//bus_release_resource(self, SYS_RES_IRQ, 0, sc->irq_res);
		sc->irq_res = NULL;
	}
	if(sc->io_res)
	{
		//bus_release_resource(self, SYS_RES_MEMORY, PCI_CBMEM,
		//		     sc->io_res);
		sc->io_res = NULL;
	}

	mtx_destroy(&sc->sc_bus.mtx);

	usb_free_mem(sc, sizeof(*sc));

	device_set_softc(self, NULL);
	return 0;
}

static driver_t ohci_driver =
{
	.name    = "ohci",
	.methods = (device_method_t [])
	{
	  /* device interface */
	  DEVMETHOD(device_probe, ohci_pci_probe),
	  DEVMETHOD(device_attach, ohci_pci_attach),
	  DEVMETHOD(device_detach, ohci_pci_detach),
	  DEVMETHOD(device_suspend, ohci_pci_suspend),
	  DEVMETHOD(device_resume, ohci_pci_resume),
	  DEVMETHOD(device_shutdown, bus_generic_shutdown),

	  /* bus interface */
	  DEVMETHOD(bus_print_child, bus_generic_print_child),

	  {0, 0}
	},
	.size = 0,
};

static devclass_t ohci_devclass;

DRIVER_MODULE(ohci, pci, ohci_driver, ohci_devclass, 0, 0);
DRIVER_MODULE(ohci, cardbus, ohci_driver, ohci_devclass, 0, 0);
#ifdef USB_MODULE

int dll_main()
{
	return 0;
}

int dll_destroy()
{
	return 0;
}

#endif

