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

#include "netbas.h"


#include <usb_port.h>
#include <usb.h>
#include <usb_subr.h>
#include <usb_hid.h>
//#include <usb_quirks.h>

__FBSDID("$FreeBSD: src/sys/dev/usb2/usbdi_util.c $");

usbd_status
usbreq_reset_port(struct usbd_device *udev, int port, usb_port_status_t *ps)
{
	usb_device_request_t req;
	usbd_status err;
	int n;

	req.bmRequestType = UT_WRITE_CLASS_OTHER;
	req.bRequest = UR_SET_FEATURE;
	USETW(req.wValue, UHF_PORT_RESET);
	USETW(req.wIndex, port);
	USETW(req.wLength, 0);
	err = usbd_do_request(udev, &req, 0);

	PRINTFN(1,("port %d reset done, error=%s\n",
		    port, usbd_errstr(err)));

	if(err)
	{
		goto done;
	}

	n = 10;
	do {
		/* wait for device to recover from reset */
		usbd_delay_ms(udev, USB_PORT_RESET_DELAY);
		err = usbreq_get_port_status(udev, port, ps);

		if(err)
		{
			PRINTF(("get status failed %d\n",
				 err));
			goto done;
		}

		/* if the device disappeared, just give up */
		if(!(UGETW(ps->wPortStatus) & UPS_CURRENT_CONNECT_STATUS))
		{
			err = USBD_NORMAL_COMPLETION;
			goto done;
		}
	} while (((UGETW(ps->wPortChange) & UPS_C_PORT_RESET) == 0) && (--n > 0));

	if(n == 0)
	{
		err = USBD_TIMEOUT;
		goto done;
	}

	err = usbreq_clear_port_feature(udev, port, UHF_C_PORT_RESET);
#ifdef USB_DEBUG
	if(err)
	{
		PRINTF(("clear port feature failed %d\n",
			 err));
	}
#endif

	/* wait for the device to recover from reset */
	usbd_delay_ms(udev, USB_PORT_RESET_RECOVERY);
 done:
	return (err);
}

usbd_status
usbreq_get_desc(struct usbd_device *udev, int type, int index,
		int len, void *desc, int timeout)
{
	usb_device_request_t req;
	usbd_status err;

	PRINTFN(3,("type=%d, index=%d, len=%d\n",
		    type, index, len));

	req.bmRequestType = UT_READ_DEVICE;
	req.bRequest = UR_GET_DESCRIPTOR;
	USETW2(req.wValue, type, index);
	USETW(req.wIndex, 0);
	USETW(req.wLength, len);

 repeat:
	err = usbd_do_request(udev, &req, desc);

	if(err && timeout--)
	{
		usbd_delay_ms(udev, 200);
		goto repeat;
	}
	return (err);
}

/* Use "usbreq_get_string_any()" instead of
 * "usbreq_get_string_desc()", when the language id is not known. The
 * maximum length of the string, "len", includes the terminating zero.
 * "usbreq_get_string_any()" will always write a terminating zero to "buf",
 * also on error.
 */
usbd_status
usbreq_get_string_any(struct usbd_device *udev, int si, char *buf, int len)
{
	int swap =0;// udev->quirks->uq_flags & UQ_SWAP_UNICODE; //dpp
	usb_string_descriptor_t us;
	char *s;
	int i, n;
	u_int16_t c;
	usbd_status err;

	if(len == 0)
	{
		return (USBD_NORMAL_COMPLETION);
	}

	buf[0] = 0;

	/* subtract the terminating zero */
	len--;

	if(si == 0)
	{
		return (USBD_INVAL);
	}
	if(0)//udev->quirks->uq_flags & UQ_NO_STRINGS) //dpp
	{
		return (USBD_STALLED);
	}
	if(udev->langid == USBD_NOLANG)
	{
		/* set up default language */
		err = usbreq_get_string_desc(udev, USB_LANGUAGE_TABLE, 0, &us, 0);
		if(err || (us.bLength < 4))
		{
			udev->langid = 0; /* well, just pick something then */
		}
		else
		{
			/* pick the first language as the default */
			udev->langid = UGETW(us.bString[0]);
		}
	}
	err = usbreq_get_string_desc(udev, si, udev->langid, &us, 0);
	if(err)
	{
		return (err);
	}
	s = buf;
	n = (us.bLength / 2) - 1;
	for(i = 0; (i < n) && len; i++, len--)
	{
		c = UGETW(us.bString[i]);

		/* convert from Unicode, handle buggy strings */
		if ((c & 0xff00) == 0)
		{
			*s++ = c;
		}
		else if(((c & 0x00ff) == 0) && swap)
		{
			*s++ = c >> 8;
		}
		else
		{
			*s++ = '?';
		}
	}
	*s++ = 0;
	return (USBD_NORMAL_COMPLETION);
}

usbd_status
usbreq_get_string_desc(struct usbd_device *udev, int sindex, int langid,
		       usb_string_descriptor_t *sdesc, int *plen)
{
	usb_device_request_t req;
	usbd_status err;
	int actlen;

	req.bmRequestType = UT_READ_DEVICE;
	req.bRequest = UR_GET_DESCRIPTOR;
	USETW2(req.wValue, UDESC_STRING, sindex);
	USETW(req.wIndex, langid);
	USETW(req.wLength, 2);	/* only size byte first */
	err = usbd_do_request_flags(udev, &req, sdesc, USBD_SHORT_XFER_OK,
				    &actlen, USBD_DEFAULT_TIMEOUT);
	if(err)
	{
		return (err);
	}

	if(actlen < 2)
	{
		return (USBD_SHORT_XFER);
	}

	if(plen)
	{
		*plen = sdesc->bLength;
	}

	USETW(req.wLength, sdesc->bLength);	/* the whole string */
	return (usbd_do_request(udev, &req, sdesc));
}

usbd_status
usbreq_get_config_desc(struct usbd_device *udev, int confidx,
		       usb_config_descriptor_t *d)
{
	usbd_status err;

	PRINTFN(3,("confidx=%d\n", confidx));
	err = usbreq_get_desc(udev, UDESC_CONFIG, confidx,
			      USB_CONFIG_DESCRIPTOR_SIZE, d, 0);
	if(err)
	{
		return (err);
	}
	if(d->bDescriptorType != UDESC_CONFIG)
	{
		PRINTFN(-1,("confidx=%d, bad desc len=%d type=%d\n",
			     confidx, d->bLength, d->bDescriptorType));
		return (USBD_INVAL);
	}
	return (USBD_NORMAL_COMPLETION);
}

usbd_status
usbreq_get_config_desc_full(struct usbd_device *udev, int conf, void *d, int size)
{
	PRINTFN(3,("conf=%d\n", conf));
	return (usbreq_get_desc(udev, UDESC_CONFIG, conf, size, d, 0));
}

usbd_status
usbreq_get_device_desc(struct usbd_device *udev, usb_device_descriptor_t *d)
{
	PRINTFN(3,("\n"));
	return (usbreq_get_desc(udev, UDESC_DEVICE,
				0, USB_DEVICE_DESCRIPTOR_SIZE, d, 3));
}

usbd_status
usbreq_get_interface(struct usbd_device *udev, u_int8_t iface_index,
		     u_int8_t *aiface)
{
	struct usbd_interface *iface = usbd_get_iface(udev,iface_index);
	usb_device_request_t req;

	if((iface == NULL) || (iface->idesc == NULL))
	{
		return (USBD_INVAL);
	}

	req.bmRequestType = UT_READ_INTERFACE;
	req.bRequest = UR_GET_INTERFACE;
	USETW(req.wValue, 0);
	USETW(req.wIndex, iface->idesc->bInterfaceNumber);
	USETW(req.wLength, 1);
	return (usbd_do_request(udev, &req, aiface));
}

usbd_status
usbreq_set_interface(struct usbd_device *udev, u_int8_t iface_index,
		     u_int8_t altno)
{
	struct usbd_interface *iface = usbd_get_iface(udev,iface_index);
	usb_device_request_t req;
	usbd_status err;

	if(iface == NULL)
	{
		return (USBD_INVAL);
	}

	err = usbd_fill_iface_data(udev, iface_index, altno);
	if(err)
	{
		return (err);
	}

	req.bmRequestType = UT_WRITE_INTERFACE;
	req.bRequest = UR_SET_INTERFACE;
	USETW(req.wValue, iface->idesc->bAlternateSetting);
	USETW(req.wIndex, iface->idesc->bInterfaceNumber);
	USETW(req.wLength, 0);
	return (usbd_do_request(udev, &req, 0));
}

usbd_status
usbreq_get_device_status(struct usbd_device *udev, usb_status_t *st)
{
	usb_device_request_t req;

	req.bmRequestType = UT_READ_DEVICE;
	req.bRequest = UR_GET_STATUS;
	USETW(req.wValue, 0);
	USETW(req.wIndex, 0);
	USETW(req.wLength, sizeof(usb_status_t));
	return (usbd_do_request(udev, &req, st));
}

usbd_status
usbreq_get_hub_descriptor(struct usbd_device *udev, usb_hub_descriptor_t *hd)
{
	usb_device_request_t req;

	req.bmRequestType = UT_READ_CLASS_DEVICE;
	req.bRequest = UR_GET_DESCRIPTOR;
	USETW(req.wValue, 0);
	USETW(req.wIndex, 0);
	USETW(req.wLength, USB_HUB_DESCRIPTOR_SIZE);
	return (usbd_do_request(udev, &req, hd));
}

usbd_status
usbreq_get_hub_status(struct usbd_device *udev, usb_hub_status_t *st)
{
	usb_device_request_t req;

	req.bmRequestType = UT_READ_CLASS_DEVICE;
	req.bRequest = UR_GET_STATUS;
	USETW(req.wValue, 0);
	USETW(req.wIndex, 0);
	USETW(req.wLength, sizeof(usb_hub_status_t));
	return (usbd_do_request(udev, &req, st));
}

usbd_status
usbreq_set_address(struct usbd_device *udev, int addr)
{
	usb_device_request_t req;

	PRINTFN(5,("setting device address=%d\n", addr));

	req.bmRequestType = UT_WRITE_DEVICE;
	req.bRequest = UR_SET_ADDRESS;
	USETW(req.wValue, addr);
	USETW(req.wIndex, 0);
	USETW(req.wLength, 0);
	return (usbd_do_request(udev, &req, 0));
}

usbd_status
usbreq_get_port_status(struct usbd_device *udev, int port, usb_port_status_t *ps)
{
	usb_device_request_t req;

	req.bmRequestType = UT_READ_CLASS_OTHER;
	req.bRequest = UR_GET_STATUS;
	USETW(req.wValue, 0);
	USETW(req.wIndex, port);
	USETW(req.wLength, sizeof *ps);
	return (usbd_do_request(udev, &req, ps));
}

usbd_status
usbreq_clear_hub_feature(struct usbd_device *udev, int sel)
{
	usb_device_request_t req;

	req.bmRequestType = UT_WRITE_CLASS_DEVICE;
	req.bRequest = UR_CLEAR_FEATURE;
	USETW(req.wValue, sel);
	USETW(req.wIndex, 0);
	USETW(req.wLength, 0);
	return (usbd_do_request(udev, &req, 0));
}

usbd_status
usbreq_set_hub_feature(struct usbd_device *udev, int sel)
{
	usb_device_request_t req;

	req.bmRequestType = UT_WRITE_CLASS_DEVICE;
	req.bRequest = UR_SET_FEATURE;
	USETW(req.wValue, sel);
	USETW(req.wIndex, 0);
	USETW(req.wLength, 0);
	return (usbd_do_request(udev, &req, 0));
}

usbd_status
usbreq_clear_port_feature(struct usbd_device *udev, int port, int sel)
{
	usb_device_request_t req;

	req.bmRequestType = UT_WRITE_CLASS_OTHER;
	req.bRequest = UR_CLEAR_FEATURE;
	USETW(req.wValue, sel);
	USETW(req.wIndex, port);
	USETW(req.wLength, 0);
	return (usbd_do_request(udev, &req, 0));
}

usbd_status
usbreq_set_port_feature(struct usbd_device *udev, int port, int sel)
{
	usb_device_request_t req;

	req.bmRequestType = UT_WRITE_CLASS_OTHER;
	req.bRequest = UR_SET_FEATURE;
	USETW(req.wValue, sel);
	USETW(req.wIndex, port);
	USETW(req.wLength, 0);
	return (usbd_do_request(udev, &req, 0));
}

usbd_status
usbreq_set_protocol(struct usbd_device *udev, u_int8_t iface_index,
		    u_int16_t report)
{
	struct usbd_interface *iface = usbd_get_iface(udev,iface_index);
	usb_device_request_t req;

	if((iface == NULL) || (iface->idesc == NULL))
	{
		return (USBD_INVAL);
	}
	PRINTFN(4, ("iface=%p, report=%d, endpt=%d\n",
		     iface, report, iface->idesc->bInterfaceNumber));

	req.bmRequestType = UT_WRITE_CLASS_INTERFACE;
	req.bRequest = UR_SET_PROTOCOL;
	USETW(req.wValue, report);
	USETW(req.wIndex, iface->idesc->bInterfaceNumber);
	USETW(req.wLength, 0);
	return (usbd_do_request(udev, &req, 0));
}

#ifdef USB_COMPAT_OLD
usbd_status
usbreq_set_report_async(struct usbd_device *udev, u_int8_t iface_index,
		  u_int8_t type, u_int8_t id, void *data, int len)
{
	struct usbd_interface *iface = usbd_get_iface(udev,iface_index);
	usb_device_request_t req;

	if((iface == NULL) || (iface->idesc == NULL))
	{
		return (USBD_INVAL);
	}
	/* this function call should be replaced by an allocated
	 * transfer that is started when a transfer is needed, and
	 * stopped when the device is detached. This implementation
	 * use polling because it may be called from an interrupt
	 * context.
	 */

	PRINTF(("this function is depreceated"));

	PRINTFN(4, ("len=%d\n", len));

	req.bmRequestType = UT_WRITE_CLASS_INTERFACE;
	req.bRequest = UR_SET_REPORT;
	USETW2(req.wValue, type, id);
	USETW(req.wIndex, iface->idesc->bInterfaceNumber);
	USETW(req.wLength, len);

	return (usbd_do_request_flags
		(udev, &req, data, USBD_USE_POLLING, 0, 500 /* ms */));
}
#endif

usbd_status
usbreq_set_report(struct usbd_device *udev, u_int8_t iface_index,
		  u_int8_t type, u_int8_t id, void *data, int len)
{
	struct usbd_interface *iface = usbd_get_iface(udev,iface_index);
	usb_device_request_t req;

	if((iface == NULL) || (iface->idesc == NULL))
	{
		return (USBD_INVAL);
	}
	PRINTFN(4, ("len=%d\n", len));

	req.bmRequestType = UT_WRITE_CLASS_INTERFACE;
	req.bRequest = UR_SET_REPORT;
	USETW2(req.wValue, type, id);
	USETW(req.wIndex, iface->idesc->bInterfaceNumber);
	USETW(req.wLength, len);
	return (usbd_do_request(udev, &req, data));
}

usbd_status
usbreq_get_report(struct usbd_device *udev, u_int8_t iface_index,
		  u_int8_t type, u_int8_t id, void *data, int len)
{
	struct usbd_interface *iface = usbd_get_iface(udev,iface_index);
	usb_device_request_t req;

	if((iface == NULL) || (iface->idesc == NULL) || (id == 0))
	{
		return (USBD_INVAL);
	}
	PRINTFN(4, ("len=%d\n", len));

	req.bmRequestType = UT_READ_CLASS_INTERFACE;
	req.bRequest = UR_GET_REPORT;
	USETW2(req.wValue, type, id);
	USETW(req.wIndex, iface->idesc->bInterfaceNumber);
	USETW(req.wLength, len);
	return (usbd_do_request(udev, &req, data));
}

usbd_status
usbreq_set_idle(struct usbd_device *udev, u_int8_t iface_index,
		int duration, int id)
{
	struct usbd_interface *iface = usbd_get_iface(udev,iface_index);
	usb_device_request_t req;

	if((iface == NULL) || (iface->idesc == NULL))
	{
		return (USBD_INVAL);
	}
	PRINTFN(4, ("%d %d\n", duration, id));

	req.bmRequestType = UT_WRITE_CLASS_INTERFACE;
	req.bRequest = UR_SET_IDLE;
	USETW2(req.wValue, duration, id);
	USETW(req.wIndex, iface->idesc->bInterfaceNumber);
	USETW(req.wLength, 0);
	return (usbd_do_request(udev, &req, 0));
}

usbd_status
usbreq_get_report_descriptor(struct usbd_device *udev, int ifcno,
			     int size, void *d)
{
	usb_device_request_t req;

	req.bmRequestType = UT_READ_INTERFACE;
	req.bRequest = UR_GET_DESCRIPTOR;
	USETW2(req.wValue, UDESC_REPORT, 0); /* report id should be 0 */
	USETW(req.wIndex, ifcno);
	USETW(req.wLength, size);
	return (usbd_do_request(udev, &req, d));
}

usbd_status
usbreq_read_report_desc(struct usbd_device *udev, u_int8_t iface_index,
 			void **descp, int *sizep, usb_malloc_type mem)
{
	struct usbd_interface *iface = usbd_get_iface(udev,iface_index);
	usb_hid_descriptor_t *hid;
	usbd_status err;

	if((iface == NULL) || (iface->idesc == NULL))
	{
		return (USBD_INVAL);
	}
	hid = usbd_get_hdesc(usbd_get_config_descriptor(udev), iface->idesc);
	if(hid == NULL)
	{
		return (USBD_IOERROR);
	}
	*sizep = UGETW(hid->descrs[0].wDescriptorLength);
	*descp = malloc(*sizep, mem, M_NOWAIT);
	if(*descp == NULL)
	{
		return (USBD_NOMEM);
	}
	err = usbreq_get_report_descriptor(udev, iface->idesc->bInterfaceNumber,
					   *sizep, *descp);
	if(err)
	{
		free(*descp, mem);
		*descp = NULL;
		return (err);
	}
	return (USBD_NORMAL_COMPLETION);
}

usbd_status
usbreq_set_config(struct usbd_device *udev, int conf)
{
	usb_device_request_t req;

	PRINTF(("setting config %d\n", conf));

	req.bmRequestType = UT_WRITE_DEVICE;
	req.bRequest = UR_SET_CONFIG;
	USETW(req.wValue, conf);
	USETW(req.wIndex, 0);
	USETW(req.wLength, 0);
	return (usbd_do_request(udev, &req, 0));
}

usbd_status
usbreq_get_config(struct usbd_device *udev, u_int8_t *conf)
{
	usb_device_request_t req;

	req.bmRequestType = UT_READ_DEVICE;
	req.bRequest = UR_GET_CONFIG;
	USETW(req.wValue, 0);
	USETW(req.wIndex, 0);
	USETW(req.wLength, 1);
	return (usbd_do_request(udev, &req, conf));
}
