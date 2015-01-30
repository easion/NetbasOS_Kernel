/*-
 * Copyright (c) 2006 Hans Petter Selasky. All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *
 * usb_cdev.c - An abstraction layer for creation of devices under /dev/...
 *
 */

#include "netbas.h"


#include <dev/usb2/usb_port.h>
#include <dev/usb2/usb.h>
#include <dev/usb2/usb_subr.h>

#ifndef __NETBAS__

__FBSDID("$FreeBSD: src/sys/dev/usb2/usb_cdev.c $");

#ifdef USB_DEBUG
#define DPRINTF(n,fmt,...)						\
  do { if (usb_cdev_debug > (n)) {						\
      printf("%s: " fmt, __FUNCTION__,## __VA_ARGS__); } } while (0)

static int usb_cdev_debug = 0;
SYSCTL_NODE(_hw_usb, OID_AUTO, cdev, CTLFLAG_RW, 0, "USB cdev");
SYSCTL_INT(_hw_usb_cdev, OID_AUTO, debug, CTLFLAG_RW,
	   &usb_cdev_debug, 0, "usb cdev debug level");
#else
#define DPRINTF(...)
#endif

#define DEV2SC(dev) (dev)->si_drv1

extern cdevsw_t usb_cdev_cdevsw;

static u_int32_t
usb_cdev_get_context(int32_t fflags)
{
	u_int32_t context_bit = 0;

	if (fflags & FREAD) {
	    context_bit |= (USB_CDEV_FLAG_GONE|

			    USB_CDEV_FLAG_OPEN_READ|
			    USB_CDEV_FLAG_SLEEP_READ|
			    USB_CDEV_FLAG_SLEEP_IOCTL_RD|
			    USB_CDEV_FLAG_WAKEUP_READ|

			    USB_CDEV_FLAG_WAKEUP_IOCTL_RD|
			    USB_CDEV_FLAG_SELECT_READ|
			    USB_CDEV_FLAG_CLOSING_READ|
			    USB_CDEV_FLAG_ERROR_READ|

			    USB_CDEV_FLAG_WRITE_ONLY|
			    0);
	}

	if (fflags & FWRITE) {
	    context_bit |= (USB_CDEV_FLAG_GONE|
			    USB_CDEV_FLAG_FLUSHING_WRITE|

			    USB_CDEV_FLAG_OPEN_WRITE|
			    USB_CDEV_FLAG_SLEEP_WRITE|
			    USB_CDEV_FLAG_SLEEP_IOCTL_WR|
			    USB_CDEV_FLAG_WAKEUP_WRITE|

			    USB_CDEV_FLAG_WAKEUP_IOCTL_WR|
			    USB_CDEV_FLAG_SELECT_WRITE|
			    USB_CDEV_FLAG_CLOSING_WRITE|
			    USB_CDEV_FLAG_ERROR_WRITE|

			    USB_CDEV_FLAG_READ_ONLY|
			    0);
	}
	return context_bit;
}

static int32_t
usb_cdev_exit_context(struct usb_cdev *sc, u_int32_t context_bit, 
		      int32_t error)
{
	sc->sc_flags &= ~context_bit;

	if (context_bit & USB_CDEV_FLAG_SLEEP_READ) {

	    if (sc->sc_flags & (USB_CDEV_FLAG_ERROR_READ|
				USB_CDEV_FLAG_GONE)) {
	        error = ENXIO;
	    }

	    if (sc->sc_flags & USB_CDEV_FLAG_CLOSING_READ) {
	        wakeup(&(sc->sc_wakeup_close_read));
		error = EINTR;
	    }
	}
	if (context_bit & USB_CDEV_FLAG_SLEEP_IOCTL_RD) {

	    if (sc->sc_flags & USB_CDEV_FLAG_GONE) {
	        error = ENXIO;
	    }

	    if (sc->sc_flags & USB_CDEV_FLAG_CLOSING_READ) {
	        wakeup(&(sc->sc_wakeup_close_read));
		error = EINTR;
	    }
	}
	if (context_bit & USB_CDEV_FLAG_SLEEP_WRITE) {

	    if (sc->sc_flags & (USB_CDEV_FLAG_ERROR_WRITE|
				USB_CDEV_FLAG_GONE)) {
	        error = ENXIO;
	    }

	    if (sc->sc_flags & USB_CDEV_FLAG_CLOSING_WRITE) {
	        wakeup(&(sc->sc_wakeup_close_write));
		error = EINTR;
	    }
	}
	if (context_bit & USB_CDEV_FLAG_SLEEP_IOCTL_WR) {

	    if (sc->sc_flags & USB_CDEV_FLAG_GONE) {
	        error = ENXIO;
	    }

	    if (sc->sc_flags & USB_CDEV_FLAG_CLOSING_WRITE) {
	        wakeup(&(sc->sc_wakeup_close_write));
		error = EINTR;
	    }
	}
	return error;
}

static int32_t
usb_cdev_uiomove(struct usb_cdev *sc, u_int32_t context_bit,
		 void *cp, int32_t n, struct uio *uio)
{
	int32_t error;

	sc->sc_flags |= context_bit;

	mtx_unlock(sc->sc_mtx_ptr);

	/* "uiomove()" can sleep so one
	 * needs to make a wrapper, exiting
	 * the mutex and checking things:
	 */
	error = uiomove(cp, n, uio);

	mtx_lock(sc->sc_mtx_ptr);

	return usb_cdev_exit_context(sc, context_bit, error);
}

static int32_t
usb_cdev_msleep(struct usb_cdev *sc, void *ident, u_int32_t context_bit)
{
	int32_t error;

	sc->sc_flags |= context_bit;

	error = msleep(ident, sc->sc_mtx_ptr, PRIBIO|PCATCH, 
		       "usb_cdev_msleep", 0);

	return usb_cdev_exit_context(sc, context_bit, error);
}

int32_t
usb_cdev_sleep(struct usb_cdev *sc, int32_t fflags)
{
	u_int32_t context_bit = usb_cdev_get_context(fflags);

	context_bit &= (USB_CDEV_FLAG_SLEEP_IOCTL_RD|
			USB_CDEV_FLAG_SLEEP_IOCTL_WR|
			USB_CDEV_FLAG_WAKEUP_IOCTL_RD|
			USB_CDEV_FLAG_WAKEUP_IOCTL_WR);
	return usb_cdev_msleep(sc, &(sc->sc_wakeup_ioctl), context_bit);
}

void
usb_cdev_wakeup(struct usb_cdev *sc)
{
	if (sc->sc_flags & (USB_CDEV_FLAG_WAKEUP_IOCTL_RD|
			    USB_CDEV_FLAG_WAKEUP_IOCTL_WR)) {
	    sc->sc_flags &= ~(USB_CDEV_FLAG_WAKEUP_IOCTL_RD|
			      USB_CDEV_FLAG_WAKEUP_IOCTL_WR);
	    wakeup(&(sc->sc_wakeup_ioctl));
	}
	return;
}

/*
 * the synchronization part is a little more
 * complicated, hence there are two modes:
 *
 * 1) read only and write only, two threads
 * 2) read and write, one thread
 *
 * the job of the following function is to ensure
 * that only one thread enters a piece of code at
 * a time:
 */
static int32_t
usb_cdev_wait_context(struct usb_cdev *sc, u_int32_t context_bit)
{
	u_int32_t temp;
	int32_t error = 0;

	if (context_bit == 0) {
	    error = EIO;
	    goto done;
	}

	if (sc->sc_flags & context_bit &
	    (USB_CDEV_FLAG_GONE|
	     USB_CDEV_FLAG_CLOSING_READ|
	     USB_CDEV_FLAG_CLOSING_WRITE|
	     USB_CDEV_FLAG_SLEEP_READ|
	     USB_CDEV_FLAG_SLEEP_WRITE|
	     USB_CDEV_FLAG_SLEEP_IOCTL_RD|
	     USB_CDEV_FLAG_SLEEP_IOCTL_WR|
	     0)) {
	    error = EIO;
	    goto done;
	}

	sc->sc_cur_context = context_bit;

	temp = (context_bit & (USB_CDEV_FLAG_SLEEP_IOCTL_RD|
			       USB_CDEV_FLAG_SLEEP_IOCTL_WR));

	/* 
	 * if two threads are waiting, the 
	 * "sc_cur_context" variable will 
	 * ensure that the first waiting 
	 * thread will run first !
	 */

	while (sc->sc_flags & (~(context_bit|sc->sc_cur_context)) & 
	       (USB_CDEV_FLAG_SLEEP_IOCTL_RD|
		USB_CDEV_FLAG_SLEEP_IOCTL_WR)) {

	    error = usb_cdev_msleep(sc, &(sc->sc_wakeup_ioctl_rdwr), 
				    temp);
	    if (error) {
		goto done;
	    }
	}
 done:
	return error;
}

static void
usb_cdev_unwait_context(struct usb_cdev *sc, u_int32_t context_bit)
{
	if (sc->sc_flags & (~context_bit) &
	    (USB_CDEV_FLAG_SLEEP_IOCTL_RD|
	     USB_CDEV_FLAG_SLEEP_IOCTL_WR)) {
	    /*
	     * if the "other thread" is waiting, 
	     * then wake it up:
	     */
	    wakeup(&(sc->sc_wakeup_ioctl_rdwr));
	}
	return;
}

static int32_t
usb_cdev_open(struct cdev *dev, int32_t oflags, int32_t devtype, 
	      struct thread *td)
{
	struct usb_cdev *sc = DEV2SC(dev);
	struct usbd_mbuf *m;
	u_int32_t context_bit = usb_cdev_get_context(oflags);
	u_int32_t temp;
	  int32_t error = 0;

	DPRINTF(1, "oflags=0x%08x devtype=0x%08x\n", 
		oflags, devtype);

	if (sc == NULL) {
	    return EIO;
	}

	if (context_bit == 0) {
	    return EINVAL;
	}

	mtx_lock(sc->sc_mtx_ptr);

	/* check if the device is already opened
	 * for the given context or if the
	 * device is gone:
	 */
	if (sc->sc_flags & context_bit &
	    (USB_CDEV_FLAG_OPEN_READ|
	     USB_CDEV_FLAG_OPEN_WRITE|
	     USB_CDEV_FLAG_READ_ONLY|
	     USB_CDEV_FLAG_WRITE_ONLY|
	     USB_CDEV_FLAG_GONE)) {

	    DPRINTF(1, "is busy, sc_flags=0x%08x "
		    "context=0x%08x\n", sc->sc_flags, 
		    context_bit);
	    error = EBUSY;
	    goto done;
	}

	temp = (context_bit & (USB_CDEV_FLAG_OPEN_READ|
			       USB_CDEV_FLAG_OPEN_WRITE));

	sc->sc_flags |= temp;

	error = usb_cdev_wait_context(sc, context_bit);
	if (error) {
	    sc->sc_flags &= ~temp;
	    goto done;
	}

	if (context_bit & USB_CDEV_FLAG_OPEN_READ) {
	    /* reset read queue */
	    while(1) {
	        USBD_IF_DEQUEUE(&(sc->sc_rdq_used), m);

		if (m) {
		    USBD_IF_ENQUEUE(&(sc->sc_rdq_free), m);
		} else {
		    break;
		}
	    }
	    sc->sc_async_rd = NULL;
	}

	if (context_bit & USB_CDEV_FLAG_OPEN_WRITE) {
	    /* reset write queue */
	    while(1) {
	        USBD_IF_DEQUEUE(&(sc->sc_wrq_used), m);

		if (m) {
		    USBD_IF_ENQUEUE(&(sc->sc_wrq_free), m);
		} else {
		    break;
		}
	    }
	    sc->sc_async_wr = NULL;
	}

	sc->sc_last_cdev = dev;
	sc->sc_first_open = 
	  !(sc->sc_flags & (~context_bit) & 
	    (USB_CDEV_FLAG_OPEN_READ|
	     USB_CDEV_FLAG_OPEN_WRITE));

	error = (sc->sc_open)(sc, oflags, devtype, td);

	if (error) {
	    sc->sc_flags &= ~temp;
	} else {
	    if (context_bit & USB_CDEV_FLAG_OPEN_READ) {
	        (sc->sc_start_read)(sc);
	    }
	}

	usb_cdev_unwait_context(sc, context_bit);

 done:
	mtx_unlock(sc->sc_mtx_ptr);

	DPRINTF(0, "done, error=%d\n", error);
	return error;
}

static int32_t
usb_cdev_close(struct cdev *dev, int32_t fflags, 
	       int32_t devtype, struct thread *td)
{
	struct usb_cdev *sc = DEV2SC(dev);
	u_int32_t context_bit = usb_cdev_get_context(fflags);
	  int32_t error;

	DPRINTF(1, "fflags=0x%08x\n", fflags);

	if (sc == NULL) {
	    return EIO;
	}

	if (context_bit == 0) {
	    return EINVAL;
	}

	mtx_lock(sc->sc_mtx_ptr);

	if (sc->sc_flags & context_bit & 
	    (USB_CDEV_FLAG_CLOSING_READ|
	     USB_CDEV_FLAG_CLOSING_WRITE|
	     USB_CDEV_FLAG_FLUSHING_WRITE)) {

	    /*
	     * double close on the same device
	     * can happen during detach 
	     */
	    if (sc->sc_flags &   USB_CDEV_FLAG_FLUSHING_WRITE) {
	        sc->sc_flags &= ~USB_CDEV_FLAG_FLUSHING_WRITE;
		/* the device is gone, kill the flush */
		wakeup(&(sc->sc_wakeup_flush));
	    }
	    goto done;
	}

	if (sc->sc_flags & context_bit & 
	    USB_CDEV_FLAG_OPEN_READ) {

	    sc->sc_flags |= USB_CDEV_FLAG_CLOSING_READ;

	    /* stop read transfer, if not already stopped */

	    (sc->sc_stop_read)(sc);

	    /* wakeup sleeping threads */

	    while (sc->sc_flags & (USB_CDEV_FLAG_SLEEP_READ|
				   USB_CDEV_FLAG_SLEEP_IOCTL_RD)) {

	        if (sc->sc_flags &   USB_CDEV_FLAG_WAKEUP_READ) {
		    sc->sc_flags &= ~USB_CDEV_FLAG_WAKEUP_READ;
		    wakeup(&(sc->sc_wakeup_read));
		}

		if (sc->sc_flags &   USB_CDEV_FLAG_WAKEUP_IOCTL_RD) {
		    sc->sc_flags &= ~USB_CDEV_FLAG_WAKEUP_IOCTL_RD;
		    wakeup(&(sc->sc_wakeup_ioctl));
		}

		error = msleep(&(sc->sc_wakeup_close_read), sc->sc_mtx_ptr, 
			       PRIBIO, "usb_cdev_sync_read", 0);
	    }

	    if (sc->sc_flags & USB_CDEV_FLAG_SELECT_READ) {
	        selwakeup(&(sc->sc_read_sel));
	    }

	    if (sc->sc_async_rd != NULL) {
	        PROC_LOCK(sc->sc_async_rd);
		psignal(sc->sc_async_rd, SIGIO);
		PROC_UNLOCK(sc->sc_async_rd);
	    }

	    sc->sc_async_rd = NULL;

	    sc->sc_flags &= ~(USB_CDEV_FLAG_OPEN_READ|
			      USB_CDEV_FLAG_SLEEP_READ|
			      USB_CDEV_FLAG_WAKEUP_READ|
			      USB_CDEV_FLAG_SELECT_READ|
			      USB_CDEV_FLAG_CLOSING_READ|
			      USB_CDEV_FLAG_ERROR_READ);
	}

	if (sc->sc_flags & context_bit & 
	    USB_CDEV_FLAG_OPEN_WRITE) {

	    /*
	     * wait for data to 
	     * be written to pipe:
	     */
	    if (!(sc->sc_flags & context_bit &
		  (USB_CDEV_FLAG_GONE|
		   USB_CDEV_FLAG_ERROR_WRITE|
		   USB_CDEV_FLAG_ERROR_READ))) {

	        sc->sc_flags |= USB_CDEV_FLAG_FLUSHING_WRITE;

	        /* start write transfer, if not already started */

		(sc->sc_start_write)(sc);

		while (sc->sc_flags & USB_CDEV_FLAG_FLUSHING_WRITE) {

		    error = msleep(&(sc->sc_wakeup_flush), sc->sc_mtx_ptr, 
				   PRIBIO|PCATCH, "usb_cdev_flush", 0);

		    if (error || (sc->sc_flags & context_bit &
				  (USB_CDEV_FLAG_GONE|
				   USB_CDEV_FLAG_ERROR_WRITE|
				   USB_CDEV_FLAG_ERROR_READ))) {
		        /* stop flush on signal from user */
		        break;
		    }
		}

		sc->sc_flags &= ~USB_CDEV_FLAG_FLUSHING_WRITE;
	    }

 	    sc->sc_flags |= USB_CDEV_FLAG_CLOSING_WRITE;

	    /* stop write transfer, if not already stopped */

	    (sc->sc_stop_write)(sc);

	    /* wakeup sleeping threads */

	    while (sc->sc_flags & (USB_CDEV_FLAG_SLEEP_WRITE|
				   USB_CDEV_FLAG_SLEEP_IOCTL_WR)) {

	        if (sc->sc_flags &   USB_CDEV_FLAG_WAKEUP_WRITE) {
		    sc->sc_flags &= ~USB_CDEV_FLAG_WAKEUP_WRITE;
		    wakeup(&(sc->sc_wakeup_write));
		}

		if (sc->sc_flags &   USB_CDEV_FLAG_WAKEUP_IOCTL_WR) {
		    sc->sc_flags &= ~USB_CDEV_FLAG_WAKEUP_IOCTL_WR;
		    wakeup(&(sc->sc_wakeup_ioctl));
		}

		if (sc->sc_flags &   USB_CDEV_FLAG_FLUSHING_WRITE) {
		    sc->sc_flags &= ~USB_CDEV_FLAG_FLUSHING_WRITE;
		    wakeup(&(sc->sc_wakeup_flush));
		}

		error = msleep(&(sc->sc_wakeup_close_write), sc->sc_mtx_ptr, 
			       PRIBIO, "usb_cdev_sync_write", 0);
	    }

	    if (sc->sc_flags & USB_CDEV_FLAG_SELECT_WRITE) {
	        selwakeup(&(sc->sc_write_sel));
	    }

	    if (sc->sc_async_wr != NULL) {
	        PROC_LOCK(sc->sc_async_wr);
		psignal(sc->sc_async_wr, SIGIO);
		PROC_UNLOCK(sc->sc_async_wr);
	    }

	    sc->sc_async_wr = NULL;

	    sc->sc_flags &= ~(USB_CDEV_FLAG_FLUSHING_WRITE|
			      USB_CDEV_FLAG_OPEN_WRITE|
			      USB_CDEV_FLAG_SLEEP_WRITE|
			      USB_CDEV_FLAG_WAKEUP_WRITE|
			      USB_CDEV_FLAG_SELECT_WRITE|
			      USB_CDEV_FLAG_CLOSING_WRITE|
			      USB_CDEV_FLAG_ERROR_WRITE);
	}

	if (sc->sc_flags & USB_CDEV_FLAG_GONE) {
	    if (!(sc->sc_flags & (USB_CDEV_FLAG_OPEN_READ|
				  USB_CDEV_FLAG_OPEN_WRITE))) {
	        /* it is safe to detach */
	        wakeup(&(sc->sc_wakeup_detach));
	    }
	}
 done:
	mtx_unlock(sc->sc_mtx_ptr);

	DPRINTF(0, "closed\n");

	return 0;
}

static int32_t
usb_cdev_write(struct cdev *dev, struct uio *uio, int32_t flags)
{
	struct usb_cdev *sc = DEV2SC(dev);
	struct usbd_mbuf *m;
	int32_t error = 0;
	int32_t io_len;
	u_int8_t tr_data = 0;

	DPRINTF(1, "\n");

	if (sc == NULL) {
	    return EIO;
	}

	mtx_lock(sc->sc_mtx_ptr);

	if(sc->sc_flags & (USB_CDEV_FLAG_GONE|
			   USB_CDEV_FLAG_CLOSING_WRITE|
			   USB_CDEV_FLAG_SLEEP_WRITE|
			   USB_CDEV_FLAG_SLEEP_IOCTL_WR|
			   USB_CDEV_FLAG_ERROR_WRITE)) {
	    error = EIO;
	    goto done;
	}

	while (uio->uio_resid > 0) {

	    USBD_IF_DEQUEUE(&(sc->sc_wrq_free), m);

	    if (m == NULL) {

	        if (flags & O_NONBLOCK) {
		    if (tr_data) {
		        /* return length before error */
		        break;
		    }
		    error = EWOULDBLOCK;
		    break;
		}

	        error = usb_cdev_msleep(sc, &(sc->sc_wakeup_write),
					(USB_CDEV_FLAG_SLEEP_WRITE|
					 USB_CDEV_FLAG_WAKEUP_WRITE));
		if (error) {
		    break;
		}
		continue;
	    } else {
	      tr_data = 1;
	    }

	    USBD_MBUF_RESET(m);

	    io_len = min(m->cur_data_len, uio->uio_resid);

	    m->cur_data_len = io_len;

	    DPRINTF(1, "transfer %d bytes to %p\n", 
		    io_len, m->cur_data_ptr);

	    error = usb_cdev_uiomove(sc, USB_CDEV_FLAG_SLEEP_WRITE,
				     m->cur_data_ptr, io_len, uio);

	    if (error) {
	        USBD_IF_ENQUEUE(&(sc->sc_wrq_free), m);
	        break;
	    } else {
	        USBD_IF_ENQUEUE(&(sc->sc_wrq_used), m);
		(sc->sc_start_write)(sc);
	    }
	}
 done:
	mtx_unlock(sc->sc_mtx_ptr);

	return error;
}

static int32_t
usb_cdev_read(struct cdev *dev, struct uio *uio, int flags)
{
	struct usb_cdev *sc = DEV2SC(dev);
	struct usbd_mbuf *m;
	int32_t error = 0;
	int32_t io_len;
	u_int8_t tr_data = 0;

	if (sc == NULL) {
	    return EIO;
	}

	DPRINTF(1, "\n");

	mtx_lock(sc->sc_mtx_ptr);

	if(sc->sc_flags & (USB_CDEV_FLAG_GONE|
			   USB_CDEV_FLAG_CLOSING_READ|
			   USB_CDEV_FLAG_SLEEP_READ|
			   USB_CDEV_FLAG_SLEEP_IOCTL_RD|
			   USB_CDEV_FLAG_ERROR_READ)) {
	    error = EIO;
	    goto done;
	}

	while (uio->uio_resid > 0) {

	    USBD_IF_DEQUEUE(&(sc->sc_rdq_used), m);

	    if (m == NULL) {

	        /* start read transfer, if not already started */

	        (sc->sc_start_read)(sc);

	        if (flags & O_NONBLOCK) {
		    if (tr_data) {
		        /* return length before error */
		        break;
		    }
		    error = EWOULDBLOCK;
		    break;
		}

	        error = usb_cdev_msleep(sc, &(sc->sc_wakeup_read),
					(USB_CDEV_FLAG_SLEEP_READ|
					 USB_CDEV_FLAG_WAKEUP_READ)); 
		if (error) {
		    break;
		}
		continue;
	    } else {
	      tr_data = 1;
	    }

	    io_len = min(m->cur_data_len, uio->uio_resid);

	    DPRINTF(1, "transfer %d bytes from %p\n", 
		    io_len, m->cur_data_ptr);

	    error = usb_cdev_uiomove(sc, USB_CDEV_FLAG_SLEEP_READ,
				     m->cur_data_ptr, io_len, uio);

	    m->cur_data_len -= io_len;
	    m->cur_data_ptr += io_len;

	    if (m->cur_data_len == 0) {
	        USBD_IF_ENQUEUE(&(sc->sc_rdq_free), m);

		if (sc->sc_flags & USB_CDEV_FLAG_FWD_SHORT) {
		    /* forward short transfers to userland */
		    if ((m->cur_data_ptr - m->min_data_ptr) < m->max_data_len) {
		        /* short transfer */
		        break;
		    }
		}

	    } else {
	        USBD_IF_PREPEND(&(sc->sc_rdq_used), m);
	    }

	    if (error) {
	       break;
	    }
	}

 done:
	mtx_unlock(sc->sc_mtx_ptr);

	return error;
}

static int
usb_cdev_ioctl(struct cdev *dev, u_long cmd, caddr_t addr, 
	       int32_t fflags, struct thread *td)
{
	struct usb_cdev *sc = DEV2SC(dev);
	u_int32_t context_bit = usb_cdev_get_context(fflags);
	int32_t error = 0;

	if (sc == NULL) {
	    return EIO;
	}

	DPRINTF(1, "fflags=0x%08x\n", fflags);

	mtx_lock(sc->sc_mtx_ptr);

	error = usb_cdev_wait_context(sc, context_bit);
	if (error) {
	    goto done;
	}

	switch(cmd) {
	case FIONBIO:
	    /* handled by upper FS layer */
	    break;

	case FIOASYNC:
	    if(fflags & FREAD) {
	        if (*(int *)addr) {
		    if (sc->sc_async_rd != NULL) {
		        error = EBUSY;
			break;
		    }
		    sc->sc_async_rd = td->td_proc;
		} else {
		    sc->sc_async_rd = NULL;
		}
	    }
	    if(fflags & FWRITE) {
	        if (*(int *)addr) {
		    if (sc->sc_async_wr != NULL) {
		        error = EBUSY;
			break;
		    }
		    sc->sc_async_wr = td->td_proc;
		} else {
		    sc->sc_async_wr = NULL;
		}
	    }
	    break;

	/* XXX this is not the most general solution */
	case TIOCSPGRP:
	    if(fflags & FREAD) {
	        if (sc->sc_async_rd == NULL) {
		    error = EINVAL;
		    break;
		}
		if (*(int *)addr != sc->sc_async_rd->p_pgid) {
		    error = EPERM;
		    break;
		}
	    }
	    if(fflags & FWRITE) {
	        if (sc->sc_async_wr == NULL) {
		    error = EINVAL;
		    break;
		}
		if (*(int *)addr != sc->sc_async_wr->p_pgid) {
		    error = EPERM;
		    break;
		}
	    }
	    break;

	default:
	    sc->sc_last_cdev = dev;
	    error = (sc->sc_ioctl)(sc, cmd, addr, fflags, td);
	    break;
	}

	usb_cdev_unwait_context(sc, context_bit);

 done:
	mtx_unlock(sc->sc_mtx_ptr);

	return error;
}

static int32_t
usb_cdev_poll(struct cdev *dev, int32_t events, struct thread *td)
{
	struct usb_cdev *sc = DEV2SC(dev);
	struct usbd_mbuf *m;
	int32_t revents = 0;

	if (sc == NULL) {
	    return POLLNVAL;
	}

	DPRINTF(1, "\n");

	mtx_lock(sc->sc_mtx_ptr);

	if (events & (POLLOUT | POLLWRNORM)) {

	    USBD_IF_POLL(&(sc->sc_wrq_free), m);

	    if (m || (sc->sc_flags &
		      (USB_CDEV_FLAG_GONE|
		       USB_CDEV_FLAG_CLOSING_WRITE|
		       USB_CDEV_FLAG_SLEEP_WRITE|
		       USB_CDEV_FLAG_SLEEP_IOCTL_WR|
		       USB_CDEV_FLAG_ERROR_WRITE))) {
	        revents = events & (POLLOUT | POLLWRNORM);
	    } else {
	        sc->sc_flags |= USB_CDEV_FLAG_SELECT_WRITE;
		selrecord(td, &(sc->sc_write_sel));
	    }
	}

	if (events & (POLLIN | POLLRDNORM)) {

	    USBD_IF_POLL(&(sc->sc_rdq_used), m);

	    if (m || (sc->sc_flags & 
		      (USB_CDEV_FLAG_GONE|
		       USB_CDEV_FLAG_CLOSING_READ|
		       USB_CDEV_FLAG_SLEEP_READ|
		       USB_CDEV_FLAG_SLEEP_IOCTL_RD|
		       USB_CDEV_FLAG_ERROR_READ))) {
	        revents = events & (POLLIN | POLLRDNORM);
	    } else {
	        sc->sc_flags |= USB_CDEV_FLAG_SELECT_READ;
		selrecord(td, &(sc->sc_read_sel));
	    }
	}

	mtx_unlock(sc->sc_mtx_ptr);

	return revents;
}

static int32_t
usb_cdev_dummy_open(struct usb_cdev *sc, int32_t fflags, 
		    int32_t mode, struct thread *td)
{
    return 0;
}

static int32_t 
usb_cdev_dummy_ioctl(struct usb_cdev *sc, u_long cmd, caddr_t addr, 
		     int32_t fflags, struct thread *td)
{
    return ENOTTY;
}

static void 
usb_cdev_dummy_cmd(struct usb_cdev *sc)
{
    return;
}

static u_int8_t minor_table[(1<<16) / 8];

static u_int32_t
usb_cdev_alloc_minor(void)
{
    u_int32_t x;

    mtx_lock(&Giant);

    x = (1<<16);
    while(--x) {
      if (minor_table[x / 8] & (1 << (x % 8))) {
	  continue;
      } else {
	  minor_table[x / 8] |= (1 << (x % 8));
	  break;
      }
    }

    x = ((x & 0xFF00) << 8) | (x & 0x00FF);

    mtx_unlock(&Giant);

    return x;
}

static void
usb_cdev_free_minor(u_int32_t x)
{
    if (x & 0x00FF00) {
        /* invalid minor */
        return;
    }

    x = ((x & 0xFF0000) >> 8) | (x & 0x0000FF);

    mtx_lock(&Giant);

    minor_table[x / 8] &= ~(1 << (x % 8));

    mtx_unlock(&Giant);

    return;
}

int32_t
usb_cdev_attach(struct usb_cdev *sc, 
		void *priv_sc, 
		struct mtx *priv_mtx,
		const char **pp_dev,
		uid_t _uid,
		gid_t _gid,
		int _perms,
		u_int32_t rd_size, 
		u_int16_t rd_packets,
		u_int32_t wr_size,
		u_int16_t wr_packets)
{
	struct cdev *cdev;
	u_int32_t minor;
	u_int8_t n;

	/* assumes that "sc->xxx" was zeroed by the caller */

	if (sc->sc_open == NULL) {
	    sc->sc_open = &usb_cdev_dummy_open;
	}

	if (sc->sc_ioctl == NULL) {
	    sc->sc_ioctl = &usb_cdev_dummy_ioctl;
	}

	if (sc->sc_start_read == NULL) {
	    sc->sc_start_read = &usb_cdev_dummy_cmd;
	}
	 
	if (sc->sc_stop_read == NULL) {
	    sc->sc_stop_read = &usb_cdev_dummy_cmd;
	}

	if (sc->sc_start_write == NULL) {
	    sc->sc_start_write = &usb_cdev_dummy_cmd;
	}

	if (sc->sc_stop_write == NULL) {
	    sc->sc_stop_write = &usb_cdev_dummy_cmd;
	}

	if (priv_mtx == NULL) {
	    priv_mtx = &Giant;
	}

	sc->sc_priv_ptr = priv_sc;
	sc->sc_mtx_ptr = priv_mtx;

	sc->sc_rdq_free.ifq_maxlen = rd_packets;
	sc->sc_rdq_used.ifq_maxlen = rd_packets;

 	sc->sc_wrq_free.ifq_maxlen = wr_packets;
	sc->sc_wrq_used.ifq_maxlen = wr_packets;

 	sc->sc_rdq_pointer = 
	  usbd_alloc_mbufs(M_DEVBUF, &(sc->sc_rdq_free), 
			   rd_size, rd_packets);

	if (sc->sc_rdq_pointer == NULL) {
	    goto detach;
	}

 	sc->sc_wrq_pointer = 
	  usbd_alloc_mbufs(M_DEVBUF, &(sc->sc_wrq_free), 
			   wr_size, wr_packets);

	if (sc->sc_wrq_pointer == NULL) {
	    goto detach;
	}

	for (n = 0; n < USB_CDEV_COUNT; n++) {

	    if (pp_dev[n] == NULL) {
	        break;
	    }

	    minor = usb_cdev_alloc_minor();

	    if (minor == 0) {
	        break;
	    }

	    DPRINTF(1, "making device '%s'\n", pp_dev[n]);

	    cdev = make_dev(&usb_cdev_cdevsw, minor, 
			    _uid, _gid, _perms, "%s", pp_dev[n]);

	    sc->sc_cdev[n] = cdev;

	    if (cdev) {
	        DEV2SC(cdev) = sc;
	    }
	}

	DPRINTF(1, "attached %p\n", sc);

	return 0;

 detach:
	usb_cdev_detach(sc);
	return ENOMEM;
}

void
usb_cdev_detach(struct usb_cdev *sc)
{
	struct cdev *cdev;
	int32_t error;
	u_int32_t _minor;
	u_int8_t n;

	if (sc->sc_mtx_ptr == NULL) {
	    sc->sc_mtx_ptr = &Giant;
	}

	mtx_lock(sc->sc_mtx_ptr);
	sc->sc_flags |= USB_CDEV_FLAG_GONE;
	mtx_unlock(sc->sc_mtx_ptr);

	for (n = 0; n < USB_CDEV_COUNT; n++) {

	    cdev = sc->sc_cdev[n];

	    if (cdev) {

	        if (DEV2SC(cdev) == sc) {

		    (void) usb_cdev_close(cdev, FREAD, 0, NULL);
		    (void) usb_cdev_close(cdev, FWRITE, 0, NULL);
		}

		DEV2SC(cdev) = NULL;

		_minor = minor(cdev);

		destroy_dev(cdev);

		usb_cdev_free_minor(_minor);
	    }

	    mtx_lock(sc->sc_mtx_ptr);
	    sc->sc_cdev[n] = NULL;
	    mtx_unlock(sc->sc_mtx_ptr);
	}

	/* wait for devices to close */

	mtx_lock(sc->sc_mtx_ptr);
	while (sc->sc_flags & (USB_CDEV_FLAG_OPEN_READ|
			       USB_CDEV_FLAG_OPEN_WRITE)) {

	    error = msleep(&(sc->sc_wakeup_detach), sc->sc_mtx_ptr, 
			   PRIBIO, "usb_cdev_sync_detach", 0);
	}
	mtx_unlock(sc->sc_mtx_ptr);

	if (sc->sc_rdq_pointer) {
	    free(sc->sc_rdq_pointer, M_DEVBUF);
	    sc->sc_rdq_pointer = NULL;
	}

	if (sc->sc_wrq_pointer) {
	    free(sc->sc_wrq_pointer, M_DEVBUF);
	    sc->sc_wrq_pointer = NULL;
	}

	DPRINTF(1, "detached %p\n", sc);

	return;
}

/*
 * what: 
 *  0 - normal operation
 *  1 - force short packet
 */
void
usb_cdev_put_data(struct usb_cdev *sc, u_int8_t *buf, u_int32_t len, 
		  u_int8_t what)
{
	struct usbd_mbuf *m;
	u_int32_t io_len;

	while (len || (what == 1)) {

	    USBD_IF_DEQUEUE(&(sc->sc_rdq_free), m);

	    if (m) {
	        USBD_MBUF_RESET(m);

		io_len = min(len, m->cur_data_len);

		bcopy(buf, m->cur_data_ptr, io_len);

		m->cur_data_len = io_len;
		buf += io_len;
		len -= io_len;

		USBD_IF_ENQUEUE(&(sc->sc_rdq_used), m);

		if ((sc->sc_rdq_used.ifq_len >= 
		     ((sc->sc_rdq_used.ifq_maxlen+1)/2)) ||
		    (sc->sc_flags & USB_CDEV_FLAG_WAKEUP_RD_IMMED)) {

		    /* buffer is half full */

		    if (sc->sc_flags &   USB_CDEV_FLAG_WAKEUP_READ) {
		        sc->sc_flags &= ~USB_CDEV_FLAG_WAKEUP_READ;
			wakeup(&(sc->sc_wakeup_read));
		    }

		    if (sc->sc_flags &   USB_CDEV_FLAG_SELECT_READ) {
		        sc->sc_flags &= ~USB_CDEV_FLAG_SELECT_READ;
			selwakeup(&(sc->sc_read_sel));
		    }

		    if (sc->sc_async_rd != NULL) {
		        PROC_LOCK(sc->sc_async_rd);
			psignal(sc->sc_async_rd, SIGIO);
			PROC_UNLOCK(sc->sc_async_rd);
		    }
		}

		if ((len == 0) || (what == 1)) {
		    break;
		}

	    } else {
		break;
	    }
	}
	return;
}

void
usb_cdev_put_data_error(struct usb_cdev *sc)
{
	sc->sc_flags |= USB_CDEV_FLAG_ERROR_READ;

	if (sc->sc_flags &   USB_CDEV_FLAG_WAKEUP_READ) {
	    sc->sc_flags &= ~USB_CDEV_FLAG_WAKEUP_READ;
	    wakeup(&(sc->sc_wakeup_read));
	}

	if (sc->sc_flags &   USB_CDEV_FLAG_SELECT_READ) {
	    sc->sc_flags &= ~USB_CDEV_FLAG_SELECT_READ;
	    selwakeup(&(sc->sc_read_sel));
	}

	if (sc->sc_async_rd != NULL) {
	    PROC_LOCK(sc->sc_async_rd);
	    psignal(sc->sc_async_rd, SIGIO);
	    PROC_UNLOCK(sc->sc_async_rd);
	}
	return;
}

/*
 * what: 
 *  0 - normal operation
 *  1 - force only one packet
 *
 * returns:
 *  0 - no more data
 *  1 - data in buffer
 */
u_int8_t
usb_cdev_get_data(struct usb_cdev *sc, u_int8_t *buf, u_int32_t len, 
		  u_int32_t *actlen, u_int8_t what)
{
	struct usbd_mbuf *m;
	u_int32_t io_len;
	u_int8_t tr_data = 0;

	actlen[0] = 0;

	while(1) {

	    USBD_IF_DEQUEUE(&(sc->sc_wrq_used), m);

	    if (m) {

	        tr_data = 1;

	        io_len = min(len, m->cur_data_len);

		bcopy(m->cur_data_ptr, buf, io_len);

		len -= io_len;
		buf += io_len;
		actlen[0] += io_len;
		m->cur_data_ptr += io_len;
		m->cur_data_len -= io_len;

		if ((m->cur_data_len == 0) || (what == 1)) {
		    USBD_IF_ENQUEUE(&(sc->sc_wrq_free), m);

		    if ((sc->sc_wrq_free.ifq_len >= 
			 ((sc->sc_wrq_free.ifq_maxlen+1)/2)) ||
			(sc->sc_flags & USB_CDEV_FLAG_WAKEUP_WR_IMMED)) {

		        /* buffer is half full */

		        if (sc->sc_flags &   USB_CDEV_FLAG_WAKEUP_WRITE) {
			    sc->sc_flags &= ~USB_CDEV_FLAG_WAKEUP_WRITE;
			    wakeup(&(sc->sc_wakeup_write));
			}

			if (sc->sc_flags &   USB_CDEV_FLAG_SELECT_WRITE) {
			    sc->sc_flags &= ~USB_CDEV_FLAG_SELECT_WRITE;
			    selwakeup(&(sc->sc_write_sel));
			}

			if (sc->sc_async_wr != NULL) {
			    PROC_LOCK(sc->sc_async_wr);
			    psignal(sc->sc_async_wr, SIGIO);
			    PROC_UNLOCK(sc->sc_async_wr);
			}
		    }

		    if (what == 1) {
		        break;
		    }

		} else {
	            USBD_IF_PREPEND(&(sc->sc_wrq_used), m);
		}
	    } else {

		if (tr_data) {
		   /* wait for data to be written out */
		   break;
		}

	        if (sc->sc_flags &   USB_CDEV_FLAG_FLUSHING_WRITE) {
		    sc->sc_flags &= ~USB_CDEV_FLAG_FLUSHING_WRITE;
		    wakeup(&(sc->sc_wakeup_flush));
		}
		break;
	    }
	    if (len == 0) {
	        break;
	    }
	}
	return tr_data;
}

void
usb_cdev_get_data_error(struct usb_cdev *sc)
{
	sc->sc_flags |= USB_CDEV_FLAG_ERROR_WRITE;

	if (sc->sc_flags &   USB_CDEV_FLAG_WAKEUP_WRITE) {
	    sc->sc_flags &= ~USB_CDEV_FLAG_WAKEUP_WRITE;
	    wakeup(&(sc->sc_wakeup_write));
	}

	if (sc->sc_flags &   USB_CDEV_FLAG_SELECT_WRITE) {
	    sc->sc_flags &= ~USB_CDEV_FLAG_SELECT_WRITE;
	    selwakeup(&(sc->sc_write_sel));
	}

	if (sc->sc_flags &   USB_CDEV_FLAG_FLUSHING_WRITE) {
	    sc->sc_flags &= ~USB_CDEV_FLAG_FLUSHING_WRITE;
	    wakeup(&(sc->sc_wakeup_flush));
	}

	if (sc->sc_async_wr != NULL) {
	    PROC_LOCK(sc->sc_async_wr);
	    psignal(sc->sc_async_wr, SIGIO);
	    PROC_UNLOCK(sc->sc_async_wr);
	}
	return;
}

cdevsw_t usb_cdev_cdevsw = {
  .d_version = D_VERSION,
  .d_open    = usb_cdev_open,
  .d_close   = usb_cdev_close,
  .d_read    = usb_cdev_read,
  .d_write   = usb_cdev_write,
  .d_ioctl   = usb_cdev_ioctl,
  .d_poll    = usb_cdev_poll,
  .d_name    = "usb_cdev",
};
#endif


