
/*
 * USB specifications and other documentation can be found at
 * http://www.usb.org/developers/docs/ and
 * http://www.usb.org/developers/devclass_docs/
 */

#include "netbas.h"



//#include <machine/bus.h>

#include <usb_port.h>
#include <usb.h>
#include <usb_subr.h>
#include <usb.h>

__FBSDID("$FreeBSD: src/sys/dev/usb2/usb.c $");

#define DEV2UNIT(d)	(((d)->devno)&0xff)
#define DEV2BUS(d)	(*((struct usbd_bus **)&((d)->data)))

#define USB_DEV_MINOR	255		/* event queue device */

MALLOC_DEFINE(M_USB, "USB", "USB");
MALLOC_DEFINE(M_USBDEV, "USBdev", "USB device");
MALLOC_DEFINE(M_USBHC, "USBHC", "USB host controller");
extern driver_ops_t usb_cdevsw;
void usbdev_init(void *arg);

/* define this unconditionally in case a kernel module is loaded that
 * has been compiled with debugging options.
 */
SYSCTL_NODE(_hw, OID_AUTO, usb, CTLFLAG_RW, 0, "USB debugging");

#ifdef USB_DEBUG
int	usbdebug = 16;
SYSCTL_INT(_hw_usb, OID_AUTO, debug, CTLFLAG_RW,
	   &usbdebug, 0, "usb debug level");

/*
 * 0  - do usual exploration
 * 1  - do not use timeout exploration
 * >1 - do no exploration
 */
int	usb_noexplore = 0;
#endif

#define USB_MAX_EVENTS 100
struct usb_event_wrapper
{
	struct usb_event ue;
	TAILQ_ENTRY(usb_event_wrapper) next;
};

static TAILQ_HEAD(, usb_event_wrapper) usb_events =
	TAILQ_HEAD_INITIALIZER(usb_events);

thread_wait_t usb_events_q;

#ifndef usb_global_lock
struct mtx usb_global_lock;
#endif

/* these variables are protected by "usb_global_lock" */
static int usb_nevents = 0;
static struct selinfo usb_selevent;
static struct proc *usb_async_proc;  /* process that wants USB SIGIO */
static int usb_dev_open = 0;

/**/
static const char * const usbrev_str[] = USBREV_STR;

/* 
 * usb_discover - explore the device tree from the root
 * 
 * usb_discover device nodes, kthread
 */
static void
usb_discover(struct usbd_bus *bus)
{
	PRINTFN(2,("\n"));

#ifdef USB_DEBUG
	if(usb_noexplore > 1)
	{
		return;
	}
#endif
	mtx_assert(&usb_global_lock, MA_OWNED);

	/* check that only one thread is exploring
	 * at a time
	 */
	while(bus->is_exploring)
	{
		bus->wait_explore = 1;

		msleep(&bus->wait_explore_q, &usb_global_lock, PWAIT,
		       "usb wait explore", 0);
	}

	bus->is_exploring = 1;

	while(bus->root_port.device &&
	      bus->root_port.device->hub &&
	      bus->needs_explore &&
	      (bus->wait_explore == 0))
	{
		bus->needs_explore = 0;

		/* explore the hub 
		 * (this call can sleep,
		 *  exiting usb_global_lock, 
		 *  which is actually Giant)
		 */
		(bus->root_port.device->hub->explore)
		  (bus->root_port.device);
	}

	bus->is_exploring = 0;

	if(bus->wait_explore)
	{
		bus->wait_explore = 0;
		wakeup(&bus->wait_explore_q);
	}
	return;
}

static void
usb_event_thread(struct usbd_bus *bus)
{
	mtx_lock(&usb_global_lock);

	PRINTFN(2,("entry\n"));

	printf("usb_event_thread try\n");


	while(1)
	{
		if(bus->root_port.device == 0)
		{
			break;
		}

#ifdef USB_DEBUG
		if(usb_noexplore < 2)
#endif
		{
			usb_discover(bus);
		}

#ifdef USB_DEBUG
		msleep(&bus->needs_explore, &usb_global_lock, PWAIT,
		       "usbevt", usb_noexplore ? 0 : hz * 60);
#else
		msleep(&bus->needs_explore, &usb_global_lock, PWAIT,
		       "usbevt", hz * 60);
#endif
		PRINTFN(2,("woke up\n"));
	}

	bus->event_thread = NULL;

	/* in case parent is waiting for us to exit */
	wakeup(&bus->bus_wait_q);

	mtx_unlock(&usb_global_lock);

	PRINTF(("exit\n"));

	kthread_exit(0);
	printf("usb_event_thread exit\n");

	return;
}

void
usb_needs_explore(struct usbd_device *udev)
{
	PRINTFN(2,("\n"));

	mtx_lock(&usb_global_lock);
	udev->bus->needs_explore = 1;
	wakeup(&udev->bus->needs_explore_q);
	mtx_unlock(&usb_global_lock);
	return;
}

u_int8_t usb_driver_added_refcount;

void
usb_needs_probe_and_attach(void)
{
	struct usbd_bus *bus;
	devclass_t dc;
	device_t dev;
	int max;

	PRINTFN(2,("\n"));

	mtx_lock(&usb_global_lock);

	usb_driver_added_refcount++;

	dc = devclass_find("usb");

	if(dc)
	{
	    max = devclass_get_maxunit(dc);
 	    while(max >= 0)
	    {
	        dev = devclass_get_device(dc, max);
		if(dev)
		{
		    bus = device_get_softc(dev);

		    bus->needs_explore = 1;
		    wakeup(&bus->needs_explore_q);
		}
		max--;
	    }
	}
	else
	{
	    printf("%s: \"usb\" devclass not present!\n",
		   __FUNCTION__);
	}
	mtx_unlock(&usb_global_lock);
	return;
}

static void
usb_create_event_thread(struct usbd_bus *bus)
{
	if(usb_kthread_create1((void*)(void*)&usb_event_thread, bus, &bus->event_thread,
			       "%s", device_get_nameunit(bus->bdev)))
	{
		device_printf(bus->bdev, "unable to create event thread for\n");
		panic("usb_create_event_thread");
	}
	return;
}

static int
usb_event_get_next(struct usb_event *ue)
{
	struct usb_event_wrapper *uew;
	int err;

	mtx_lock(&usb_global_lock);

	uew = TAILQ_FIRST(&usb_events);

	if(uew == NULL)
	{
		usb_nevents = 0;
		err = 0;
	}
	else
	{
		*ue = uew->ue;

		TAILQ_REMOVE(&usb_events, uew, next);

		free(uew, M_USBDEV);

		if(usb_nevents)
		{
		   usb_nevents--;
		}
		err = 1;
	}
	mtx_unlock(&usb_global_lock);
	return (err);
}

static void
usb_event_add(int type, struct usb_event *uep)
{
	struct usb_event_wrapper *uew;
	struct timeval thetime;

	uew = malloc(sizeof *uew, M_USBDEV, M_WAITOK|M_ZERO);
	if(uew == NULL)
	{
		return;
	}
	uew->ue = *uep;
	uew->ue.ue_type = type;
	microtime(&thetime);
	TIMEVAL_TO_TIMESPEC(&thetime, &uew->ue.ue_time);

	mtx_lock(&usb_global_lock);

	if(USB_EVENT_IS_DETACH(type))
	{
		struct usb_event_wrapper *uewi, *uewi_next;

		for (uewi = TAILQ_FIRST(&usb_events);
		     uewi;
		     uewi = uewi_next)
		{
			uewi_next = TAILQ_NEXT(uewi, next);
			if(uewi->ue.u.ue_driver.ue_cookie.cookie ==
			    uep->u.ue_device.udi_cookie.cookie)
			{
				TAILQ_REMOVE(&usb_events, uewi, next);
				free(uewi, M_USBDEV);
				usb_nevents--;
				uewi_next = TAILQ_FIRST(&usb_events);
			}
		}
	}
	if(usb_nevents >= USB_MAX_EVENTS)
	{
		/* too many queued events, drop an old one */
		PRINTF(("event dropped\n"));

		struct usb_event ue;
		(void)usb_event_get_next(&ue);
	}
	TAILQ_INSERT_TAIL(&usb_events, uew, next);
	usb_nevents++;
	wakeup(&usb_events_q);
	selwakeuppri(&usb_selevent, PZERO);
	if(usb_async_proc != NULL)
	{
		TRACE_HERE; //dpp
		//PROC_LOCK(usb_async_proc);
		//psignal(usb_async_proc, SIGIO);
		//PROC_UNLOCK(usb_async_proc);
	}

	mtx_unlock(&usb_global_lock);
	return;
}

void
usbd_add_dev_event(int type, struct usbd_device *udev)
{
	struct usb_event ue;

	bzero(&ue, sizeof(ue));

	usbd_fill_deviceinfo(udev, &ue.u.ue_device,
			     USB_EVENT_IS_ATTACH(type));
	usb_event_add(type, &ue);
	return;
}

void
usbd_add_drv_event(int type, struct usbd_device *udev, device_t dev)
{
	struct usb_event ue;

	bzero(&ue, sizeof(ue));

	ue.u.ue_driver.ue_cookie = udev->cookie;
	strncpy(ue.u.ue_driver.ue_devname, device_get_nameunit(dev),
		sizeof ue.u.ue_driver.ue_devname);
	usb_event_add(type, &ue);
	return;
}

/* called from uhci_pci_attach */

static int
usb_probe(device_t dev)
{
	PRINTF(("\n"));
	return (UMATCH_GENERIC);
}
#define UID_ROOT 129
#define GID_OPERATOR 0


//extern cdevsw_t usb_cdevsw;

static void
__usb_attach(device_t dev, struct usbd_bus *bus)
{
	usbd_status err;
	u_int8_t speed;
	struct usb_event ue;
	dev_prvi_t *cdev;

	PRINTF(("\n"));

	mtx_assert(&usb_global_lock, MA_OWNED);

	bus->root_port.power = USB_MAX_POWER;

	device_printf(bus->bdev, "USB revision %s",
		      usbrev_str[bus->usbrev]);

	switch (bus->usbrev)
	{
	case USBREV_1_0:
	case USBREV_1_1:
		speed = USB_SPEED_FULL;
		break;

	case USBREV_2_0:
		speed = USB_SPEED_HIGH;
		break;

	default:
		printf(", not supported\n");
		return;
	}

	printf("\n");

	/* make sure not to use tsleep() if we are cold booting */
	if(cold)
	{
		bus->use_polling++;
	}

	ue.u.ue_ctrlr.ue_bus = device_get_unit(bus->bdev);
	usb_event_add(USB_EVENT_CTRLR_ATTACH, &ue);

	printf("usb_event_add succ\n");

	err = usbd_new_device(bus->bdev, bus, 0, speed, 0,
			      &bus->root_port);
	if(!err)
	{
		if(bus->root_port.device->hub == NULL)
		{
			device_printf(bus->bdev, 
				      "root device is not a hub\n");
			return;
		}

		/*
		 * the USB bus is explored here so that devices, 
		 * for example the keyboard, can work during boot
		 */

		/* make sure that the bus is explored */
		bus->needs_explore = 1;

		usb_discover(bus);
	}
	else
	{
		device_printf(bus->bdev, "root hub problem, error=%s\n",
			      usbd_errstr(err));
	}

	if(cold)
	{
		bus->use_polling--;
	}

	usb_create_event_thread(bus);

	printf("usb_create_event_thread succ\n");

	/* the per controller devices (used for usb_discover) */
	/* XXX This is redundant now, but old usbd's will want it */
	cdev = make_dev(&usb_cdevsw, device_get_unit(dev), UID_ROOT, GID_OPERATOR,
			0660, "usb%d", device_get_unit(dev));

	if(cdev)
	{
		DEV2BUS(cdev) = bus;
	}
	return;
}

static u_int8_t usb_post_init_called = 0;

static int
usb_attach(device_t dev)
{
	struct usbd_bus *bus = device_get_softc(dev);

	mtx_lock(&usb_global_lock);

	if(usb_post_init_called != 0)
	{
		__usb_attach(dev, bus);
	}

	mtx_unlock(&usb_global_lock);

	USB_ATTACH_SUCCESS_RETURN;
}

usb_init()
{
	mtx_init(&Giant,"giant", NULL, MTX_DEF|MTX_RECURSE);
	minit(&usb_events_q);
	usbdev_init(NULL);
}

 void
usb_post_init(void *arg)
{
	struct usbd_bus *bus;
	devclass_t dc;
	device_t dev;
	int max;
	int n;


	mtx_lock(&usb_global_lock);

	dc = devclass_find("usb");

	if(dc)
	{
	    max = devclass_get_maxunit(dc);

		printf("devclass_get_maxunit max=%d ...\n",max);
	    for(n = 0; n <= max; n++)
	    {
	        dev = devclass_get_device(dc, n);
		if(dev)
		{
			printf("__usb_attach ...\n");
		    bus = device_get_softc(dev);

		    __usb_attach(dev, bus);
		}
	    }
	}
	else
	{
	    printf("%s: \"usb\" devclass not present!\n",
		   __FUNCTION__);
	}

	usb_post_init_called = 1;

	mtx_unlock(&usb_global_lock);

	return;
}

//SYSINIT(usb_post_init, SI_SUB_PSEUDO, SI_ORDER_ANY, usb_post_init, NULL);

static int
usb_detach(device_t dev, int flags)
{
	struct usbd_bus *bus = device_get_softc(dev);
	struct usb_event ue;

	PRINTF(("start\n"));

	mtx_lock(&usb_global_lock);

	/* wait for any possible explore calls to finish */
	while(bus->is_exploring)
	{
		bus->wait_explore = 1;

		msleep(&bus->wait_explore_q, &usb_global_lock, PWAIT,
		       "usb wait explore", 0);
	}

	if(bus->root_port.device != NULL)
	{
		/* free device, but not sub-devices,
		 * hence they are freed by the 
		 * caller of this function
		 */
		usbd_free_device(&bus->root_port, 0);
	}

	/* kill off event thread */
	if(bus->event_thread != NULL)
	{
		wakeup(&bus->needs_explore_q);

		if(msleep(&bus->bus_wait_q, &usb_global_lock, PWAIT, "usbdet", hz * 60))
		{
			device_printf(bus->bdev,
				      "event thread didn't die\n");
		}
		PRINTF(("event thread dead\n"));
	}

	mtx_unlock(&usb_global_lock);

	ue.u.ue_ctrlr.ue_bus = device_get_unit(bus->bdev);
	usb_event_add(USB_EVENT_CTRLR_DETACH, &ue);

	mtx_lock(&bus->mtx);
	if(bus->bdev == dev)
	{
		/* need to clear bus->bdev
		 * here so that the parent
		 * detach routine does not
		 * free this device again
		 */
		bus->bdev = NULL;
	}
	else
	{
		device_printf(dev, "unexpected bus->bdev value!\n");
	}
	mtx_unlock(&bus->mtx);
	return (0);
}

static int
usbopen(char *path, int flag, dev_prvi_t *dev)
{
	int error = 0;

	mtx_lock(&usb_global_lock);

	if(DEV2UNIT(dev) == USB_DEV_MINOR)
	{
		if(usb_dev_open)
		{
			error = EBUSY;
			goto done;
		}
		usb_dev_open = 1;
		usb_async_proc = 0;
	}
	else
	{
		struct usbd_bus *bus = DEV2BUS(dev);

		if(bus->root_port.device == NULL)
		{
			/* device is beeing detached */
			error = EIO;
			goto done;
		}
	}

 done:
	mtx_unlock(&usb_global_lock);
	return (error);
}



static int
usbread(dev_prvi_t *dev,  off_t  pos, char * buf,int count)
{
	struct usb_event ue;
	int error = 0;

	if(DEV2UNIT(dev) != USB_DEV_MINOR)
	{
		return (ENODEV);
	}

	if(count != sizeof(struct usb_event))
	{
		return (EINVAL);
	}

	mtx_lock(&usb_global_lock);

	for(;;)
	{
		if(usb_event_get_next(&ue) != 0)
		{
			break;
		}
		/*if(flag & IO_NDELAY)
		{
			error = EWOULDBLOCK;
			break;
		}*/
		error = msleep(&usb_events_q, &usb_global_lock,
			       (PZERO|PCATCH), "usbrea", 0);
		if(error)
		{
			break;
		}
	}

	mtx_unlock(&usb_global_lock);

	if(!error)
	{
		error = uiomove((void *)&ue, count, buf);
	}
	return (error);
}

static int
usbclose(dev_prvi_t *dev)
{
	if(DEV2UNIT(dev) == USB_DEV_MINOR)
	{
		mtx_lock(&usb_global_lock);

		usb_async_proc = 0;
		usb_dev_open = 0;

		mtx_unlock(&usb_global_lock);
	}
	return (0);
}

#define UIO_WRITE 0
#define UIO_READ 1


static int
usbioctl(dev_prvi_t *dev, u_long cmd, caddr_t data, int len, int fromkernel)
{
	int error = 0;

	mtx_lock(&usb_global_lock);

	if(DEV2UNIT(dev) == USB_DEV_MINOR)
	{
		switch (cmd)
		{
	/*

		case FIOASYNC:
			if(*(int *)data)
#if __FreeBSD_version >= 500000
				usb_async_proc = p->td_proc;
#else
				usb_async_proc = p;
#endif
			else
				usb_async_proc = 0;

			break;
			*/

		default:
			error = EINVAL;
			break;
		}
	}
	else
	{
		struct usbd_bus *bus = DEV2BUS(dev);

		if(bus->root_port.device == NULL)
		{
			/* detached */
			error = EIO;
			goto done;
		}

		switch (cmd)
		{
#if defined(__FreeBSD__)
		/* this part should be deleted */
		case USB_DISCOVER:
			break;
#endif
		case USB_REQUEST:
		{
			struct usb_ctl_request *ur = (void *)data;
			int len = UGETW(ur->ucr_request.wLength);
			//struct iovec iov;
			//struct uio uio;
			void *ptr = 0;
			int addr = ur->ucr_addr;
			usbd_status err;
			int error = 0;

			PRINTF(("USB_REQUEST addr=%d len=%d\n", addr, len));
			if((len < 0) ||
			   (len > 32768))
			{
				error = EINVAL;
				goto done;
			}

			if((addr < 0) || 
			   (addr >= USB_MAX_DEVICES) ||
			   (bus->devices[addr] == 0 /* might be checked by usbd_do_request_flags */))
			{
				error = EINVAL;
				goto done;
			}

			int rw;
			if(len != 0)
			{
				//iov.iov_base = (caddr_t)ur->ucr_data;
				//iov.iov_len = len;
				//uio.uio_iov = &iov;
				//uio.uio_iovcnt = 1;
				//uio.uio_resid = len;
				//uio.uio_offset = 0;
				//uio.uio_segflg = UIO_USERSPACE;
				//uio.uio_rw =
				rw =  ur->ucr_request.bmRequestType & UT_READ ?
				  UIO_READ : UIO_WRITE;
				//uio.uio_procp = p;
				ptr = malloc(len, M_TEMP, M_WAITOK);
				if(rw == UIO_WRITE)
				{
					error = uiomove(ptr, len, ur->ucr_data);
					if(error)
					{
						goto ret;
					}
				}
			}
			err = usbd_do_request_flags
			  (bus->devices[addr], &ur->ucr_request, ptr,
			   ur->ucr_flags, &ur->ucr_actlen,
			   USBD_DEFAULT_TIMEOUT);
			if(err)
			{
				error = EIO;
				goto ret;
			}

			if(len != 0)
			{
				if(rw == UIO_READ)
				{
					error = uiomove(ptr, len, ur->ucr_data);
					if(error)
					{
						goto ret;
					}
				}
			}

		ret:
			if(ptr)
			{
				free(ptr, M_TEMP);
			}
			goto done;
		}

		case USB_DEVICEINFO:
		{
			struct usb_device_info *di = (void *)data;
			int addr = di->udi_addr;

			if((addr < 1) ||
			   (addr >= USB_MAX_DEVICES) ||
			   (bus->devices[addr] == 0))
			{
				error = EINVAL;
				goto done;
			}

			error = usbd_fill_deviceinfo(bus->devices[addr], di, 1);
			goto done;
		}

		case USB_DEVICESTATS:
			*(struct usb_device_stats *)data = bus->stats;
			break;

		default:
			error = EINVAL;
			break;
		}
	}

 done:
	mtx_unlock(&usb_global_lock);
	return (error);
}
#if 0

static int
usbpoll(dev_prvi_t *dev, int events, struct thread *td)
{
	int revents, mask;
	int unit = DEV2UNIT(dev);

	if(unit == USB_DEV_MINOR)
	{
		revents = 0;
		mask = POLLIN | POLLRDNORM;

		mtx_lock(&usb_global_lock);

		if((events & mask) && (usb_nevents > 0))
		{
			revents |= events & mask;
		}
		if((revents == 0) && (events & mask))
		{
			selrecord(td, &usb_selevent);
		}

		mtx_unlock(&usb_global_lock);

		return (revents);
	}
	else
	{
		/* select/poll never wakes up - back compat */
		return 0;
	}
}

cdevsw_t usb_cdevsw = {
#ifdef D_VERSION
	.d_version =	D_VERSION,
#endif
	.d_open =	usbopen,
	.d_close =	usbclose,
	.d_read =	usbread,
	.d_ioctl =	usbioctl,
	.d_poll =	usbpoll,
	.d_name =	"usb",
};
#endif

driver_ops_t usb_cdevsw =
{
	d_name:		"usb",
	d_author:	"NetBSD",
	d_version:	" 1.2 $",
	d_index:	-1,
	d_kver:	CURRENT_KERNEL_VERSION,
	open:		usbopen,
	close:		usbclose,
	read:		usbread,
	write:		NULL,
	ioctl:		usbioctl,					
};



void usbdev_init(void *arg)
{
	dev_prvi_t *cdev=NULL;

#ifndef usb_global_lock
	mtx_init(&usb_global_lock, "usb_global_lock",
		 NULL, MTX_DEF|MTX_RECURSE);
#endif
	/* the device spitting out events */
	cdev = make_dev(&usb_cdevsw, USB_DEV_MINOR, UID_ROOT, 
			GID_OPERATOR, 0660, "usb");

	if(cdev)
	{
		DEV2BUS(cdev) = NULL;
	}
	return;
}




static devclass_t usb_devclass;
static driver_t usb_driver =
{
	.name    = "usb",
	.methods = (device_method_t [])
	{
	  DEVMETHOD(device_probe, usb_probe),
	  DEVMETHOD(device_attach, usb_attach),
	  DEVMETHOD(device_detach, usb_detach),
	  DEVMETHOD(device_suspend, bus_generic_suspend),
	  DEVMETHOD(device_resume, bus_generic_resume),
	  DEVMETHOD(device_shutdown, bus_generic_shutdown),
	  {0,0}
	},
	.size    = 0, /* the softc must be set by the attacher! */
};



DRIVER_MODULE(usb, ohci, usb_driver, usb_devclass, 0, 0);
DRIVER_MODULE(usb, uhci, usb_driver, usb_devclass, 0, 0);
DRIVER_MODULE(usb, ehci, usb_driver, usb_devclass, 0, 0);

MODULE_DEPEND(usb, usb, 1, 1, 1);
MODULE_VERSION(usb, 1);

static struct _export_table_entry pnp_sym_table []=
{
	/*file system*/
	EXPORT_PC_SYMBOL(usbd_transfer_dequeue),
	EXPORT_PC_SYMBOL(usbd_transfer_enqueue),
	EXPORT_PC_SYMBOL(usbd_do_callback),
	EXPORT_PC_SYMBOL(usbd_get_pipe),
	EXPORT_PC_SYMBOL(__usbd_callback),
	EXPORT_PC_SYMBOL(usbd_transfer_done),
};

void usb_loadsym()
{
   int pnpn;

	pnpn =sizeof(pnp_sym_table)/sizeof(struct _export_table_entry);

	install_dll_table("usb_bus.sym", 1,pnpn, pnp_sym_table);
}
