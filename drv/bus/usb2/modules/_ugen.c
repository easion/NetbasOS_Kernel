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

__FBSDID("$FreeBSD: src/sys/dev/usb2/ugen.c $");

#define	UGEN_BULK_BUFFER_SIZE	(1024*64) /* bytes */

#define UGEN_HW_FRAMES	50 /* number of milliseconds per transfer */

#define ADD_BYTES(ptr,size) ((void *)(((u_int8_t *)(ptr)) + (size)))

struct ugen_frame_ring {
  u_int16_t input_index;
  u_int16_t output_index;
  u_int16_t end_index; /* exclusive */
  u_int16_t frame_size;
  u_int16_t *frlengths;
  void *buf;
};

struct ugen_endpoint {
  struct ugen_softc *	  sc;
  struct cdev *		  dev;
  struct usbd_pipe *	  pipe_in; /* pipe for reading data from USB */
  struct usbd_pipe *	  pipe_out; /* pipe for writing data to USB */
  struct usbd_xfer *	  xfer_in[2];
  struct usbd_xfer *	  xfer_out[2];

  struct ugen_frame_ring  in_queue; /* (isoc/interrupt) */
  struct ugen_frame_ring  out_queue; /* (isoc) */
  u_int32_t		  in_timeout; /* (bulk/interrupt) */
  u_int32_t		  in_frames; /* number of frames to use (isoc) */
  u_int32_t		  out_frames; /* number of frames to use (isoc) */
  u_int16_t		  out_frame_size; /* maximum frame size (isoc) */

  u_int32_t		  io_buffer_size; /* (bulk) */

  struct selinfo	  selinfo;

  u_int16_t		  state;
#define UGEN_OPEN_IN      0x0001
#define UGEN_OPEN_OUT     0x0002
#define UGEN_OPEN_DEV     0x0004
#define UGEN_CLOSING      0x0008
#define UGEN_GONE         0x0010
#define UGEN_SHORT_OK     0x0020 /* short xfers are OK */

  /* context bits */
#define UGEN_RD_CFG	  0x0040
#define UGEN_RD_SLP	  0x0080 /* sleep entered */
#define UGEN_RD_WUP	  0x0100 /* need wakeup */
#define UGEN_RD_UIO	  0x0200

  /* context bits */
#define UGEN_WR_CFG	  0x0400
#define UGEN_WR_SLP	  0x0800
#define UGEN_WR_WUP	  0x1000
#define UGEN_WR_UIO	  0x2000

  /* context bit */
#define UGEN_IOCTL	  0x4000
};

struct ugen_softc {
  device_t       	  sc_dev;
  struct usbd_device *	  sc_udev;
  struct ugen_endpoint	  sc_endpoints[USB_MAX_ENDPOINTS];
  struct ugen_endpoint	  sc_endpoints_end[0];
  struct mtx		  sc_mtx;
  struct usbd_memory_wait sc_mem_wait;
};

extern cdevsw_t ugen_cdevsw;

static void
ugen_make_devnodes(struct ugen_softc *sc);

static void
ugen_destroy_devnodes(struct ugen_softc *sc, int skip_first);

static void
ugen_interrupt_callback(struct usbd_xfer *xfer);

static void
ugenisoc_read_callback(struct usbd_xfer *xfer);

static void
ugenisoc_write_callback(struct usbd_xfer *xfer);

static int
ugen_set_config(struct ugen_softc *sc, int configno);

static int
ugen_set_interface(struct ugen_softc *sc, int ifaceidx, int altno);

static usb_config_descriptor_t *
ugen_get_cdesc(struct usbd_device *udev, int index, int *lenp);

static int
ugen_get_alt_index(struct usbd_device *udev, int ifaceidx);

#define UGENMINOR(unit, endpoint) (((unit) << 4) | (endpoint))

#define DEV2SC(dev) ((dev)->si_drv1)
#define DEV2SCE(dev) ((dev)->si_drv2)

static int 
ugen_probe(device_t dev)
{
	struct usb_attach_arg *uaa = device_get_ivars(dev);

	if(uaa->usegeneric)
		return (UMATCH_GENERIC);
	else
		return (UMATCH_NONE);
}

static int
ugen_attach(device_t dev)
{
	struct usb_attach_arg *uaa = device_get_ivars(dev);
	struct ugen_softc *sc = device_get_softc(dev);
	struct ugen_endpoint *sce;
	int conf;

	sc->sc_dev = dev;
	sc->sc_udev = uaa->device;

        mtx_init(&sc->sc_mtx, "ugen lock",
                 NULL, MTX_DEF|MTX_RECURSE);

	/**/
	usbd_set_desc(dev, sc->sc_udev);

	/* first set configuration index 0, the default one for ugen */
	if(usbd_set_config_index(sc->sc_udev, 0, 0))
	{
		device_printf(dev, "setting configuration index 0 failed\n");
		return ENXIO;
	}
	conf = usbd_get_config_descriptor(sc->sc_udev)->bConfigurationValue;

	/* set up all the local state for this configuration */
	if(ugen_set_config(sc, conf))
	{
		device_printf(dev, "setting configuration "
			      "%d failed\n", conf);
		return ENXIO;
	}

	sce = &sc->sc_endpoints[0];

	/* make control endpoint */
	sce->dev = make_dev(&ugen_cdevsw, UGENMINOR(device_get_unit(sc->sc_dev), 0),
			    UID_ROOT, GID_OPERATOR, 0644, "%s", 
			    device_get_nameunit(sc->sc_dev));
	if(sce->dev)
	{
		DEV2SC(sce->dev) = sc;
		DEV2SCE(sce->dev) = sce;
	}
	return 0; /* success */
}

static int
ugen_detach(device_t dev)
{
	struct ugen_softc *sc = device_get_softc(dev);
	struct ugen_endpoint *sce = &sc->sc_endpoints[0];
	struct ugen_endpoint *sce_end = &sc->sc_endpoints_end[0];

	mtx_lock(&sc->sc_mtx);
	while(sce < sce_end)
	{
		sce->state |= UGEN_GONE;
		sce++;
	}
	mtx_unlock(&sc->sc_mtx);

	/* destroy all devices */
	ugen_destroy_devnodes(sc, 0);

	/* wait for memory to get freed */
	usbd_transfer_drain(&(sc->sc_mem_wait), &(sc->sc_mtx));

	mtx_destroy(&sc->sc_mtx);

	return 0;
}

#define ugen_inc_input_index(ufr) \
{\
  (ufr)->input_index++;\
  if((ufr)->input_index >= (ufr)->end_index)\
	(ufr)->input_index = 0;\
}

#define ugen_inc_output_index(ufr) \
{\
  (ufr)->output_index++;\
  if((ufr)->output_index >= (ufr)->end_index)\
	(ufr)->output_index = 0;\
}

#define ugen_half_empty(ufr) \
((ufr)->end_index ? \
((((ufr)->end_index + \
   (ufr)->input_index - \
   (ufr)->output_index) % (ufr)->end_index) \
 < ((ufr)->end_index / 2)) : 0)

#define ugen_not_empty(ufr) \
((ufr)->output_index != \
 (ufr)->input_index)

static void
ugen_get_input_block(struct ugen_frame_ring *ufr, void **ptr, u_int16_t **len)
{
	u_int16_t next;

	next = ufr->input_index +1;
	if(next >= ufr->end_index)
	{
		next = 0;
	}
	if(next == ufr->output_index)
	{
		/* buffer full */
		*ptr = 0;
		*len = 0;
	}
	else
	{
		*ptr = ADD_BYTES
		  (ufr->buf, 
		   ufr->frame_size*ufr->input_index);

		*len = &ufr->frlengths[ufr->input_index];

		/* set default block length */
		ufr->frlengths[ufr->input_index] = ufr->frame_size; 
	}
	return;
}

static void
ugen_get_output_block(struct ugen_frame_ring *ufr, void **ptr, u_int16_t *len)
{
	if(ufr->output_index == ufr->input_index)
	{
		/* buffer empty */
		*ptr = 0;
		*len = 0;
	}
	else
	{
		*ptr = ADD_BYTES
		  (ufr->buf,
		   ufr->frame_size*ufr->output_index);

		*len = ufr->frlengths[ufr->output_index];
	}
	return;
}

static int
ugen_allocate_blocks(struct ugen_softc *sc,
		     struct ugen_endpoint *sce,
		     u_int16_t context_bit,
		     struct ugen_frame_ring *ufr, 
		     u_int16_t frames, 
		     u_int16_t frame_size)
{
	void *ptr;

	bzero(ufr, sizeof(*ufr));

	if(frames == 0) 
	{
		return 0;
	}

	/* one frame will always be unused
	 * to make things simple, so allocate
	 * one extra frame:
	 */
	frames++;

	sce->state |= context_bit;

	mtx_unlock(&sc->sc_mtx);

	ptr = malloc(frames*(frame_size + sizeof(u_int16_t)), 
		     M_USBDEV, M_WAITOK);

	mtx_lock(&sc->sc_mtx);

	sce->state &= ~context_bit;

	if(sce->state & UGEN_CLOSING)
	{
		wakeup(sce);
		if(ptr)
		{
			free(ptr, M_USBDEV);
			ptr = NULL;
		}
	}

	if(ptr == NULL)
	{
		return 0;
	}

	ufr->end_index = frames;
	ufr->frame_size = frame_size;
	ufr->frlengths = ADD_BYTES(ptr, frames*frame_size);
	ufr->buf = ADD_BYTES(ptr, 0);
	return 1;
}

static void
ugen_free_blocks(struct ugen_frame_ring *ufr)
{
	if(ufr->buf)
	{
		free(ufr->buf, M_USBDEV);
		ufr->buf = NULL;
	}
	return;
}

static usbd_status
__usbd_transfer_setup(struct ugen_softc *sc,
		      struct ugen_endpoint *sce,
		      u_int16_t context_bit,
		      struct usbd_device *udev,
		      u_int8_t iface_index,
		      struct usbd_xfer **pxfer,
		      const struct usbd_config *setup,
		      u_int8_t n_setup)
{
	struct usbd_xfer * temp[n_setup];

	usbd_status error;

	sce->state |= context_bit;

	mtx_unlock(&sc->sc_mtx);

	/* "usbd_transfer_setup()" can sleep so one
	 * needs to make a wrapper, exiting
	 * the mutex and checking things
	 */
	error = usbd_transfer_setup(udev, iface_index, &temp[0], 
				    setup, n_setup, 
				    sce, &(sc->sc_mtx), &(sc->sc_mem_wait));

	mtx_lock(&sc->sc_mtx);

	sce->state &= ~context_bit;

	if(sce->state & UGEN_CLOSING)
	{
		wakeup(sce);
		error = USBD_CANCELLED;

		/* "usbd_transfer_unsetup()" will clear "temp[]" */
		usbd_transfer_unsetup(&temp[0], n_setup);
	}

	while(n_setup--)
	{
		pxfer[n_setup] = temp[n_setup];
	}
	return error;
}

static int
__uiomove(struct ugen_softc *sc, struct ugen_endpoint *sce,
	  u_int16_t context_bit, void *cp, int n, 
	  struct uio *uio)
{
	int error;

	sce->state |= context_bit;

	mtx_unlock(&sc->sc_mtx);

	/* "uiomove()" can sleep so one
	 * needs to make a wrapper, exiting
	 * the mutex and checking things
	 */
	error = uiomove(cp, n, uio);

	mtx_lock(&sc->sc_mtx);

	sce->state &= ~context_bit;

	if(sce->state & UGEN_CLOSING)
	{
		wakeup(sce);
		error = EINTR;
	}
	return error;
}

static int
ugenopen(struct cdev *dev, int flag, int mode, struct thread *p)
{
	struct ugen_softc *sc = DEV2SC(dev);
	struct ugen_endpoint *sce = DEV2SCE(dev);
	int error = 0;

 	PRINTFN(5, ("flag=%d, mode=%d\n", flag, mode));

	if((sc == NULL) || (sce == NULL))
	{
		return (EIO);
	}

	mtx_lock(&sc->sc_mtx);

	if(sce->state & (UGEN_OPEN_DEV|UGEN_GONE))
	{
		error = EBUSY;
		goto done;
	}

	if(sce != &sc->sc_endpoints[0])
	{
		/* non-control endpoint(s) ;
		 * make sure that there are 
		 * pipes for all directions
		 */
		if(((flag & FWRITE) && !sce->pipe_out) ||
		   ((flag & FREAD) && !sce->pipe_in))
		{
			error = ENXIO;
			goto done;
		}
	}

	sce->state |= UGEN_OPEN_DEV;

 done:
	mtx_unlock(&sc->sc_mtx);
	return error;
}

static int
ugenclose(struct cdev *dev, int flag, int mode, struct thread *p)
{
	struct ugen_softc *sc = DEV2SC(dev);
	struct ugen_endpoint *sce = DEV2SCE(dev);

	PRINTFN(5, ("flag=%d, mode=%d\n", flag, mode));

	if((sc == NULL) || (sce == NULL))
	{
		return (0);
	}

	mtx_lock(&sc->sc_mtx);

	if(sce->state & (UGEN_OPEN_DEV|UGEN_OPEN_IN|UGEN_OPEN_OUT))
	{
		/* control endpoint is also ``closed'' here */

		sce->state |= UGEN_CLOSING;

		if(sce->xfer_in[0])
		{
			usbd_transfer_stop(sce->xfer_in[0]);
		}
		if(sce->xfer_in[1])
		{
			usbd_transfer_stop(sce->xfer_in[1]);
		}
		if(sce->xfer_out[0])
		{
			usbd_transfer_stop(sce->xfer_out[0]);
		}
		if(sce->xfer_out[1])
		{
			usbd_transfer_stop(sce->xfer_out[1]);
		}

		while(sce->state & 
		      (UGEN_RD_CFG|UGEN_RD_SLP|UGEN_RD_WUP|UGEN_RD_UIO|
		       UGEN_WR_CFG|UGEN_WR_SLP|UGEN_WR_WUP|UGEN_WR_UIO|
		       UGEN_IOCTL))
		{
			if(sce->state & (UGEN_RD_WUP|UGEN_WR_WUP))
			{
				sce->state &= ~(UGEN_RD_WUP|UGEN_WR_WUP);
				wakeup(sce);
			}

			/* wait for routine(s) to exit */
			msleep(sce, &sc->sc_mtx, PRIBIO, "ugensync", 0);
		}

		/* free all memory after that one has
		 * waited for all context bits to clear,
		 * hence functions accessing memory
		 * like "uiomove", might be sleeping !
		 */

		ugen_free_blocks(&sce->in_queue);
		ugen_free_blocks(&sce->out_queue);

		sce->state &= ~(UGEN_OPEN_DEV|UGEN_OPEN_IN|UGEN_OPEN_OUT|UGEN_CLOSING);

		usbd_transfer_unsetup(&sce->xfer_in[0], 2);
		usbd_transfer_unsetup(&sce->xfer_out[0], 2);
	}
	mtx_unlock(&sc->sc_mtx);
	return (0);
}

static int
ugen_open_pipe_write(struct ugen_softc *sc, struct ugen_endpoint *sce)
{
	u_int16_t isize;
	usbd_status err;

	mtx_assert(&sc->sc_mtx, MA_OWNED);

	if(!(sce->state & UGEN_OPEN_OUT))
	{
	  if(sce->pipe_out)
	  {
		usb_endpoint_descriptor_t *ed = sce->pipe_out->edesc;

		struct usbd_config usbd_config[2] = { /* zero */ };

		usbd_config[1].type = UE_CONTROL;
		usbd_config[1].endpoint = 0;
		usbd_config[1].direction = -1;
		usbd_config[1].timeout = USBD_DEFAULT_TIMEOUT;
		usbd_config[1].flags = 0;
		usbd_config[1].bufsize = sizeof(usb_device_request_t);
		usbd_config[1].callback = &usbd_clearstall_callback;

		usbd_config[0].type = ed->bmAttributes & UE_XFERTYPE;
		usbd_config[0].endpoint = ed->bEndpointAddress & UE_ADDR;
		usbd_config[0].direction = UE_DIR_OUT;
		usbd_config[0].callback = usbd_default_callback;
		usbd_config[0].interval = USBD_DEFAULT_INTERVAL;
		usbd_config[0].timeout = sce->in_timeout;

		switch(ed->bmAttributes & UE_XFERTYPE)
		{
		case UE_INTERRUPT:
		case UE_BULK:
			usbd_config[0].flags = USBD_SYNCHRONOUS;
			usbd_config[0].bufsize = UGEN_BULK_BUFFER_SIZE;
			
			if(__usbd_transfer_setup
			   (sc, sce, UGEN_WR_CFG,
			    sc->sc_udev, sce->pipe_out->iface_index,
			    &sce->xfer_out[0], &usbd_config[0], 2))
			{
				return (EIO);
			}
			/* setup clear stall */
			sce->xfer_out[0]->clearstall_xfer =
			  sce->xfer_out[1];
			break;

		case UE_ISOCHRONOUS:
			isize = UGETW(ed->wMaxPacketSize);

			/* wMaxPacketSize is validated 
			 * by "usbd_fill_iface_data()" 
			 */

			if(usbd_get_speed(sc->sc_udev) == USB_SPEED_HIGH)
			{
				sce->out_frames = UGEN_HW_FRAMES*8;
			}
			else
			{
				sce->out_frames = UGEN_HW_FRAMES;
			}

			if(ugen_allocate_blocks
			   (sc, sce, UGEN_WR_CFG, 
			    &sce->out_queue, sce->out_frames * 10, isize) == 0)
			{
				return ENOMEM;
			}

			usbd_config[0].flags = USBD_SHORT_XFER_OK;
			usbd_config[0].bufsize = isize * sce->out_frames;
			usbd_config[0].frames = sce->out_frames;
			usbd_config[0].callback = ugenisoc_write_callback;
			usbd_config[0].timeout = 0;

			err = __usbd_transfer_setup
			  (sc, sce, UGEN_WR_CFG,
			   sc->sc_udev, sce->pipe_out->iface_index,
			   &sce->xfer_out[0], &usbd_config[0], 1);

			if(!err)
			{
				err = __usbd_transfer_setup
				  (sc, sce, UGEN_WR_CFG,
				   sc->sc_udev, sce->pipe_out->iface_index,
				   &sce->xfer_out[1], &usbd_config[0], 1);
			}
			if(err)
			{
				usbd_transfer_unsetup(&sce->xfer_out[0], 1);
				usbd_transfer_unsetup(&sce->xfer_out[1], 1);
				ugen_free_blocks(&sce->out_queue);
				return (EIO);
			}
			break;

		default:
			return (EINVAL);
		}
		sce->state |= UGEN_OPEN_OUT;
	  }
	  else
	  {
		return ENXIO;
	  }
	}
	return 0;
}

static int
ugen_open_pipe_read(struct ugen_softc *sc, struct ugen_endpoint *sce)
{
	int isize;
	usbd_status err;

	mtx_assert(&sc->sc_mtx, MA_OWNED);

	if(!(sce->state & UGEN_OPEN_IN))
	{
	  if(sce->pipe_in)
	  {
		usb_endpoint_descriptor_t *ed = sce->pipe_in->edesc;

		struct usbd_config usbd_config[2] = { /* zero */ };

		usbd_config[1].type = UE_CONTROL;
		usbd_config[1].endpoint = 0;
		usbd_config[1].direction = -1;
		usbd_config[1].timeout = USBD_DEFAULT_TIMEOUT;
		usbd_config[1].flags = 0;
		usbd_config[1].bufsize = sizeof(usb_device_request_t);
		usbd_config[1].callback = &usbd_clearstall_callback;

		usbd_config[0].type = ed->bmAttributes & UE_XFERTYPE;
		usbd_config[0].endpoint = ed->bEndpointAddress & UE_ADDR;
		usbd_config[0].direction = UE_DIR_IN;
		usbd_config[0].timeout = sce->in_timeout;

		switch(ed->bmAttributes & UE_XFERTYPE)
		{
		case UE_INTERRUPT:
		  usbd_config[0].flags = USBD_SHORT_XFER_OK;
		  usbd_config[0].callback = ugen_interrupt_callback;
		  usbd_config[0].bufsize = UGETW(ed->wMaxPacketSize);
		  usbd_config[0].interval = USBD_DEFAULT_INTERVAL;

		  if(ugen_allocate_blocks
		     (sc, sce, UGEN_RD_CFG,
		      &sce->in_queue, 1, UGETW(ed->wMaxPacketSize)) == 0)
		  {
			return ENOMEM;
		  }

		  if(__usbd_transfer_setup
		     (sc, sce, UGEN_RD_CFG,
		      sc->sc_udev, sce->pipe_in->iface_index,
		      &sce->xfer_in[0], &usbd_config[0], 2))
		  {
			ugen_free_blocks(&sce->in_queue);
			return (EIO);
		  }

		  /* setup clear stall */
		  sce->xfer_in[0]->clearstall_xfer =
		    sce->xfer_in[1];

		  usbd_transfer_start(sce->xfer_in[0]);
		  PRINTFN(5, ("interrupt open done\n"));
		  break;

		case UE_BULK:
		  usbd_config[0].flags = ((sce->state & UGEN_SHORT_OK) ? 
					  USBD_SHORT_XFER_OK : 0) | USBD_SYNCHRONOUS;
		  usbd_config[0].callback = usbd_default_callback;
		  usbd_config[0].bufsize = UGEN_BULK_BUFFER_SIZE;

		  if(__usbd_transfer_setup
		     (sc, sce, UGEN_RD_CFG,
		      sc->sc_udev, sce->pipe_in->iface_index,
		      &sce->xfer_in[0], &usbd_config[0], 2))
		  {
			return (EIO);
		  }

		  /* setup clear stall */
		  sce->xfer_in[0]->clearstall_xfer =
		    sce->xfer_in[1];
		  break;

		case UE_ISOCHRONOUS:

		  isize = UGETW(ed->wMaxPacketSize);

		  /* wMaxPacketSize is validated by "usbd_fill_iface_data()" */

		  if(usbd_get_speed(sc->sc_udev) == USB_SPEED_HIGH)
		  {
			sce->in_frames = UGEN_HW_FRAMES*8;
		  }
		  else
		  {
			sce->in_frames = UGEN_HW_FRAMES;
		  }

		  if(ugen_allocate_blocks
		     (sc, sce, UGEN_RD_CFG,
		      &sce->in_queue, sce->in_frames * 10, isize) == 0)
		  {
			return ENOMEM;
		  }

		  usbd_config[0].flags = USBD_SHORT_XFER_OK;
		  usbd_config[0].bufsize = isize * sce->in_frames;
		  usbd_config[0].frames = sce->in_frames;
		  usbd_config[0].callback = ugenisoc_read_callback;
		  usbd_config[0].timeout = 0;

		  err = __usbd_transfer_setup
		    (sc, sce, UGEN_RD_CFG,
		     sc->sc_udev, sce->pipe_in->iface_index,
		     &sce->xfer_in[0], &usbd_config[0], 1);

		  if(!err)
		  {
			err = __usbd_transfer_setup
			  (sc, sce, UGEN_RD_CFG,
			   sc->sc_udev, sce->pipe_in->iface_index,
			   &sce->xfer_in[1], &usbd_config[0], 1);
		  }

		  if(err)
		  {
			usbd_transfer_unsetup(&sce->xfer_in[0], 1);
			usbd_transfer_unsetup(&sce->xfer_in[1], 1);
			ugen_free_blocks(&sce->in_queue);
			return (EIO);
		  }

		  usbd_transfer_start(sce->xfer_in[0]);
		  usbd_transfer_start(sce->xfer_in[1]);
		  PRINTFN(5, ("isoc open done\n"));
		  break;

		default:
		  return EINVAL;
		}
		sce->state |= UGEN_OPEN_IN;
	  }
	  else
	  {
		return ENXIO;
	  }
	}
	return 0;
}

static void
ugen_make_devnodes(struct ugen_softc *sc)
{
	struct usbd_pipe *pipe;
	struct usbd_pipe *pipe_end;
	struct ugen_endpoint *sce;
	struct cdev *dev;
	int endpoint;

	mtx_lock(&sc->sc_mtx);

	pipe = &sc->sc_udev->pipes[0];
	pipe_end = &sc->sc_udev->pipes_end[0];

	while(pipe < pipe_end)
	{
		if(pipe->edesc)
		{
			endpoint = pipe->edesc->bEndpointAddress & UE_ADDR;
			sce = &sc->sc_endpoints[endpoint];

			if(!sce->dev && (endpoint != 0))
			{
				mtx_unlock(&sc->sc_mtx); /* XXX "make_dev()" can sleep, 
							  * XXX caller should have
							  * XXX set a context bit !
							  */

				dev = make_dev(&ugen_cdevsw,
					   UGENMINOR(device_get_unit(sc->sc_dev), endpoint),
					   UID_ROOT, GID_OPERATOR, 0644, "%s.%d",
					   device_get_nameunit(sc->sc_dev), endpoint);

				mtx_lock(&sc->sc_mtx); /* XXX */

				sce->dev = dev;

				if(sce->dev)
				{
					DEV2SCE(sce->dev) = sce;
					DEV2SC(sce->dev) = sc;
				}

			}

			sce->in_timeout = USBD_NO_TIMEOUT;
			sce->out_frame_size = -1; /* set maximum value */
			sce->io_buffer_size = UGEN_BULK_BUFFER_SIZE; /* set default value */

			if((pipe->edesc->bEndpointAddress & 
			    (UE_DIR_IN|UE_DIR_OUT)) == UE_DIR_IN)
			{
				sce->pipe_in = pipe;
			}
			else
			{
				sce->pipe_out = pipe;
			}
		}
		pipe++;
	}
	mtx_unlock(&sc->sc_mtx);
	return;
}

static void
ugen_destroy_devnodes(struct ugen_softc *sc, int skip_first)
{
	struct ugen_endpoint *sce = &sc->sc_endpoints[0];
	struct ugen_endpoint *sce_end = &sc->sc_endpoints_end[0];

	if(skip_first)
	{
		sce++; /* skip control endpoint */
	}

	while(sce < sce_end)
	{
		if(sce->dev)
		{
			ugenclose(sce->dev, 0, 0, 0);

			DEV2SCE(sce->dev) = NULL;
			DEV2SC(sce->dev) = NULL;

			destroy_dev(sce->dev);
		}

		sce->pipe_in = NULL;
		sce->pipe_out = NULL;
		sce->dev = NULL;

		sce++;
	}
	return;
}

static int
ugenread(struct cdev *dev, struct uio *uio, int flag)
{
	struct ugen_softc *sc = DEV2SC(dev);
	struct ugen_endpoint *sce = DEV2SCE(dev);
	struct usbd_xfer *xfer;
	void *ptr;
	int error;
	int n;
	u_int16_t len;

	PRINTFN(5, ("\n"));

	if((sc == NULL) || (sce == NULL))
	{
		return (EIO);
	}

	/* check for control endpoint */
	if(sce == &sc->sc_endpoints[0])
	{
		return (ENODEV);
	}

	mtx_lock(&sc->sc_mtx);

	if(sce->state & (UGEN_CLOSING|UGEN_GONE|UGEN_RD_CFG|
			 UGEN_RD_UIO|UGEN_RD_SLP))
	{
		error = EIO;
		goto done;
	}

	error = ugen_open_pipe_read(sc,sce);
	if(error)
	{
		goto done;
	}

	switch (sce->pipe_in->edesc->bmAttributes & UE_XFERTYPE) {
	case UE_ISOCHRONOUS:
		n = 1;
		goto ue_interrupt;

	case UE_INTERRUPT:
		n = 2;

	ue_interrupt:
		while(uio->uio_resid)
		{
			if((uio->uio_resid < sce->in_queue.frame_size) && 
			   (n == 0))
			{
				/* try to keep data synchronization */
				break;
			}

			/* get one frame from input queue */

			ugen_get_output_block(&sce->in_queue, &ptr, &len);

			if(ptr == NULL)
			{
			    if(n == 0)
			    {
			        /* let application process data */
			        break;
			    }
			    if(flag & IO_NDELAY)
			    {
			        if(n)
				{
				    error = EWOULDBLOCK;
				}
				break;
			    }

			    /* wait for data */

			    sce->state |= (UGEN_RD_SLP|UGEN_RD_WUP);

			    error = msleep(sce, &sc->sc_mtx, (PZERO|PCATCH),
					   "ugen wait callback", 0);

			    sce->state &= ~(UGEN_RD_SLP|UGEN_RD_WUP);

			    if(sce->state & UGEN_CLOSING)
			    {
			        wakeup(sce);
                                error = EIO;
				break;
			    }
			    if(error)
			    {
			        break;
			    }
			    continue;
			}

			if(len > uio->uio_resid)
			{
			    PRINTFN(5, ("dumping %d bytes!\n", 
					len - (u_int16_t)(uio->uio_resid)));

			    /* rest of this frame will get dumped
			     * for sake of synchronization!
			     */
			    len = uio->uio_resid;
			}

			PRINTFN(10, ("transferring %d bytes\n", len));

			/* copy data to user memory */
			error = __uiomove(sc, sce, UGEN_RD_UIO, ptr, len, uio);

			if(error) break;

			ugen_inc_output_index(&sce->in_queue);

			/* only transfer one interrupt frame per read ! */

			if(n == 2) 
			{
				/* start interrupt transfer again */
				usbd_transfer_start(sce->xfer_in[0]);
				break;
			}

			n = 0;
		}
		break;

	case UE_BULK:
		while((n = min(UGEN_BULK_BUFFER_SIZE, uio->uio_resid)) != 0)
		{
#if 0
			if(flag & IO_NDELAY)
			{
				error = EWOULDBLOCK;
				break;
			}
#endif
			xfer = sce->xfer_in[0];

			/* update length */
			xfer->length = n;

			sce->state |= UGEN_RD_UIO;

			/* start transfer */
			usbd_transfer_start(xfer);

			sce->state &= ~UGEN_RD_UIO;

			if(sce->state & UGEN_CLOSING)
			{
				error = EIO;
				wakeup(sce);
				break;
			}

			if(xfer->error)
			{
				error = (xfer->error == USBD_CANCELLED) ? 
				  EINTR : EIO;
				break;
			}

			PRINTFN(1, ("got %d of %d bytes\n", xfer->actlen, n));
			error = __uiomove
			  (sc, sce, UGEN_RD_UIO, 
			   xfer->buffer, xfer->actlen, uio);

			if(error || (xfer->actlen < n))
			{
				break;
			}
		}
		break;

	default:
		error = ENXIO;
		break;
	}

 done:
	mtx_unlock(&sc->sc_mtx);
	return (error);
}

static int
ugenwrite(struct cdev *dev, struct uio *uio, int flag)
{
	struct ugen_softc *sc = DEV2SC(dev);
	struct ugen_endpoint *sce = DEV2SCE(dev);
	struct usbd_xfer *xfer;
	u_int16_t *plen;
	void *ptr;
	int error;
	int n;

	PRINTFN(5, ("\n"));

	if((sc == NULL) || (sce == NULL))
	{
		return (EIO);
	}

	/* check for control endpoint */
	if(sce == &sc->sc_endpoints[0])
	{
		return (ENODEV);
	}

	mtx_lock(&sc->sc_mtx);

	if(sce->state & (UGEN_CLOSING|UGEN_GONE|UGEN_WR_CFG|
			 UGEN_WR_SLP|UGEN_WR_UIO))
	{
		error = EIO;
		goto done;
	}

	error = ugen_open_pipe_write(sc,sce);
	if(error)
	{
		goto done;
	}

	switch (sce->pipe_out->edesc->bmAttributes & UE_XFERTYPE) {
	case UE_BULK:
	case UE_INTERRUPT:
		while((n = min(UGEN_BULK_BUFFER_SIZE, uio->uio_resid)) != 0)
		{
#if 0
			if(flag & IO_NDELAY)
			{
				error = EWOULDBLOCK;
				break;
			}
#endif
			xfer = sce->xfer_out[0];

			error = __uiomove
			  (sc, sce, UGEN_WR_UIO, xfer->buffer, n, uio);
			if(error)
			{
				break;
			}

			PRINTFN(1, ("transferred %d bytes\n", n));

			/* update length */
			xfer->length = n;

			sce->state |= UGEN_WR_UIO;

			/* start transfer */
			usbd_transfer_start(xfer);

			sce->state &= ~UGEN_WR_UIO;

			if(sce->state & UGEN_CLOSING)
			{
				error = EIO;
				wakeup(sce);
				break;
			}

			if(xfer->error)
			{
				error = (xfer->error == USBD_CANCELLED) ? 
				  EINTR : EIO;
				break;
			}
		}
		break;

	case UE_ISOCHRONOUS:

		n = 1;

		while(uio->uio_resid || n)
		{
			ugen_get_input_block(&sce->out_queue, &ptr, &plen);

			if(ptr == NULL)
			{
				/* make sure that the transfers are 
				 * started, if not already started.
				 */
				usbd_transfer_start(sce->xfer_out[0]);
				usbd_transfer_start(sce->xfer_out[1]);

				if(flag & IO_NDELAY)
				{
					if(n)
					{
						error = EWOULDBLOCK;
					}
					break;
				}

				sce->state |= (UGEN_WR_SLP|UGEN_WR_WUP);

				error = msleep(sce, &sc->sc_mtx, (PZERO|PCATCH),
					       "ugen wait callback", 0);

				sce->state &= ~(UGEN_WR_SLP|UGEN_WR_WUP);

				if(sce->state & UGEN_CLOSING)
				{
					wakeup(sce);
					error = EIO;
					break;
				}
				if(error)
				{
					break;
				}
				continue;
			}

			if(*plen > sce->out_frame_size)
			{
				*plen = sce->out_frame_size;
			}

			if(*plen > uio->uio_resid)
			{
				*plen = uio->uio_resid;
			}

			error = __uiomove(sc, sce, UGEN_WR_UIO, ptr, *plen, uio);

			if(error) break;

			ugen_inc_input_index(&sce->out_queue);

			n = 0;
		}
		if(n == 0)
		{
			/* make sure that the transfers are 
			 * started, if not already started.
			 */
			usbd_transfer_start(sce->xfer_out[0]);
			usbd_transfer_start(sce->xfer_out[1]);
		}
		break;

	default:
		error = ENXIO;
		break;
	}

 done:
	mtx_unlock(&sc->sc_mtx);
	return (error);
}

static void
ugen_interrupt_callback(struct usbd_xfer *xfer)
{
	struct ugen_endpoint *sce = xfer->priv_sc;
	u_int16_t *plen;
	void *ptr;

	USBD_CHECK_STATUS(xfer);

 tr_transferred:

	PRINTFN(5, ("xfer=%p actlen=%d\n",
		     xfer, xfer->actlen));

	ugen_get_input_block(&sce->in_queue, &ptr, &plen);

	if(ptr == NULL)
	{
		PRINTFN(5, ("dropping one packet, sce=%p\n", sce));
	}
	else
	{
		bcopy(xfer->buffer, ptr, xfer->actlen);

		if(xfer->actlen > *plen)
		{
			xfer->actlen = *plen;
		}

		*plen = xfer->actlen;

		ugen_inc_input_index(&sce->in_queue);
	}

	if(sce->state & UGEN_RD_WUP)
	{
		sce->state &= ~UGEN_RD_WUP;

		PRINTFN(5, ("waking %p\n", sce));
		wakeup(sce);
	}
	selwakeuppri(&sce->selinfo, PZERO);

	/* the transfer will be restarted after that
	 * the packet has been read
	 */
	return; 

 tr_setup:
 tr_error:
	usbd_start_hardware(xfer);
	return;
}

static void
ugenisoc_read_callback(struct usbd_xfer *xfer)
{
	struct ugen_endpoint *sce = xfer->priv_sc;
	u_int16_t *plen1;
	u_int16_t *plen2;

	void *ptr1;
	void *ptr2;

	u_int16_t isize;
	u_int16_t n;

	USBD_CHECK_STATUS(xfer);

 tr_transferred:

	PRINTFN(5,("actlen=%d\n", xfer->actlen));

	plen1 = xfer->frlengths;
	ptr1 = xfer->buffer;

	isize = UGETW(sce->pipe_in->edesc->wMaxPacketSize);
	n = sce->in_frames;
	while(n--)
	{
		if(*plen1 != 0)
		{
			ugen_get_input_block(&sce->in_queue, &ptr2, &plen2);

			if(ptr2 == NULL)
			{
				break;
			}

			if(*plen1 > *plen2)
			{
				*plen1 = *plen2;
			}

			bcopy(ptr1, ptr2, *plen1);

			*plen2 = *plen1;

			ugen_inc_input_index(&sce->in_queue);
		}

		ptr1 = ADD_BYTES(ptr1, isize);
		plen1++;
	}

	if(sce->state & UGEN_RD_WUP)
	{
		sce->state &= ~UGEN_RD_WUP;
		wakeup(sce);
	}
	selwakeuppri(&sce->selinfo, PZERO);

 tr_setup:
 tr_error:
	isize = UGETW(sce->pipe_in->edesc->wMaxPacketSize);
	for(n = 0; n < sce->in_frames; n++)
	{
		/* setup size for next transfer */
		xfer->frlengths[n] = isize;
	}
	usbd_start_hardware(xfer);
	return;
}

static void
ugenisoc_write_callback(struct usbd_xfer *xfer)
{
	struct ugen_endpoint *sce = xfer->priv_sc;
	u_int16_t *plen1;
	u_int16_t len2;

	void *ptr1;
	void *ptr2;

	u_int16_t isize;
	u_int16_t n;

	USBD_CHECK_STATUS(xfer);

 tr_transferred:
 tr_setup:

	plen1 = xfer->frlengths;
	ptr1 = xfer->buffer;

	isize = UGETW(sce->pipe_out->edesc->wMaxPacketSize);
	n = sce->out_frames;
	while(n--)
	{
		ugen_get_output_block(&sce->out_queue, &ptr2, &len2);

		if(ptr2 == NULL)
		{
		    break;
		}

		if(len2 > isize)
		{
		    len2 = isize;
		}

		bcopy(ptr2, ptr1, len2);

		*plen1 = len2;

		ugen_inc_output_index(&sce->out_queue);

		ptr1 = ADD_BYTES(ptr1, len2);
		plen1++;
	}

	n = plen1 - xfer->frlengths;

	/* update number of frames */
	xfer->nframes = n;

	if(sce->state & UGEN_WR_WUP)
	{
		sce->state &= ~UGEN_WR_WUP;
		wakeup(sce);
	}
	selwakeuppri(&sce->selinfo, PZERO);

	if(n)
	{
 tr_error:
		usbd_start_hardware(xfer);
	}
	return;
}

static int
ugen_set_config(struct ugen_softc *sc, int configno)
{
	PRINTFN(1,("configno %d, sc=%p\n", configno, sc));

	/* destroy all but control device */
	ugen_destroy_devnodes(sc, 1);

	/* avoid setting the current value */
	if((usbd_get_config_descriptor(sc->sc_udev) == NULL) ||
	   (usbd_get_config_descriptor(sc->sc_udev)->bConfigurationValue != configno))
	{
		if(usbd_set_config_no(sc->sc_udev, configno, 1))
		{
			return EIO;
		}
	}

	/* make devices */
	ugen_make_devnodes(sc);
	return 0;
}

static int
ugen_set_interface(struct ugen_softc *sc, int ifaceidx, int altno)
{
	PRINTFN(15, ("%d %d\n", ifaceidx, altno));

	/* destroy all but control device */
	ugen_destroy_devnodes(sc, 1);

	/* change setting */
	if(usbreq_set_interface(sc->sc_udev, ifaceidx, altno))
	{
		return EIO;
	}

	/* make the new devices */
	ugen_make_devnodes(sc);
	return (0);
}

/* retrieve a complete descriptor for a certain device and index */
static usb_config_descriptor_t *
ugen_get_cdesc(struct usbd_device *udev, int index, int *lenp)
{
	usb_config_descriptor_t *cdesc, *tdesc, cdescr;
	int len;

	if(index == USB_CURRENT_CONFIG_INDEX)
	{
		tdesc = usbd_get_config_descriptor(udev);
		len = UGETW(tdesc->wTotalLength);
		if(lenp)
		{
			*lenp = len;
		}
		cdesc = malloc(len, M_TEMP, M_WAITOK);
		if(cdesc == NULL)
		{
			return 0;
		}
		memcpy(cdesc, tdesc, len);
		PRINTFN(5,("current, len=%d\n", len));
	}
	else
	{
		if(usbreq_get_config_desc(udev, index, &cdescr))
		{
			return (0);
		}
		len = UGETW(cdescr.wTotalLength);
		PRINTFN(5,("index=%d, len=%d\n", index, len));
		if(lenp)
		{
			*lenp = len;
		}
		cdesc = malloc(len, M_TEMP, M_WAITOK);
		if(cdesc == NULL)
		{
			return 0;
		}
		if(usbreq_get_config_desc_full(udev, index, cdesc, len))
		{
			free(cdesc, M_TEMP);
			return (0);
		}
	}
	return (cdesc);
}

static int
ugen_get_alt_index(struct usbd_device *udev, int ifaceidx)
{
	struct usbd_interface *iface = usbd_get_iface(udev, ifaceidx);

	return (iface) ? (iface->alt_index) : -1;
}

static int
ugenioctl(struct cdev *dev, u_long cmd, caddr_t addr, int flag, struct thread *p)
{
	struct ugen_softc *sc = DEV2SC(dev);
	struct ugen_endpoint *sce = DEV2SCE(dev);
	struct usbd_interface *iface;
	usb_interface_descriptor_t *idesc;
	usb_endpoint_descriptor_t *edesc;
	void *data = 0;
	int error = 0;
	int len;
	u_int8_t conf;
	u_int8_t alt;

	PRINTFN(5, ("cmd=%08lx\n", cmd));
	if((sc == NULL) || (sce == NULL))
	{
		return (EIO);
	}

	mtx_lock(&sc->sc_mtx);

	if(sce->state & (UGEN_CLOSING|UGEN_GONE|UGEN_IOCTL))
	{
		error = EIO;
		goto done;
	}

	switch (cmd) {
	case FIONBIO:
		/* all handled in the upper FS layer */
		goto done;

	case USB_SET_SHORT_XFER:
		/* this flag only affects read */
		/* check for control endpoint */
		if(sce == &sc->sc_endpoints[0])
		{
			error = EINVAL;
			goto done;
		}
		if(sce->pipe_in == NULL)
		{
#ifdef USB_DEBUG
			printf("%s: USB_SET_SHORT_XFER, no pipe\n",
			       __FUNCTION__);
#endif
			error = EIO;
			goto done;
		}

		if(*(int *)addr)
			sce->state |= UGEN_SHORT_OK;
		else
			sce->state &= ~UGEN_SHORT_OK;
		goto done;

	case USB_SET_TIMEOUT:
		sce->in_timeout = *(int *)addr;
		goto done;

	case USB_GET_FRAME_SIZE:
		if(sce->pipe_in)
		{
			*(int *)addr = UGETW(sce->pipe_in->edesc->wMaxPacketSize);
		}
		else
		{
			error = EINVAL;
		}
		goto done;

	case USB_SET_FRAME_SIZE:
		if(!(flag & FWRITE))
		{
			error = EPERM;
			goto done;
		}
		if((*((int *)addr) <= 0) ||
		   (*((int *)addr) >= 65536))
		{
			error = EINVAL;
			goto done;
		}
		sce->out_frame_size = *(int *)addr;
		goto done;

	case USB_SET_BUFFER_SIZE:
		if(*(int *)addr < 1024)
		  sce->io_buffer_size = 1024;
		else if(*(int *)addr < (256*1024))
		  sce->io_buffer_size = *(int *)addr;
		else
		  sce->io_buffer_size = 256*1024;
		break;

	case USB_GET_BUFFER_SIZE:
		*(int *)addr = sce->io_buffer_size;
		break;

	default:
		break;
	}

	/* the following ioctls will sleep
	 * and are only allowed on the
	 * control endpoint
	 */
	if(sce != &sc->sc_endpoints[0])
	{
		error = EINVAL;
		goto done;
	}

	sce->state |= UGEN_IOCTL;

	mtx_unlock(&sc->sc_mtx);

	switch (cmd) {
#ifdef USB_DEBUG
	case USB_SETDEBUG:
		if(!(flag & FWRITE))
		{
			error = EPERM;
			break;
		}
		usbdebug = *(int *)addr;
		break;
#endif
	case USB_GET_CONFIG:
		error = usbreq_get_config(sc->sc_udev, &conf);
		if(error)
		{
			error = EIO;
			break;
		}
		*(int *)addr = conf;
		break;
	case USB_SET_CONFIG:
		if(!(flag & FWRITE))
		{
			error = EPERM;
			break;
		}
		error = ugen_set_config(sc, *(int *)addr);
		if(error)
		{
			break;
		}
		break;
	case USB_GET_ALTINTERFACE:
#define ai ((struct usb_alt_interface *)addr)
		iface = usbd_get_iface(sc->sc_udev, ai->uai_interface_index);

		if(!iface ||
		   !iface->idesc)
		{
			error = EINVAL;
			break;
		}

		ai->uai_alt_no = iface->idesc->bAlternateSetting;
		break;
	case USB_SET_ALTINTERFACE:
		if(!(flag & FWRITE))
		{
			error = EPERM;
			break;
		}
		error = ugen_set_interface(sc, ai->uai_interface_index, ai->uai_alt_no);
		if(error)
		{
			break;
		}
		break;
	case USB_GET_NO_ALT:
		data = ugen_get_cdesc(sc->sc_udev, ai->uai_config_index, 0);
		if(data == NULL)
		{
			error = EINVAL;
			break;
		}
		idesc = usbd_find_idesc(data, ai->uai_interface_index, 0);
		if(idesc == NULL)
		{
			error = EINVAL;
			break;
		}
		ai->uai_alt_no = usbd_get_no_alts(data, idesc->bInterfaceNumber);
#undef ai
		break;
	case USB_GET_DEVICE_DESC:
		if(!usbd_get_device_descriptor(sc->sc_udev))
		{
			error = EIO;
			break;
		}

		*(usb_device_descriptor_t *)addr =
			*usbd_get_device_descriptor(sc->sc_udev);
		break;
	case USB_GET_CONFIG_DESC:
#define cd ((struct usb_config_desc *)addr)
		data = ugen_get_cdesc(sc->sc_udev, cd->ucd_config_index, 0);
		if(data == NULL)
		{
			error = EINVAL;
			break;
		}
		cd->ucd_desc = *(usb_config_descriptor_t *)data;
#undef cd
		break;
	case USB_GET_INTERFACE_DESC:
#define id  ((struct usb_interface_desc *)addr)
		data = ugen_get_cdesc(sc->sc_udev, id->uid_config_index, 0);
		if(data == NULL)
		{
			error = EINVAL;
			break;
		}
		if((id->uid_config_index == USB_CURRENT_CONFIG_INDEX) &&
		   (id->uid_alt_index == USB_CURRENT_ALT_INDEX))
			alt = ugen_get_alt_index(sc->sc_udev, id->uid_interface_index);
		else
			alt = id->uid_alt_index;
		idesc = usbd_find_idesc(data, id->uid_interface_index, alt);
		if(idesc == NULL)
		{
			error = EINVAL;
			break;
		}
		id->uid_desc = *idesc;
#undef id
		break;
	case USB_GET_ENDPOINT_DESC:
#define ed ((struct usb_endpoint_desc *)addr)
		data = ugen_get_cdesc(sc->sc_udev, ed->ued_config_index, 0);
		if(data == NULL)
		{
			error = EINVAL;
			break;
		}
		if((ed->ued_config_index == USB_CURRENT_CONFIG_INDEX) &&
		   (ed->ued_alt_index == USB_CURRENT_ALT_INDEX))
			alt = ugen_get_alt_index(sc->sc_udev, ed->ued_interface_index);
		else
			alt = ed->ued_alt_index;
		edesc = usbd_find_edesc(data, ed->ued_interface_index,
					alt, ed->ued_endpoint_index);
		if(edesc == NULL)
		{
			error = EINVAL;
			break;
		}
		ed->ued_desc = *edesc;
#undef ed
		break;
	case USB_GET_FULL_DESC:
#define fd ((struct usb_full_desc *)addr)
		data = ugen_get_cdesc(sc->sc_udev, fd->ufd_config_index, &len);
		if(data == NULL)
		{
			error = EINVAL;
			break;
		}
		if(len > fd->ufd_size)
		{
			len = fd->ufd_size;
		}
		if(fd->ufd_data == NULL)
		{
			error = EINVAL;
			break;
		}
		error = copyout(data, fd->ufd_data, len);
#undef fd
		break;
	case USB_GET_STRING_DESC:
#define si ((struct usb_string_desc *)addr)
		if(usbreq_get_string_desc
		   (sc->sc_udev, si->usd_string_index,
		    si->usd_language_id, &si->usd_desc, &len))
		{
			error = EINVAL;
			break;
		}
#undef si
		break;
	case USB_DO_REQUEST:
#define ur ((struct usb_ctl_request *)addr)
		if(!(flag & FWRITE))
		{
			error = EPERM;
			break;
		}
		/* avoid requests that would damage the bus integrity */
		if(((ur->ucr_request.bmRequestType == UT_WRITE_DEVICE) &&
		    (ur->ucr_request.bRequest == UR_SET_ADDRESS)) ||
		   ((ur->ucr_request.bmRequestType == UT_WRITE_DEVICE) &&
		    (ur->ucr_request.bRequest == UR_SET_CONFIG)) ||
		   ((ur->ucr_request.bmRequestType == UT_WRITE_INTERFACE) &&
		    (ur->ucr_request.bRequest == UR_SET_INTERFACE)))
		{
			error = EINVAL;
			break;
		}

		len = UGETW(ur->ucr_request.wLength);

		if((len < 0) || (len > 32767))
		{
			error = EINVAL;
			break;
		}

		if(len != 0)
		{
			if(ur->ucr_data == NULL)
			{
				error = EINVAL;
				break;
			}

			data = malloc(len, M_TEMP, M_WAITOK);
			if(data == NULL)
			{
				error = ENOMEM;
				break;
			}
			if(!(ur->ucr_request.bmRequestType & UT_READ))
			{
				error = copyin(ur->ucr_data, data, len);
				if(error)
				{
					break;
				}
			}
		}

		if(usbd_do_request_flags
		   (sc->sc_udev, &ur->ucr_request, data,
		    ur->ucr_flags, &ur->ucr_actlen, USBD_DEFAULT_TIMEOUT))
		{
			error = EIO;
			break;
		}

		if((len != 0) && (ur->ucr_request.bmRequestType & UT_READ))
		{
			error = copyout(data, ur->ucr_data, len);
			if(error)
			{
				break;
			}
		}
#undef ur
		break;

	case USB_GET_DEVICEINFO:
		usbd_fill_deviceinfo(sc->sc_udev,
		    (struct usb_device_info *)addr, 1);
		break;
	default:
		error = EINVAL;
		break;
	}

	if(data)
	{
		free(data, M_TEMP);
	}

	mtx_lock(&sc->sc_mtx);

	sce->state &= ~UGEN_IOCTL;

	if(sce->state & UGEN_CLOSING)
	{
		wakeup(sce);
		error = EIO;
	}

 done:
	mtx_unlock(&sc->sc_mtx);
	return (error);
}

static int
ugenpoll(struct cdev *dev, int events, struct thread *p)
{
	struct ugen_softc *sc = DEV2SC(dev);
	struct ugen_endpoint *sce = DEV2SCE(dev);
	int revents = 0;

	PRINTFN(5, ("\n"));

	if((sc == NULL) || (sce == NULL))
	{
		return POLLNVAL;
	}

	if(sce == &sc->sc_endpoints[0])
	{
		/* not control endpoint */
		return 0;
	}

	mtx_lock(&sc->sc_mtx);

	if(sce->state & (UGEN_CLOSING|UGEN_GONE))
	{
		goto done;
	}

	if(sce->pipe_out && (events & (POLLOUT | POLLWRNORM)) &&
	   (!(sce->state & (UGEN_WR_CFG|UGEN_WR_SLP|UGEN_WR_UIO))))
	{
	    if(ugen_open_pipe_write(sc,sce))
	    {
		/* must return, hence the error
		 * might indicate that the device
		 * is about to be detached
		 */
		revents = POLLNVAL;
		goto done;
	    }

	    switch (sce->pipe_out->edesc->bmAttributes & UE_XFERTYPE) {
	    case UE_ISOCHRONOUS:
		if(ugen_half_empty(&sce->out_queue))
		{
		    revents |= events & (POLLOUT | POLLWRNORM);
		}
		else
		{
		    selrecord(p, &sce->selinfo);
		}
		break;

	    case UE_BULK:
	    case UE_INTERRUPT:
		/* pretend that one can write data, hence
		 * no buffering is done:
		 */
		revents |= events & (POLLOUT | POLLWRNORM);
		break;
	    }
	}

	if(sce->pipe_in && (events & (POLLIN | POLLRDNORM)) &&
	   (!(sce->state & (UGEN_RD_CFG|UGEN_RD_SLP|UGEN_RD_UIO))))
	{
	    /* check that pipes are open, so that
	     * selwakeup is called
	     */
	    if(ugen_open_pipe_read(sc,sce))
	    {
		/* must return, hence the error
		 * might indicate that the device
		 * is about to be detached
		 */
		revents = POLLNVAL;
		goto done;
	    }

	    switch (sce->pipe_in->edesc->bmAttributes & UE_XFERTYPE) {
	    case UE_INTERRUPT:
	    case UE_ISOCHRONOUS:
	        if(ugen_not_empty(&sce->in_queue))
		{
		    revents |= events & (POLLIN | POLLRDNORM);
		}
		else
		{
		    selrecord(p, &sce->selinfo);
		}
		break;

	    case UE_BULK:
		/* pretend that one can read data,
		 * hence no buffering is done:
		 */
		revents |= events & (POLLIN | POLLRDNORM);
		break;
	    }
	}
 done:
	mtx_unlock(&sc->sc_mtx);
	return (revents);
}

cdevsw_t ugen_cdevsw = {
#ifdef D_VERSION
	.d_version =	D_VERSION,
#endif
	.d_open =	ugenopen,
	.d_close =	ugenclose,
	.d_read =	ugenread,
	.d_write =	ugenwrite,
	.d_ioctl =	ugenioctl,
	.d_poll =	ugenpoll,
	.d_name =	"ugen",
};

static devclass_t ugen_devclass;

static driver_t ugen_driver =
{
        .name    = "ugen",
        .methods = (device_method_t [])
        {
          DEVMETHOD(device_probe, ugen_probe),
          DEVMETHOD(device_attach, ugen_attach),
          DEVMETHOD(device_detach, ugen_detach),
          { 0, 0 }
        },
        .size    = sizeof(struct ugen_softc),
};

DRIVER_MODULE(ugen, uhub, ugen_driver, ugen_devclass, usbd_driver_load, 0);
MODULE_DEPEND(ugen, usb, 1, 1, 1);
