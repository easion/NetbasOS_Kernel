/*-
 * Copyright (c) 1998 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Lennart Augustsson (lennart@augustsson.net) at
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
 * OHCI spec: http://www.compaq.com/productinfo/development/openhci.html
 * USB spec: http://www.usb.org/developers/docs/usbspec.zip
 */

#include "netbas.h"

#define INCLUDE_PCIXXX_H

#include <usb_port.h>
#include <usb.h>
#include <usb_subr.h>
#include <ohci.h>

__FBSDID("$FreeBSD: src/sys/dev/usb2/ohci.c,v 1.145 2004/11/09 20:51:32 iedowse Exp $");

#define OHCI_BUS2SC(bus) ((ohci_softc_t *)(((u_int8_t *)(bus)) - \
   POINTER_TO_UNSIGNED(&(((ohci_softc_t *)0)->sc_bus))))

#ifdef USB_DEBUG
#undef DPRINTF
#undef DPRINTFN
#define DPRINTF(x)	{ if (ohcidebug) { printf("%s: ", __FUNCTION__); printf x ; } }
#define DPRINTFN(n,x)	{ if (ohcidebug > (n)) { printf("%s: ", __FUNCTION__); printf x ; } }
int ohcidebug = 16;
SYSCTL_NODE(_hw_usb, OID_AUTO, ohci, CTLFLAG_RW, 0, "USB ohci");
SYSCTL_INT(_hw_usb_ohci, OID_AUTO, debug, CTLFLAG_RW,
	   &ohcidebug, 0, "ohci debug level");
static void		ohci_dumpregs(ohci_softc_t *);
static void		ohci_dump_tds(ohci_td_t *);
static void		ohci_dump_td(ohci_td_t *);
static void		ohci_dump_ed(ohci_ed_t *);
static void		ohci_dump_itd(ohci_itd_t *);
static void		ohci_dump_itds(ohci_itd_t *);
#endif


#define OBARR(sc) bus_space_barrier((sc)->iot, (sc)->ioh, 0, (sc)->sc_size, \
			BUS_SPACE_BARRIER_READ|BUS_SPACE_BARRIER_WRITE)
#define OWRITE1(sc, r, x) \
 do { OBARR(sc); bus_space_write_1((sc)->iot, (sc)->ioh, (r), (x)); } while (0)
#define OWRITE2(sc, r, x) \
 do { OBARR(sc); bus_space_write_2((sc)->iot, (sc)->ioh, (r), (x)); } while (0)
#define OWRITE4(sc, r, x) \
 do { OBARR(sc); bus_space_write_4((sc)->iot, (sc)->ioh, (r), (x)); } while (0)
#define OREAD1(sc, r) (OBARR(sc), bus_space_read_1((sc)->iot, (sc)->ioh, (r)))
#define OREAD2(sc, r) (OBARR(sc), bus_space_read_2((sc)->iot, (sc)->ioh, (r)))
#define OREAD4(sc, r) (OBARR(sc), bus_space_read_4((sc)->iot, (sc)->ioh, (r)))

#define OHCI_INTR_ENDPT 1

extern struct usbd_bus_methods ohci_bus_methods;
extern struct usbd_pipe_methods ohci_device_bulk_methods;
extern struct usbd_pipe_methods ohci_device_ctrl_methods;
extern struct usbd_pipe_methods ohci_device_intr_methods;
extern struct usbd_pipe_methods ohci_device_isoc_methods;
extern struct usbd_pipe_methods ohci_root_ctrl_methods;
extern struct usbd_pipe_methods ohci_root_intr_methods;

#define PHYSADDR(sc,what) \
  ((sc)->sc_physaddr + POINTER_TO_UNSIGNED(&(((struct ohci_softc *)0)->what)))

static usbd_status
ohci_controller_init(ohci_softc_t *sc)
{
	int i;
	u_int32_t s, ctl, ival, hcr, fm, per, desca;

	/* Determine in what context we are running. */
	ctl = OREAD4(sc, OHCI_CONTROL);
	if(ctl & OHCI_IR)
	{
		/* SMM active, request change */
		DPRINTF(("SMM active, request owner change\n"));
		s = OREAD4(sc, OHCI_COMMAND_STATUS);
		OWRITE4(sc, OHCI_COMMAND_STATUS, s | OHCI_OCR);
		for(i = 0; (i < 100) && (ctl & OHCI_IR); i++)
		{
			DELAY(1000*1);
			ctl = OREAD4(sc, OHCI_CONTROL);
		}
		if((ctl & OHCI_IR) == 0)
		{
			device_printf(sc->sc_bus.bdev, "SMM does not respond, resetting\n");
			OWRITE4(sc, OHCI_CONTROL, OHCI_HCFS_RESET);
			goto reset;
		}
#if 0
/* Don't bother trying to reuse the BIOS init, we'll reset it anyway. */
	} else if((ctl & OHCI_HCFS_MASK) != OHCI_HCFS_RESET) {
		/* BIOS started controller. */
		DPRINTF(("BIOS active\n"));
		if((ctl & OHCI_HCFS_MASK) != OHCI_HCFS_OPERATIONAL) {
			OWRITE4(sc, OHCI_CONTROL, OHCI_HCFS_OPERATIONAL);
			DELAY(1000*USB_RESUME_DELAY);
		}
#endif
	} else {
		DPRINTF(("cold started\n"));
	reset:
		/* controller was cold started */
		DELAY(1000*USB_BUS_RESET_DELAY);
	}

	/*
	 * This reset should not be necessary according to the OHCI spec, but
	 * without it some controllers do not start.
	 */
	DPRINTF(("%s: resetting\n", device_get_nameunit(sc->sc_bus.bdev)));
	OWRITE4(sc, OHCI_CONTROL, OHCI_HCFS_RESET);
	DELAY(1000*USB_BUS_RESET_DELAY);

	/* we now own the host controller and the bus has been reset */
	ival = OHCI_GET_IVAL(OREAD4(sc, OHCI_FM_INTERVAL));

	OWRITE4(sc, OHCI_COMMAND_STATUS, OHCI_HCR); /* Reset HC */
	/* nominal time for a reset is 10 us */
	for(i = 0; i < 10; i++)
	{
		DELAY(10);
		hcr = OREAD4(sc, OHCI_COMMAND_STATUS) & OHCI_HCR;
		if(!hcr)
		{
			break;
		}
	}
	if(hcr)
	{
		device_printf(sc->sc_bus.bdev, "reset timeout\n");
		return (USBD_IOERROR);
	}
#ifdef USB_DEBUG
	if(ohcidebug > 15)
	{
		ohci_dumpregs(sc);
	}
#endif

	/* The controller is now in SUSPEND state, we have 2ms to finish. */

	/* set up HC registers */
	OWRITE4(sc, OHCI_HCCA, PHYSADDR(sc, sc_hw.hcca));
	OWRITE4(sc, OHCI_CONTROL_HEAD_ED, PHYSADDR(sc, sc_hw.ctrl_start));
	OWRITE4(sc, OHCI_BULK_HEAD_ED, PHYSADDR(sc, sc_hw.bulk_start));
	/* disable all interrupts and then switch on all desired interrupts */
	OWRITE4(sc, OHCI_INTERRUPT_DISABLE, OHCI_ALL_INTRS);
	OWRITE4(sc, OHCI_INTERRUPT_ENABLE, sc->sc_eintrs | OHCI_MIE);
	/* switch on desired functional features */
	ctl = OREAD4(sc, OHCI_CONTROL);
	ctl &= ~(OHCI_CBSR_MASK | OHCI_LES | OHCI_HCFS_MASK | OHCI_IR);
	ctl |= OHCI_PLE | OHCI_IE | OHCI_CLE | OHCI_BLE |
		OHCI_RATIO_1_4 | OHCI_HCFS_OPERATIONAL;
	/* And finally start it! */
	OWRITE4(sc, OHCI_CONTROL, ctl);

	/*
	 * The controller is now OPERATIONAL.  Set a some final
	 * registers that should be set earlier, but that the
	 * controller ignores when in the SUSPEND state.
	 */
	fm = (OREAD4(sc, OHCI_FM_INTERVAL) & OHCI_FIT) ^ OHCI_FIT;
	fm |= OHCI_FSMPS(ival) | ival;
	OWRITE4(sc, OHCI_FM_INTERVAL, fm);
	per = OHCI_PERIODIC(ival); /* 90% periodic */
	OWRITE4(sc, OHCI_PERIODIC_START, per);

	/* Fiddle the No OverCurrent Protection bit to avoid chip bug. */
	desca = OREAD4(sc, OHCI_RH_DESCRIPTOR_A);
	OWRITE4(sc, OHCI_RH_DESCRIPTOR_A, desca | OHCI_NOCP);
	OWRITE4(sc, OHCI_RH_STATUS, OHCI_LPSC); /* Enable port power */
	DELAY(1000*OHCI_ENABLE_POWER_DELAY);
	OWRITE4(sc, OHCI_RH_DESCRIPTOR_A, desca);

	/*
	 * The AMD756 requires a delay before re-reading the register,
	 * otherwise it will occasionally report 0 ports.
	 */
  	sc->sc_noport = 0;
 	for(i = 0; (i < 10) && (sc->sc_noport == 0); i++)
	{
 		DELAY(1000*OHCI_READ_DESC_DELAY);
 		sc->sc_noport = OHCI_GET_NDP(OREAD4(sc, OHCI_RH_DESCRIPTOR_A));
 	}

#ifdef USB_DEBUG
	if(ohcidebug > 5)
	{
		ohci_dumpregs(sc);
	}
#endif
	return (USBD_NORMAL_COMPLETION);
}

usbd_status
ohci_init(ohci_softc_t *sc)
{
	u_int i;
	u_int32_t rev;
	u_int16_t bit;
	u_int16_t x;
	u_int16_t y;

	mtx_lock(&sc->sc_bus.mtx);

	DPRINTF(("start\n"));

	sc->sc_eintrs = OHCI_NORMAL_INTRS;

	/*
	 * setup self pointers
	 */
	sc->sc_hw.ctrl_start.ed_self = htole32(PHYSADDR(sc,sc_hw.ctrl_start));
	sc->sc_hw.ctrl_start.ed_flags = htole32(OHCI_ED_SKIP);
	sc->sc_ctrl_p_last = &sc->sc_hw.ctrl_start;

	sc->sc_hw.bulk_start.ed_self = htole32(PHYSADDR(sc,sc_hw.bulk_start));
	sc->sc_hw.bulk_start.ed_flags = htole32(OHCI_ED_SKIP);
	sc->sc_bulk_p_last = &sc->sc_hw.bulk_start;

	sc->sc_hw.isoc_start.ed_self = htole32(PHYSADDR(sc,sc_hw.isoc_start));
	sc->sc_hw.isoc_start.ed_flags = htole32(OHCI_ED_SKIP);
	sc->sc_isoc_p_last = &sc->sc_hw.isoc_start;

	for(i = 0;
	    i < OHCI_NO_EDS;
	    i++)
	{
		sc->sc_hw.intr_start[i].ed_self = htole32(PHYSADDR(sc,sc_hw.intr_start[i]));
		sc->sc_hw.intr_start[i].ed_flags = htole32(OHCI_ED_SKIP);
		sc->sc_intr_p_last[i] = &sc->sc_hw.intr_start[i];
	}

	/*
	 * the QHs are arranged to give poll intervals that are
	 * powers of 2 times 1ms
	 */
	bit = OHCI_NO_EDS/2;
	while(bit)
	{
		x = bit;
		while(x & bit)
		{
			y = (x ^ bit)|(bit/2);
			/* the next QH has half the
			 * poll interval
			 */
			sc->sc_hw.intr_start[x].next = NULL;
			sc->sc_hw.intr_start[x].ed_next =
			  sc->sc_hw.intr_start[y].ed_self;
			x++;
		}
		bit >>= 1;
	}

	/* the last (1ms) QH */
	sc->sc_hw.intr_start[0].next = &sc->sc_hw.isoc_start;
	sc->sc_hw.intr_start[0].ed_next = sc->sc_hw.isoc_start.ed_self;

	/*
	 * Fill HCCA interrupt table.  The bit reversal is to get
	 * the tree set up properly to spread the interrupts.
	 */
	for(i = 0;
	    i < OHCI_NO_INTRS;
	    i++)
	{
		sc->sc_hw.hcca.hcca_interrupt_table[i] =
		  sc->sc_hw.intr_start[i|(OHCI_NO_EDS/2)].ed_self;
	}

	LIST_INIT(&sc->sc_interrupt_list_head);

	/* set up the bus struct */
	sc->sc_bus.methods = &ohci_bus_methods;

	__callout_init_mtx(&sc->sc_tmo_rhsc,  &sc->sc_bus.mtx, 
			   CALLOUT_RETURNUNLOCKED);

#ifdef USB_DEBUG
	if(ohcidebug > 15)
	{
		for(i = 0; 
		    i < OHCI_NO_EDS; 
		    i++)
		{
			printf("ed#%d ", i);
			ohci_dump_ed(&sc->sc_hw.intr_start[i]);
		}
		printf("iso ");
		ohci_dump_ed(&sc->sc_hw.isoc_start);
	}
#endif

	sc->sc_control = sc->sc_intre = 0;

#if defined(__OpenBSD__)
	printf(",");
#else
	device_printf(sc->sc_bus.bdev, " ");
#endif
	rev = OREAD4(sc, OHCI_REVISION);
	printf("OHCI version %d.%d%s\n", OHCI_REV_HI(rev), OHCI_REV_LO(rev),
	       OHCI_REV_LEGACY(rev) ? ", legacy support" : "");

	if((OHCI_REV_HI(rev) != 1) ||
	   (OHCI_REV_LO(rev) != 0))
	{
		device_printf(sc->sc_bus.bdev, "unsupported OHCI revision\n");
		sc->sc_bus.usbrev = USBREV_UNKNOWN;
		goto error;
	}
	sc->sc_bus.usbrev = USBREV_1_0;

	if(ohci_controller_init(sc))
	{
	error:
		mtx_unlock(&sc->sc_bus.mtx);
		return (USBD_INVAL);
	}
	else
	{
		mtx_unlock(&sc->sc_bus.mtx);
		return (USBD_NORMAL_COMPLETION);
	}
}

/*
 * shut down the controller when the system is going down
 */
void
ohci_detach(struct ohci_softc *sc)
{
	mtx_lock(&sc->sc_bus.mtx);

	__callout_stop(&sc->sc_tmo_rhsc);

	OWRITE4(sc, OHCI_INTERRUPT_DISABLE, OHCI_ALL_INTRS);
	OWRITE4(sc, OHCI_CONTROL, OHCI_HCFS_RESET);

	DELAY(1000*300); /* XXX let stray task complete */

	mtx_unlock(&sc->sc_bus.mtx);
	return;
}

/* NOTE: suspend/resume is called from
 * interrupt context and cannot sleep!
 */
void
ohci_suspend(ohci_softc_t *sc)
{
	u_int32_t ctl;
	mtx_lock(&sc->sc_bus.mtx);

#ifdef USB_DEBUG
	DPRINTF(("\n"));
	if(ohcidebug > 2)
	{
		ohci_dumpregs(sc);
	}
#endif

	ctl = OREAD4(sc, OHCI_CONTROL) & ~OHCI_HCFS_MASK;
	if(sc->sc_control == 0)
	{
		/*
		 * Preserve register values, in case that APM BIOS
		 * does not recover them.
		 */
		sc->sc_control = ctl;
		sc->sc_intre = OREAD4(sc, OHCI_INTERRUPT_ENABLE);
	}
	ctl |= OHCI_HCFS_SUSPEND;
	OWRITE4(sc, OHCI_CONTROL, ctl);
	DELAY(1000*USB_RESUME_WAIT);

	mtx_unlock(&sc->sc_bus.mtx);
	return;
}

void
ohci_resume(ohci_softc_t *sc)
{
	u_int32_t ctl;
	mtx_lock(&sc->sc_bus.mtx);

#ifdef USB_DEBUG
	DPRINTF(("\n"));
	if(ohcidebug > 2)
	{
		ohci_dumpregs(sc);
	}
#endif
	/* some broken BIOSes never initialize the Controller chip */
	ohci_controller_init(sc);

	if(sc->sc_intre)
	{
		OWRITE4(sc, OHCI_INTERRUPT_ENABLE,
			sc->sc_intre & (OHCI_ALL_INTRS | OHCI_MIE));
	}

	if(sc->sc_control)
		ctl = sc->sc_control;
	else
		ctl = OREAD4(sc, OHCI_CONTROL);
	ctl |= OHCI_HCFS_RESUME;
	OWRITE4(sc, OHCI_CONTROL, ctl);
	DELAY(1000*USB_RESUME_DELAY);
	ctl = (ctl & ~OHCI_HCFS_MASK) | OHCI_HCFS_OPERATIONAL;
	OWRITE4(sc, OHCI_CONTROL, ctl);
	DELAY(1000*USB_RESUME_RECOVERY);
	sc->sc_control = sc->sc_intre = 0;

	mtx_unlock(&sc->sc_bus.mtx);
	return;
}

#ifdef USB_DEBUG
static void
ohci_dumpregs(ohci_softc_t *sc)
{
	DPRINTF(("ohci_dumpregs: rev=0x%08x control=0x%08x command=0x%08x\n",
		 OREAD4(sc, OHCI_REVISION),
		 OREAD4(sc, OHCI_CONTROL),
		 OREAD4(sc, OHCI_COMMAND_STATUS)));
	DPRINTF(("               intrstat=0x%08x intre=0x%08x intrd=0x%08x\n",
		 OREAD4(sc, OHCI_INTERRUPT_STATUS),
		 OREAD4(sc, OHCI_INTERRUPT_ENABLE),
		 OREAD4(sc, OHCI_INTERRUPT_DISABLE)));
	DPRINTF(("               hcca=0x%08x percur=0x%08x ctrlhd=0x%08x\n",
		 OREAD4(sc, OHCI_HCCA),
		 OREAD4(sc, OHCI_PERIOD_CURRENT_ED),
		 OREAD4(sc, OHCI_CONTROL_HEAD_ED)));
	DPRINTF(("               ctrlcur=0x%08x bulkhd=0x%08x bulkcur=0x%08x\n",
		 OREAD4(sc, OHCI_CONTROL_CURRENT_ED),
		 OREAD4(sc, OHCI_BULK_HEAD_ED),
		 OREAD4(sc, OHCI_BULK_CURRENT_ED)));
	DPRINTF(("               done=0x%08x fmival=0x%08x fmrem=0x%08x\n",
		 OREAD4(sc, OHCI_DONE_HEAD),
		 OREAD4(sc, OHCI_FM_INTERVAL),
		 OREAD4(sc, OHCI_FM_REMAINING)));
	DPRINTF(("               fmnum=0x%08x perst=0x%08x lsthrs=0x%08x\n",
		 OREAD4(sc, OHCI_FM_NUMBER),
		 OREAD4(sc, OHCI_PERIODIC_START),
		 OREAD4(sc, OHCI_LS_THRESHOLD)));
	DPRINTF(("               desca=0x%08x descb=0x%08x stat=0x%08x\n",
		 OREAD4(sc, OHCI_RH_DESCRIPTOR_A),
		 OREAD4(sc, OHCI_RH_DESCRIPTOR_B),
		 OREAD4(sc, OHCI_RH_STATUS)));
	DPRINTF(("               port1=0x%08x port2=0x%08x\n",
		 OREAD4(sc, OHCI_RH_PORT_STATUS(1)),
		 OREAD4(sc, OHCI_RH_PORT_STATUS(2))));
	DPRINTF(("         HCCA: frame_number=0x%04x done_head=0x%08x\n",
		 le32toh(sc->sc_hw.hcca.hcca_frame_number),
		 le32toh(sc->sc_hw.hcca.hcca_done_head)));
}
static void
ohci_dump_tds(ohci_td_t *std)
{
	for(; std; std = std->next)
	{
		ohci_dump_td(std);
	}
}

static void
ohci_dump_td(ohci_td_t *std)
{
	u_int32_t td_flags = le32toh(std->td_flags);

	printf("TD(%p) at %08lx: %s%s%s%s%s delay=%d ec=%d "
	       "cc=%d\ncbp=0x%08lx next=0x%08lx be=0x%08lx\n",
	       std, (long)le32toh(std->td_self),
	       (td_flags & OHCI_TD_R) ? "-R" : "",
	       (td_flags & OHCI_TD_OUT) ? "-OUT" : "",
	       (td_flags & OHCI_TD_IN) ? "-IN" : "",
	       ((td_flags & OHCI_TD_TOGGLE_MASK) == OHCI_TD_TOGGLE_1) ? "-TOG1" : "",
	       ((td_flags & OHCI_TD_TOGGLE_MASK) == OHCI_TD_TOGGLE_0) ? "-TOG0" : "",
	       OHCI_TD_GET_DI(le32toh(std->td_flags)),
	       OHCI_TD_GET_EC(le32toh(std->td_flags)),
	       OHCI_TD_GET_CC(le32toh(std->td_flags)),
	       (long)le32toh(std->td_cbp),
	       (long)le32toh(std->td_next),
	       (long)le32toh(std->td_be));
}

static void
ohci_dump_itd(ohci_itd_t *sitd)
{
	int i;

	printf("ITD(%p) at %08lx: sf=%d di=%d fc=%d cc=%d\n"
	       "bp0=0x%08lx next=0x%08lx be=0x%08lx\n",
	       sitd, (long)le32toh(sitd->itd_self),
	       OHCI_ITD_GET_SF(le32toh(sitd->itd_flags)),
	       OHCI_ITD_GET_DI(le32toh(sitd->itd_flags)),
	       OHCI_ITD_GET_FC(le32toh(sitd->itd_flags)),
	       OHCI_ITD_GET_CC(le32toh(sitd->itd_flags)),
	       (long)le32toh(sitd->itd_bp0),
	       (long)le32toh(sitd->itd_next),
	       (long)le32toh(sitd->itd_be));
	for(i = 0; i < OHCI_ITD_NOFFSET; i++)
	{
		printf("offs[%d]=0x%04x ", i,
		       (u_int)le16toh(sitd->itd_offset[i]));
	}
	printf("\n");
}

static void
ohci_dump_itds(ohci_itd_t *sitd)
{
	for(; sitd; sitd = sitd->next)
	{
		ohci_dump_itd(sitd);
	}
}

static void
ohci_dump_ed(ohci_ed_t *sed)
{
	u_int32_t ed_flags = le32toh(sed->ed_flags);
	u_int32_t ed_headp = le32toh(sed->ed_headp);

	printf("ED(%p) at 0x%08lx: addr=%d endpt=%d maxp=%d flags=%s%s%s%s%s\n"
	       "tailp=0x%08lx headflags=%s%s headp=0x%08lx nexted=0x%08lx\n",
	       sed, (long)le32toh(sed->ed_self),
	       OHCI_ED_GET_FA(le32toh(sed->ed_flags)),
	       OHCI_ED_GET_EN(le32toh(sed->ed_flags)),
	       OHCI_ED_GET_MAXP(le32toh(sed->ed_flags)),
	       (ed_flags & OHCI_ED_DIR_OUT) ? "-OUT" : "",
	       (ed_flags & OHCI_ED_DIR_IN) ? "-IN" : "",
	       (ed_flags & OHCI_ED_SPEED) ? "-LOWSPEED" : "",
	       (ed_flags & OHCI_ED_SKIP) ? "-SKIP" : "",
	       (ed_flags & OHCI_ED_FORMAT_ISO) ? "-ISO" : "",
	       (long)le32toh(sed->ed_tailp), 
	       (ed_headp & OHCI_HALTED) ? "-HALTED" : "",
	       (ed_headp & OHCI_TOGGLECARRY) ? "-CARRY" : "",
	       (long)le32toh(sed->ed_headp),
	       (long)le32toh(sed->ed_next));
}
#endif


#define OHCI_APPEND_QH(sed,last) (last) = _ohci_append_qh(sed,last)
static ohci_ed_t *
_ohci_append_qh(ohci_ed_t *sed, ohci_ed_t *last)
{
	DPRINTFN(10, ("%p to %p\n", sed, last));

	/* (sc->sc_bus.mtx) must be locked */

	sed->next = last->next;
	sed->ed_next = last->ed_next;

	sed->prev = last;
	
	/* the last->next->prev is never followed:
	 * sed->next->prev = sed;
	 */

	last->next = sed;
	last->ed_next = sed->ed_self;
	return(sed);
}
/**/

#define OHCI_REMOVE_QH(sed,last) (last) = _ohci_remove_qh(sed,last)
static ohci_ed_t *
_ohci_remove_qh(ohci_ed_t *sed, ohci_ed_t *last)
{
	DPRINTFN(10, ("%p from %p\n", sed, last));

	/* (sc->sc_bus.mtx) must be locked */

	/* only remove if not removed from a queue */
	if(sed->prev)
	{
		sed->prev->next = sed->next;
		sed->prev->ed_next = sed->ed_next;

		if(sed->next)
		{
			sed->next->prev = sed->prev;
		}

		/* terminate transfer in case the
		 * transferred packet was short so
		 * that the ED still points at the
		 * last used TD
		 */
		sed->ed_flags |= htole32(OHCI_ED_SKIP);
		sed->ed_headp = sed->ed_tailp;

		last = ((last == sed) ? sed->prev : last);

		sed->prev = 0;
	}
	return(last);
}

static void
ohci_device_done(struct usbd_xfer *xfer, usbd_status error);

static void
ohci_isoc_done(struct usbd_xfer *xfer)
{
	u_int8_t nframes;
	u_int32_t actlen = 0;
	u_int16_t *plen = xfer->frlengths;
	__volatile__ u_int16_t *olen;
	u_int16_t len = 0;
	ohci_itd_t *td = xfer->td_transfer_first;

	while(((void *)td) <= xfer->td_transfer_last)
	{
#ifdef USB_DEBUG
		if(ohcidebug > 5)
		{
			DPRINTFN(-1,("isoc TD\n"));
			ohci_dump_itd(td);
		}
#endif
		nframes = td->frames;
		olen = &td->itd_offset[0];

		while(nframes--)
		{
			len = le16toh(*olen);

			if((len >> 12) == OHCI_CC_NOT_ACCESSED)
			{
				len = 0;
			}
			else
			{
				len &= ((1<<12)-1);
			}

			*plen = len;
			actlen += len;
			plen++;
			olen++;
		}
		td++;
	}
	xfer->actlen = actlen;
	ohci_device_done(xfer,USBD_NORMAL_COMPLETION);
	return;
}

#ifdef USB_DEBUG
static const char * const
ohci_cc_strs[] =
{
	"NO_ERROR",
	"CRC",
	"BIT_STUFFING",
	"DATA_TOGGLE_MISMATCH",
	"STALL",
	"DEVICE_NOT_RESPONDING",
	"PID_CHECK_FAILURE",
	"UNEXPECTED_PID",
	"DATA_OVERRUN",
	"DATA_UNDERRUN",
	"BUFFER_OVERRUN",
	"BUFFER_UNDERRUN",
	"reserved",
	"reserved",
	"NOT_ACCESSED",
	"NOT_ACCESSED"
};
#endif

static void
ohci_non_isoc_done(struct usbd_xfer *xfer)
{
	u_int16_t cc = 0;
	u_int32_t actlen = 0;
	u_int32_t len;
	ohci_td_t *td = xfer->td_transfer_first;

	DPRINTFN(12, ("xfer=%p pipe=%p transfer done\n",
		      xfer, xfer->pipe));

#ifdef USB_DEBUG
	if(ohcidebug > 10)
	{
		ohci_dump_tds(td);
	}
#endif

	while(((void *)td) <= xfer->td_transfer_last)
	{
		len = td->len;
		if(td->td_cbp != 0)
		{
			len -= le32toh(td->td_be) -
			       le32toh(td->td_cbp) + 1;
		}
		DPRINTFN(10, ("len=%d\n", len));

		actlen += len;

		cc = OHCI_TD_GET_CC(le32toh(td->td_flags));
		if(cc)
		{
			DPRINTFN(15,("error cc=%d (%s)\n",
				     cc, ohci_cc_strs[cc]));
			break;
		}

		if(td->td_cbp != 0)
		{
			/* short transfer */
			break;
		}
		td++;
	}

	DPRINTFN(10, ("actlen=%d\n", actlen));

	xfer->actlen = actlen;

	ohci_device_done(xfer, 
			 (cc == 0) ? USBD_NORMAL_COMPLETION :
			 (cc == OHCI_CC_STALL) ? USBD_STALLED : USBD_IOERROR);
	return;
}

/* returns one when transfer is finished 
 * and callback must be called; else zero
 */
static u_int8_t
ohci_check_transfer(struct usbd_xfer *xfer)
{
	ohci_ed_t *ed = xfer->qh_start;

	DPRINTFN(15, ("xfer=%p\n", xfer));

	if(xfer->usb_thread)
	{
	    if(xfer->usb_thread != curthread)
	    {
	        /* cannot call this transfer 
		 * back due to locking !
		 */
	        return 0;
	    }
	}

	if((ed->ed_flags & htole32(OHCI_ED_SKIP)) ||
	   (ed->ed_headp & htole32(OHCI_HALTED)) ||
	   (((ed->ed_headp ^ ed->ed_tailp) & htole32(-0x10)) == 0))
	{
		if(xfer->pipe->methods == &ohci_device_isoc_methods)
		{
			/* isochronous transfer */
			ohci_isoc_done(xfer);
		}
		else
		{
			/* store data-toggle */
			if(ed->ed_headp & htole32(OHCI_TOGGLECARRY))
			{
				xfer->pipe->toggle_next = 1;
			}
			else
			{
				xfer->pipe->toggle_next = 0;
			}

			/* non-isochronous transfer */
			ohci_non_isoc_done(xfer);
		}
		return 1;
	}
	else
	{
		return 0;
	}
}

static void
ohci_rhsc_enable(ohci_softc_t *sc)
{
	struct usbd_callback_info info[1];
	struct usbd_xfer *xfer;

	DPRINTFN(4, ("\n"));

	mtx_assert(&sc->sc_bus.mtx, MA_OWNED);

	sc->sc_eintrs |= OHCI_RHSC;
	OWRITE4(sc, OHCI_INTERRUPT_ENABLE, OHCI_RHSC);

	/* acknowledge any RHSC interrupt */
	OWRITE4(sc, OHCI_INTERRUPT_STATUS, OHCI_RHSC);

	xfer = sc->sc_intrxfer;

 	if(xfer)
	{
	    /* transfer is transferred */
	    ohci_device_done(xfer, USBD_NORMAL_COMPLETION);

	    /* queue callback */
	    info[0].xfer = xfer;
	    info[0].refcount = xfer->usb_refcount;

	    xfer->usb_root->memory_refcount++;

	    mtx_unlock(&sc->sc_bus.mtx);

	    usbd_do_callback(&info[0],&info[1]);
	}
	else
	{
	    mtx_unlock(&sc->sc_bus.mtx);
	}
	return;
}

void
ohci_interrupt(ohci_softc_t *sc)
{
	enum { FINISH_LIST_MAX = 16 };

	struct usbd_callback_info info[FINISH_LIST_MAX];
	struct usbd_callback_info *ptr = &info[0];
	struct usbd_xfer *xfer;
	u_int32_t status;
	u_int32_t done;
	u_int8_t need_repeat = 0;

	mtx_lock(&sc->sc_bus.mtx);

	if(sc->sc_bus.bdev == NULL)
	{
		/* too early interrupt */
		goto done;
	}

	sc->sc_bus.no_intrs++;

	DPRINTFN(15,("%s: real interrupt\n",
		     device_get_nameunit(sc->sc_bus.bdev)));

#ifdef USB_DEBUG
	if(ohcidebug > 15)
	{
		DPRINTF(("%s:\n", device_get_nameunit(sc->sc_bus.bdev)));
		ohci_dumpregs(sc);
	}
#endif

	status = 0;
	done = le32toh(sc->sc_hw.hcca.hcca_done_head);

	/* The LSb of done is used to inform the HC Driver that an interrupt
	 * condition exists for both the Done list and for another event
	 * recorded in HcInterruptStatus. On an interrupt from the HC, the HC
	 * Driver checks the HccaDoneHead Value. If this value is 0, then the
	 * interrupt was caused by other than the HccaDoneHead update and the
	 * HcInterruptStatus register needs to be accessed to determine that
	 * exact interrupt cause. If HccaDoneHead is nonzero, then a Done list
	 * update interrupt is indicated and if the LSb of done is nonzero,
	 * then an additional interrupt event is indicated and
	 * HcInterruptStatus should be checked to determine its cause.
	 */
	if(done != 0)
	{
		if(done & ~OHCI_DONE_INTRS)
		{
			status |= OHCI_WDH;
		}
		if(done & OHCI_DONE_INTRS)
		{
			status |= OREAD4(sc, OHCI_INTERRUPT_STATUS);
			done &= ~OHCI_DONE_INTRS;
		}
		sc->sc_hw.hcca.hcca_done_head = 0;
	}
	else
	{
		status = OREAD4(sc, OHCI_INTERRUPT_STATUS) & ~OHCI_WDH;
	}

	if(status == 0)		/* nothing to be done (PCI shared interrupt) */
	{
		goto done;
	}

	status &= ~OHCI_MIE;
	OWRITE4(sc, OHCI_INTERRUPT_STATUS, status); /* Acknowledge */

	status &= sc->sc_eintrs;
	if(status == 0)
	{
		goto done;
	}

#if 0
	if(status & OHCI_SO)
	{
		/* XXX do what */
	}
#endif
	if(status & OHCI_RD)
	{
		device_printf(sc->sc_bus.bdev, "resume detect\n");
		/* XXX process resume detect */
	}
	if(status & OHCI_UE)
	{
		device_printf(sc->sc_bus.bdev, "unrecoverable error, "
			      "controller halted\n");
		OWRITE4(sc, OHCI_CONTROL, OHCI_HCFS_RESET);
		/* XXX what else */
	}
	if(status & OHCI_RHSC)
	{
		xfer = sc->sc_intrxfer;

		if(xfer)
		{
		    ohci_device_done(xfer, USBD_NORMAL_COMPLETION);

		    /* queue callback */
		    ptr->xfer = xfer;
		    ptr->refcount = xfer->usb_refcount;
		    ptr++;

		    xfer->usb_root->memory_refcount++;
		}

		/*
		 * Disable RHSC interrupt for now, because it will be
		 * on until the port has been reset.
		 */
		sc->sc_eintrs &= ~OHCI_RHSC;
		OWRITE4(sc, OHCI_INTERRUPT_DISABLE, OHCI_RHSC);

		/* do not allow RHSC interrupts > 1 per second */
		__callout_reset(&sc->sc_tmo_rhsc, hz,
				(void *)(void *)ohci_rhsc_enable, sc);
	}

	status &= ~(OHCI_RHSC|OHCI_WDH|OHCI_SO);
	if(status != 0)
	{
		/* Block unprocessed interrupts. XXX */
		OWRITE4(sc, OHCI_INTERRUPT_DISABLE, status);
		sc->sc_eintrs &= ~status;
		device_printf(sc->sc_bus.bdev, 
			      "blocking intrs 0x%x\n", status);
	}

	/*
	 * when the host controller interrupts because a transfer
	 * is completed, all active transfers are checked!
	 */

 repeat:
	LIST_FOREACH(xfer, &sc->sc_interrupt_list_head, interrupt_list)
	{
		/* check if transfer is
		 * transferred 
		 */
		if(ohci_check_transfer(xfer))
		{
		    /* queue callback */
		    ptr->xfer = xfer;
		    ptr->refcount = xfer->usb_refcount;
		    ptr++;

		    xfer->usb_root->memory_refcount++;

		    /* check queue length */
		    if(ptr >= &info[FINISH_LIST_MAX])
		    {
		        need_repeat = 1;
			break;
		    }
		}
	}

 done:
	mtx_unlock(&sc->sc_bus.mtx);

	usbd_do_callback(&info[0],ptr);

	if(need_repeat)
	{
		ptr = &info[0];

		need_repeat = 0;

		mtx_lock(&sc->sc_bus.mtx);

		goto repeat;
	}
	return;
}

/*
 * called when a request does not complete
 */
static void
ohci_timeout(struct usbd_xfer *xfer)
{
	struct usbd_callback_info info[1];
	ohci_softc_t *sc = xfer->usb_sc;

	DPRINTF(("xfer=%p\n", xfer));

	mtx_assert(&sc->sc_bus.mtx, MA_OWNED);

	/* transfer is transferred */
	ohci_device_done(xfer, USBD_TIMEOUT);

	/* queue callback */
	info[0].xfer = xfer;
	info[0].refcount = xfer->usb_refcount;

	xfer->usb_root->memory_refcount++;

	mtx_unlock(&sc->sc_bus.mtx);

	usbd_do_callback(&info[0],&info[1]);

	return;
}

static void
ohci_do_poll(struct usbd_bus *bus)
{
	ohci_interrupt(OHCI_BUS2SC(bus));
	return;
}

#define ohci_add_interrupt_info(sc, xfer) \
	LIST_INSERT_HEAD(&(sc)->sc_interrupt_list_head, (xfer), interrupt_list)

static void
ohci_remove_interrupt_info(struct usbd_xfer *xfer)
{
	if((xfer)->interrupt_list.le_prev)
	{
		LIST_REMOVE((xfer), interrupt_list);
		(xfer)->interrupt_list.le_prev = 0;
	}
	return;
}

static void
ohci_setup_standard_chain(struct usbd_xfer *xfer, ohci_ed_t **ed_last)
{
	/* the OHCI hardware can handle at most one 4k crossing per TD */
	u_int32_t average = ((unsigned)(OHCI_PAGE_SIZE / xfer->max_packet_size))
	  * xfer->max_packet_size;
	u_int32_t td_flags;
	u_int32_t physbuffer = xfer->physbuffer;
	u_int32_t len = xfer->length;
	u_int8_t isread;
	u_int8_t shortpkt = 0;
	ohci_td_t *td;
	ohci_ed_t *ed;

	DPRINTFN(8, ("addr=%d endpt=%d len=%d speed=%d\n", 
		     xfer->address, UE_GET_ADDR(xfer->endpoint),
		     xfer->length, xfer->udev->speed));

	td = (xfer->td_transfer_first = xfer->td_start);

	if(xfer->pipe->methods == &ohci_device_ctrl_methods)
	{
		isread = ((usb_device_request_t *)xfer->buffer)->bmRequestType & UT_READ;

		/* xfer->length = sizeof(usb_device_request_t) + UGETW(req->wLength)
		 * check length ??
		 */

		xfer->pipe->toggle_next = 1;

		/* SETUP message */

		td->next = (td+1);
		td->td_next = (td+1)->td_self;

		td->td_flags = htole32(OHCI_TD_SETUP | OHCI_TD_NOCC |
				       OHCI_TD_TOGGLE_0 | OHCI_TD_NOINTR);

		td->td_cbp = htole32(physbuffer);
		td->td_be = htole32(physbuffer + (sizeof(usb_device_request_t) - 1));
		td->len = sizeof(usb_device_request_t);

		physbuffer += sizeof(usb_device_request_t);
		len -= sizeof(usb_device_request_t);
		td++;
	}
	else
	{
		isread = (UE_GET_DIR(xfer->endpoint) == UE_DIR_IN);

		if(xfer->length == 0)
		{
			/* must allow access to (td-1),
			 * so xfer->length cannot be zero
			 */
			printf("%s: setting USBD_FORCE_SHORT_XFER!\n", __FUNCTION__);
			xfer->flags |= USBD_FORCE_SHORT_XFER;
		}
	}

	td_flags = htole32(OHCI_TD_NOCC | OHCI_TD_NOINTR);

 	if(xfer->flags & USBD_SHORT_XFER_OK)
	{
		td_flags |= htole32(OHCI_TD_R);
	}
	if(xfer->pipe->toggle_next)
	{
		td_flags |= htole32(OHCI_TD_TOGGLE_1);
	}
	else
	{
		td_flags |= htole32(OHCI_TD_TOGGLE_0);
	}
	if(isread)
        {
		td_flags |= htole32(OHCI_TD_IN);
	}
	else
	{
		td_flags |= htole32(OHCI_TD_OUT);
	}

	while(1)
	{
		if(len == 0)
		{
			if(xfer->flags & USBD_FORCE_SHORT_XFER)
			{
				if(shortpkt)
				{
					break;
				}
			}
			else
			{
				break;
			}
		}

		if(len < average)
		{
			if((len % xfer->max_packet_size) || 
			   (len == 0))
			{
				shortpkt = 1;
			}

			average = len;
		}

		 if(((void *)td) >= xfer->td_end)
		 {
			panic("%s: software wants to write more data "
			      "than there is in the buffer!", __FUNCTION__);
		 }

		 /* fill out TD */

		 td->next = (td+1);
		 td->td_next = (td+1)->td_self;
		 td->td_flags = td_flags;

		 /* the next td uses TOGGLE_CARRY */
		 td_flags &= htole32(~OHCI_TD_TOGGLE_MASK);

		 if(average == 0)
		 {
			td->td_cbp = 0;
			td->td_be = ~0;
		 }
		 else
		 {
			td->td_cbp = htole32(physbuffer);
			td->td_be = htole32(physbuffer + average - 1);
		 }
		 td->len = average;

		 physbuffer += average;
		 len -= average;
		 td++;
	}

	if(xfer->pipe->methods == &ohci_device_ctrl_methods)
	{
		/* STATUS message */

		td->next = NULL;
		td->td_next = 0;
		td->td_flags = htole32(OHCI_TD_NOCC | OHCI_TD_TOGGLE_1 |
				       OHCI_TD_SET_DI(1));
		if(isread)
		{
			td->td_flags |= htole32(OHCI_TD_OUT);
		}
		else
		{
			td->td_flags |= htole32(OHCI_TD_IN);
		}
		td->td_cbp = 0;
		td->td_be = ~0;
		td->len = 0;

		physbuffer += 0;
		len -= 0;
		td++;
	}
	else
	{
		(td-1)->next = NULL;
		(td-1)->td_next = 0;
		(td-1)->td_flags &= htole32(~OHCI_TD_INTR_MASK);
		(td-1)->td_flags |= htole32(OHCI_TD_SET_DI(1));
	}

	/* must have at least one frame! */

	xfer->td_transfer_last = (td-1);

#ifdef USB_DEBUG
	if(ohcidebug > 8)
	{
		DPRINTF(("nexttog=%d; data before transfer:\n",
			 xfer->pipe->toggle_next));
		ohci_dump_tds(xfer->td_start);
	}
#endif

	ed = xfer->qh_start;

	ed->ed_flags = htole32(OHCI_ED_FORMAT_GEN | OHCI_ED_DIR_TD);
	ed->ed_flags |= htole32(OHCI_ED_SET_FA(xfer->address)|
				OHCI_ED_SET_EN(UE_GET_ADDR(xfer->endpoint))|
				OHCI_ED_SET_MAXP(xfer->max_packet_size));

	if(xfer->udev->speed == USB_SPEED_LOW)
	{
		ed->ed_flags |= htole32(OHCI_ED_SPEED);
	}

	td = xfer->td_transfer_first;

	ed->ed_tailp = 0;
	ed->ed_headp = td->td_self;

	OHCI_APPEND_QH(ed, *ed_last);

	if(xfer->pipe->methods == &ohci_device_bulk_methods)
	{
		ohci_softc_t *sc = xfer->usb_sc;
		OWRITE4(sc, OHCI_COMMAND_STATUS, OHCI_BLF);
	}

	if(xfer->pipe->methods == &ohci_device_ctrl_methods)
	{
		ohci_softc_t *sc = xfer->usb_sc;
		OWRITE4(sc, OHCI_COMMAND_STATUS, OHCI_CLF);
	}
	return;
}

static void
ohci_root_intr_done(ohci_softc_t *sc, struct usbd_xfer *xfer)
{
	u_int8_t *p;
	u_int i, m;
	u_int32_t hstatus;

	if(sc->sc_intrxfer)
	{
		/* disable further interrupts */
		sc->sc_intrxfer = NULL;

		hstatus = OREAD4(sc, OHCI_RH_STATUS);
		DPRINTF(("sc=%p xfer=%p hstatus=0x%08x\n",
			 sc, xfer, hstatus));

		p = xfer->buffer;
		m = min(sc->sc_noport, (xfer->length * 8) - 1);
		memset(p, 0, xfer->length);
		for(i = 1; i <= m; i++)
		{
			/* pick out CHANGE bits from the status register */
			if(OREAD4(sc, OHCI_RH_PORT_STATUS(i)) >> 16)
			{
				p[i/8] |= 1 << (i%8);
			}
		}
		DPRINTF(("change=0x%02x\n", *p));
		xfer->actlen = xfer->length;
	}
	return;
}

/* NOTE: "done" can be run two times in a row,
 * from close and from interrupt
 */
static void
ohci_device_done(struct usbd_xfer *xfer, usbd_status error)
{
	ohci_softc_t *sc = xfer->usb_sc;
	ohci_ed_t *ed;
	u_int8_t need_delay;

	mtx_assert(&sc->sc_bus.mtx, MA_OWNED);

	need_delay = 0;

	DPRINTFN(1,("xfer=%p, pipe=%p length=%d error=%d\n",
		    xfer, xfer->pipe, xfer->actlen, error));

	for(ed = xfer->qh_start;
	    ((void *)ed) < xfer->qh_end;
	    ed++)
	{
		if((!(ed->ed_flags & htole32(OHCI_ED_SKIP))) &&
		   (!(ed->ed_headp & htole32(OHCI_HALTED))) &&
		   ((ed->ed_headp ^ ed->ed_tailp) & htole32(-0x10)))
		{
			need_delay = 1;
		}
	}

	if(xfer->pipe->methods == &ohci_device_bulk_methods)
	{
		OHCI_REMOVE_QH(xfer->qh_start, sc->sc_bulk_p_last);
	}

	if(xfer->pipe->methods == &ohci_device_ctrl_methods)
	{
		OHCI_REMOVE_QH(xfer->qh_start, sc->sc_ctrl_p_last);
	}

	if(xfer->pipe->methods == &ohci_device_intr_methods)
	{
		OHCI_REMOVE_QH(xfer->qh_start, sc->sc_intr_p_last[xfer->qh_pos]);
	}

	if(xfer->pipe->methods == &ohci_device_isoc_methods)
	{
		OHCI_REMOVE_QH(xfer->qh_start, sc->sc_isoc_p_last);
	}

	xfer->td_transfer_first = 0;
	xfer->td_transfer_last = 0;

	/* finish root interrupt transfer
	 * (will update xfer->buffer and xfer->actlen)
	 */
	if(xfer->pipe->methods == &ohci_root_intr_methods)
	{
		ohci_root_intr_done(sc, xfer);
	}

	/* stop timeout */
	__callout_stop(&xfer->timeout_handle);

	/* remove interrupt info */
	ohci_remove_interrupt_info(xfer);

	/* wait until hardware has finished any possible
	 * use of the transfer and QH
	 *
	 * hardware finishes in 1 millisecond
	 */
	DELAY(need_delay ? (2*1000) : (5));

	if(error)
	{
		/* next transfer needs to clear stall */
		xfer->pipe->clearstall = 1;
	}

	/* transfer is transferred ! */
	usbd_transfer_done(xfer,error);

	/* dequeue transfer (and start next transfer)
	 *
	 * if two transfers are queued, the second
	 * transfer must be started before the
	 * first is called back!
	 */
	usbd_transfer_dequeue(xfer);

	return;
}

/*---------------------------------------------------------------------------*
 * ohci bulk support
 *---------------------------------------------------------------------------*/
static void
ohci_device_bulk_open(struct usbd_xfer *xfer)
{
	return;
}

static void
ohci_device_bulk_close(struct usbd_xfer *xfer)
{
	ohci_device_done(xfer, USBD_CANCELLED);
	return;
}

static void
ohci_device_bulk_enter(struct usbd_xfer *xfer)
{
	/* enqueue transfer */
	usbd_transfer_enqueue(xfer);
	return;
}

static void
ohci_device_bulk_start(struct usbd_xfer *xfer)
{
	ohci_softc_t *sc = xfer->usb_sc;

	DPRINTFN(3, ("xfer=%p len=%d\n",
		     xfer, xfer->length));

	/* setup TD's and QH */
	ohci_setup_standard_chain(xfer, &sc->sc_bulk_p_last);

	/**/
	ohci_add_interrupt_info(sc, xfer);

	if(xfer->timeout && (!(xfer->flags & USBD_USE_POLLING)))
	{
		__callout_reset(&xfer->timeout_handle, MS_TO_TICKS(xfer->timeout),
				(void *)(void *)ohci_timeout, xfer);
	}
	return;
}

struct usbd_pipe_methods ohci_device_bulk_methods = 
{
  .open = ohci_device_bulk_open,
  .close = ohci_device_bulk_close,
  .enter = ohci_device_bulk_enter,
  .start = ohci_device_bulk_start,
};

/*---------------------------------------------------------------------------*
 * ohci control support
 *---------------------------------------------------------------------------*/
static void
ohci_device_ctrl_open(struct usbd_xfer *xfer)
{
	return;
}

static void
ohci_device_ctrl_close(struct usbd_xfer *xfer)
{
	ohci_device_done(xfer, USBD_CANCELLED);
	return;
}

static void
ohci_device_ctrl_enter(struct usbd_xfer *xfer)
{
	/* enqueue transfer */
	usbd_transfer_enqueue(xfer);
	return;
}

static void
ohci_device_ctrl_start(struct usbd_xfer *xfer)
{
	ohci_softc_t *sc = xfer->usb_sc;

	DPRINTFN(3,("type=0x%02x, request=0x%02x, "
		    "wValue=0x%04x, wIndex=0x%04x len=%d, addr=%d, endpt=%d\n",
		    ((usb_device_request_t *)(xfer->buffer))->bmRequestType,
		    ((usb_device_request_t *)(xfer->buffer))->bRequest,
		    UGETW(((usb_device_request_t *)(xfer->buffer))->wValue),
		    UGETW(((usb_device_request_t *)(xfer->buffer))->wIndex), 
		    UGETW(((usb_device_request_t *)(xfer->buffer))->wLength),
		    xfer->address, xfer->endpoint));

	/* setup TD's and QH */
	ohci_setup_standard_chain(xfer, &sc->sc_ctrl_p_last);

	/**/
	ohci_add_interrupt_info(sc, xfer);

	if(xfer->timeout && (!(xfer->flags & USBD_USE_POLLING)))
	{
		__callout_reset(&xfer->timeout_handle, MS_TO_TICKS(xfer->timeout),
				(void *)(void *)ohci_timeout, xfer);
	}
	return;
}

struct usbd_pipe_methods ohci_device_ctrl_methods = 
{
  .open = ohci_device_ctrl_open,
  .close = ohci_device_ctrl_close,
  .enter = ohci_device_ctrl_enter,
  .start = ohci_device_ctrl_start,
};

/*---------------------------------------------------------------------------*
 * ohci interrupt support
 *---------------------------------------------------------------------------*/
static void
ohci_device_intr_open(struct usbd_xfer *xfer)
{
	ohci_softc_t *sc = xfer->usb_sc;
	u_int16_t best;
	u_int16_t bit;
	u_int16_t x;

	best = 0;
	bit = OHCI_NO_EDS/2;
	while(bit)
	{
		if(xfer->interval >= bit)
		{
			x = bit;
			best = bit;
			while(x & bit)
			{
				if(sc->sc_intr_stat[x] < 
				   sc->sc_intr_stat[best])
				{
					best = x;
				}
				x++;
			}
			break;
		}
		bit >>= 1;
	}

	sc->sc_intr_stat[best]++;
	xfer->qh_pos = best;

	DPRINTFN(2, ("best=%d interval=%d\n",
		     best, xfer->interval));
	return;
}

static void
ohci_device_intr_close(struct usbd_xfer *xfer)
{
	ohci_softc_t *sc = xfer->usb_sc;

	sc->sc_intr_stat[xfer->qh_pos]--;

	ohci_device_done(xfer,USBD_CANCELLED);
	return;
}

static void
ohci_device_intr_enter(struct usbd_xfer *xfer)
{
	/* enqueue transfer */
	usbd_transfer_enqueue(xfer);
	return;
}

static void
ohci_device_intr_start(struct usbd_xfer *xfer)
{
	ohci_softc_t *sc = xfer->usb_sc;

	DPRINTFN(3,("xfer=%p len=%d\n",
		    xfer, xfer->length));

	/* setup TD's and QH */
	ohci_setup_standard_chain(xfer, &sc->sc_intr_p_last[xfer->qh_pos]);

	/**/
	ohci_add_interrupt_info(sc, xfer);

	if(xfer->timeout && (!(xfer->flags & USBD_USE_POLLING)))
	{
		__callout_reset(&xfer->timeout_handle, MS_TO_TICKS(xfer->timeout),
				(void *)(void *)ohci_timeout, xfer);
	}
	return;
}

struct usbd_pipe_methods ohci_device_intr_methods = 
{
  .open = ohci_device_intr_open,
  .close = ohci_device_intr_close,
  .enter = ohci_device_intr_enter,
  .start = ohci_device_intr_start,
};

/*---------------------------------------------------------------------------*
 * ohci isochronous support
 *---------------------------------------------------------------------------*/
static void
ohci_device_isoc_open(struct usbd_xfer *xfer)
{
	return;
}

static void
ohci_device_isoc_close(struct usbd_xfer *xfer)
{
	/**/
	ohci_device_done(xfer, USBD_CANCELLED);
	return;
}

static void
ohci_device_isoc_enter(struct usbd_xfer *xfer)
{
	ohci_softc_t *sc = xfer->usb_sc;
	u_int32_t physbuffer;
	u_int32_t nframes;
	u_int32_t bp0;
	u_int16_t *plen;
	u_int8_t ncur;
	ohci_itd_t *td;
	ohci_ed_t *ed;

	DPRINTFN(5,("xfer=%p next=%d nframes=%d\n",
		    xfer, xfer->pipe->isoc_next, xfer->nframes));

	nframes = le32toh(sc->sc_hw.hcca.hcca_frame_number);

	if((((nframes - xfer->pipe->isoc_next) & ((1<<16)-1)) < xfer->nframes) ||
	   (((xfer->pipe->isoc_next - nframes) & ((1<<16)-1)) >= 256))
	{
		/* not in use yet, schedule it a few frames ahead */
		/* data underflow */
		xfer->pipe->isoc_next = (nframes + 5) & ((1<<16)-1);
		DPRINTFN(2,("start next=%d\n", xfer->pipe->isoc_next));
	}

	nframes = xfer->nframes;

	if(nframes == 0)
	{
		/* transfer transferred */
		ohci_device_done(xfer, USBD_NORMAL_COMPLETION);

		/* call callback recursively */
		__usbd_callback(xfer);

		return;
	}

	physbuffer = xfer->physbuffer;

	plen = xfer->frlengths;

	td = xfer->td_start;

	xfer->td_transfer_first = td;

	ncur = 0;

	bp0 = OHCI_PAGE(physbuffer);

	while(nframes--)
	{
		td->itd_offset[ncur] = htole16(OHCI_ITD_MK_OFFS(physbuffer-bp0));

		ncur++;
		physbuffer += *plen;
		plen++;

		if((ncur == OHCI_ITD_NOFFSET) ||
		   (OHCI_PAGE(physbuffer) != bp0) ||
		   (nframes == 0))
		{
			/* fill current ITD */
			td->next = (td+1);
			td->itd_next = (td+1)->itd_self;
			td->itd_flags = htole32(
				OHCI_ITD_NOCC |
				OHCI_ITD_SET_SF(xfer->pipe->isoc_next) |
				OHCI_ITD_NOINTR |
				OHCI_ITD_SET_FC(ncur));
			td->itd_bp0 = htole32(bp0);
			td->itd_be = htole32(physbuffer - 1);
			td->frames = ncur;

			xfer->pipe->isoc_next += ncur;
			bp0 = OHCI_PAGE(physbuffer);
			ncur = 0;
			td++;
		}
	}

	/* fixup last used ITD */
	(td-1)->itd_flags &= htole32(~OHCI_ITD_NOINTR);
	(td-1)->itd_flags |= htole32(OHCI_ITD_SET_DI(0));
	(td-1)->next = 0;
	(td-1)->itd_next = 0;

	xfer->td_transfer_last = (td-1);

#ifdef USB_DEBUG
	if(ohcidebug > 8)
	{
		DPRINTF(("data before transfer:\n"));
		ohci_dump_itds(xfer->td_start);
	}
#endif
	ed = xfer->qh_start;

	if(UE_GET_DIR(xfer->endpoint) == UE_DIR_IN)
		ed->ed_flags = htole32(OHCI_ED_DIR_IN|OHCI_ED_FORMAT_ISO);
	else
		ed->ed_flags = htole32(OHCI_ED_DIR_OUT|OHCI_ED_FORMAT_ISO);

	ed->ed_flags |= htole32(OHCI_ED_SET_FA(xfer->address)|
				OHCI_ED_SET_EN(UE_GET_ADDR(xfer->endpoint))|
				OHCI_ED_SET_MAXP(xfer->max_packet_size));
	if(xfer->udev->speed == USB_SPEED_LOW)
	{
		ed->ed_flags |= htole32(OHCI_ED_SPEED);
	}

	ed->ed_tailp = 0;
	td = xfer->td_transfer_first;
	ed->ed_headp = td->itd_self;

	OHCI_APPEND_QH(ed, sc->sc_isoc_p_last);

	/**/
	ohci_add_interrupt_info(sc, xfer);

	if(!xfer->timeout)
	{
		/* in case the frame start number is wrong */
		xfer->timeout = 1000/4;
	}

	if(xfer->timeout && (!(xfer->flags & USBD_USE_POLLING)))
	{
		__callout_reset(&xfer->timeout_handle, MS_TO_TICKS(xfer->timeout),
				(void *)(void *)ohci_timeout, xfer);
	}

	/* enqueue transfer 
	 * (so that it can be aborted through pipe abort)
	 */
	usbd_transfer_enqueue(xfer);
	return;
}

static void
ohci_device_isoc_start(struct usbd_xfer *xfer)
{
	/* already started, nothing to do */
	return;
}

struct usbd_pipe_methods ohci_device_isoc_methods = 
{
  .open = ohci_device_isoc_open,
  .close = ohci_device_isoc_close,
  .enter = ohci_device_isoc_enter,
  .start = ohci_device_isoc_start,
};

/*---------------------------------------------------------------------------*
 * ohci root control support
 *---------------------------------------------------------------------------*
 * simulate a hardware hub by handling
 * all the necessary requests
 *---------------------------------------------------------------------------*/

static void
ohci_root_ctrl_open(struct usbd_xfer *xfer)
{
	return;
}

static void
ohci_root_ctrl_close(struct usbd_xfer *xfer)
{
	ohci_device_done(xfer,USBD_CANCELLED);
	return;
}

/* data structures and routines
 * to emulate the root hub:
 */
static const
usb_device_descriptor_t ohci_devd =
{
	USB_DEVICE_DESCRIPTOR_SIZE,
	UDESC_DEVICE,		/* type */
	{0x00, 0x01},		/* USB version */
	UDCLASS_HUB,		/* class */
	UDSUBCLASS_HUB,		/* subclass */
	UDPROTO_FSHUB,		/* protocol */
	64,			/* max packet */
	{0},{0},{0x00,0x01},	/* device id */
	1,2,0,			/* string indicies */
	1			/* # of configurations */
};

static const
usb_config_descriptor_t ohci_confd =
{
	USB_CONFIG_DESCRIPTOR_SIZE,
	UDESC_CONFIG,
	{USB_CONFIG_DESCRIPTOR_SIZE +
	 USB_INTERFACE_DESCRIPTOR_SIZE +
	 USB_ENDPOINT_DESCRIPTOR_SIZE},
	1,
	1,
	0,
	UC_SELF_POWERED,
	0			/* max power */
};

static const
usb_interface_descriptor_t ohci_ifcd =
{
	USB_INTERFACE_DESCRIPTOR_SIZE,
	UDESC_INTERFACE,
	0,
	0,
	1,
	UICLASS_HUB,
	UISUBCLASS_HUB,
	UIPROTO_FSHUB,
	0
};

static const
usb_endpoint_descriptor_t ohci_endpd =
{
	USB_ENDPOINT_DESCRIPTOR_SIZE,
	UDESC_ENDPOINT,
	UE_DIR_IN | OHCI_INTR_ENDPT,
	UE_INTERRUPT,
	{8, 0},			/* max packet */
	255
};

static const
usb_hub_descriptor_t ohci_hubd =
{
	USB_HUB_DESCRIPTOR_SIZE,
	UDESC_HUB,
	0,
	{0,0},
	0,
	0,
	{0},
};

static int
ohci_str(usb_string_descriptor_t *p, int l, const char *s)
{
	int i;

	if(l == 0)
	{
		return (0);
	}
	p->bLength = (2 * strlen(s)) + 2;
	if(l == 1)
	{
		return (1);
	}
	p->bDescriptorType = UDESC_STRING;
	l -= 2;
	for(i = 0; s[i] && (l > 1); i++, l -= 2)
	{
		USETW2(p->bString[i], 0, s[i]);
	}
	return ((2 * i) + 2);
}

static void
ohci_root_ctrl_enter(struct usbd_xfer *xfer)
{
	ohci_softc_t *sc = xfer->usb_sc;
	usb_device_request_t *req = xfer->buffer;
	void *buf;
	int port, i;
	int len, value, index, l, totlen = 0;
	usb_port_status_t ps;
	usb_hub_descriptor_t hubd;
	usbd_status err;
	u_int32_t v;

	DPRINTFN(2,("type=0x%02x request=0x%02x\n",
		    req->bmRequestType, req->bRequest));

	mtx_assert(&sc->sc_bus.mtx, MA_OWNED);

	/* set default actual length */
	xfer->actlen = sizeof(*req);

	len = UGETW(req->wLength);
	value = UGETW(req->wValue);
	index = UGETW(req->wIndex);

	if(len != 0)
	{
		buf = (req+1);
	}
	else
	{
		buf = NULL;
	}

#define C(x,y) ((x) | ((y) << 8))
	switch(C(req->bRequest, req->bmRequestType)) {
	case C(UR_CLEAR_FEATURE, UT_WRITE_DEVICE):
	case C(UR_CLEAR_FEATURE, UT_WRITE_INTERFACE):
	case C(UR_CLEAR_FEATURE, UT_WRITE_ENDPOINT):
		/*
		 * DEVICE_REMOTE_WAKEUP and ENDPOINT_HALT are no-ops
		 * for the integrated root hub.
		 */
		break;
	case C(UR_GET_CONFIG, UT_READ_DEVICE):
		if(len > 0)
		{
			*(u_int8_t *)buf = sc->sc_conf;
			totlen = 1;
		}
		break;
	case C(UR_GET_DESCRIPTOR, UT_READ_DEVICE):
		DPRINTFN(8,("wValue=0x%04x\n", value));
		switch(value >> 8) {
		case UDESC_DEVICE:
			if((value & 0xff) != 0)
			{
				err = USBD_IOERROR;
				goto done;
			}
			totlen = l = min(len, USB_DEVICE_DESCRIPTOR_SIZE);
			memcpy(buf, &ohci_devd, l);
#if 0
			if(len >= 12)
			{
			  USETW(((usb_device_descriptor_t *)buf)->idVendor,
				sc->sc_id_vendor);
			}
#endif
			break;
		case UDESC_CONFIG:
			if((value & 0xff) != 0)
			{
				err = USBD_IOERROR;
				goto done;
			}
			totlen = l = min(len, USB_CONFIG_DESCRIPTOR_SIZE);
			memcpy(buf, &ohci_confd, l);
			buf = ((u_int8_t *)buf) + l;
			len -= l;
			l = min(len, USB_INTERFACE_DESCRIPTOR_SIZE);
			totlen += l;
			memcpy(buf, &ohci_ifcd, l);
			buf = ((u_int8_t *)buf) + l;
			len -= l;
			l = min(len, USB_ENDPOINT_DESCRIPTOR_SIZE);
			totlen += l;
			memcpy(buf, &ohci_endpd, l);
			break;
		case UDESC_STRING:
			if(len == 0)
			{
				break;
			}
			*(u_int8_t *)buf = 0;
			totlen = 1;
			switch (value & 0xff) {
			case 1: /* Vendor */
				totlen = ohci_str(buf, len, sc->sc_vendor);
				break;
			case 2: /* Product */
				totlen = ohci_str(buf, len, "OHCI root hub");
				break;
			}
			break;
		default:
			err = USBD_IOERROR;
			goto done;
		}
		break;
	case C(UR_GET_INTERFACE, UT_READ_INTERFACE):
		if(len > 0)
		{
			*(u_int8_t *)buf = 0;
			totlen = 1;
		}
		break;
	case C(UR_GET_STATUS, UT_READ_DEVICE):
		if(len > 1)
		{
			USETW(((usb_status_t *)buf)->wStatus,UDS_SELF_POWERED);
			totlen = 2;
		}
		break;
	case C(UR_GET_STATUS, UT_READ_INTERFACE):
	case C(UR_GET_STATUS, UT_READ_ENDPOINT):
		if(len > 1)
		{
			USETW(((usb_status_t *)buf)->wStatus, 0);
			totlen = 2;
		}
		break;
	case C(UR_SET_ADDRESS, UT_WRITE_DEVICE):
		if(value >= USB_MAX_DEVICES)
		{
			err = USBD_IOERROR;
			goto done;
		}
		sc->sc_addr = value;
		break;
	case C(UR_SET_CONFIG, UT_WRITE_DEVICE):
		if((value != 0) && (value != 1))
		{
			err = USBD_IOERROR;
			goto done;
		}
		sc->sc_conf = value;
		break;
	case C(UR_SET_DESCRIPTOR, UT_WRITE_DEVICE):
		break;
	case C(UR_SET_FEATURE, UT_WRITE_DEVICE):
	case C(UR_SET_FEATURE, UT_WRITE_INTERFACE):
	case C(UR_SET_FEATURE, UT_WRITE_ENDPOINT):
		err = USBD_IOERROR;
		goto done;
	case C(UR_SET_INTERFACE, UT_WRITE_INTERFACE):
		break;
	case C(UR_SYNCH_FRAME, UT_WRITE_ENDPOINT):
		break;
	/* Hub requests */
	case C(UR_CLEAR_FEATURE, UT_WRITE_CLASS_DEVICE):
		break;
	case C(UR_CLEAR_FEATURE, UT_WRITE_CLASS_OTHER):
		DPRINTFN(8, ("UR_CLEAR_PORT_FEATURE "
			     "port=%d feature=%d\n",
			     index, value));
		if((index < 1) ||
		   (index > sc->sc_noport))
		{
			err = USBD_IOERROR;
			goto done;
		}
		port = OHCI_RH_PORT_STATUS(index);
		switch(value) {
		case UHF_PORT_ENABLE:
			OWRITE4(sc, port, UPS_CURRENT_CONNECT_STATUS);
			break;
		case UHF_PORT_SUSPEND:
			OWRITE4(sc, port, UPS_OVERCURRENT_INDICATOR);
			break;
		case UHF_PORT_POWER:
			/* Yes, writing to the LOW_SPEED bit clears power. */
			OWRITE4(sc, port, UPS_LOW_SPEED);
			break;
		case UHF_C_PORT_CONNECTION:
			OWRITE4(sc, port, UPS_C_CONNECT_STATUS << 16);
			break;
		case UHF_C_PORT_ENABLE:
			OWRITE4(sc, port, UPS_C_PORT_ENABLED << 16);
			break;
		case UHF_C_PORT_SUSPEND:
			OWRITE4(sc, port, UPS_C_SUSPEND << 16);
			break;
		case UHF_C_PORT_OVER_CURRENT:
			OWRITE4(sc, port, UPS_C_OVERCURRENT_INDICATOR << 16);
			break;
		case UHF_C_PORT_RESET:
			OWRITE4(sc, port, UPS_C_PORT_RESET << 16);
			break;
		default:
			err = USBD_IOERROR;
			goto done;
		}
		switch(value) {
		case UHF_C_PORT_CONNECTION:
		case UHF_C_PORT_ENABLE:
		case UHF_C_PORT_SUSPEND:
		case UHF_C_PORT_OVER_CURRENT:
		case UHF_C_PORT_RESET:
			/* enable RHSC interrupt if condition is cleared. */
			if((OREAD4(sc, port) >> 16) == 0)
			{
				mtx_lock(&sc->sc_bus.mtx);
				ohci_rhsc_enable(sc);
			}
			break;
		default:
			break;
		}
		break;
	case C(UR_GET_DESCRIPTOR, UT_READ_CLASS_DEVICE):
		if((value & 0xff) != 0)
		{
			err = USBD_IOERROR;
			goto done;
		}
		v = OREAD4(sc, OHCI_RH_DESCRIPTOR_A);
		hubd = ohci_hubd;
		hubd.bNbrPorts = sc->sc_noport;
		USETW(hubd.wHubCharacteristics,
		      (v & OHCI_NPS ? UHD_PWR_NO_SWITCH :
		       v & OHCI_PSM ? UHD_PWR_GANGED : UHD_PWR_INDIVIDUAL)
		      /* XXX overcurrent */
		      );
		hubd.bPwrOn2PwrGood = OHCI_GET_POTPGT(v);
		v = OREAD4(sc, OHCI_RH_DESCRIPTOR_B);
		for(i = 0, l = sc->sc_noport; l > 0; i++, l -= 8, v >>= 8)
		{
			hubd.DeviceRemovable[i++] = (u_int8_t)v;
		}
		hubd.bDescLength = USB_HUB_DESCRIPTOR_SIZE + i;
		l = min(len, hubd.bDescLength);
		totlen = l;
		memcpy(buf, &hubd, l);
		break;
	case C(UR_GET_STATUS, UT_READ_CLASS_DEVICE):
		if(len != 4)
		{
			err = USBD_IOERROR;
			goto done;
		}
		memset(buf, 0, len); /* ? XXX */
		totlen = len;
		break;
	case C(UR_GET_STATUS, UT_READ_CLASS_OTHER):
		DPRINTFN(8,("get port status i=%d\n",
			    index));
		if((index < 1) ||
		   (index > sc->sc_noport))
		{
			err = USBD_IOERROR;
			goto done;
		}
		if(len != 4)
		{
			err = USBD_IOERROR;
			goto done;
		}
		v = OREAD4(sc, OHCI_RH_PORT_STATUS(index));
		DPRINTFN(8,("port status=0x%04x\n",
			    v));
		USETW(ps.wPortStatus, v);
		USETW(ps.wPortChange, v >> 16);
		l = min(len, sizeof ps);
		memcpy(buf, &ps, l);
		totlen = l;
		break;
	case C(UR_SET_DESCRIPTOR, UT_WRITE_CLASS_DEVICE):
		err = USBD_IOERROR;
		goto done;
	case C(UR_SET_FEATURE, UT_WRITE_CLASS_DEVICE):
		break;
	case C(UR_SET_FEATURE, UT_WRITE_CLASS_OTHER):
		if((index < 1) || (index > sc->sc_noport))
		{
			err = USBD_IOERROR;
			goto done;
		}
		port = OHCI_RH_PORT_STATUS(index);
		switch(value) {
		case UHF_PORT_ENABLE:
			OWRITE4(sc, port, UPS_PORT_ENABLED);
			break;
		case UHF_PORT_SUSPEND:
			OWRITE4(sc, port, UPS_SUSPEND);
			break;
		case UHF_PORT_RESET:
			DPRINTFN(5,("reset port %d\n",
				    index));
			OWRITE4(sc, port, UPS_RESET);
			for(i = 0; i < 5; i++)
			{
				DELAY(1000*USB_PORT_ROOT_RESET_DELAY);

				if((OREAD4(sc, port) & UPS_RESET) == 0)
				{
					break;
				}
			}
			DPRINTFN(8,("ohci port %d reset, status = 0x%04x\n",
				    index, OREAD4(sc, port)));
			break;
		case UHF_PORT_POWER:
			DPRINTFN(2,("set port power %d\n", index));
			OWRITE4(sc, port, UPS_PORT_POWER);
			break;
		default:
			err = USBD_IOERROR;
			goto done;
		}
		break;
	default:
		err = USBD_IOERROR;
		goto done;
	}
	xfer->actlen = totlen + sizeof(*req);
	err = USBD_NORMAL_COMPLETION;

 done:
	/* transfer transferred */
	ohci_device_done(xfer,err);

	/* call callback recursively */
	__usbd_callback(xfer);

	return;
}

static void
ohci_root_ctrl_start(struct usbd_xfer *xfer)
{
	/* not used */
	return;
}

struct usbd_pipe_methods ohci_root_ctrl_methods = 
{
  .open = ohci_root_ctrl_open,
  .close = ohci_root_ctrl_close,
  .enter = ohci_root_ctrl_enter,
  .start = ohci_root_ctrl_start,
};

/*---------------------------------------------------------------------------*
 * ohci root interrupt support
 *---------------------------------------------------------------------------*/
static void
ohci_root_intr_open(struct usbd_xfer *xfer)
{
	return;
}

static void
ohci_root_intr_close(struct usbd_xfer *xfer)
{
	ohci_device_done(xfer, USBD_CANCELLED);
	return;
}

static void
ohci_root_intr_enter(struct usbd_xfer *xfer)
{
	DPRINTFN(3, ("xfer=%p len=%d\n",
		     xfer, xfer->length));

	/* enqueue transfer 
	 * (so that it can be aborted through pipe abort)
	 */
	usbd_transfer_enqueue(xfer);
	return;
}

static void
ohci_root_intr_start(struct usbd_xfer *xfer)
{
	ohci_softc_t *sc = xfer->usb_sc;

	/* only one transfer at a time 
	 * (sc_intrxfer is cleared by ohci_root_intr_done())
	 */
	sc->sc_intrxfer = xfer;
	return;
}

struct usbd_pipe_methods ohci_root_intr_methods = 
{
  .open = ohci_root_intr_open,
  .close = ohci_root_intr_close,
  .enter = ohci_root_intr_enter,
  .start = ohci_root_intr_start,
};

#define ADD_BYTES(ptr,size) ((void *)(((u_int8_t *)(ptr)) + (size)))

static usbd_status
ohci_xfer_setup(struct usbd_device *udev,
		u_int8_t iface_index,
		struct usbd_xfer **pxfer,
		const struct usbd_config *setup_start,
		const struct usbd_config *setup_end)
{
	enum { max_frames = 128 };
	ohci_softc_t *sc = OHCI_BUS2SC(udev->bus);
	const struct usbd_config *setup;
	struct usbd_memory_info *info;
	struct usbd_xfer dummy;
	struct usbd_xfer *xfer;
	u_int32_t physbuffer;
	u_int32_t size;
	u_int32_t total_size;
	u_int32_t ntd;
	u_int32_t nitd;
	u_int32_t nqh;
	u_int32_t n;
	void *buf;
	usbd_status error = 0;

	buf = 0;
	physbuffer = 0;
	total_size = 0;

 repeat:
	size = 0;

	/* align data to 8 byte boundary */
	size += ((-size) & (USB_HOST_ALIGN-1));

	if(buf)
	{
	    info = ADD_BYTES(buf,size);

	    info->memory_base = buf;
	    info->memory_size = total_size;
	    info->usb_mtx = &sc->sc_bus.mtx;
	}
	else
	{
	    info = NULL;
	}

	size += sizeof(info[0]);

	for(setup = setup_start;
	    setup < setup_end;
	    setup++)
	{
	  ntd = 0;
	  nitd = 0;
	  nqh = 0;

	  /* align data to 8 byte boundary */
	  size += ((-size) & (USB_HOST_ALIGN-1));

	  if(buf)
	  {
		*pxfer++ = (xfer = ADD_BYTES(buf,size));
	  }
	  else
	  {
		/* need dummy xfer to 
		 * calculate ntd and nqh !
		 */
		xfer = &dummy;
		bzero(&dummy, sizeof(dummy));
	  }

	  /*
	   * setup xfer
	   */
	  xfer->usb_sc = sc;
	  xfer->usb_mtx = &sc->sc_bus.mtx;
	  xfer->usb_root = info;
	  xfer->flags = setup->flags;
	  xfer->nframes = setup->frames;
	  xfer->timeout = setup->timeout;
	  xfer->callback = setup->callback;

	  __callout_init_mtx(&xfer->timeout_handle, &sc->sc_bus.mtx, 
			     CALLOUT_RETURNUNLOCKED);

	  xfer->pipe = usbd_get_pipe(udev, iface_index, setup);

	  if(!xfer->pipe)
	  {
		/* continue though this error is critical */
		error = USBD_NO_PIPE;
		DPRINTF(("no pipe for endpoint %d\n",
			 setup->endpoint));
	  }
	  else
	  {
		if(xfer->nframes >= max_frames)
		{
			/* continue, though this error is critical */
			error = USBD_INVAL;
			DPRINTF(("isochronous frame-limit "
				 "exceeded by 0x%x frames; endpoint 0x%02x\n",
				 setup->frames - max_frames,
				 setup->endpoint));
		}

		if((xfer->nframes == 0) &&
		   (xfer->pipe->methods == &ohci_device_isoc_methods))
		{
			/* continue, though this error is critical */
			error = USBD_ZERO_FRAMES_IN_ISOC_MODE;
			DPRINTF(("frames == 0 in isochronous mode; "
				 "endpoint 0x%02x\n", setup->endpoint));
		}

		xfer->interval = setup->interval;

		if(xfer->interval == 0)
		{
			xfer->interval = xfer->pipe->edesc->bInterval;
		}

		if(xfer->interval == 0)
		{
			/* one is the smallest interval */
			xfer->interval = 1;
		}

		xfer->address = udev->address;
		xfer->endpoint = xfer->pipe->edesc->bEndpointAddress;
		xfer->max_packet_size = UGETW(xfer->pipe->edesc->wMaxPacketSize);

		xfer->length = setup->bufsize;

		if (xfer->length == 0) {
		    xfer->length = xfer->max_packet_size;
		    if (setup->frames) {
		        xfer->length *= setup->frames;
		    }
		}

		/* wMaxPacketSize is validated by "usbd_fill_iface_data()" */

		/*
		 * calculate ntd and nqh
		 */
		if((xfer->pipe->methods == &ohci_device_ctrl_methods) ||
		   (xfer->pipe->methods == &ohci_device_bulk_methods) ||
		   (xfer->pipe->methods == &ohci_device_intr_methods))
		{
			nitd = 0;
			ntd = (1+ /* SETUP */ 1+ /* STATUS */
			       1+  /* SHORTPKT */ 1 /* EXTRA */) +
			  (xfer->length / (OHCI_PAGE_SIZE/2)) /* DATA */;
		}
		else if(xfer->pipe->methods == &ohci_device_isoc_methods)
		{
			nitd = (xfer->length / OHCI_PAGE_SIZE) +
			  (max_frames / OHCI_ITD_NOFFSET) + 1 /* EXTRA */;
			ntd = 0;
		}
		else
		{
			nitd = 0;
			ntd = 0;
		}

		if((xfer->pipe->methods == &ohci_device_ctrl_methods) ||
		   (xfer->pipe->methods == &ohci_device_bulk_methods) ||
		   (xfer->pipe->methods == &ohci_device_intr_methods) ||
		   (xfer->pipe->methods == &ohci_device_isoc_methods))
		{
			nqh = 1;
		}
		else
		{
			nqh = 0;
		}
	  }

	  size += sizeof(xfer[0]);

	  /* align data to 8 byte boundary */
	  size += ((-size) & (USB_HOST_ALIGN-1));

	  if(buf)
	  {
		xfer->frlengths = ADD_BYTES(buf,size);
	  }

	  size += max_frames * sizeof(xfer->frlengths[0]);

	  /* align data to 8 byte boundary */
	  size += ((-size) & (USB_HOST_ALIGN-1));

	  if(buf)
	  {
		xfer->buffer = ADD_BYTES(buf,size);
		xfer->physbuffer = (physbuffer + size);
	  }

	  size += xfer->length;

	  size += ((-size) & (OHCI_ITD_ALIGN-1)); /* align data */

	  if(buf)
	  {
		xfer->td_start = ADD_BYTES(buf,size);
	  }

	  for(n = 0;
	      n < ntd;
	      n++)
	  {
	    if(buf)
	    {
		/* init TD */
		((ohci_td_t *)ADD_BYTES(buf,size))->td_self = 
		  htole32(physbuffer + size);
	    }
	    size += sizeof(ohci_td_t);
	  }

	  for(n = 0;
	      n < nitd;
	      n++)
	  {
	    if(buf)
	    {
		/* init TD */
		((ohci_itd_t *)ADD_BYTES(buf,size))->itd_self = 
		  htole32(physbuffer + size);
	    }
	    size += sizeof(ohci_itd_t);
	  }

	  if(buf)
	  {
		xfer->td_end = ADD_BYTES(buf,size);
	  }

	  size += ((-size) & (OHCI_ED_ALIGN-1)); /* align data */

	  if(buf)
	  {
		xfer->qh_start = ADD_BYTES(buf,size);
	  }

	  for(n = 0;
	      n < nqh;
	      n++)
	  {
	    if(buf)
	    {
		/* init QH */
		((ohci_ed_t *)ADD_BYTES(buf,size))->ed_self = 
		  htole32(physbuffer + size);
	    }
	    size += sizeof(ohci_ed_t);
	  }

	  if(buf)
	  {
		xfer->qh_end = ADD_BYTES(buf,size);
	  }
	}

	if(!buf && !error)
	{
		/* store total buffer size */
		total_size = size;

		/* allocate zeroed memory */
		buf = usb_alloc_mem(device_get_dma_tag(sc->sc_dev),
				    size, LOG2(OHCI_TD_ALIGN));

		if(!buf)
		{
			error = USBD_NOMEM;
			DPRINTF(("cannot allocate memory block for "
				 "configuration (%d bytes)\n", size));
		}
		else
		{
#if 1
			bzero(buf, size);
#endif
			physbuffer = usb_vtophys(buf, size);
			goto repeat;
		}
	}
	return error;
}

static void
ohci_pipe_init(struct usbd_device *udev, usb_endpoint_descriptor_t *edesc, 
               struct usbd_pipe *pipe)
{
	ohci_softc_t *sc = OHCI_BUS2SC(udev->bus);

	DPRINTFN(1, ("pipe=%p, addr=%d, endpt=%d (%d)\n",
		     pipe, udev->address,
		     edesc->bEndpointAddress, sc->sc_addr));

	if(udev->address == sc->sc_addr)
	{
		switch (edesc->bEndpointAddress)
		{
		case USB_CONTROL_ENDPOINT:
			pipe->methods = &ohci_root_ctrl_methods;
			break;
		case UE_DIR_IN | OHCI_INTR_ENDPT:
			pipe->methods = &ohci_root_intr_methods;
			break;
		default:
			panic("invalid endpoint address: 0x%02x",
			      edesc->bEndpointAddress);
			break;
		}
	}
	else
        {
		switch (edesc->bmAttributes & UE_XFERTYPE)
		{
		case UE_CONTROL:
			pipe->methods = &ohci_device_ctrl_methods;
			break;
		case UE_INTERRUPT:
			pipe->methods = &ohci_device_intr_methods;
			break;
		case UE_ISOCHRONOUS:
			pipe->methods = &ohci_device_isoc_methods;
			break;
		case UE_BULK:
			pipe->methods = &ohci_device_bulk_methods;
			break;
		}
	}
	return;
}

struct usbd_bus_methods ohci_bus_methods = 
{
	.pipe_init  = ohci_pipe_init,
	.xfer_setup = ohci_xfer_setup,
	.do_poll    = ohci_do_poll,
};
