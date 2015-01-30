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

__FBSDID("$FreeBSD: src/sys/dev/usb2/usb_transfer.c $");

#ifdef USB_DEBUG
void
usbd_dump_iface(struct usbd_interface *iface)
{
	printf("usbd_dump_iface: iface=%p\n", iface);
	if(iface == NULL)
	{
		return;
	}
	printf(" iface=%p idesc=%p altindex=%d\n",
	       iface, iface->idesc, iface->alt_index);
	return;
}

void
usbd_dump_device(struct usbd_device *udev)
{
	printf("usbd_dump_device: dev=%p\n", udev);
	if(udev == NULL)
	{
		return;
	}
	printf(" bus=%p \n"
	       " address=%d config=%d depth=%d speed=%d self_powered=%d\n"
	       " power=%d langid=%d\n",
	       udev->bus, 
	       udev->address, udev->config, udev->depth, udev->speed,
	       udev->self_powered, udev->power, udev->langid);
	return;
}

void
usbd_dump_queue(struct usbd_pipe *pipe)
{
	struct usbd_xfer *xfer;

	printf("usbd_dump_queue: pipe=%p\n", pipe);
	LIST_FOREACH(xfer, &pipe->list_head, pipe_list)
	{
		printf("  xfer=%p\n", xfer);
	}
	return;
}

void
usbd_dump_pipe(struct usbd_pipe *pipe)
{
	if(pipe)
	{
		printf("usbd_dump_pipe: pipe=%p", pipe);

		printf(" edesc=%p isoc_next=%d toggle_next=%d",
		       pipe->edesc, pipe->isoc_next, pipe->toggle_next);

		if(pipe->edesc)
		{
			printf(" bEndpointAddress=0x%02x",
			       pipe->edesc->bEndpointAddress);
		}
		printf("\n");
		usbd_dump_queue(pipe);
	}
	else
	{
		printf("usbd_dump_pipe: pipe=NULL\n");
	}
	return;
}

void
usbd_dump_xfer(struct usbd_xfer *xfer)
{
	printf("usbd_dump_xfer: xfer=%p\n", xfer);
	if(xfer == NULL)
	{
		return;
	}
        if(xfer->pipe == NULL)
	{
		printf("xfer %p: pipe=NULL\n",
		       xfer);
                return;
	}
	printf("xfer %p: udev=%p vid=0x%04x pid=0x%04x addr=%d "
	       "pipe=%p ep=0x%02x attr=0x%02x\n",
	       xfer, xfer->udev,
	       UGETW(xfer->udev->ddesc.idVendor),
	       UGETW(xfer->udev->ddesc.idProduct),
	       xfer->udev->address, xfer->pipe,
	       xfer->pipe->edesc->bEndpointAddress, 
	       xfer->pipe->edesc->bmAttributes);
	return;
}
#endif

u_int32_t
usb_get_devid(device_t dev)
{
	struct usb_attach_arg *uaa = device_get_ivars(dev);
	return ((uaa->vendor << 16) | (uaa->product));
}

struct usbd_pipe *
usbd_get_pipe(struct usbd_device *udev, u_int8_t iface_index,
	      const struct usbd_config *setup)
{
	struct usbd_pipe *pipe;
	u_int8_t index = setup->index;

	PRINTFN(8,("udev=%p iface_index=%d address=0x%x "
		    "type=0x%x dir=0x%x index=%d\n",
		    udev, iface_index, setup->endpoint,
		    setup->type, setup->direction, setup->index));

	pipe = &udev->pipes_end[0];
	while(--pipe >= &udev->pipes[0])
	{
		if((pipe->edesc) &&
		   (pipe->iface_index == iface_index) &&
		   (((pipe->edesc->bEndpointAddress & (UE_DIR_IN|UE_DIR_OUT)) == setup->direction) || (setup->direction == 0xff)) &&
		   (((pipe->edesc->bEndpointAddress & UE_ADDR) == setup->endpoint) || (setup->endpoint == 0xff)) &&
		   (((pipe->edesc->bmAttributes & UE_XFERTYPE) == setup->type) || (setup->type == 0xff))
		   )
		{
			if(!index--)
			{
				goto found;
			}
		}
	}

	/* match against default pipe last, so that "any pipe", 
	 * "any address" and "any direction" returns the first 
	 * pipe of the interface
	 */
	if((setup->endpoint == 0) &&
	   (setup->type == 0))
	  /* "iface_index" and "direction" is ignored */
	{
		pipe = &udev->default_pipe;
		goto found;
	}
	return NULL;

 found:
	return pipe;
}

usbd_status
usbd_interface_count(struct usbd_device *udev, u_int8_t *count)
{
	if(udev->cdesc == NULL)
	{
		return (USBD_NOT_CONFIGURED);
	}
	*count = udev->cdesc->bNumInterface;
	return (USBD_NORMAL_COMPLETION);
}

/*---------------------------------------------------------------------------*
 *	usbd_transfer_setup - setup an array of USB transfers
 *
 * NOTE: must always call unsetup after setup 
 * NOTE: the parameter "iface_index" is ignored in
 *       case of control pipes
 *
 * The idea that the USB device driver should pre-allocate all
 * its transfers by one call to this function.
 *---------------------------------------------------------------------------*/
usbd_status
usbd_transfer_setup(struct usbd_device *udev, 
		    u_int8_t iface_index, 
		    struct usbd_xfer **pxfer,
		    const struct usbd_config *setup_start, 
		    u_int16_t n_setup, 
		    void *priv_sc,
		    struct mtx *priv_mtx,
		    struct usbd_memory_wait *priv_wait)
{
	const struct usbd_config *setup_end = setup_start + n_setup;
	const struct usbd_config *setup;
	struct usbd_memory_info *info;
	struct usbd_xfer *xfer;
	usbd_status error = 0;
	u_int16_t n;

	/* do some checking first */

	if(n_setup == 0)
	{
		PRINTFN(5, ("setup array has zero length!\n"));
		return USBD_INVAL;
	}

	if(priv_mtx == NULL)
	{
		PRINTFN(5, ("using global lock\n"));
		priv_mtx = &usb_global_lock;
	}

	for(setup = setup_start, n = n_setup; n--; setup++)
	{
		if(setup->bufsize == 0xffffffff)
		{
		    error = USBD_BAD_BUFSIZE;
		    PRINTF(("invalid bufsize\n"));
		}
		if(setup->flags & USBD_SYNCHRONOUS)
		{
		    if(setup->callback != &usbd_default_callback)
		    {
		        error = USBD_SYNC_TRANSFER_MUST_USE_DEFAULT_CALLBACK;
			PRINTF(("synchronous transfers "
				"must use default callback\n"));
		    }
		}
		if(setup->flags & 
		   (~(USBD_SYNCHRONOUS|
		      USBD_FORCE_SHORT_XFER|
		      USBD_SHORT_XFER_OK|
		      USBD_CUSTOM_CLEARSTALL|
		      USBD_USE_POLLING|
		      USBD_SELF_DESTRUCT)))
		{
		    error = USBD_BAD_FLAG;
		    PRINTF(("invalid flag(s) specified: "
			    "0x%08x\n", setup->flags));
		}
		if(setup->callback == NULL)
		{
		    error = USBD_NO_CALLBACK;
		    PRINTF(("no callback\n"));
		}
		pxfer[n] = NULL;
	}

	if(error)
	{
		goto done;
	}

	error = (udev->bus->methods->xfer_setup)
	  (udev,iface_index,pxfer,setup_start,setup_end);


	/* common setup */

	for(setup = setup_start, n = n_setup; n--; setup++)
	{
		xfer = pxfer[n];

		if(xfer)
		{
		    xfer->priv_sc = priv_sc;
		    xfer->priv_mtx = priv_mtx;
		    xfer->udev = udev;

		    if(xfer->pipe)
		    {
		        xfer->pipe->refcount++;
		    }

		    info = xfer->usb_root;
		    info->memory_refcount++;

		    if (info->priv_wait == NULL) {

		        info->priv_wait = priv_wait;
			info->priv_mtx = priv_mtx;

			if (priv_wait) {
			    mtx_lock(priv_mtx);
			    priv_wait->priv_refcount++;
			    mtx_unlock(priv_mtx);
			}
		    }
		}
	}

 done:
	if(error)
	{
		usbd_transfer_unsetup(pxfer, n_setup);
	}
	return (error);
}

/*---------------------------------------------------------------------------*
 *	usbd_drop_refcount 
 *
 * This function is called from various places, and its job is to
 * free the memory holding a set of "transfers", when it 
 * is safe to do so.
 *---------------------------------------------------------------------------*/
static void
usbd_drop_refcount(struct usbd_memory_info *info)
{
    struct usbd_memory_wait *priv_wait;
    u_int8_t free_memory;

    mtx_lock(info->usb_mtx);

    __KASSERT(info->memory_refcount != 0, ("Invalid memory "
					   "reference count!\n"));

    free_memory = ((--(info->memory_refcount)) == 0);

    mtx_unlock(info->usb_mtx);

    if(free_memory)
    {
        priv_wait = info->priv_wait;

	/* check if someone is waiting for 
	 * the memory to be released:
	 */
	if (priv_wait) {
	    mtx_lock(info->priv_mtx);

	    __KASSERT(priv_wait->priv_refcount != 0, ("Invalid private "
					       "reference count!\n"));
	    priv_wait->priv_refcount--;

	    if ((priv_wait->priv_refcount == 0) &&
		(priv_wait->priv_sleeping != 0)) {
	        wakeup(&priv_wait->wait_q);
	    }
	    mtx_unlock(info->priv_mtx);
	}

	usb_free_mem(info->memory_base, info->memory_size);
    }
    return;
}

/*---------------------------------------------------------------------------*
 *	usbd_transfer_unsetup - unsetup/free an array of USB transfers
 *
 * NOTE: if the transfer was in progress, the callback will 
 * called with "xfer->error=USBD_CANCELLED", before this
 * function returns
 *
 * NOTE: the mutex "xfer->priv_mtx" might be in use by 
 * the USB system after that this function has returned! 
 * Therefore the mutex, "xfer->priv_mtx", should be allocated 
 * in static memory.
 *---------------------------------------------------------------------------*/
void
usbd_transfer_unsetup(struct usbd_xfer **pxfer, u_int16_t n_setup)
{
	struct usbd_xfer *xfer;

	while(n_setup--)
	{
	    xfer = pxfer[n_setup];
	    pxfer[n_setup] = NULL;

	    if(xfer)
	    {
		if(xfer->pipe)
		{
		    mtx_lock(xfer->priv_mtx);

		    usbd_transfer_stop(xfer);

		    /* NOTE: default pipe does not
		     * have an interface, even if
		     * pipe->iface_index == 0
		     */
		    xfer->pipe->refcount--;

		    mtx_unlock(xfer->priv_mtx);
		}

		if(xfer->usb_root)
		{
		    usbd_drop_refcount(xfer->usb_root);
		}
	    }
	}
	return;
}

/*---------------------------------------------------------------------------*
 *	usbd_transfer_drain - wait for USB memory to get freed
 *
 * This function returns when the mutex "priv_mtx", is not used by the
 * USB system any more.
 *---------------------------------------------------------------------------*/
void
usbd_transfer_drain(struct usbd_memory_wait *priv_wait, struct mtx *priv_mtx)
{
    int error;

    mtx_lock(priv_mtx);

    priv_wait->priv_sleeping = 1;

    while (priv_wait->priv_refcount > 0) {

        error = msleep(&priv_wait->wait_q, priv_mtx, PRIBIO, "usb_drain", 0);
    }

    priv_wait->priv_sleeping = 0;

    mtx_unlock(priv_mtx);

    return;
}


/* CALLBACK EXAMPLES:
 * ==================
 *
 * USBD_CHECK_STATUS() overview of possible program paths:
 * =======================================================
 *
 *       +->-----------------------+
 *       |                         |    
 *   +-<-+-------[tr_setup]--------+-<-+-<-[start/restart]
 *   |                                 |
 *   |                                 |
 *   |                                 |
 *   +------>-[tr_transferred]---------+
 *   |                                 |
 *   +--------->-[tr_error]------------+
 *
 * NOTE: the USB-driver automatically
 * recovers from errors if 
 * "xfer->clearstall_xfer" is set
 *
 * Host-transmit callback example (bulk/interrupt/isochronous):
 * ============================================================
 * static void
 * usb_callback_tx(struct usbd_xfer *xfer)
 * {
 *   USBD_CHECK_STATUS(xfer);
 *
 * tr_transferred:
 * tr_setup:
 *
 *   ... setup "xfer->length" ...
 *
 *   ... write data to buffer ...
 *
 * tr_error:
 *
 *   ... [re-]transfer "xfer->buffer" ...
 *
 *   usbd_start_hardware(xfer);
 *   return;
 * }
 *
 * Host-receive callback example (bulk/interrupt/isochronous):
 * ===========================================================
 * static void
 * usb_callback_rx(struct usbd_xfer *xfer)
 * {
 *   USBD_CHECK_STATUS(xfer);
 *
 * tr_transferred:
 *
 *   ... process data in buffer ...
 *
 * tr_setup:
 *
 *   ... setup "xfer->length" ...
 *
 * tr_error:
 *
 *   ... [re-]transfer "xfer->buffer" ...
 *
 *   usbd_start_hardware(xfer);
 *   return;
 * }
 *
 *
 * "usbd_start_hardware()" is called when 
 * "xfer->buffer" is ready for transfer
 *
 * "usbd_start_hardware()" should only be called
 *  from callback
 */

/*---------------------------------------------------------------------------*
 *	usbd_start_hardware - start USB hardware for the given transfer
 *---------------------------------------------------------------------------*/
void
usbd_start_hardware(struct usbd_xfer *xfer)
{
	PRINTFN(0,("xfer=%p, pipe=%p len=%d dir=%s\n",
		    xfer, xfer->pipe, xfer->length, 
		    ((xfer->pipe->edesc->bEndpointAddress & 
		      (UE_DIR_IN|UE_DIR_OUT)) == UE_DIR_IN) ? "in" : "out"));

#ifdef USB_DEBUG
	if(usbdebug > 0)
	{
		mtx_lock(xfer->usb_mtx);

		usbd_dump_pipe(xfer->pipe);

		mtx_unlock(xfer->usb_mtx);
	}
#endif

	mtx_assert(xfer->priv_mtx, MA_OWNED);

	if(xfer->flags & USBD_DEV_OPEN)
	{
		/* set USBD_DEV_TRANSFERRING and USBD_DEV_RECURSED_2 */
		xfer->flags |= (USBD_DEV_TRANSFERRING|USBD_DEV_RECURSED_2);

		if(xfer->pipe->clearstall &&
		   xfer->clearstall_xfer)
		{
#ifdef USB_DEBUG
			if(xfer->clearstall_xfer->flags & USBD_DEV_TRANSFERRING)
			{
				PRINTF(("clearstall_xfer is transferrring!\n"));
			}
#endif
			/* the polling flag is inherited */

			if(xfer->flags & USBD_USE_POLLING)
			  xfer->clearstall_xfer->flags |= USBD_USE_POLLING;
			else
			  xfer->clearstall_xfer->flags &= ~USBD_USE_POLLING;

			/* store pointer to transfer */
			xfer->clearstall_xfer->priv_sc = xfer;

			usbd_transfer_start(xfer->clearstall_xfer);
		}
		else
		{
#ifdef USB_COMPAT_OLD
			if(xfer->d_copy_src)
			{
				bcopy(xfer->d_copy_src, xfer->d_copy_ptr, xfer->d_copy_len);
			}
			if(xfer->f_copy_src)
			{
				bcopy(xfer->f_copy_src, xfer->f_copy_ptr, xfer->f_copy_len);
			}
#endif
			mtx_lock(xfer->usb_mtx);

			/* enter the transfer */
			(xfer->pipe->methods->enter)(xfer);

			xfer->usb_thread = (xfer->flags & USBD_USE_POLLING) ? 
			  curthread : NULL;

			mtx_unlock(xfer->usb_mtx);
 		}
	}
	return;
}

void
usbd_transfer_start_safe(struct usbd_xfer *xfer)
{
	mtx_lock(xfer->priv_mtx);
	usbd_transfer_start(xfer);
	mtx_unlock(xfer->priv_mtx);
	return;
}

/*---------------------------------------------------------------------------*
 *	usbd_transfer_start - start a USB transfer
 *
 * NOTE: this function can be called any number of times
 * NOTE: if USBD_SYNCHRONOUS is set in "xfer->flags", then this
 *       function will sleep for transfer completion
 * NOTE: if USBD_USE_POLLING is set in "xfer->flags", then this
 *       function will spin until transfer is completed
 *---------------------------------------------------------------------------*/
void
usbd_transfer_start(struct usbd_xfer *xfer)
{
	mtx_assert(xfer->priv_mtx, MA_OWNED);

	if(!(xfer->flags & USBD_DEV_OPEN))
	{
		xfer->flags |= USBD_DEV_OPEN;

		/*
		 * open transfer
		 */
		mtx_lock(xfer->usb_mtx);
		(xfer->pipe->methods->open)(xfer);
		mtx_unlock(xfer->usb_mtx);
	}

	/* "USBD_DEV_TRANSFERRING" is only changed
	 * when "priv_mtx" is locked;
	 * check first recurse flag
	 */
	if(!(xfer->flags & (USBD_DEV_TRANSFERRING)))
	{
		if(xfer->flags & USBD_SELF_DESTRUCT)
		{
		    /* increment refcount 
		     * in case the callback
		     * frees itself:
		     */
		    mtx_lock(xfer->usb_mtx);
		    xfer->usb_root->memory_refcount++;
		    mtx_unlock(xfer->usb_mtx);
		}

		/* call callback */
		__usbd_callback(xfer);

		/* wait for completion
		 * if the transfer is synchronous
		 */
		if(xfer->flags & (USBD_SYNCHRONOUS|USBD_USE_POLLING))
		{
			u_int timeout = xfer->timeout +1;
			struct usbd_bus *bus = xfer->udev->bus;

			while(xfer->flags & USBD_DEV_TRANSFERRING)
			{
				if(bus->use_polling ||
				   (xfer->flags & USBD_USE_POLLING))
				{
					if(!timeout--)
					{
						/* stop the transfer */
						usbd_transfer_stop(xfer);
						break;
					}

					/* delay one millisecond */
					DELAY(1000);

					/* call the interrupt handler,
					 * which will call __usbd_callback():
					 */
					(bus->methods->do_poll)(bus);
				}
				else
				{
					u_int recurse = 0;// dpp xfer->priv_mtx->mtx_recurse;
					u_int __recurse = recurse;

					/* need to unlock private mutex so that
					 * callback can be called
					 */
					while(__recurse--)
					{
						mtx_unlock(xfer->priv_mtx);
					}

					if(msleep(&xfer->wait_q, xfer->priv_mtx,
						  (PRIBIO|PCATCH), "usbsync", 0))
					{
						/* stop the transfer */
						usbd_transfer_stop(xfer);
					}

					while(recurse--)
					{
						mtx_lock(xfer->priv_mtx);
					}
					break;
				}
			}
		}
		if(xfer->flags & USBD_SELF_DESTRUCT)
		{
		    usbd_drop_refcount(xfer->usb_root);
		}
	}
	return;
}

/*---------------------------------------------------------------------------*
 *	usbd_transfer_stop - stop a USB transfer
 *
 * NOTE: this function can be called any number of times
 *---------------------------------------------------------------------------*/
void
usbd_transfer_stop(struct usbd_xfer *xfer)
{
	mtx_assert(xfer->priv_mtx, MA_OWNED);

	if(xfer->flags & USBD_DEV_OPEN)
	{
		xfer->flags &= ~USBD_DEV_OPEN;

		/*
		 * stop clearstall first
		 */
		if(xfer->clearstall_xfer)
		{
			usbd_transfer_stop(xfer->clearstall_xfer);
		}

		/*
		 * close transfer (should not call callback)
		 */
		mtx_lock(xfer->usb_mtx);
		(xfer->pipe->methods->close)(xfer);

		/* always set error */
		xfer->error = USBD_CANCELLED;

		if(xfer->flags & USBD_SELF_DESTRUCT)
		{
		    /* increment refcount 
		     * in case the callback
		     * frees itself:
		     */
		    mtx_lock(xfer->usb_mtx);
		    xfer->usb_root->memory_refcount++;
		    mtx_unlock(xfer->usb_mtx);
		}

		if(xfer->flags & USBD_DEV_TRANSFERRING)
		{
		    /* increment refcount so that scheduled
		     * callbacks, if any, are not called by 
		     * the interrupt or timeout routines:
		     */
		    xfer->usb_refcount++;

		    /* call callback, which 
		     * will clear USBD_DEV_TRANSFERRING
		     */
		    __usbd_callback(xfer);
		}
		mtx_unlock(xfer->usb_mtx);

		if(xfer->flags & USBD_SELF_DESTRUCT)
		{
		    usbd_drop_refcount(xfer->usb_root);
		}
	}
	return;
}

/*---------------------------------------------------------------------------*
 *	__usbd_callback
 *
 * This is a wrapper for USB callbacks, which handles
 * recursation, which can happen during boot.
 *---------------------------------------------------------------------------*/
void
__usbd_callback(struct usbd_xfer *xfer)
{
	mtx_assert(xfer->priv_mtx, MA_OWNED);

	/* check first recurse flag */
	if(!(xfer->flags & USBD_DEV_RECURSED_1))
	{
		do {
			/* set both recurse flags */
			xfer->flags |= (USBD_DEV_RECURSED_2|
					USBD_DEV_RECURSED_1);

			/* call processing routine */
			printf("__usbd_callback: call processing routine\n");
			(xfer->callback)(xfer);

		/* check second recurse flag */
		} while(!(xfer->flags & USBD_DEV_RECURSED_2));

		/* clear first recurse flag */
		xfer->flags &= ~USBD_DEV_RECURSED_1;
	}
	else
	{
		/* clear second recurse flag */
		xfer->flags &= ~USBD_DEV_RECURSED_2;
	}
	return;
}

/*---------------------------------------------------------------------------*
 *	usbd_do_callback
 *
 * This function is used to call back a list of USB callbacks. 
 *---------------------------------------------------------------------------*/
void
usbd_do_callback(struct usbd_callback_info *ptr, 
		 struct usbd_callback_info *limit)
{
    struct usbd_xfer *xfer;

    if(limit < ptr)
    {
        /* parameter order switched */
        register void *temp = ptr;
	ptr = limit;
	limit = temp;
    }

    while(ptr < limit)
    {
        xfer = ptr->xfer;

	/*
	 * During the unlocked period, the
	 * transfer can be restarted by 
	 * another thread, which must be
	 * checked here:
	 */
	mtx_lock(xfer->priv_mtx);

	if(xfer->usb_refcount == ptr->refcount)
	{
	    /* call callback */
	    __usbd_callback(xfer);
	}
	/* 
	 * else already called back !
	 */
	mtx_unlock(xfer->priv_mtx);

	usbd_drop_refcount(xfer->usb_root);

	ptr++;
    }
    return;
}

/*---------------------------------------------------------------------------*
 *	usbd_transfer_done
 *
 * NOTE: this function does not call the callback!
 *---------------------------------------------------------------------------*/
void
usbd_transfer_done(struct usbd_xfer *xfer, usbd_status error)
{
	mtx_assert(xfer->usb_mtx, MA_OWNED);

	PRINTFN(5,("xfer=%p pipe=%p status=%d "
		    "actlen=%d\n", xfer, xfer->pipe, error, xfer->actlen));

#if defined(DIAGNOSTIC) || defined(USB_DEBUG)
	if(xfer->pipe == NULL)
	{
		printf("xfer=%p, pipe=NULL\n", xfer);
		return;
	}
#endif
       	/* count completed transfers */
	++(xfer->udev->bus->stats.uds_requests
		[xfer->pipe->edesc->bmAttributes & UE_XFERTYPE]);

	/* check for short transfers */
	if(!error)
	{
		if(xfer->actlen > xfer->length)
		{
			printf("%s: overrun actlen(%d) > len(%d)\n",
			       __FUNCTION__, xfer->actlen, xfer->length);
			xfer->actlen = xfer->length;
		}

		if((xfer->actlen < xfer->length) &&
		   !(xfer->flags & USBD_SHORT_XFER_OK))
		{
			printf("%s: short transfer actlen(%d) < len(%d)\n",
			       __FUNCTION__, xfer->actlen, xfer->length);
			error = USBD_SHORT_XFER;
		}
	}
	xfer->error = error;
	return;
}

/*---------------------------------------------------------------------------*
 *	usbd_transfer_enqueue
 *
 * This function is used to put a USB transfer on
 * the pipe list. If there was no previous 
 * USB transfer on the list, the start method of
 * the transfer will be called.
 *---------------------------------------------------------------------------*/
void
usbd_transfer_enqueue(struct usbd_xfer *xfer)
{
	mtx_assert(xfer->usb_mtx, MA_OWNED);

	/* if xfer is not inserted, 
	 * insert xfer in xfer queue
	 */
	if(xfer->pipe_list.le_prev == NULL)
	{
		LIST_INSERT_HEAD(&xfer->pipe->list_head, xfer, pipe_list);

		/* start first transfer enqueued */

		if(xfer->pipe_list.le_next == NULL)
		{
			(xfer->pipe->methods->start)(xfer);
		}
	}
	return;
}

/*---------------------------------------------------------------------------*
 *	usbd_transfer_dequeue
 *
 * This function is used to remove a USB transfer from
 * the pipe list. If the first USB transfer on the pipe
 * list is removed, the start method of the next USB
 * transfer will be called, if any.
 *---------------------------------------------------------------------------*/
void
usbd_transfer_dequeue(struct usbd_xfer *xfer)
{
	mtx_assert(xfer->usb_mtx, MA_OWNED);

	/* if two transfers are queued, the
	 * second transfer must be started
	 * before the first is called back
	 */

	/* if xfer is not removed,
	 * remove xfer from xfer queue
	 */
	if(xfer->pipe_list.le_prev)
	{
		LIST_REMOVE(xfer,pipe_list);

		/* if started transfer is dequeued,
		 * start next transfer
		 */
		if((xfer->pipe_list.le_next == 0) && /* last xfer */
		   (!LIST_EMPTY(&xfer->pipe->list_head)))
		{
		  (xfer->pipe->methods->start)
		    ((void *)
		     (((u_int8_t *)(xfer->pipe_list.le_prev)) - 
		      POINTER_TO_UNSIGNED(&LIST_NEXT((struct usbd_xfer *)0,pipe_list))));
		}
		xfer->pipe_list.le_prev = 0;
	}
	return;
}

void
usbd_default_callback(struct usbd_xfer *xfer)
{
	USBD_CHECK_STATUS(xfer);

 tr_setup:
	/**/
	usbd_start_hardware(xfer);
	return;

 tr_transferred:
 tr_error:
	if((xfer->flags & USBD_SYNCHRONOUS) && 
	   (!(xfer->udev->bus->use_polling || (xfer->flags & USBD_USE_POLLING))))
	{
		wakeup(&xfer->wait_q);
	}

#ifdef USB_COMPAT_OLD

	if(xfer->d_copy_dst)
	{
		bcopy(xfer->d_copy_ptr, xfer->d_copy_dst, xfer->d_copy_len);
	}

	if(xfer->f_copy_dst)
	{
		bcopy(xfer->f_copy_ptr, xfer->f_copy_dst, xfer->f_copy_len);
	}

	if(xfer->d_callback)
	{
		PRINTFN(3,("xfer=%p, error=0x%x(%s)\n", xfer, 
			    xfer->error, usbd_errstr(xfer->error)));

		(xfer->d_callback)(xfer->priv_fifo, xfer->priv_sc, xfer->error);

		if(((void *)(xfer->pipe->alloc_xfer)) == xfer->priv_fifo)
		{
			/* restart transfer */
			usbd_start_hardware(xfer);
		}
	}
#endif
	return;
}

usbd_status
usbd_do_request(struct usbd_device *udev, usb_device_request_t *req, void *data)
{
	return (usbd_do_request_flags(udev, req, data, 0, 0, 
				      USBD_DEFAULT_TIMEOUT));
}

/*---------------------------------------------------------------------------*
 *	usbd_do_request_flags
 *
 * NOTE: the caller should hold "Giant" while calling this function
 *---------------------------------------------------------------------------*/
usbd_status
usbd_do_request_flags(struct usbd_device *udev, usb_device_request_t *req,
		      void *data, u_int32_t flags, int *actlen,
		      u_int32_t timeout)
{
	struct usbd_config usbd_config[1] = { /* zero */ };
	struct usbd_xfer *xfer = NULL;
	usbd_status err;

	usbd_config[0].type = UE_CONTROL;
	usbd_config[0].endpoint = 0; /* control pipe */
	usbd_config[0].direction = -1;
	usbd_config[0].timeout = timeout;
	usbd_config[0].flags = flags|USBD_SYNCHRONOUS;
	usbd_config[0].bufsize = sizeof(req[0]) + UGETW(req->wLength);
	usbd_config[0].callback = &usbd_default_callback;

	/* setup transfer */
	err = usbd_transfer_setup(udev, 0, &xfer, &usbd_config[0], 1,
				  NULL, NULL, NULL);
	if(err)
	{
	    goto done;
	}

	/* copy IN */

	bcopy(req, xfer->buffer, sizeof(req[0]));

	if(!(req->bmRequestType & UT_READ))
	{
	    bcopy(data, ((u_int8_t *)xfer->buffer) + sizeof(req[0]), UGETW(req->wLength));
	}

	usbd_transfer_start_safe(xfer);

	/* copy OUT */

	if(req->bmRequestType & UT_READ)
	{
	    bcopy(((u_int8_t *)xfer->buffer) + sizeof(req[0]), data, UGETW(req->wLength));
	}

	err = xfer->error;

	if(actlen != NULL)
	{
	    actlen[0] = (xfer->actlen < sizeof(req[0])) ?
	      0 : (xfer->actlen - sizeof(req[0]));
	}

 done:
	usbd_transfer_unsetup(&xfer, 1);
	return (err);
}

void
usbd_fill_get_report(usb_device_request_t *req, u_int8_t iface_no, 
		     u_int8_t type, u_int8_t id, u_int16_t size)
{
        req->bmRequestType = UT_READ_CLASS_INTERFACE;
        req->bRequest = UR_GET_REPORT;
        USETW2(req->wValue, type, id);
        USETW(req->wIndex, iface_no);
        USETW(req->wLength, size);
	return;
}

void
usbd_fill_set_report(usb_device_request_t *req, u_int8_t iface_no,
		     u_int8_t type, u_int8_t id, u_int16_t size)
{
        req->bmRequestType = UT_WRITE_CLASS_INTERFACE;
        req->bRequest = UR_SET_REPORT;
        USETW2(req->wValue, type, id);
        USETW(req->wIndex, iface_no);
        USETW(req->wLength, size);
	return;
}

void
usbd_clear_stall_tr_setup(struct usbd_xfer *xfer1, 
			  struct usbd_xfer *xfer2)
{
	usb_device_request_t *req = xfer1->buffer;

	mtx_assert(xfer1->priv_mtx, MA_OWNED);
	mtx_assert(xfer2->priv_mtx, MA_OWNED);

	/* setup a clear-stall packet */

	req->bmRequestType = UT_WRITE_ENDPOINT;
	req->bRequest = UR_CLEAR_FEATURE;
	USETW(req->wValue, UF_ENDPOINT_HALT);
	req->wIndex[0] = xfer2->pipe->edesc->bEndpointAddress;
	req->wIndex[1] = 0;
	USETW(req->wLength, 0);

	usbd_start_hardware(xfer1);
	return;
}

void
usbd_clear_stall_tr_transferred(struct usbd_xfer *xfer1, 
				struct usbd_xfer *xfer2)
{
	mtx_assert(xfer1->priv_mtx, MA_OWNED);
	mtx_assert(xfer2->priv_mtx, MA_OWNED);

	mtx_lock(xfer2->usb_mtx);

	/* 
	 * clear any stall and make sure 
	 * that DATA0 toggle will be 
	 * used next:
	 */

	xfer2->pipe->clearstall = 0;
	xfer2->pipe->toggle_next = 0;

	mtx_unlock(xfer2->usb_mtx);

	return;
}

void
usbd_clearstall_callback(struct usbd_xfer *xfer)
{
	USBD_CHECK_STATUS(xfer);

 tr_setup:
	usbd_clear_stall_tr_setup(xfer, xfer->priv_sc);
	return;

 tr_transferred:
 tr_error:
	PRINTFN(3,("xfer=%p\n", xfer));

	/* NOTE: some devices reject this command,
	 * so ignore a STALL
	 */
	usbd_clear_stall_tr_transferred(xfer, xfer->priv_sc);

	usbd_start_hardware(xfer->priv_sc);
	return;
}

/* clearstall config:
 *
 *	.type = UE_CONTROL,
 *	.endpoint = 0,
 *	.direction = -1,
 *	.timeout = USBD_DEFAULT_TIMEOUT,
 *	.flags = 0,
 *	.bufsize = sizeof(usb_device_request_t),
 *	.callback = &usbd_clearstall_callback,
 */

/*
 * called from keyboard driver when in polling mode
 */
void
usbd_do_poll(struct usbd_device *udev)
{
	(udev->bus->methods->do_poll)(udev->bus);
	return;
}

void
usbd_set_polling(struct usbd_device *udev, int on)
{
	if(on)
	{
		udev->bus->use_polling++;
	}
	else
	{
		udev->bus->use_polling--;
	}

	/* make sure there is nothing pending to do */
	if(udev->bus->use_polling)
	{
		(udev->bus->methods->do_poll)(udev->bus);
	}
	return;
}

/*
 * usbd_ratecheck() can limit the number of error messages that occurs.
 * When a device is unplugged it may take up to 0.25s for the hub driver
 * to notice it.  If the driver continuosly tries to do I/O operations
 * this can generate a large number of messages.
 */
int
usbd_ratecheck(struct timeval *last)
{
	/*if(last->tv_sec == time_second)
	{
		return (0);
	}
	last->tv_sec = time_second;
	*/
	return (1);
}

/*
 * Search for a vendor/product pair in an array.  The item size is
 * given as an argument.
 */
const struct usb_devno *
usb_match_device(const struct usb_devno *tbl, u_int nentries, u_int size,
		 u_int16_t vendor, u_int16_t product)
{
	while(nentries-- > 0)
	{
		if((tbl->ud_vendor == vendor) &&
		   ((tbl->ud_product == product) ||
		    (tbl->ud_product == USB_PRODUCT_ANY)))
		{
			return (tbl);
		}
		tbl = (const struct usb_devno *)
		  (((const u_int8_t *)tbl) + size);
	}
	return (NULL);
}

int
usbd_driver_load(struct module *mod, int what, void *arg)
{
	/* XXX should implement something like a 
	 * function that removes all generic devices
	 */

 	return (0);
}

#ifdef USB_COMPAT_OLD

/*****************************************************************************
 * compatibility layer for the old USB drivers
 *****************************************************************************/

usbd_status
usbd_transfer(struct usbd_xfer *xfer)
{
	if(xfer->alloc_xfer)
	{
	    usbd_transfer_start_safe(xfer->alloc_xfer);
	    return USBD_NORMAL_COMPLETION;
	}
	else
	{
	    return USBD_NOMEM;
	}
}

usbd_status
usbd_sync_transfer(struct usbd_xfer *xfer)
{
	if(xfer->alloc_xfer)
	{
	    /* XXX USBD_SYNCHRONOUS should be set in struct usbd_config 
	     * passed to usbd_transfer_setup (!)
	     */
	    xfer->alloc_xfer->flags |= USBD_SYNCHRONOUS;
	}
	return usbd_transfer(xfer);
}

void *
usbd_alloc_buffer(struct usbd_xfer *xfer, u_int32_t size)
{
	if(xfer->alloc_ptr)
	{
	    printf("%s: buffer already "
		   "allocated\n", __FUNCTION__);
	    return NULL;
	}

	xfer->alloc_ptr = malloc(size, M_USBDEV, M_NOWAIT);
	xfer->alloc_len = size;

	return xfer->alloc_ptr;
}

void
usbd_free_buffer(struct usbd_xfer *xfer)
{
	if(xfer->alloc_ptr)
	{
	    free(xfer->alloc_ptr, M_USBDEV);
	    xfer->alloc_ptr = NULL;
	}
	return;
}

void
usbd_get_xfer_status(struct usbd_xfer *xfer, void **priv,
		     void **buffer, u_int32_t *count, usbd_status *status)
{
	PRINTFN(8, ("\n"));

	if(xfer->alloc_xfer)
	{
	    if(priv != NULL)
	      *priv = xfer->alloc_xfer->priv_sc;
	    if(buffer != NULL)
	      *buffer = xfer->alloc_xfer->buffer;
	    if(count != NULL)
	      *count = xfer->alloc_xfer->actlen;
	    if(status != NULL)
	      *status = xfer->alloc_xfer->error;
	}
	else
	{
	    if(priv != NULL)
	      *priv = xfer->priv_sc;
	    if(buffer != NULL)
	      *buffer = xfer->buffer;
	    if(count != NULL)
	      *count = xfer->actlen;
	    if(status != NULL)
	      *status = xfer->error;
	}
	return;
}

struct usbd_xfer *
usbd_alloc_xfer(struct usbd_device *dev)
{
	struct usbd_xfer *xfer;

	xfer = malloc(sizeof(*xfer), M_USBDEV, M_NOWAIT);

	if(xfer)
	{
	    bzero(xfer, sizeof(*xfer));
	}
	return xfer;
}

static void
__usbd_free_xfer(struct usbd_xfer *xfer)
{
	if(xfer)
	{
	    if(xfer->clearstall_xfer)
	    {
	        usbd_transfer_unsetup(&xfer->clearstall_xfer, 1);
	    }
	    usbd_transfer_unsetup(&xfer, 1);
	}
	return;
}

usbd_status 
usbd_free_xfer(struct usbd_xfer *xfer)
{
	if(xfer)
	{
	    usbd_free_buffer(xfer);

	    __usbd_free_xfer(xfer->alloc_xfer);

	    free(xfer, M_USBDEV);
	}
	return USBD_NORMAL_COMPLETION;
}

usbd_status 
usbd_open_pipe(struct usbd_interface *iface, u_int8_t address,
               u_int8_t flags, struct usbd_pipe **pp)
{ 
	u_int8_t iface_index = iface - &iface->udev->ifaces[0];
        struct usbd_pipe *pipe = &iface->udev->pipes[0];
        struct usbd_pipe *pipe_end = &iface->udev->pipes_end[0];

	while(pipe < pipe_end)
	{
	    if((pipe->iface_index == iface_index) && 
	       (pipe->edesc) &&
	       (pipe->edesc->bEndpointAddress == address))
	    {
	        (*pp) = pipe;
		return USBD_NORMAL_COMPLETION;
	    }
	    pipe++;
	}
	(*pp) = NULL;
	return USBD_INVAL;
}

usbd_status 
usbd_open_pipe_intr(struct usbd_interface *iface, u_int8_t address,
                    u_int8_t flags, struct usbd_pipe **pipe,
                    void *priv, void *buffer, u_int32_t len,
                    usbd_callback callback, int ival)
{
	usbd_status error;

	if(usbd_open_pipe(iface, address, 0, pipe))
	{
	    return USBD_INVAL;
	}

	if((*pipe)->alloc_xfer)
	{
	    printf("%s: warning: pipe already opened\n",
		   __FUNCTION__);
	}

	(*pipe)->alloc_xfer = usbd_alloc_xfer(NULL);

	if(!(*pipe)->alloc_xfer)
	{
	    return USBD_NOMEM;
	}

        error = usbd_setup_xfer((*pipe)->alloc_xfer, *pipe, priv, buffer, len, flags,
			        USBD_NO_TIMEOUT, callback);
	if(error == 0)
	{
	    error = usbd_transfer((*pipe)->alloc_xfer);
	}
	if(error)
	{
	    usbd_free_xfer((*pipe)->alloc_xfer);
	    (*pipe)->alloc_xfer = 0;
	    *pipe = NULL;
	}
	return error;
}

usbd_status
usbd_setup_xfer(struct usbd_xfer *xfer, struct usbd_pipe *pipe,
                void *priv, void *buffer, u_int32_t length,
                u_int32_t flags, u_int32_t timeout,
                usbd_callback callback)
{
	struct usbd_config usbd_config[2] = { /* zero */ };
	struct usbd_xfer *__xfer[2];

	/* free current transfer, if any */
	if(xfer->alloc_xfer)
	{
	    if(xfer->alloc_xfer->flags & USBD_DEV_TRANSFERRING)
	    {
	        /* transfer is in progress */
	        PRINTFN(3,("transfer is already in progress\n"));
		return USBD_NORMAL_COMPLETION;
	    }

	    __usbd_free_xfer(xfer->alloc_xfer);
	    xfer->alloc_xfer = NULL;
	}

	usbd_config[1].type = UE_CONTROL;
	usbd_config[1].endpoint = 0;
	usbd_config[1].direction = -1;
	usbd_config[1].timeout = USBD_DEFAULT_TIMEOUT;
	usbd_config[1].flags = USBD_SELF_DESTRUCT;
	usbd_config[1].bufsize = sizeof(usb_device_request_t);
	usbd_config[1].callback = &usbd_clearstall_callback;

	usbd_config[0].type = pipe->edesc->bmAttributes & UE_XFERTYPE;
	usbd_config[0].endpoint = pipe->edesc->bEndpointAddress & UE_ADDR;
	usbd_config[0].direction = pipe->edesc->bEndpointAddress & (UE_DIR_IN|UE_DIR_OUT);
	usbd_config[0].callback = usbd_default_callback;
	usbd_config[0].interval = USBD_DEFAULT_INTERVAL;
	usbd_config[0].timeout = timeout;
	usbd_config[0].flags = flags|USBD_SELF_DESTRUCT;
	usbd_config[0].bufsize = length;

	if(usbd_transfer_setup(pipe->udev, pipe->iface_index, 
			       &__xfer[0], &usbd_config[0], 
			       (flags & USBD_CUSTOM_CLEARSTALL) ? 1 : 2,
			       priv, NULL, NULL))
	{
	    PRINTFN(3,("USBD_NOMEM\n"));
	    return USBD_NOMEM;
	}

	xfer->alloc_xfer = __xfer[0];

	/* automatic clear-stall */
	xfer->alloc_xfer->clearstall_xfer = 
	  (flags & USBD_CUSTOM_CLEARSTALL) ? NULL : __xfer[1];

	xfer->alloc_xfer->priv_fifo = xfer; /* used by callback */
	xfer->alloc_xfer->d_copy_ptr = xfer->alloc_xfer->buffer;
	xfer->alloc_xfer->d_copy_len = length;
	xfer->alloc_xfer->d_callback = callback;

	if(length && buffer)
	{
	    if(usbd_config[0].direction == UE_DIR_IN)
	    {
	        xfer->alloc_xfer->d_copy_dst = buffer;
	    }
	    else
	    {
	        xfer->alloc_xfer->d_copy_src = buffer;
	    }
	}
	return USBD_NORMAL_COMPLETION;
}

usbd_status
usbd_setup_default_xfer(struct usbd_xfer *xfer, struct usbd_device *udev,
                        void *priv, u_int32_t timeout,
                        usb_device_request_t *req, void *buffer,
                        u_int32_t length, u_int16_t flags,
                        usbd_callback callback)
{
	struct usbd_config usbd_config[1] = { /* zero */ };

	/* free current transfer, if any */
	if(xfer->alloc_xfer)
	{
	    if(xfer->alloc_xfer->flags & USBD_DEV_TRANSFERRING)
	    {
	        /* transfer is in progress */
	        PRINTFN(3,("transfer is already in progress\n"));
		return USBD_NORMAL_COMPLETION;
	    }

	    __usbd_free_xfer(xfer->alloc_xfer);
	    xfer->alloc_xfer = NULL;
	}
	
	usbd_config[0].type = UE_CONTROL;
	usbd_config[0].endpoint = 0;
	usbd_config[0].direction = -1;
	usbd_config[0].timeout = timeout;
	usbd_config[0].flags = flags|USBD_SELF_DESTRUCT;
	usbd_config[0].bufsize = sizeof(usb_device_request_t) + length;
	usbd_config[0].callback = usbd_default_callback;

	if(usbd_transfer_setup(udev, 0, 
			       &xfer->alloc_xfer, &usbd_config[0], 1, 
			       priv, NULL, NULL))
	{
	    PRINTFN(3,("USBD_NOMEM\n"));
	    return USBD_NOMEM;
	}

	bcopy(req, xfer->alloc_xfer->buffer, sizeof(*req));

	xfer->alloc_xfer->priv_fifo = xfer; /* used by callback */
	xfer->alloc_xfer->d_copy_ptr = ((u_int8_t *)(xfer->alloc_xfer->buffer)) + sizeof(*req);
	xfer->alloc_xfer->d_copy_len = length;
	xfer->alloc_xfer->d_callback = callback;

	if(length && buffer)
	{
	    if(req->bmRequestType & UT_READ)
	    {
	        xfer->alloc_xfer->d_copy_dst = buffer;
	    }
	    else
	    {
	        xfer->alloc_xfer->d_copy_src = buffer;
	    }
	}
	return USBD_NORMAL_COMPLETION;
}

usbd_status
usbd_setup_isoc_xfer(struct usbd_xfer *xfer, struct usbd_pipe *pipe,
                     void *priv, u_int16_t *frlengths, u_int32_t nframes, 
		     u_int16_t flags, usbd_callback callback)
{
	struct usbd_config usbd_config[1] = { /* zero */ };

	/* free current transfer, if any */
	if(xfer->alloc_xfer)
	{
	    if(xfer->alloc_xfer->flags & USBD_DEV_TRANSFERRING)
	    {
	        /* transfer is in progress */
	        PRINTFN(3,("transfer is already in progress\n"));
		return USBD_NORMAL_COMPLETION;
	    }
	    __usbd_free_xfer(xfer->alloc_xfer);
	    xfer->alloc_xfer = NULL;
	}

	if(xfer->alloc_ptr == NULL)
	{
	    /* no data-buffer allocated */
	    return USBD_NOMEM;
	}

	usbd_config[0].type = pipe->edesc->bmAttributes & UE_XFERTYPE;
	usbd_config[0].endpoint = pipe->edesc->bEndpointAddress & UE_ADDR;
	usbd_config[0].direction = pipe->edesc->bEndpointAddress & (UE_DIR_IN|UE_DIR_OUT);
	usbd_config[0].callback = usbd_default_callback;
	usbd_config[0].interval = USBD_DEFAULT_INTERVAL;
	usbd_config[0].flags = flags|USBD_SELF_DESTRUCT;
	usbd_config[0].bufsize = xfer->alloc_len;
	usbd_config[0].frames = nframes;

	if(usbd_transfer_setup(pipe->udev, pipe->iface_index, 
			       &xfer->alloc_xfer, &usbd_config[0], 1,
			       priv, NULL, NULL))
	{
	    return USBD_NOMEM;
	}

	xfer->alloc_xfer->priv_fifo = xfer; /* used by callback */
	xfer->alloc_xfer->d_copy_ptr = xfer->alloc_xfer->buffer;
	xfer->alloc_xfer->d_copy_len = xfer->alloc_len;
	xfer->alloc_xfer->f_copy_ptr = &xfer->alloc_xfer->frlengths[0];
	xfer->alloc_xfer->f_copy_len = nframes*sizeof(xfer->alloc_xfer->frlengths[0]);
	xfer->alloc_xfer->d_callback = callback;

	xfer->alloc_xfer->f_copy_src = frlengths;
	xfer->alloc_xfer->f_copy_dst = frlengths;

	if(usbd_config[0].direction == UE_DIR_IN)
	{
	    xfer->alloc_xfer->d_copy_dst = xfer->alloc_ptr;
	}
	else
	{
	    xfer->alloc_xfer->d_copy_src = xfer->alloc_ptr;
	}
	return USBD_NORMAL_COMPLETION;
}

usbd_status
usbd_bulk_transfer(struct usbd_xfer *xfer, struct usbd_pipe *pipe,
                   u_int16_t flags, u_int32_t timeout, void *buf,
                   u_int32_t *size, char *lbl)
{
        usbd_status err;

        if(usbd_setup_xfer(xfer, pipe, 0, buf, *size,
			   flags|USBD_SYNCHRONOUS, timeout, NULL))
	{
	    return USBD_NOMEM;
	}

	if(usbd_transfer(xfer))
	{
	    return USBD_NOMEM;
	}

        usbd_get_xfer_status(xfer, NULL, NULL, size, &err);
        return (err);
}

usbd_status 
usbd_abort_pipe(struct usbd_pipe *pipe)
{
	enum { FINISH_LIST_MAX = 16 };

	struct usbd_xfer * finish_list[FINISH_LIST_MAX];
	struct usbd_xfer **ptr;

	ptr = &finish_list[0];

	struct usbd_xfer *xfer;

	mtx_lock(&pipe->udev->bus->mtx);

	xfer = pipe->alloc_xfer;

	if(xfer)
	{
	    xfer = xfer->alloc_xfer;

	    if(xfer)
	    {
	        *ptr++ = xfer;

		/* make sure that memory doesn't
		 * get freed during the lock switch:
		 */
		xfer->usb_root->memory_refcount++;
	    }
	}

	LIST_FOREACH(xfer, &pipe->list_head, pipe_list)
	{
	    *ptr++ = xfer;

	    /* make sure that memory doesn't
	     * get freed during the lock switch:
	     */
	    xfer->usb_root->memory_refcount++;

	    if(ptr >= &finish_list[FINISH_LIST_MAX])
	    {
	        printf("%s: too many xfers "
		       "on pipe list!\n", __FUNCTION__);
		break;
	    }
	}

	mtx_unlock(&pipe->udev->bus->mtx);

	while(--ptr >= &finish_list[0])
	{
	    xfer = ptr[0];

	    mtx_lock(xfer->priv_mtx);

	    usbd_transfer_stop(xfer);

	    mtx_unlock(xfer->priv_mtx);

	    usbd_drop_refcount(xfer->usb_root);
	}
	return USBD_NORMAL_COMPLETION;
}

usbd_status 
usbd_abort_default_pipe(struct usbd_device *udev)
{
	return (usbd_abort_pipe(&udev->default_pipe));
}

usbd_status
usbd_close_pipe(struct usbd_pipe *pipe)
{
	usbd_free_xfer(pipe->alloc_xfer);
	pipe->alloc_xfer = NULL;

	return USBD_NORMAL_COMPLETION;
}

usbd_status 
usbd_clear_endpoint_stall(struct usbd_pipe *pipe)
{
	return USBD_NORMAL_COMPLETION;
}

usbd_status 
usbd_clear_endpoint_stall_async(struct usbd_pipe *pipe)
{
	return USBD_NORMAL_COMPLETION;
}

usbd_status 
usbd_endpoint_count(struct usbd_interface *iface, u_int8_t *count)
{
        *count = iface->idesc->bNumEndpoints;
        return (USBD_NORMAL_COMPLETION);
}

void
usbd_interface2device_handle(struct usbd_interface *iface,
                             struct usbd_device **udev)
{
        *udev = iface->udev;
}

struct usbd_device *
usbd_pipe2device_handle(struct usbd_pipe *pipe)
{
        return (pipe->udev);
}

usbd_status 
usbd_device2interface_handle(struct usbd_device *udev,
                             u_int8_t iface_index, struct usbd_interface **iface)
{
	(*iface) = usbd_get_iface(udev, iface_index);

	return ((*iface) ? USBD_NORMAL_COMPLETION : USBD_INVAL);
}

usb_endpoint_descriptor_t *
usbd_interface2endpoint_descriptor(struct usbd_interface *iface, u_int8_t index)
{
	u_int8_t iface_index = iface - &iface->udev->ifaces[0];
        struct usbd_pipe *pipe = &iface->udev->pipes[0];
        struct usbd_pipe *pipe_end = &iface->udev->pipes_end[0];

        if(index >= iface->idesc->bNumEndpoints)
	{
	    return (NULL);
	}

	while(pipe < pipe_end)
	{
	    if((pipe->edesc) &&
	       (pipe->iface_index == iface_index))
	    {
	        if(!index--)
		{
		    return(pipe->edesc);
		}
	    }
	    pipe++;
	}
	return (NULL);
}

usb_endpoint_descriptor_t *
usbd_get_endpoint_descriptor(struct usbd_interface *iface, u_int8_t address)
{
	struct usbd_pipe *pipe = &iface->udev->pipes[0];
	struct usbd_pipe *pipe_end = &iface->udev->pipes_end[0];

	while(pipe < pipe_end)
	{
	    if(pipe->edesc &&
	       (pipe->edesc->bEndpointAddress == address))
	    {
	        return pipe->edesc;
	    }
	    pipe++;
	}
	return NULL;
}

void
usb_call_task(void *arg, int count)
{
	struct usb_task *task = arg;

	((task)->func)((task)->arg);

	return;
}

#endif /* USB_COMPAT_OLD */
