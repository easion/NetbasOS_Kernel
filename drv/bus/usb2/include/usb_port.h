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

#ifndef _USB_PORT_H
#define _USB_PORT_H

# ifdef _KERNEL

#  ifdef __FreeBSD__

//#   include <sys/conf.h>
//#   include <machine/resource.h> /* SYS_XXX */
//#   include <sys/bus.h> /* device_xxx() */
#   ifdef INCLUDE_PCIXXX_H
/* NOTE: one does not want to include these
 * files when building the USB device driver
 * modules!
 */
//#    include <dev/pci/pcireg.h>
//#    include <dev/pci/pcivar.h>
#   endif
//#   include <sys/lockmgr.h>
//#   include <sys/module.h>
//#   include <sys/mutex.h>
//#   include <sys/rman.h> 
//#   include <sys/selinfo.h>
//#   include <sys/sysctl.h> /* SYSCTL_XXX() */
//#   include <sys/fcntl.h>
//#   include <sys/taskqueue.h>
//#   include <sys/callout.h> /* callout_xxx() */

#   ifndef __KASSERT
     typedef struct cdevsw cdevsw_t;
#    define __lockmgr lockmgr
#    define __KASSERT KASSERT
#    define uio_procp uio_td
#   endif
#   ifndef __callout_init_mtx
#    define __callout_init_mtx(c,m,f) callout_init_mtx(&(c)->co,m,f)
#    define __callout_reset(c,t,f,d) callout_reset(&(c)->co,t,f,d)
#    define __callout_stop(c) callout_stop(&(c)->co)
#    define __callout_pending(c) callout_pending(&(c)->co)
     struct __callout { struct callout co; };
#   endif

#  else
//#   include <sys/freebsd_compat.h>
#  endif
# endif

/*
 * Macros to cope with the differences between operating systems.
 */

#ifdef __NetBSD__
/*
 * NetBSD
 */

#include "opt_usbverbose.h"

#define SCSI_MODE_SENSE		MODE_SENSE

#define usb_kthread_create1	kthread_create1
#define usb_kthread_create	kthread_create

#if (__NetBSD_Version__ >= 300000000)
typedef void * usb_malloc_type;
#else
typedef int usb_malloc_type;
#endif

#define Ether_ifattach ether_ifattach
#define IF_INPUT(ifp, m) (*(ifp)->if_input)((ifp), (m))

#elif defined(__OpenBSD__)
/*
 * OpenBSD
 */

#define UCOMBUSCF_PORTNO		-1
#define UCOMBUSCF_PORTNO_DEFAULT	-1

#define SCSI_MODE_SENSE		MODE_SENSE
#define XS_STS_DONE		ITSDONE
#define XS_CTL_POLL		SCSI_POLL
#define XS_CTL_DATA_IN		SCSI_DATA_IN
#define XS_CTL_DATA_OUT		SCSI_DATA_OUT
#define scsipi_adapter		scsi_adapter
#define scsipi_cmd		scsi_cmd
#define scsipi_device		scsi_device
#define scsipi_done		scsi_done
#define scsipi_link		scsi_link
#define scsipi_minphys		scsi_minphys
#define scsipi_sense		scsi_sense
#define scsipi_xfer		scsi_xfer
#define xs_control		flags
#define xs_status		status

#define	memcpy(d, s, l)		bcopy((s),(d),(l))
#define	memset(d, v, l)		bzero((d),(l))
#define bswap32(x)		swap32(x)
#define bswap16(x)		swap16(x)

/*
 * The UHCI/OHCI controllers are little endian, so on big endian machines
 * the data strored in memory needs to be swapped.
 */

#if defined(letoh32)
#define le32toh(x) letoh32(x)
#define le16toh(x) letoh16(x)
#endif

#if (BYTE_ORDER == BIG_ENDIAN)
#define htole32(x) (bswap32(x))
#define le32toh(x) (bswap32(x))
#else
#define htole32(x) (x)
#define le32toh(x) (x)
#endif

#define usb_kthread_create1	kthread_create
#define usb_kthread_create	kthread_create_deferred

typedef int usb_malloc_type;

#define Ether_ifattach(ifp, eaddr) ether_ifattach(ifp)
#define if_deactivate(x)
#define IF_INPUT(ifp, m) do {						\
	struct ether_header *eh;					\
									\
	eh = mtod(m, struct ether_header *);				\
	m_adj(m, sizeof(struct ether_header));				\
	ether_input((ifp), (eh), (m));					\
} while (0)

#define powerhook_establish(fn, sc) (fn)
#define powerhook_disestablish(hdl)
#define PWR_RESUME 0

#define swap_bytes_change_sign16_le swap_bytes_change_sign16
#define change_sign16_swap_bytes_le change_sign16_swap_bytes
#define change_sign16_le change_sign16

extern int cold;

#elif defined(__FreeBSD__)
/*
 * FreeBSD
 */

#include "opt_usb.h"

#define usb_kthread_create1(f, s, p, a0, a1) \
		kthread_create((f), (s), (p), RFHIGHPID, 0, (a0), (a1))
#define usb_kthread_create2(f, s, p, a0) \
		kthread_create((f), (s), (p), RFHIGHPID, 0, (a0))
#define usb_kthread_create	kthread_create

#define clalloc(p, s, x) (clist_alloc_cblocks((p), (s), (s)), 0)
#define clfree(p) clist_free_cblocks((p))

#define PWR_RESUME 0
#define PWR_SUSPEND 1

//typedef struct malloc_type *usb_malloc_type;

#endif /* __FreeBSD__ */

#undef USBVERBOSE //dpp
//#define USBVERBOSE

#ifndef Static
#define Static               static
#endif

#ifndef logprintf
#define logprintf printf
#endif

#ifdef MALLOC_DECLARE
MALLOC_DECLARE(M_USB);
MALLOC_DECLARE(M_USBDEV);
MALLOC_DECLARE(M_USBHC);
#endif

#ifdef SYSCTL_DECL
SYSCTL_DECL(_hw_usb);
#endif

/* enable support for the old USB interface: */
#define USB_COMPAT_OLD

/* force debugging until further */
#ifndef USB_DEBUG
#define USB_DEBUG
#endif

#ifdef USB_DEBUG
#define PRINTF(x)      { if (usbdebug) { printf("%s: ", __FUNCTION__); printf x ; } }
#define PRINTFN(n,x)   { if (usbdebug > (n)) { printf("%s: ", __FUNCTION__); printf x ; } }
extern int usbdebug;
#else
#define PRINTF(x)
#define PRINTFN(n,x)
#endif

#define USBD_CHECK_STATUS(xfer)			\
{ if((xfer)->flags & USBD_DEV_TRANSFERRING)	\
  {						\
     (xfer)->flags &= ~USBD_DEV_TRANSFERRING;	\
     if( (xfer)->error )			\
     { goto tr_error; }				\
     else					\
     { goto tr_transferred; }			\
  }						\
  else						\
  { goto tr_setup; }				\
}						\
/**/

#define _MAKE_ENUM(enum,value,arg...)		\
        enum value,				\
/**/

#define MAKE_ENUM(macro,end...)			\
enum { macro(_MAKE_ENUM) end }			\
/**/

#define __MAKE_TABLE(a...) a    /* double pass to expand all macros */
#define _MAKE_TABLE(a...) (a),  /* add comma */
#define MAKE_TABLE(m,field,p,a...) m##_##field p = { __MAKE_TABLE(m(m##_##field _MAKE_TABLE)) a }

#ifndef LOG2
#define LOG2(x) ( \
((x) <= (1<<0x0)) ? 0x0 : \
((x) <= (1<<0x1)) ? 0x1 : \
((x) <= (1<<0x2)) ? 0x2 : \
((x) <= (1<<0x3)) ? 0x3 : \
((x) <= (1<<0x4)) ? 0x4 : \
((x) <= (1<<0x5)) ? 0x5 : \
((x) <= (1<<0x6)) ? 0x6 : \
((x) <= (1<<0x7)) ? 0x7 : \
((x) <= (1<<0x8)) ? 0x8 : \
((x) <= (1<<0x9)) ? 0x9 : \
((x) <= (1<<0xA)) ? 0xA : \
((x) <= (1<<0xB)) ? 0xB : \
((x) <= (1<<0xC)) ? 0xC : \
((x) <= (1<<0xD)) ? 0xD : \
((x) <= (1<<0xE)) ? 0xE : \
((x) <= (1<<0xF)) ? 0xF : \
0x10)
#endif /* LOG2 */

/* preliminary fix for a bug in msleep on FreeBSD, 
 * which cannot sleep with Giant:
 */
#define msleep(i,m,p,w,t) msleep(i,(((m) == &Giant) ? NULL : (m)),p,w,t)

#ifdef USB_COMPAT_OLD

/*
 * NOTE: All macros in this section 
 * should be expanded in new drivers.
 * One day USB_COMPAT_OLD will
 * be undefined!
 */

typedef struct thread *usb_proc_ptr;


#define device_ptr_t         device_t
#define USBBASEDEVICE        device_t
#define USBDEV(bdev)         (bdev)
#define USBDEVNAME(bdev)     device_get_nameunit(bdev)
#define USBDEVUNIT(bdev)     device_get_unit(bdev)
#define USBDEVPTRNAME(bdev)  device_get_nameunit(bdev)
#define USBDEVUNIT(bdev)     device_get_unit(bdev)
#define USBGETSOFTC(bdev)    device_get_softc(bdev)
#define	USB_DNAME(dname)     dname

typedef struct __callout              usb_callout_t;
#define usb_callout_init(h)           __callout_init_mtx(&(h), &Giant, 0)
#define usb_callout(h, t, f, d)       __callout_reset(&(h), (t), (f), (d))
#define usb_uncallout(h, f, d)        __callout_stop(&(h))
#define usb_uncallout_drain(h, f, d)  __callout_stop(&(h))

#define USB_DECLARE_DRIVER_INIT(dname, init...)		\
  Static device_probe_t __CONCAT(dname,_match);		\
  Static device_attach_t __CONCAT(dname,_attach);	\
  Static device_detach_t __CONCAT(dname,_detach);	\
							\
  Static devclass_t __CONCAT(dname,_devclass);		\
							\
  Static device_method_t __CONCAT(dname,_methods)[] = {	\
    DEVMETHOD(device_probe, __CONCAT(dname,_match)),	\
    DEVMETHOD(device_attach, __CONCAT(dname,_attach)),	\
    DEVMETHOD(device_detach, __CONCAT(dname,_detach)),	\
    init,						\
    {0,0}						\
  };							\
							\
  Static driver_t __CONCAT(dname,_driver) = {		\
      #dname,						\
      __CONCAT(dname,_methods),				\
      sizeof(struct __CONCAT(dname,_softc))		\
  };							\
  MODULE_DEPEND(dname, usb, 1, 1, 1)

#define METHODS_NONE				\
	{0,0}

#define USB_DECLARE_DRIVER(dname)			\
	USB_DECLARE_DRIVER_INIT(dname, METHODS_NONE)

#define USB_MATCH(dname)			\
	Static int				\
	__CONCAT(dname,_match)(device_t self)

#define USB_MATCH_START(dname, uaa) \
        struct usb_attach_arg *uaa = device_get_ivars(self)

#define USB_MATCH_SETUP \
	sc->sc_dev = self

#define USB_ATTACH(dname)			\
	Static int				\
	__CONCAT(dname,_attach)(device_t self)

#define USB_ATTACH_START(dname, sc, uaa)				\
        struct __CONCAT(dname,_softc) *sc = device_get_softc(self);	\
        struct usb_attach_arg *uaa = device_get_ivars(self)

#define USB_ATTACH_SETUP			\
	sc->sc_dev = self;			\
	device_set_desc_copy(self, devinfo)

#define USB_ATTACH_ERROR_RETURN			\
	return ENXIO

#define USB_ATTACH_SUCCESS_RETURN		\
	return 0

#define USB_DETACH(dname)			\
	Static int				\
	__CONCAT(dname,_detach)(device_t self)

#define USB_DETACH_START(dname, sc)					\
	struct __CONCAT(dname,_softc) *sc = device_get_softc(self)

#define USB_GET_SC_OPEN(dname, unit, sc)				\
	(sc) = devclass_get_softc(__CONCAT(dname,_devclass), unit);	\
	if ((sc) == NULL)						\
	     return (ENXIO)

#define USB_GET_SC(dname, unit, sc)					\
	(sc) = devclass_get_softc(__CONCAT(dname,_devclass), unit)

#define USBD_IN_PROGRESS USBD_NORMAL_COMPLETION
#define USBD_NO_COPY 0
#define USBD_EXCLUSIVE_USE 0
#define USBD_SHOW_INTERFACE_CLASS 0
#define splusb splbio
#define usb_find_desc(udev, type, subtype) usbd_find_descriptor(usbd_get_config_descriptor(udev), type, subtype)
#define usbd_get_desc usbreq_get_desc
#define usbd_get_string(udev, si, ptr) usbreq_get_string_any(udev, si, ptr, USB_MAX_STRING_LEN)
#define usbd_get_string_desc usbreq_get_string_desc
#define usbd_get_config_desc usbreq_get_config_desc
#define usbd_get_config_desc_full usbreq_get_config_desc_full
#define usbd_get_device_desc usbreq_get_device_desc
#define usbd_get_interface(iface,args...) usbreq_get_interface((iface)->udev, (iface) - &(iface)->udev->ifaces[0], args)
#define usbd_set_interface(iface,args...) usbreq_set_interface((iface)->udev, (iface) - &(iface)->udev->ifaces[0], args)
#define usbd_get_device_status usbreq_get_device_status
#define usbd_get_hub_descriptor usbreq_get_hub_descriptor
#define usbd_get_hub_status usbreq_get_hub_status
#define usbd_set_address usbreq_set_address
#define usbd_get_port_status usbreq_get_port_status
#define usbd_clear_hub_feature usbreq_clear_hub_feature
#define usbd_set_hub_feature usbreq_set_hub_feature
#define usbd_clear_port_feature usbreq_clear_port_feature
#define usbd_set_port_feature usbreq_set_port_feature
#define usbd_set_protocol(iface,args...) usbreq_set_protocol((iface)->udev, (iface) - &(iface)->udev->ifaces[0], args)
#define usbd_set_report(iface,args...) usbreq_set_report((iface)->udev, (iface) - &(iface)->udev->ifaces[0], args)
#define usbd_set_report_async(iface,args...) usbreq_set_report_async((iface)->udev, (iface) - &(iface)->udev->ifaces[0], args)
#define usbd_get_report(iface,args...) usbreq_get_report((iface)->udev, (iface) - &(iface)->udev->ifaces[0], args)
#define usbd_set_idle(iface,args...) usbreq_set_idle((iface)->udev, (iface) - &(iface)->udev->ifaces[0], args)
#define usbd_get_report_descriptor usbreq_get_report_descriptor
#define usbd_read_report_desc(iface,args...) usbreq_read_report_desc((iface)->udev, (iface) - &(iface)->udev->ifaces[0], args)
#define usbd_set_config usbreq_set_config
#define usbd_get_config usbreq_get_config
#define usbd_dopoll(iface) usbd_do_poll((iface)->udev)
#define ifaceno iface_index /* umass.c */
#define usbd_do_request_async(udev, req, data) \
usbd_do_request_flags(udev, req, data, USBD_USE_POLLING, 0, 500 /* ms */)

/*
 * depreciated Giant locked task-queue
 * (only used by if_udav.c)
 */
struct usb_task  {

#ifdef __FreeBSD__
	struct task task;
#endif
	void (*func)(void *);
	void *arg;
};

void
usb_call_task(void *arg, int count);

#define usb_init_task(_task, _func, _arg) \
{ (_task)->func = (_func); (_task)->arg = (_arg); \
  TASK_INIT(&(_task)->task, 0, &usb_call_task, (_task)); }
#define usb_add_task(udev, _task) \
taskqueue_enqueue(taskqueue_swi_giant, &(_task)->task)
#define usb_rem_task(udev, _task) \
taskqueue_drain(taskqueue_swi_giant, &(_task)->task)

#endif /* USB_COMPAT_OLD */
extern int cold;
#endif /* _USB_PORT_H */
