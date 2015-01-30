/*	$NetBSD: uhid.c,v 1.46 2001/11/13 06:24:55 lukem Exp $	*/

/* Also already merged from NetBSD:
 *	$NetBSD: uhid.c,v 1.54 2002/09/23 05:51:21 simonb Exp $
 */

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
 * HID spec: http://www.usb.org/developers/devclass_docs/HID1_11.pdf
 */

#include <sys/cdefs.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/malloc.h>
#include <sys/ioccom.h>
#include <sys/filio.h>
#include <sys/tty.h>
#include <sys/file.h>
#include <sys/vnode.h>
#include <sys/poll.h>

#include <dev/usb2/usb_port.h>
#include <dev/usb2/usb.h>
#include <dev/usb2/usb_subr.h>
#include <dev/usb2/usb_hid.h>
#include <dev/usb2/usb_rdesc.h>
#include <dev/usb2/usb_quirks.h>

#include "usbdevs.h"

__FBSDID("$FreeBSD: src/sys/dev/usb2/uhid.c $");

#ifdef USB_DEBUG
#define DPRINTF(n,fmt,...)						\
  do { if (uhid_debug > (n)) {						\
      printf("%s: " fmt, __FUNCTION__,## __VA_ARGS__); } } while (0)

static int uhid_debug = 0;
SYSCTL_NODE(_hw_usb, OID_AUTO, uhid, CTLFLAG_RW, 0, "USB uhid");
SYSCTL_INT(_hw_usb_uhid, OID_AUTO, debug, CTLFLAG_RW,
	   &uhid_debug, 0, "uhid debug level");
#else
#define DPRINTF(...)
#endif

/* temporary compile hacks for old USB systems: */

#ifndef UQ_HID_IGNORE
#define UQ_HID_IGNORE 0
#endif

#ifndef USB_PRODUCT_WACOM_GRAPHIRE3_4X5
#define USB_PRODUCT_WACOM_GRAPHIRE3_4X5 0
#endif

#define UHID_N_TRANSFER    5 /* units */
#define	UHID_BSIZE	1024 /* bytes, buffer size */
#define	UHID_FRAME_NUM 	  50 /* bytes, frame number */

struct uhid_softc {
	struct usb_cdev         sc_cdev;
	struct mtx              sc_mtx;
	struct usbd_memory_wait sc_mem_wait;

	struct usbd_xfer *      sc_xfer[UHID_N_TRANSFER];
	void *                  sc_repdesc_ptr;

	u_int32_t sc_isize;
	u_int32_t sc_osize;
	u_int32_t sc_fsize;
	u_int32_t sc_repdesc_size;
	u_int32_t sc_transfer_len;

	u_int8_t sc_transfer_buf[sizeof(usb_device_request_t) + UHID_BSIZE];
	u_int8_t sc_iface_no;
	u_int8_t sc_iid;
	u_int8_t sc_oid;
	u_int8_t sc_fid;
	u_int8_t sc_flags;
#define UHID_FLAG_IMMED        0x01 /* set if read should be immediate */
#define UHID_FLAG_INTR_STALL   0x02 /* set if interrupt transfer stalled */
#define UHID_FLAG_STATIC_DESC  0x04 /* set if report descriptors are static */
#define UHID_FLAG_COMMAND_ERR  0x08 /* set if control transfer had an error */
};

static u_int8_t uhid_xb360gp_report_descr[] = { UHID_XB360GP_REPORT_DESCR() };
static u_int8_t uhid_graphire_report_descr[] = { UHID_GRAPHIRE_REPORT_DESCR() };
static u_int8_t uhid_graphire3_4x5_report_descr[] = { UHID_GRAPHIRE3_4X5_REPORT_DESCR() };

static void
uhid_intr_callback(struct usbd_xfer *xfer)
{
	struct uhid_softc *sc = xfer->priv_sc;
	struct usbd_mbuf *m;

	USBD_CHECK_STATUS(xfer);

 tr_transferred:
	DPRINTF(0, "transferred!\n");

	if (xfer->actlen >= sc->sc_isize) {
	    usb_cdev_put_data(&(sc->sc_cdev), 
			      xfer->buffer, sc->sc_isize, 1);
	} else {
	    /* ignore it */
	    DPRINTF(0, "ignored short transfer, "
		    "%d bytes\n", xfer->actlen);
	}

 tr_setup:
	if (sc->sc_flags & UHID_FLAG_INTR_STALL) {
	    usbd_transfer_start(sc->sc_xfer[1]);
	} else {
	    USBD_IF_POLL(&(sc->sc_cdev.sc_rdq_free), m);

	    if (m) {
		usbd_start_hardware(xfer);
	    }
	}
	return;

 tr_error:
	if (xfer->error != USBD_CANCELLED) {
	    /* try to clear stall first */
	    sc->sc_flags |= UHID_FLAG_INTR_STALL;
	    usbd_transfer_start(sc->sc_xfer[1]);
	}
	return;
}

static void
uhid_intr_clear_stall_callback(struct usbd_xfer *xfer)
{
	struct uhid_softc *sc = xfer->priv_sc;
	USBD_CHECK_STATUS(xfer);

 tr_setup:
	/* start clear stall */
	usbd_clear_stall_tr_setup(xfer, sc->sc_xfer[0]);
	return;

 tr_transferred:
	usbd_clear_stall_tr_transferred(xfer, sc->sc_xfer[0]);

	sc->sc_flags &= ~UHID_FLAG_INTR_STALL;
	usbd_transfer_start(sc->sc_xfer[0]);
	return;

 tr_error:
	/* bomb out */
	sc->sc_flags &= ~UHID_FLAG_INTR_STALL;
	usb_cdev_put_data_error(&(sc->sc_cdev));
	return;
}

static void
uhid_write_callback(struct usbd_xfer *xfer)
{
	struct uhid_softc *sc = xfer->priv_sc;
	usb_device_request_t *req = xfer->buffer;
	u_int32_t size = sc->sc_osize;
	u_int32_t actlen;
	u_int8_t id;

	USBD_CHECK_STATUS(xfer);

 tr_transferred:
 tr_setup:
	/* try to extract the ID byte */
	if (sc->sc_oid) {

	    if (usb_cdev_get_data(&(sc->sc_cdev), &id, 1, &actlen, 0)) {
	        if (actlen != 1) {
		    goto tr_error;
		}
	    } else {
	      return;
	    }
	    if (size) {
	        size--;
	    }
	} else {
	  id = 0;
	}

	if (usb_cdev_get_data(&(sc->sc_cdev), req->bData, 
			      UHID_BSIZE, &actlen, 1)) {
	    if (actlen != size) {
	        goto tr_error;
	    }
	    usbd_fill_set_report
	      (req, sc->sc_iface_no,
	       UHID_OUTPUT_REPORT, id, size);

	    xfer->length = sizeof(*req) + size;

	    usbd_start_hardware(xfer);
	}
	return;

 tr_error:
	/* bomb out */
	usb_cdev_get_data_error(&(sc->sc_cdev));
	return;
}

static void
uhid_read_callback(struct usbd_xfer *xfer)
{
	struct uhid_softc *sc = xfer->priv_sc;
	usb_device_request_t *req = xfer->buffer;
	struct usbd_mbuf *m;

	USBD_CHECK_STATUS(xfer);

 tr_transferred:
	usb_cdev_put_data(&(sc->sc_cdev), req->bData, sc->sc_isize, 1);
	return;

 tr_setup:
	USBD_IF_POLL(&(sc->sc_cdev.sc_rdq_free), m);

	if (m) {
	  usbd_fill_get_report
	    (req, sc->sc_iface_no, UHID_INPUT_REPORT, 
	     sc->sc_iid, sc->sc_isize);

	  xfer->length = sizeof(*req) + sc->sc_isize;

	  usbd_start_hardware(xfer);
	}
	return;

 tr_error:
	/* bomb out */
	usb_cdev_put_data_error(&(sc->sc_cdev));
	return;
}

static void
uhid_ioctl_callback(struct usbd_xfer *xfer)
{
	struct uhid_softc *sc = xfer->priv_sc;

	USBD_CHECK_STATUS(xfer);

 tr_transferred:
	bcopy(xfer->buffer, sc->sc_transfer_buf, sc->sc_transfer_len);
	sc->sc_flags &= ~UHID_FLAG_COMMAND_ERR;
	usb_cdev_wakeup(&(sc->sc_cdev));
	return;

 tr_error:
	DPRINTF(0, "error=%s\n", usbd_errstr(xfer->error));
	sc->sc_flags |= UHID_FLAG_COMMAND_ERR;
	usb_cdev_wakeup(&(sc->sc_cdev));
	return;

 tr_setup:
	bcopy(sc->sc_transfer_buf, xfer->buffer, sc->sc_transfer_len);
	xfer->length = sc->sc_transfer_len;
	usbd_start_hardware(xfer);
	return;
}

static const struct usbd_config uhid_config[UHID_N_TRANSFER] = {

    [0] = {
      .type      = UE_INTERRUPT,
      .endpoint  = -1, /* any */
      .direction = UE_DIR_IN,
      .flags     = USBD_SHORT_XFER_OK,
      .bufsize   = 0, /* use wMaxPacketSize */
      .callback  = &uhid_intr_callback,
    },

    [1] = {
      .type      = UE_CONTROL,
      .endpoint  = 0x00, /* Control pipe */
      .direction = -1,
      .bufsize   = sizeof(usb_device_request_t),
      .callback  = &uhid_intr_clear_stall_callback,
      .timeout   = 1000, /* 1 second */
    },

    [2] = {
      .type      = UE_CONTROL,
      .endpoint  = 0x00, /* Control pipe */
      .direction = -1,
      .bufsize   = sizeof(usb_device_request_t) + UHID_BSIZE,
      .callback  = &uhid_write_callback,
      .timeout   = 1000, /* 1 second */
    },

    [3] = {
      .type      = UE_CONTROL,
      .endpoint  = 0x00, /* Control pipe */
      .direction = -1,
      .bufsize   = sizeof(usb_device_request_t) + UHID_BSIZE,
      .callback  = &uhid_read_callback,
      .timeout   = 1000, /* 1 second */
    },

    [4] = {
      .type      = UE_CONTROL,
      .endpoint  = 0x00, /* Control pipe */
      .direction = -1,
      .bufsize   = sizeof(usb_device_request_t) + UHID_BSIZE,
      .callback  = &uhid_ioctl_callback,
      .timeout   = 1000, /* 1 second */
    },
};

static void
uhid_start_read(struct usb_cdev *cdev)
{
	struct uhid_softc *sc = cdev->sc_priv_ptr;

	if (sc->sc_flags & UHID_FLAG_IMMED) {
	    usbd_transfer_start(sc->sc_xfer[3]);
	} else {
	    usbd_transfer_start(sc->sc_xfer[0]);
	}
	return;
}

static void
uhid_stop_read(struct usb_cdev *cdev)
{
	struct uhid_softc *sc = cdev->sc_priv_ptr;

	usbd_transfer_stop(sc->sc_xfer[3]);
	usbd_transfer_stop(sc->sc_xfer[0]);
	return;
}

static void
uhid_start_write(struct usb_cdev *cdev)
{
	struct uhid_softc *sc = cdev->sc_priv_ptr;

	usbd_transfer_start(sc->sc_xfer[2]);
	return;
}

static void
uhid_stop_write(struct usb_cdev *cdev)
{
	struct uhid_softc *sc = cdev->sc_priv_ptr;

	usbd_transfer_stop(sc->sc_xfer[2]);
	return;
}

static int32_t
uhid_do_control_transfer(struct uhid_softc *sc, int32_t fflags)
{
	int32_t error;

	sc->sc_flags |=  UHID_FLAG_COMMAND_ERR;

	usbd_transfer_start(sc->sc_xfer[4]);

	error = usb_cdev_sleep(&(sc->sc_cdev), fflags);

	usbd_transfer_stop(sc->sc_xfer[4]);

	if (error) {
	    return error;
	}

	if (sc->sc_flags & UHID_FLAG_COMMAND_ERR) {
	    return ENXIO;
	}
	return 0;
}

static int32_t
uhid_get_report(struct uhid_softc *sc, int32_t fflags, 
		u_int8_t type, u_int8_t id, void *data, u_int16_t len)
{
	usb_device_request_t *req = (void *)(sc->sc_transfer_buf);
	int error;

	if (len > UHID_BSIZE) {
	    len = UHID_BSIZE;
	}

	usbd_fill_get_report
	  (req, sc->sc_iface_no, type, id, len);

	sc->sc_transfer_len = sizeof(*req) + len;

	error = uhid_do_control_transfer(sc, fflags);

	if (data) {
	    bcopy(req->bData, data, len);
	}
	return error;
}

static int32_t
uhid_set_report(struct uhid_softc *sc, int32_t fflags, 
		u_int8_t type, u_int8_t id, void *data, u_int16_t len)
{
	usb_device_request_t *req = (void *)(sc->sc_transfer_buf);

	if (len > UHID_BSIZE) {
	    len = UHID_BSIZE;
	}

	usbd_fill_set_report
	  (req, sc->sc_iface_no, type, id, len);

	bcopy(data, req->bData, len);

	sc->sc_transfer_len = sizeof(*req) + len;

	return uhid_do_control_transfer(sc, fflags);
}

static int32_t
uhid_open(struct usb_cdev *cdev, int32_t fflags, 
	  int32_t devtype, struct thread *td)
{
	struct uhid_softc *sc = cdev->sc_priv_ptr;

	if (fflags & FREAD) {
	    /* reset flags */
	    sc->sc_flags &= ~UHID_FLAG_IMMED;
	}
	return 0;
}

static int32_t
uhid_ioctl(struct usb_cdev *cdev, u_long cmd, caddr_t addr, 
	   int32_t fflags, struct thread *td)
{
	struct uhid_softc *sc = cdev->sc_priv_ptr;
	struct usb_ctl_report_desc *rd;
	struct usb_ctl_report *re;
	u_int32_t size;
	int32_t error = 0;
	u_int8_t id;

	switch (cmd) {
	case USB_GET_REPORT_DESC:
		rd = (void *)addr;
		size = min(sc->sc_repdesc_size, sizeof(rd->ucrd_data));
		rd->ucrd_size = size;
		bcopy(sc->sc_repdesc_ptr, rd->ucrd_data, size);
		break;

	case USB_SET_IMMED:

		if (!(fflags & FREAD)) {
		    error = EPERM;
		    goto done;
		}

		if (*(int *)addr) {

		    /* do a test read */

		    error = uhid_get_report(sc, fflags, UHID_INPUT_REPORT,
					    sc->sc_iid, NULL, sc->sc_isize);
		    if (error) {
		        goto done;
		    }
		    sc->sc_flags |=  UHID_FLAG_IMMED;
		} else {
		    sc->sc_flags &= ~UHID_FLAG_IMMED;
		}
		break;

	case USB_GET_REPORT:

		if (!(fflags & FREAD)) {
		    error = EPERM;
		    goto done;
		}

		re = (void *)addr;
		switch (re->ucr_report) {
		case UHID_INPUT_REPORT:
			size = sc->sc_isize;
			id = sc->sc_iid;
			break;
		case UHID_OUTPUT_REPORT:
			size = sc->sc_osize;
			id = sc->sc_oid;
			break;
		case UHID_FEATURE_REPORT:
			size = sc->sc_fsize;
			id = sc->sc_fid;
			break;
		default:
			error = EINVAL;
			goto done;
		}
		error = uhid_get_report(sc, fflags, re->ucr_report, id, 
					re->ucr_data, size);
		if (error) {
		    goto done;
		}
		break;

	case USB_SET_REPORT:

		if (!(fflags & FWRITE)) {
		    error = EPERM;
		    goto done;
		}

		re = (void *)addr;
		switch (re->ucr_report) {
		case UHID_INPUT_REPORT:
			size = sc->sc_isize;
			id = sc->sc_iid;
			break;
		case UHID_OUTPUT_REPORT:
			size = sc->sc_osize;
			id = sc->sc_oid;
			break;
		case UHID_FEATURE_REPORT:
			size = sc->sc_fsize;
			id = sc->sc_fid;
			break;
		default:
			return (EINVAL);
		}
		error = uhid_set_report(sc, fflags, re->ucr_report, id, 
					re->ucr_data, size);
		if (error) {
		  goto done;
		}
		break;

	case USB_GET_REPORT_ID:
		*(int *)addr = 0; /* XXX: we only support reportid 0? */
		break;

	default:
		error = EINVAL;
		break;
	}

 done:
	return error;
}

static device_probe_t uhid_probe;
static device_attach_t uhid_attach;
static device_detach_t uhid_detach;

static int
uhid_probe(device_t dev)
{
	struct usb_attach_arg *uaa = device_get_ivars(dev);
	usb_interface_descriptor_t *id;

	DPRINTF(10, "\n");

	if (uaa->iface == NULL) {
	    return UMATCH_NONE;
	}

	id = usbd_get_interface_descriptor(uaa->iface);
	if (id == NULL) {
	    return UMATCH_NONE;
	}

	if  (id->bInterfaceClass != UICLASS_HID) {

	    /* the Xbox 360 gamepad doesn't use the HID class */

	    if ((id->bInterfaceClass != UICLASS_VENDOR) ||
		(id->bInterfaceSubClass != UISUBCLASS_XBOX360_CONTROLLER) ||
		(id->bInterfaceProtocol != UIPROTO_XBOX360_GAMEPAD)) {
	        return UMATCH_NONE;
	    }
	}

	if (usbd_get_quirks(uaa->device)->uq_flags & UQ_HID_IGNORE) {
	    return UMATCH_NONE;
	}

	return UMATCH_IFACECLASS_GENERIC;
}

static int
uhid_attach(device_t dev)
{
	struct usb_attach_arg *uaa = device_get_ivars(dev);
	struct uhid_softc *sc = device_get_softc(dev);
	usb_interface_descriptor_t *id = 
	  usbd_get_interface_descriptor(uaa->iface);
	const char * p_buf[2];
	int32_t unit = device_get_unit(dev);
	int32_t error = 0;
	char buf[16];

	DPRINTF(10, "sc=%p\n", sc);

	if (sc == NULL) {
	    return ENOMEM;
	}

	usbd_set_desc(dev, uaa->device);

	mtx_init(&(sc->sc_mtx), "uhid lock", NULL, MTX_DEF|MTX_RECURSE);

	sc->sc_iface_no = uaa->iface->idesc->bInterfaceNumber;

	error = usbd_transfer_setup(uaa->device, uaa->iface_index, 
				    sc->sc_xfer, uhid_config, UHID_N_TRANSFER,
				    sc, &(sc->sc_mtx), &(sc->sc_mem_wait));
	if (error) {
	    DPRINTF(0, "error=%s\n", usbd_errstr(error)) ;
	    goto detach;
	}

	if (uaa->vendor == USB_VENDOR_WACOM) {

	    /* the report descriptor for the Wacom Graphire is broken */

	    if (uaa->product == USB_PRODUCT_WACOM_GRAPHIRE) {

	        sc->sc_repdesc_size = sizeof(uhid_graphire_report_descr);
		sc->sc_repdesc_ptr = uhid_graphire_report_descr;
		sc->sc_flags |= UHID_FLAG_STATIC_DESC;

	    } else if (uaa->product == USB_PRODUCT_WACOM_GRAPHIRE3_4X5) {

	        static u_int8_t reportbuf[] = { 2, 2, 2 };

		/*
		 * The Graphire3 needs 0x0202 to be written to
		 * feature report ID 2 before it'll start
		 * returning digitizer data.
		 */
		error = usbreq_set_report
		  (uaa->device, uaa->iface_index, 
		   UHID_FEATURE_REPORT, 2,
		   reportbuf, sizeof(reportbuf));

		if (error) {
		    DPRINTF(0, "set report failed, error=%s (ignored)\n", 
			    usbd_errstr(error));
		}

		sc->sc_repdesc_size = sizeof(uhid_graphire3_4x5_report_descr);
		sc->sc_repdesc_ptr = uhid_graphire3_4x5_report_descr;
		sc->sc_flags |= UHID_FLAG_STATIC_DESC;
	    }
	} else if ((id->bInterfaceClass == UICLASS_VENDOR) &&
		   (id->bInterfaceSubClass == UISUBCLASS_XBOX360_CONTROLLER) &&
		   (id->bInterfaceProtocol == UIPROTO_XBOX360_GAMEPAD)) {

	    /* the Xbox 360 gamepad has no report descriptor */
	    sc->sc_repdesc_size = sizeof(uhid_xb360gp_report_descr);
	    sc->sc_repdesc_ptr = uhid_xb360gp_report_descr;
	    sc->sc_flags |= UHID_FLAG_STATIC_DESC;
	}

	if (sc->sc_repdesc_ptr == NULL) {

	    error = usbreq_read_report_desc
	      (uaa->device, uaa->iface_index, 
	       &(sc->sc_repdesc_ptr), &(sc->sc_repdesc_size), M_USBDEV);

	    if (error) {
	        device_printf(dev, "no report descriptor\n");
		goto detach;
	    }
	}

	error = usbreq_set_idle(uaa->device, uaa->iface_index, 0, 0);

	if (error) {
	    DPRINTF(0, "set idle failed, error=%s (ignored)\n", 
		    usbd_errstr(error));
	}

	sc->sc_isize = hid_report_size
	  (sc->sc_repdesc_ptr, sc->sc_repdesc_size, hid_input,   &sc->sc_iid);

	sc->sc_osize = hid_report_size
	  (sc->sc_repdesc_ptr, sc->sc_repdesc_size, hid_output,  &sc->sc_oid);

	sc->sc_fsize = hid_report_size
	  (sc->sc_repdesc_ptr, sc->sc_repdesc_size, hid_feature, &sc->sc_fid);

	if (sc->sc_isize > UHID_BSIZE) {
	    DPRINTF(0, "input size is too large, "
		    "%d bytes (truncating)\n", 
		    sc->sc_isize);
	    sc->sc_isize = UHID_BSIZE;
	}
	if (sc->sc_osize > UHID_BSIZE) {
	    DPRINTF(0, "output size is too large, "
		    "%d bytes (truncating)\n", 
		    sc->sc_osize);
	    sc->sc_osize = UHID_BSIZE;
	}
	if (sc->sc_fsize > UHID_BSIZE) {
	    DPRINTF(0, "feature size is too large, "
		    "%d bytes (truncating)\n", 
		    sc->sc_fsize);
	    sc->sc_fsize = UHID_BSIZE;
	}

	snprintf(buf, sizeof(buf), "uhid%d", unit);

	p_buf[0] = buf;
	p_buf[1] = NULL;

	sc->sc_cdev.sc_start_read = &uhid_start_read;
	sc->sc_cdev.sc_start_write = &uhid_start_write;
	sc->sc_cdev.sc_stop_read = &uhid_stop_read;
	sc->sc_cdev.sc_stop_write = &uhid_stop_write;
	sc->sc_cdev.sc_open = &uhid_open;
	sc->sc_cdev.sc_ioctl = &uhid_ioctl;
	sc->sc_cdev.sc_flags |= (USB_CDEV_FLAG_FWD_SHORT|
				 USB_CDEV_FLAG_WAKEUP_RD_IMMED|
				 USB_CDEV_FLAG_WAKEUP_WR_IMMED);

	/* make the buffers one byte larger than maximum so
	 * that one can detect too large read/writes and 
	 * short transfers:
	 */
	error = usb_cdev_attach(&(sc->sc_cdev), sc, &(sc->sc_mtx), p_buf,
				UID_ROOT, GID_OPERATOR, 0644, 
				sc->sc_isize+1, UHID_FRAME_NUM,
				sc->sc_osize+1, UHID_FRAME_NUM);
	if (error) {
	    goto detach;
	}
	return 0; /* success */

 detach:
	uhid_detach(dev);
	return ENOMEM;
}

static int
uhid_detach(device_t dev)
{
	struct uhid_softc *sc = device_get_softc(dev);

	usb_cdev_detach(&(sc->sc_cdev));

	usbd_transfer_unsetup(sc->sc_xfer, UHID_N_TRANSFER);

	if (sc->sc_repdesc_ptr) {
	    if (!(sc->sc_flags & UHID_FLAG_STATIC_DESC)) {
	        free(sc->sc_repdesc_ptr, M_USBDEV);
	    }
	}

	usbd_transfer_drain(&(sc->sc_mem_wait), &(sc->sc_mtx));

	mtx_destroy(&(sc->sc_mtx));

	return 0;
}

static devclass_t uhid_devclass;

static device_method_t uhid_methods[] = {
    DEVMETHOD(device_probe, uhid_probe),
    DEVMETHOD(device_attach, uhid_attach),
    DEVMETHOD(device_detach, uhid_detach),
    { 0, 0 }
};

static driver_t uhid_driver = {
  .name    = "uhid",
  .methods = uhid_methods,
  .size    = sizeof(struct uhid_softc),
};

DRIVER_MODULE(uhid, uhub, uhid_driver, uhid_devclass, usbd_driver_load, 0);
MODULE_DEPEND(uhid, usb, 1, 1, 1);
