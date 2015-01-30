/*	$NetBSD: ulpt.c,v 1.60 2003/10/04 21:19:50 augustss Exp $	*/

/*-
 * Copyright (c) 1998, 2003 The NetBSD Foundation, Inc.
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
 * Printer Class spec: http://www.usb.org/developers/data/devclass/usbprint109.PDF
 * Printer Class spec: http://www.usb.org/developers/devclass_docs/usbprint11.pdf
 */

#include "netbas.h"


#include <dev/usb2/usb_port.h>
#include <dev/usb2/usb.h>
#include <dev/usb2/usb_subr.h>
#include <dev/usb2/usb_quirks.h>

__FBSDID("$FreeBSD: src/sys/dev/usb2/ulpt.c $");

#ifdef USB_DEBUG
#define DPRINTF(n,fmt,...)						\
  do { if (ulpt_debug > (n)) {						\
      printf("%s: " fmt, __FUNCTION__,## __VA_ARGS__); } } while (0)

static int ulpt_debug = 0;
SYSCTL_NODE(_hw_usb, OID_AUTO, ulpt, CTLFLAG_RW, 0, "USB ulpt");
SYSCTL_INT(_hw_usb_ulpt, OID_AUTO, debug, CTLFLAG_RW,
	   &ulpt_debug, 0, "ulpt debug level");
#else
#define DPRINTF(...)
#endif

#define	ULPT_BSIZE		(1<<17) /* bytes */
#define ULPT_IFQ_MAXLEN         2 /* units */
#define ULPT_WATCHDOG_INTERVAL  5 /* times per second */
#define ULPT_N_TRANSFER         6 /* units */

#define UR_GET_DEVICE_ID        0x00
#define UR_GET_PORT_STATUS      0x01
#define UR_SOFT_RESET           0x02

#define	LPS_NERR		0x08	/* printer no error */
#define	LPS_SELECT		0x10	/* printer selected */
#define	LPS_NOPAPER		0x20	/* printer out of paper */
#define LPS_INVERT      (LPS_SELECT|LPS_NERR)
#define LPS_MASK        (LPS_SELECT|LPS_NERR|LPS_NOPAPER)

struct ulpt_softc {
	struct usb_cdev         sc_cdev;
	struct __callout	sc_watchdog;
	spinlock_t		sc_mtx;
	struct usbd_memory_wait sc_mem_wait;

	device_t		sc_dev;
	struct usbd_xfer *	sc_xfer[ULPT_N_TRANSFER];

	u_int8_t		sc_flags;
#define ULPT_FLAG_NO_READ       0x01 /* device has no read endpoint */
#define ULPT_FLAG_DUMP_READ     0x02 /* device is not opened for read */
#define ULPT_FLAG_READ_STALL    0x04 /* read transfer stalled */
#define ULPT_FLAG_WRITE_STALL   0x08 /* write transfer stalled */
#define ULPT_FLAG_RESETTING     0x10 /* device is resetting */

	u_int8_t		sc_iface_no;
	u_int8_t		sc_last_status;
};

static void
ulpt_watchdog(void *__sc)
{
	struct ulpt_softc *sc = __sc;

	mtx_assert(&(sc->sc_mtx), MA_OWNED);

	DPRINTF(2, "start sc=%p\n", sc);

	/* start reading of status, if not already started */

	usbd_transfer_start(sc->sc_xfer[2]);

	if ((sc->sc_flags & (ULPT_FLAG_NO_READ|
			     ULPT_FLAG_DUMP_READ)) &&
	    (!(sc->sc_flags & ULPT_FLAG_RESETTING)) &&
	    (sc->sc_cdev.sc_flags & (USB_CDEV_FLAG_OPEN_READ|
				     USB_CDEV_FLAG_OPEN_WRITE)) &&
	    (!(sc->sc_cdev.sc_flags & USB_CDEV_FLAG_CLOSING_READ))) {

	    /* start reading of data, if not already started */

	    usbd_transfer_start(sc->sc_xfer[1]);
	}

	__callout_reset(&(sc->sc_watchdog), 
			hz / ULPT_WATCHDOG_INTERVAL, 
			&ulpt_watchdog, sc);

	mtx_unlock(&(sc->sc_mtx));

	return;
}

static void
ulpt_write_callback(struct usbd_xfer *xfer)
{
	struct ulpt_softc *sc = xfer->priv_sc;
	u_int32_t actlen;

	USBD_CHECK_STATUS(xfer);

 tr_transferred:
 tr_setup:
	if (sc->sc_flags & ULPT_FLAG_WRITE_STALL) {
	    usbd_transfer_start(sc->sc_xfer[4]);
	    return;
	}
	if (usb_cdev_get_data(&(sc->sc_cdev), xfer->buffer, 
			      ULPT_BSIZE, &actlen, 0)) {

	    xfer->length = actlen;
	    usbd_start_hardware(xfer);
	}
	return;

 tr_error:
	if (xfer->error != USBD_CANCELLED) {
	    /* try to clear stall first */
	    sc->sc_flags |= ULPT_FLAG_WRITE_STALL;
	    usbd_transfer_start(sc->sc_xfer[4]);
	}
	return;
}

static void
ulpt_write_clear_stall_callback(struct usbd_xfer *xfer)
{
	struct ulpt_softc *sc = xfer->priv_sc;

	USBD_CHECK_STATUS(xfer);

 tr_setup:
	/* start clear stall */
	usbd_clear_stall_tr_setup(xfer, sc->sc_xfer[0]);
	return;

 tr_transferred:
	usbd_clear_stall_tr_transferred(xfer, sc->sc_xfer[0]);

	sc->sc_flags &= ~ULPT_FLAG_WRITE_STALL;
	usbd_transfer_start(sc->sc_xfer[0]);
	return;

 tr_error:
	/* bomb out */
	sc->sc_flags &= ~ULPT_FLAG_WRITE_STALL;
	usb_cdev_get_data_error(&(sc->sc_cdev));
	return;
}

static void
ulpt_read_callback(struct usbd_xfer *xfer)
{
	struct ulpt_softc *sc = xfer->priv_sc;
	struct usbd_mbuf *m;

	USBD_CHECK_STATUS(xfer);

 tr_transferred:
	if (sc->sc_flags & (ULPT_FLAG_NO_READ|ULPT_FLAG_DUMP_READ)) {
	    return;
	}

	usb_cdev_put_data(&(sc->sc_cdev), xfer->buffer, xfer->actlen, 1);

 tr_setup:
	if (sc->sc_flags & ULPT_FLAG_READ_STALL) {
	    usbd_transfer_start(sc->sc_xfer[5]);
	    return;
	}

	USBD_IF_POLL(&sc->sc_cdev.sc_rdq_free, m);

	if (m) {
	    usbd_start_hardware(xfer);
	}
	return;

 tr_error:
	if (xfer->error != USBD_CANCELLED) {
	    /* try to clear stall first */
	    sc->sc_flags |= ULPT_FLAG_READ_STALL;
	    usbd_transfer_start(sc->sc_xfer[5]);
	}
	return;
}

static void
ulpt_read_clear_stall_callback(struct usbd_xfer *xfer)
{
	struct ulpt_softc *sc = xfer->priv_sc;

	USBD_CHECK_STATUS(xfer);

 tr_setup:
	/* start clear stall */
	usbd_clear_stall_tr_setup(xfer, sc->sc_xfer[1]);
	return;

 tr_transferred:
	usbd_clear_stall_tr_transferred(xfer, sc->sc_xfer[1]);

	sc->sc_flags &= ~ULPT_FLAG_READ_STALL;
	usbd_transfer_start(sc->sc_xfer[1]);
	return;

 tr_error:
	/* bomb out */
	sc->sc_flags &= ~ULPT_FLAG_READ_STALL;
	usb_cdev_put_data_error(&(sc->sc_cdev));
	return;
}

static void
ulpt_status_callback(struct usbd_xfer *xfer)
{
	struct ulpt_softc *sc = xfer->priv_sc;
	usb_device_request_t *req = xfer->buffer;
	u_int8_t cur_status = req->bData[0];
	u_int8_t new_status;

	USBD_CHECK_STATUS(xfer);

 tr_transferred:
	cur_status = (cur_status ^ LPS_INVERT) & LPS_MASK;
	new_status = cur_status & ~sc->sc_last_status;
	sc->sc_last_status = cur_status;

	if (new_status & LPS_SELECT)
		log(LOG_NOTICE, "%s: offline\n", 
		    device_get_nameunit(sc->sc_dev));
	else if (new_status & LPS_NOPAPER)
		log(LOG_NOTICE, "%s: out of paper\n", 
		    device_get_nameunit(sc->sc_dev));
	else if (new_status & LPS_NERR)
		log(LOG_NOTICE, "%s: output error\n", 
		    device_get_nameunit(sc->sc_dev));
	return;

 tr_setup:
	req->bmRequestType = UT_READ_CLASS_INTERFACE;
	req->bRequest = UR_GET_PORT_STATUS;
	USETW(req->wValue, 0);
	USETW(req->wIndex, sc->sc_iface_no);
	USETW(req->wLength, 1);

	usbd_start_hardware(xfer);

	return;

 tr_error:
	DPRINTF(0, "error=%s\n", usbd_errstr(xfer->error));
	return;
}

static void
ulpt_reset_callback(struct usbd_xfer *xfer)
{
	struct ulpt_softc *sc = xfer->priv_sc;
	usb_device_request_t *req = xfer->buffer;

	USBD_CHECK_STATUS(xfer);

 tr_error:
	if (xfer->error == USBD_CANCELLED) {
	    return;
	}

	if (req->bmRequestType == UT_WRITE_CLASS_OTHER) {
	    /*
	     * There was a mistake in the USB printer 1.0 spec that
	     * gave the request type as UT_WRITE_CLASS_OTHER; it
	     * should have been UT_WRITE_CLASS_INTERFACE.  Many
	     * printers use the old one, so try both:
	     */
	    req->bmRequestType = UT_WRITE_CLASS_INTERFACE; /* 1.1 */
	    req->bRequest = UR_SOFT_RESET;
	    USETW(req->wValue, 0);
	    USETW(req->wIndex, sc->sc_iface_no);
	    USETW(req->wLength, 0);

	    usbd_start_hardware(xfer);

	    return;
	}

 tr_transferred:
	usb_cdev_wakeup(&(sc->sc_cdev));
	return;

 tr_setup:
	req->bmRequestType = UT_WRITE_CLASS_OTHER; /* 1.0 */
	req->bRequest = UR_SOFT_RESET;
	USETW(req->wValue, 0);
	USETW(req->wIndex, sc->sc_iface_no);
	USETW(req->wLength, 0);

	usbd_start_hardware(xfer);

	return;
}

static const struct usbd_config ulpt_config[ULPT_N_TRANSFER] = {
    [0] = {
      .type      = UE_BULK,
      .endpoint  = -1, /* any */
      .direction = UE_DIR_OUT,
      .bufsize   = ULPT_BSIZE,
      .flags     = 0,
      .callback  = &ulpt_write_callback,
    },

    [1] = {
      .type      = UE_BULK,
      .endpoint  = -1, /* any */
      .direction = UE_DIR_IN,
      .bufsize   = ULPT_BSIZE,
      .flags     = USBD_SHORT_XFER_OK,
      .callback  = &ulpt_read_callback,
    },

    [2] = {
      .type      = UE_CONTROL,
      .endpoint  = 0x00, /* Control pipe */
      .direction = -1,
      .bufsize   = sizeof(usb_device_request_t) + 1,
      .callback  = &ulpt_status_callback,
      .timeout   = 1000, /* 1 second */
    },

    [3] = {
      .type      = UE_CONTROL,
      .endpoint  = 0x00, /* Control pipe */
      .direction = -1,
      .bufsize   = sizeof(usb_device_request_t),
      .callback  = &ulpt_reset_callback,
      .timeout   = 1000, /* 1 second */
    },

    [4] = {
      .type      = UE_CONTROL,
      .endpoint  = 0x00, /* Control pipe */
      .direction = -1,
      .bufsize   = sizeof(usb_device_request_t),
      .callback  = &ulpt_write_clear_stall_callback,
      .timeout   = 1000, /* 1 second */
    },

    [5] = {
      .type      = UE_CONTROL,
      .endpoint  = 0x00, /* Control pipe */
      .direction = -1,
      .bufsize   = sizeof(usb_device_request_t),
      .callback  = &ulpt_read_clear_stall_callback,
      .timeout   = 1000, /* 1 second */
    },
};

static void
ulpt_start_read(struct usb_cdev *cdev)
{
	struct ulpt_softc *sc = cdev->sc_priv_ptr;
	usbd_transfer_start(sc->sc_xfer[1]);
	return;
}

static void
ulpt_stop_read(struct usb_cdev *cdev)
{
	struct ulpt_softc *sc = cdev->sc_priv_ptr;
	usbd_transfer_stop(sc->sc_xfer[5]);
	usbd_transfer_stop(sc->sc_xfer[1]);
	return;
}

static void
ulpt_start_write(struct usb_cdev *cdev)
{
	struct ulpt_softc *sc = cdev->sc_priv_ptr;
	usbd_transfer_start(sc->sc_xfer[0]);
	return;
}

static void
ulpt_stop_write(struct usb_cdev *cdev)
{
	struct ulpt_softc *sc = cdev->sc_priv_ptr;
	usbd_transfer_stop(sc->sc_xfer[4]);
	usbd_transfer_stop(sc->sc_xfer[0]);
	return;
}

static int32_t
ulpt_open(struct usb_cdev *cdev, int32_t fflags,
	  int32_t devtype, struct thread *td)
{
	u_int8_t prime = ((cdev->sc_last_cdev == cdev->sc_cdev[0]) &&
			  (cdev->sc_first_open));
	struct ulpt_softc *sc = cdev->sc_priv_ptr;
	int32_t error = 0;

	if (prime) {
	    DPRINTF(0, "opening prime device (reset)\n");

	    sc->sc_flags |= ULPT_FLAG_RESETTING;

	    usbd_transfer_start(sc->sc_xfer[3]);

	    error = usb_cdev_sleep(&(sc->sc_cdev), fflags);

	    usbd_transfer_stop(sc->sc_xfer[3]);

	    sc->sc_flags &= ~ULPT_FLAG_RESETTING;

	    if (error) {
	        goto done;
	    }
	}

	if (cdev->sc_flags & USB_CDEV_FLAG_OPEN_READ) {
	    sc->sc_flags &= ~ULPT_FLAG_DUMP_READ;
	} else {
	    sc->sc_flags |=  ULPT_FLAG_DUMP_READ;
	}
 done:
	return error;
}

static int32_t
ulpt_ioctl(struct usb_cdev *cdev, u_long cmd, caddr_t data, 
	   int32_t fflags, struct thread *td)
{
	return ENODEV;
}


/* prototypes */

static device_probe_t ulpt_probe;
static device_attach_t ulpt_attach;
static device_detach_t ulpt_detach;

static int
ulpt_probe(device_t dev)
{
	struct usb_attach_arg *uaa = device_get_ivars(dev);
	usb_interface_descriptor_t *id;

	DPRINTF(10, "\n");

	if (uaa->iface == NULL) {
		return UMATCH_NONE;
	}

	id = usbd_get_interface_descriptor(uaa->iface);

	if ((id != NULL) &&
	    (id->bInterfaceClass == UICLASS_PRINTER) &&
	    (id->bInterfaceSubClass == UISUBCLASS_PRINTER) &&
	    ((id->bInterfaceProtocol == UIPROTO_PRINTER_UNI) ||
	     (id->bInterfaceProtocol == UIPROTO_PRINTER_BI) ||
	     (id->bInterfaceProtocol == UIPROTO_PRINTER_1284))) {
		return UMATCH_IFACECLASS_IFACESUBCLASS_IFACEPROTO;
	}
	return UMATCH_NONE;
}

static int
ulpt_attach(device_t dev)
{
	struct usb_attach_arg *uaa = device_get_ivars(dev);
	struct ulpt_softc *sc = device_get_softc(dev);
	struct usbd_interface *iface_ptr = uaa->iface;
	usb_interface_descriptor_t *id;
	const char * p_buf[3];
	int32_t iface_index = uaa->iface_index;
	int32_t iface_alt_index = 0;
	int32_t unit = device_get_unit(dev);
	int32_t error;
	char buf_1[16];
	char buf_2[16];

	DPRINTF(10, "sc=%p\n", sc);

	sc->sc_dev = dev;

	usbd_set_desc(dev, uaa->device);

	mtx_init(&(sc->sc_mtx), "ulpt lock", NULL, MTX_DEF|MTX_RECURSE);

	__callout_init_mtx(&(sc->sc_watchdog),
			   &(sc->sc_mtx), CALLOUT_RETURNUNLOCKED);

	/* search through all the descriptors looking for bidir mode */

	while(iface_alt_index < 32) {

	    error = usbd_fill_iface_data
	      (uaa->device, iface_index, iface_alt_index);

	    if (error) {
	        DPRINTF(0, "end of alternate settings, "
			"error=%s\n", usbd_errstr(error));
	        goto detach;
	    }

	    id = usbd_get_interface_descriptor(iface_ptr);

	    if ((id->bInterfaceClass == UICLASS_PRINTER) &&
		(id->bInterfaceSubClass == UISUBCLASS_PRINTER) &&
		(id->bInterfaceProtocol == UIPROTO_PRINTER_BI)) {
	        goto found;
	    }

	    iface_alt_index++;
	}
	goto detach;

 found:

	DPRINTF(0, "setting alternate "
		"config number: %d\n", iface_alt_index);

	if (iface_alt_index) {

	    error = usbreq_set_interface
	      (uaa->device, iface_index, iface_alt_index);

	    if (error) {
	        DPRINTF(0, "could not set alternate "
			"config, error=%s\n", usbd_errstr(error));
	        goto detach;
	    }
	}

	sc->sc_iface_no = id->bInterfaceNumber;

	error = usbd_transfer_setup(uaa->device, iface_index, 
				    sc->sc_xfer, ulpt_config, ULPT_N_TRANSFER, 
				    sc, &(sc->sc_mtx), &(sc->sc_mem_wait));
	if (error) {
	    DPRINTF(0, "error=%s\n", usbd_errstr(error)) ;
	    goto detach;
	}

	if (usbd_get_quirks(uaa->device)->uq_flags & UQ_BROKEN_BIDIR) {
		/* this device doesn't handle reading properly. */
		sc->sc_flags |= ULPT_FLAG_NO_READ;
	}

	device_printf(sc->sc_dev, "using %s-directional mode\n",
		      (sc->sc_flags & ULPT_FLAG_NO_READ) ? "uni" : "bi");


#if 0
/*
 * This code is disabled because for some mysterious reason it causes
 * printing not to work.  But only sometimes, and mostly with
 * UHCI and less often with OHCI.  *sigh*
 */
	{
	usb_config_descriptor_t *cd = usbd_get_config_descriptor(dev);
	usb_device_request_t req;
	int len, alen;

	req.bmRequestType = UT_READ_CLASS_INTERFACE;
	req.bRequest = UR_GET_DEVICE_ID;
	USETW(req.wValue, cd->bConfigurationValue);
	USETW2(req.wIndex, id->bInterfaceNumber, id->bAlternateSetting);
	USETW(req.wLength, sizeof devinfo - 1);
	error = usbd_do_request_flags(dev, &req, devinfo, USBD_SHORT_XFER_OK,
		  &alen, USBD_DEFAULT_TIMEOUT);
	if (error) {
	    device_printf(sc->sc_dev, "cannot get device id\n");
	} else if (alen <= 2) {
	    device_printf(sc->sc_dev, "empty device id, no "
			  "printer connected?\n");
	} else {
		/* devinfo now contains an IEEE-1284 device ID */
		len = ((devinfo[0] & 0xff) << 8) | (devinfo[1] & 0xff);
		if (len > sizeof devinfo - 3)
			len = sizeof devinfo - 3;
		devinfo[len] = 0;
		printf("%s: device id <", device_get_nameunit(sc->sc_dev));
		ieee1284_print_id(devinfo+2);
		printf(">\n");
	}
	}
#endif

	snprintf(buf_1, sizeof(buf_1), "ulpt%d", unit);
	snprintf(buf_2, sizeof(buf_2), "unlpt%d", unit);

	p_buf[0] = buf_1;
	p_buf[1] = buf_2;
	p_buf[2] = NULL;

	sc->sc_cdev.sc_start_read = &ulpt_start_read;
	sc->sc_cdev.sc_start_write = &ulpt_start_write;
	sc->sc_cdev.sc_stop_read = &ulpt_stop_read;
	sc->sc_cdev.sc_stop_write = &ulpt_stop_write;
	sc->sc_cdev.sc_open = &ulpt_open;
	sc->sc_cdev.sc_ioctl = &ulpt_ioctl;
	sc->sc_cdev.sc_flags |= (USB_CDEV_FLAG_FWD_SHORT|
				 USB_CDEV_FLAG_WAKEUP_RD_IMMED|
				 USB_CDEV_FLAG_WAKEUP_WR_IMMED);

	error = usb_cdev_attach(&(sc->sc_cdev), sc, &(sc->sc_mtx), p_buf,
				UID_ROOT, GID_OPERATOR, 0644, 
				ULPT_BSIZE, ULPT_IFQ_MAXLEN,
				ULPT_BSIZE, ULPT_IFQ_MAXLEN);
	if (error) {
	    goto detach;
	}

	/* start watchdog (returns unlocked) */

	mtx_lock(&(sc->sc_mtx));

	ulpt_watchdog(sc);

	return 0;

 detach:
	ulpt_detach(dev);
	return ENOMEM;
}

static int
ulpt_detach(device_t dev)
{
	struct ulpt_softc *sc = device_get_softc(dev);

	DPRINTF(0, "sc=%p\n", sc);

	usb_cdev_detach(&(sc->sc_cdev));

	mtx_lock(&(sc->sc_mtx));
	__callout_stop(&(sc->sc_watchdog));
	mtx_unlock(&(sc->sc_mtx));

	usbd_transfer_unsetup(sc->sc_xfer, ULPT_N_TRANSFER);

	usbd_transfer_drain(&(sc->sc_mem_wait), &(sc->sc_mtx));

	mtx_destroy(&(sc->sc_mtx));

	return 0;
}

#if 0
/* XXX This does not belong here. */
/*
 * Print select parts of an IEEE 1284 device ID.
 */
void
ieee1284_print_id(char *str)
{
	char *p, *q;

	for (p = str-1; p; p = strchr(p, ';')) {
		p++;		/* skip ';' */
		if (strncmp(p, "MFG:", 4) == 0 ||
		    strncmp(p, "MANUFACTURER:", 14) == 0 ||
		    strncmp(p, "MDL:", 4) == 0 ||
		    strncmp(p, "MODEL:", 6) == 0) {
			q = strchr(p, ';');
			if (q)
				printf("%.*s", (int)(q - p + 1), p);
		}
	}
}
#endif

static devclass_t ulpt_devclass;

static device_method_t ulpt_methods[] = {
    DEVMETHOD(device_probe, ulpt_probe),
    DEVMETHOD(device_attach, ulpt_attach),
    DEVMETHOD(device_detach, ulpt_detach),
    { 0, 0 }
};

static driver_t ulpt_driver = {
    .name    = "ulpt",
    .methods = ulpt_methods,
    .size    = sizeof(struct ulpt_softc),
};

DRIVER_MODULE(ulpt, uhub, ulpt_driver, ulpt_devclass, usbd_driver_load, 0);
MODULE_DEPEND(ulpt, usb, 1, 1, 1);
