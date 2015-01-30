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

#ifndef _USB_SUBR_H_
#define _USB_SUBR_H_

#define USBD_STATUS_DESC(enum,value) #enum
#define USBD_STATUS(m)\
m(USBD_NORMAL_COMPLETION,=0 /* must be zero*/)\
/* errors */\
m(USBD_PENDING_REQUESTS,)\
m(USBD_NOT_STARTED,)\
m(USBD_INVAL,)\
m(USBD_NOMEM,)\
m(USBD_CANCELLED,)\
m(USBD_BAD_ADDRESS,)\
m(USBD_BAD_BUFSIZE,)\
m(USBD_BAD_FLAG,)\
m(USBD_NO_CALLBACK,)\
m(USBD_SYNC_TRANSFER_MUST_USE_DEFAULT_CALLBACK,)\
m(USBD_IN_USE,)\
m(USBD_NO_ADDR,)\
m(USBD_NO_PIPE,)\
m(USBD_ZERO_FRAMES_IN_ISOC_MODE,)\
m(USBD_SET_ADDR_FAILED,)\
m(USBD_NO_POWER,)\
m(USBD_TOO_DEEP,)\
m(USBD_IOERROR,)\
m(USBD_NOT_CONFIGURED,)\
m(USBD_TIMEOUT,)\
m(USBD_SHORT_XFER,)\
m(USBD_STALLED,)\
m(USBD_INTERRUPTED,)\
/**/

MAKE_ENUM(USBD_STATUS,
	N_USBD_STATUS);

struct usbd_xfer;
struct usbd_pipe;
struct usbd_bus;
struct usbd_config;
struct usbd_device;
struct usbd_interface;
struct usbd_memory_info;
struct usbd_ifqueue;
struct __callout;
struct module;
struct bus_dma_tag;
struct malloc_type;
struct usb_device;

typedef u_int8_t usbd_status;

typedef void (*usbd_callback_t)(struct usbd_xfer *);
#ifdef USB_COMPAT_OLD
typedef void (*usbd_callback)(struct usbd_xfer *, void *, usbd_status);
typedef struct usbd_xfer *usbd_xfer_handle;
typedef struct usbd_device *usbd_device_handle;
typedef struct usbd_pipe *usbd_pipe_handle;
typedef struct usbd_interface *usbd_interface_handle;
typedef void *usbd_private_handle;
#endif

struct usbd_bus_methods {
	void (*pipe_init)(struct usbd_device *udev, 
			  usb_endpoint_descriptor_t *edesc, 
			  struct usbd_pipe *pipe);
	void (*do_poll)(struct usbd_bus *);

	usbd_status (*xfer_setup)(struct usbd_device *udev,
				  u_int8_t iface_index, 
				  struct usbd_xfer **pxfer, 
				  const struct usbd_config *setup_start, 
				  const struct usbd_config *setup_end);
};

struct usbd_pipe_methods {
	void (*open)(struct usbd_xfer *xfer);
	void (*close)(struct usbd_xfer *xfer);
	void (*enter)(struct usbd_xfer *xfer);
	void (*start)(struct usbd_xfer *xfer);
};

struct usbd_port {
	usb_port_status_t	status;
	u_int16_t		power;	/* mA of current on port */
	u_int8_t		portno;
	u_int8_t		restartcnt;
	u_int8_t		last_refcount;
#define USBD_RESTART_MAX 5
	struct usbd_device     *device;	/* connected device */
	struct usbd_device     *parent;	/* the ports hub */
};

struct usbd_hub {
	usbd_status	      (*explore)(struct usbd_device *hub);
	void		       *hubsoftc;
	usb_hub_descriptor_t	hubdesc;
	struct usbd_port        ports[0];
};

/*****/

struct usbd_bus {
	/* filled by HC driver */
	device_t                bdev; /* base device, host adapter */
	struct usbd_bus_methods	*methods;
	thread_wait_t bus_wait_q;
	thread_wait_t wait_explore_q;
	thread_wait_t needs_explore_q;
	/* filled by USB driver */
	struct usbd_port	root_port; /* dummy port for root hub */
	struct usbd_device *	devices[USB_MAX_DEVICES];
	u_int8_t		is_exploring;
	u_int8_t		wait_explore;

	u_int8_t		needs_explore;/* a hub signalled a change
					       * this variable is protected by
					       * "usb_global_lock"
					       */
	u_int8_t		use_polling;
	u_int8_t		usbrev;	/* USB revision */
#define USBREV_UNKNOWN	0
#define USBREV_PRE_1_0	1
#define USBREV_1_0	2
#define USBREV_1_1	3
#define USBREV_2_0	4
#define USBREV_STR { "unknown", "pre 1.0", "1.0", "1.1", "2.0" }

 	struct usb_device_stats	stats;
	void*		mtx;
 	//struct proc *		event_thread;
 	void*		event_thread; //dpp
 	u_int			no_intrs;
};

struct usbd_interface {
#ifdef USB_COMPAT_OLD
	struct usbd_device	   *udev;
#endif
	usb_interface_descriptor_t *idesc;
	u_int8_t		    alt_index;
};

#define usbd_clear_endpoint_toggle(pipe) { \
(pipe)->clearstall = 0; (pipe)->toggle_next = 0; }

struct usbd_pipe {
#ifdef USB_COMPAT_OLD
	struct usbd_device *	  udev;
	struct usbd_xfer *	  alloc_xfer;
#endif
	usb_endpoint_descriptor_t *edesc;
	LIST_HEAD(, usbd_xfer)	  list_head;
	u_int16_t		  isoc_next;
	u_int8_t		  toggle_next;
	u_int8_t		  refcount;
	u_int8_t		  clearstall;
	u_int8_t		  iface_index;
	/* default pipe does not use ``iface_index'' */

	/* filled by HC driver */
	struct usbd_pipe_methods  *methods;
};

struct usbd_device {
	struct usbd_bus	       *bus;           /* our controller */
	struct usbd_pipe        default_pipe;  /* pipe 0 */
	usb_endpoint_descriptor_t default_ep_desc; /* for pipe 0 */
	u_int8_t		address;       /* device addess */
	u_int8_t		config;	       /* current configuration # */
	u_int8_t		depth;         /* distance from root hub */
	u_int8_t		speed;         /* low/full/high speed */
	u_int8_t		self_powered;  /* flag for self powered */
	u_int16_t		power;         /* mA the device uses */
	int16_t			langid;	       /* language for strings */
#define USBD_NOLANG (-1)
	usb_event_cookie_t	cookie;	       /* unique connection id */
	struct usbd_port *	powersrc;      /* upstream hub port, or 0 */
	struct usbd_port *	myhsport;       /* closest high speed port */
	struct usbd_device *	myhub;	       /* upstream hub */

	usb_device_descriptor_t ddesc;         /* device descriptor */

	usb_config_descriptor_t *cdesc;	       /* full config descr */
	const struct usbd_quirks *quirks;  /* device quirks, always set */
	struct usbd_hub	*	hub;           /* only if this is a hub */
	struct usb_device *linux_dev;

	device_t                subdevs[USB_MAX_ENDPOINTS]; /* array of all sub-devices */
	device_t                subdevs_end[0];
	struct usbd_interface   ifaces[USB_MAX_ENDPOINTS]; /* array of all interfaces */
	struct usbd_interface   ifaces_end[0];
	struct usbd_pipe        pipes[USB_MAX_ENDPOINTS]; /* array of all pipes */
	struct usbd_pipe        pipes_end[0];

	u_int8_t                ifaces_no_probe[(USB_MAX_ENDPOINTS + 7) / 8];
#define USBD_SET_IFACE_NO_PROBE(udev, ii) \
  { (udev)->ifaces_no_probe[(ii) >> 3] |= (1 << ((ii) & 7)); }
#define USBD_CLR_IFACE_NO_PROBE(udev, ii) \
  { (udev)->ifaces_no_probe[(ii) >> 3] &= ~(1 << ((ii) & 7)); }
#define USBD_GET_IFACE_NO_PROBE(udev, ii) \
  ((udev)->ifaces_no_probe[(ii) >> 3] & (1 << ((ii) & 7)))

	u_int8_t                probed; /* probe state */
#define USBD_PROBED_NOTHING              0 /* default value */
#define USBD_PROBED_SPECIFIC_AND_FOUND   1
#define USBD_PROBED_IFACE_AND_FOUND      2
#define USBD_PROBED_GENERIC_AND_FOUND    3

	u_int8_t		serial[32];
 };

/* USB transfer states */

#define	USB_ST_SETUP       0
#define	USB_ST_TRANSFERRED 1
#define	USB_ST_ERROR       2

#define	USB_USE_POLLING         0x0001	/* internal flag */
#define	USB_SHORT_XFER_OK       0x0004	/* allow short reads */
#define	USB_DELAY_STATUS_STAGE  0x0010	/* insert delay before STATUS stage */
#define	USB_USER_DATA_PTR	0x0020	/* internal flag */

typedef uint8_t usb2_error_t;

typedef void (usb2_callback_t)(struct usbd_xfer *);

/*
 * The following structure defines a set of USB transfer flags.
 */
struct usb2_xfer_flags {
	uint8_t	force_short_xfer:1;	/* force a short transmit transfer
					 * last */
	uint8_t	short_xfer_ok:1;	/* allow short receive transfers */
	uint8_t	short_frames_ok:1;	/* allow short frames */
	uint8_t	pipe_bof:1;		/* block pipe on failure */
	uint8_t	proxy_buffer:1;		/* makes buffer size a factor of
					 * "max_frame_size" */
	uint8_t	ext_buffer:1;		/* uses external DMA buffer */
	uint8_t	manual_status:1;	/* non automatic status stage on
					 * control transfers */
	uint8_t	no_pipe_ok:1;		/* set if "USB_ERR_NO_PIPE" error can
					 * be ignored */
	uint8_t	stall_pipe:1;		/* set if the endpoint belonging to
					 * this USB transfer should be stalled
					 * before starting this transfer! */
};

/*
 * The following structure defines a set of internal USB transfer
 * flags.
 */
struct usb2_xfer_flags_int {
	uint16_t control_rem;		/* remainder in bytes */

	uint8_t	open:1;			/* set if USB pipe has been opened */
	uint8_t	transferring:1;		/* set if an USB transfer is in
					 * progress */
	uint8_t	did_dma_delay:1;	/* set if we waited for HW DMA */
	uint8_t	did_close:1;		/* set if we closed the USB transfer */
	uint8_t	draining:1;		/* set if we are draining an USB
					 * transfer */
	uint8_t	started:1;		/* keeps track of started or stopped */
	uint8_t	bandwidth_reclaimed:1;
	uint8_t	control_xfr:1;		/* set if control transfer */
	uint8_t	control_hdr:1;		/* set if control header should be
					 * sent */
	uint8_t	control_act:1;		/* set if control transfer is active */

	uint8_t	short_frames_ok:1;	/* filtered version */
	uint8_t	short_xfer_ok:1;	/* filtered version */
	uint8_t	bdma_enable:1;		/* filtered version (only set if
					 * hardware supports DMA) */
	uint8_t	bdma_no_post_sync:1;	/* set if the USB callback wrapper
					 * should not do the BUS-DMA post sync
					 * operation */
	uint8_t	bdma_setup:1;		/* set if BUS-DMA has been setup */
	uint8_t	isochronous_xfr:1;	/* set if isochronous transfer */
	uint8_t	usb2_mode:1;		/* shadow copy of "udev->usb2_mode" */
	uint8_t	curr_dma_set:1;		/* used by USB HC/DC driver */
	uint8_t	can_cancel_immed:1;	/* set if USB transfer can be
					 * cancelled immediately */
};


/*
 * The following structure defines the symmetric part of an USB config
 * structure.
 */
struct usb2_config_sub {
	usb2_callback_t *callback;	/* USB transfer callback */
	uint32_t bufsize;		/* total pipe buffer size in bytes */
	uint32_t frames;		/* maximum number of USB frames */
	uint16_t interval;		/* interval in milliseconds */
#define	USB_DEFAULT_INTERVAL	0
	uint16_t timeout;		/* transfer timeout in milliseconds */
	struct usb2_xfer_flags flags;	/* transfer flags */
};

struct usbd_config {
	struct usb2_config_sub mh;	/* parameters for USB_MODE_HOST */
	struct usb2_config_sub md;	/* parameters for USB_MODE_DEVICE */

	u_int8_t	type;		/* pipe type */
	u_int8_t	endpoint;	/* pipe number */

	u_int8_t	direction;	/* pipe direction */
	u_int8_t	interval;	/* interrupt interval in milliseconds;
					 * used by interrupt pipes
					 */
#define USBD_DEFAULT_INTERVAL	0

	u_int16_t	timeout;	/* milliseconds */

	u_int8_t	frames;		/* number of frames
					 * used in isochronous
					 * mode
					 */

	u_int8_t	index;	/* pipe index to use, if more
				 * than one descriptor matches
				 * type, address, direction ...
				 */

	u_int32_t	flags;	/* flags */
#define USBD_SYNCHRONOUS         0x0001 /* wait for completion */
#define USBD_FORCE_SHORT_XFER    0x0002 /* force a short packet last */
#if (USBD_SHORT_XFER_OK != 0x0004)
#define USBD_SHORT_XFER_OK       0x0004 /* allow short reads 
					 * NOTE: existing software
					 * expects USBD_SHORT_XFER_OK
					 * to have a value of 0x4. This
					 * flag is also exported by usb.h
					 */
#endif
#if (defined(USB_COMPAT_OLD) || 1)
#define USBD_CUSTOM_CLEARSTALL   0x0008 /* used to disable automatic clear-stall
					 * when a device reset request is needed
					 * in addition to the clear stall request
					 */
#endif
#define USBD_DEV_OPEN            0x0010
#define USBD_DEV_RECURSED_1      0x0020
#define USBD_DEV_RECURSED_2      0x0040
#define USBD_DEV_TRANSFERRING    0x0080
#define USBD_BANDWIDTH_RECLAIMED 0x0100
#define USBD_USE_POLLING         0x0200 /* used to make synchronous transfers
					 * use polling instead of sleep/wakeup
					 */
#define USBD_SELF_DESTRUCT       0x0400 /* set if callback is allowed to unsetup itself */
#define USBD_UNUSED_3            0x0800
#define USBD_UNUSED_4            0x1000
#define USBD_UNUSED_5            0x2000
#define USBD_UNUSED_6            0x4000
#define USBD_UNUSED_7            0x8000

	u_int32_t	bufsize;       	/* total pipe buffer size in bytes */
	usbd_callback_t	callback;
};

#define USBD_TRANSFER_IN_PROGRESS(xfer)		\
	((xfer)->flags & USBD_DEV_TRANSFERRING)

struct usb2_page_cache
{
	int a; //dpp fixme
};



struct usbd_xfer {
	struct usbd_pipe *	pipe;
	struct usbd_device *	udev;
	void *			buffer;
 	void *			priv_sc;
	void *			priv_fifo;
	struct mtx *		priv_mtx;
	struct usbd_xfer *	clearstall_xfer;
	u_int32_t		length; /* bytes */
	u_int32_t		actlen; /* bytes */
	u_int32_t		timeout; /* milliseconds */
	thread_wait_t wait_q;
#define USBD_NO_TIMEOUT 0
#define USBD_DEFAULT_TIMEOUT 5000 /* 5000 ms = 5 seconds */

	usbd_status		error;

	/* for isochronous transfers */

	uint32_t max_frame_count;	/* initial value of "nframes" after
					 * setup */
	uint32_t nframes;		/* number of USB frames to transfer */
	uint32_t aframes;		/* actual number of USB frames
					 * transferred */


	/*
	 * used by HC driver
	 */

	void *			usb_sc;
	struct mtx *		usb_mtx;
	struct usbd_memory_info *usb_root;
	struct thread *		usb_thread;
	u_int32_t		usb_refcount;

	/* pipe_list is used to start next transfer */

	LIST_ENTRY(usbd_xfer)	pipe_list; 

	/* interrupt_list is used to check
	 * for finished transfers
	 */

	LIST_ENTRY(usbd_xfer)	interrupt_list;

	struct __callout	timeout_handle;

	u_int8_t		address;
	u_int8_t		endpoint;
	u_int8_t		interval; /* milliseconds */

	u_int16_t		max_packet_size;

	u_int32_t		physbuffer;

	void *			td_start;
	void *			td_end;

	void *			td_transfer_first;
	void *			td_transfer_last;

	void *			qh_start;
	void *			qh_end;

	u_int16_t		qh_pos;
	//uint16_t max_packet_size; //dpp
	uint16_t max_frame_size; //dpp

#ifdef USB_COMPAT_OLD
	struct usbd_xfer *	alloc_xfer; /* the real transfer */
	void *			alloc_ptr;
	u_int32_t		alloc_len;
	void *			d_copy_ptr;
	void *			d_copy_src;
	void *			d_copy_dst;
	u_int32_t		d_copy_len;
	usbd_callback		d_callback;

	void *			f_copy_ptr;
	void *			f_copy_src;
	void *			f_copy_dst;
	u_int32_t		f_copy_len;
#endif

	uint32_t *frlengths;
	struct usb2_page_cache *frbuffers;
	usb2_callback_t *callback;

	uint32_t max_usb2_frame_size;
	uint32_t max_data_length;
	uint32_t sumlen;		/* sum of all lengths in bytes */
	uint32_t flags;		/*  */

	uint8_t	usb2_state;

	struct usb2_xfer_flags flags2;
	struct usb2_xfer_flags_int flags_int;

};

struct usbd_memory_wait {
    struct mtx *   priv_mtx;
	thread_wait_t wait_q;
    u_int16_t      priv_refcount;
    u_int16_t      priv_sleeping;
};

struct usbd_memory_info {
    void *         memory_base;
    u_int32_t      memory_size;
    u_int32_t      memory_refcount;
    struct usbd_memory_wait * priv_wait;
    struct mtx *   priv_mtx;
    struct mtx *   usb_mtx;
};

struct usbd_callback_info {
    struct usbd_xfer *xfer;
    u_int32_t refcount;
};

struct usbd_mbuf {
  u_int8_t *cur_data_ptr;
  u_int8_t *min_data_ptr;
  struct usbd_mbuf *usbd_nextpkt;
  struct usbd_mbuf *usbd_next;

  u_int32_t cur_data_len;
  u_int32_t max_data_len;
};

struct usbd_ifqueue {
  struct  usbd_mbuf *ifq_head;
  struct  usbd_mbuf *ifq_tail;

  int32_t ifq_len;
  int32_t ifq_maxlen;
};

#define USBD_IF_ENQUEUE(ifq, m) do {		\
    (m)->usbd_nextpkt = NULL;			\
    if ((ifq)->ifq_tail == NULL)		\
        (ifq)->ifq_head = (m);			\
    else					\
        (ifq)->ifq_tail->usbd_nextpkt = (m);	\
    (ifq)->ifq_tail = (m);			\
    (ifq)->ifq_len++;				\
  } while (0)

#define USBD_IF_DEQUEUE(ifq, m) do {				\
    (m) = (ifq)->ifq_head;					\
    if (m) {							\
        if (((ifq)->ifq_head = (m)->usbd_nextpkt) == NULL) {	\
	     (ifq)->ifq_tail = NULL;				\
	}							\
	(m)->usbd_nextpkt = NULL;				\
	(ifq)->ifq_len--;					\
    }								\
  } while (0)

#define USBD_IF_PREPEND(ifq, m) do {		\
      (m)->usbd_nextpkt = (ifq)->ifq_head;	\
      if ((ifq)->ifq_tail == NULL) {		\
	  (ifq)->ifq_tail = (m);		\
      }						\
      (ifq)->ifq_head = (m);			\
      (ifq)->ifq_len++;				\
  } while (0)

#define USBD_IF_QFULL(ifq)   ((ifq)->ifq_len >= (ifq)->ifq_maxlen)
#define USBD_IF_QLEN(ifq)    ((ifq)->ifq_len)
#define USBD_IF_POLL(ifq, m) ((m) = (ifq)->ifq_head)

#define USBD_MBUF_RESET(m) do {			\
    (m)->cur_data_ptr = (m)->min_data_ptr;	\
    (m)->cur_data_len = (m)->max_data_len;	\
  } while (0)

/*---------------------------------------------------------------------------*
 * structures used by probe and attach
 *---------------------------------------------------------------------------*/
struct usb_devno {
    u_int16_t ud_vendor;
    u_int16_t ud_product;
} __packed;

#define usb_lookup(tbl, vendor, product) usb_match_device			\
	((const struct usb_devno *)(tbl), (sizeof (tbl) / sizeof ((tbl)[0])),	\
	 sizeof ((tbl)[0]), (vendor), (product))				\
/**/

#define	USB_PRODUCT_ANY		0xffff

struct usb2_lookup_info {
	uint16_t idVendor;
	uint16_t idProduct;
	uint16_t bcdDevice;
	uint8_t	bDeviceClass;
	uint8_t	bDeviceSubClass;
	uint8_t	bDeviceProtocol;
	uint8_t	bInterfaceClass;
	uint8_t	bInterfaceSubClass;
	uint8_t	bInterfaceProtocol;
	uint8_t	bIfaceIndex;
	uint8_t	bIfaceNum;
	uint8_t	bConfigIndex;
	uint8_t	bConfigNum;
};

struct usb_attach_arg
{
	struct usb2_lookup_info info;
	int			port;
	int			configno;
	int			iface_index;
	int			vendor;
	int			product;
	int			release;
	int			matchlvl;
	struct usbd_device     *device;	/* current device */
	struct usbd_interface  *iface; /* current interface */
	int			usegeneric;
	uint8_t	usb2_mode;		/* see USB_MODE_XXX */
	struct usbd_interface  *ifaces_start; /* all interfaces */
	struct usbd_interface  *ifaces_end; /* exclusive */
#ifdef USB_COMPAT_OLD
	int			nifaces;
	struct usbd_interface * ifaces[USB_MAX_ENDPOINTS];
#endif
};

/* return values for device_probe() method: */

#define UMATCH_VENDOR_PRODUCT_REV			(-10)
#define UMATCH_VENDOR_PRODUCT				(-20)
#define UMATCH_VENDOR_DEVCLASS_DEVPROTO			(-30)
#define UMATCH_DEVCLASS_DEVSUBCLASS_DEVPROTO		(-40)
#define UMATCH_DEVCLASS_DEVSUBCLASS			(-50)
#define UMATCH_VENDOR_PRODUCT_REV_CONF_IFACE		(-60)
#define UMATCH_VENDOR_PRODUCT_CONF_IFACE		(-70)
#define UMATCH_VENDOR_IFACESUBCLASS_IFACEPROTO		(-80)
#define UMATCH_VENDOR_IFACESUBCLASS			(-90)
#define UMATCH_IFACECLASS_IFACESUBCLASS_IFACEPROTO	(-100)
#define UMATCH_IFACECLASS_IFACESUBCLASS			(-110)
#define UMATCH_IFACECLASS				(-120)
#define UMATCH_IFACECLASS_GENERIC			(-130)
#define UMATCH_GENERIC					(-140)
#define UMATCH_NONE					(ENXIO)

/*---------------------------------------------------------------------------*
 * prototypes
 *---------------------------------------------------------------------------*/

/* routines from usb_subr.c */

void
usbd_devinfo(struct usbd_device *udev, int showclass, 
	     char *dst_ptr, u_int16_t dst_len);

const char *
usbd_errstr(usbd_status err);

void
usb_delay_ms(struct usbd_bus *bus, u_int ms);

void
usbd_delay_ms(struct usbd_device *udev, u_int ms);

usb_descriptor_t *
usbd_desc_foreach(usb_config_descriptor_t *cd, usb_descriptor_t *desc);

struct usb_hid_descriptor;
struct usb_hid_descriptor *
usbd_get_hdesc(usb_config_descriptor_t *cd, usb_interface_descriptor_t *id);

usb_interface_descriptor_t *
usbd_find_idesc(usb_config_descriptor_t *cd, u_int16_t iface_index, u_int16_t alt_index);

usb_endpoint_descriptor_t *
usbd_find_edesc(usb_config_descriptor_t *cd, u_int16_t iface_index, u_int16_t alt_index,
		u_int16_t endptidx);

usb_descriptor_t *
usbd_find_descriptor(usb_config_descriptor_t *cd, int type, int subtype);

#define USBD_SUBTYPE_ANY (-1)

int
usbd_get_no_alts(usb_config_descriptor_t *cd, u_int8_t ifaceno);

usbd_status
usbd_search_and_set_config(struct usbd_device *udev, int no, int msg);

usbd_status
usbd_set_config_index(struct usbd_device *udev, int index, int msg);

int
usbd_fill_deviceinfo(struct usbd_device *udev, struct usb_device_info *di,
		     int usedev);

usbd_status
usbd_fill_iface_data(struct usbd_device *udev, int iface_index, int alt_index);

usbd_status
usbd_probe_and_attach(device_t parent, 
		      int port, struct usbd_port *up);

usbd_status
usbd_new_device(device_t parent, struct usbd_bus *bus, int depth,
		int speed, int port, struct usbd_port *up);

void
usbd_free_device(struct usbd_port *up, u_int8_t free_subdev);

void
usb_detach_wait(device_t dv);

void
usb_detach_wakeup(device_t dv);

struct usbd_interface *
usbd_get_iface(struct usbd_device *udev, u_int8_t iface_index);

void
usbd_set_desc(device_t dev, struct usbd_device *udev);

void *
usbd_alloc_mbufs(struct malloc_type *type, struct usbd_ifqueue *ifq, 
		 u_int32_t block_size, u_int16_t block_number);

/* routines from usb.c */

#if 0
extern struct mtx usb_global_lock;
#else
/* XXX currently only the Giant lock can sleep */
extern struct mtx Giant;
#define usb_global_lock Giant
#endif

void
usbd_add_dev_event(int type, struct usbd_device *udev);

void
usbd_add_drv_event(int type, struct usbd_device *udev, device_t dev);

void
usb_needs_explore(struct usbd_device *udev);

extern u_int8_t usb_driver_added_refcount;

void
usb_needs_probe_and_attach(void);

#ifdef __FreeBSD__
#define device_get_dma_tag(dev) NULL

void *
usb_alloc_mem(struct bus_dma_tag *tag, u_int32_t size, u_int8_t align_power);

bus_size_t
usb_vtophys(void *ptr, u_int32_t size);

void
usb_free_mem(void *ptr, u_int32_t size);
#endif

/* routines from usb_transfer.c */

#ifdef USB_DEBUG

void
usbd_dump_iface(struct usbd_interface *iface);

void
usbd_dump_device(struct usbd_device *udev);

void
usbd_dump_queue(struct usbd_pipe *pipe);

void
usbd_dump_pipe(struct usbd_pipe *pipe);

void
usbd_dump_xfer(struct usbd_xfer *xfer);

#endif

u_int32_t
usb_get_devid(device_t dev);

struct usbd_pipe *
usbd_get_pipe(struct usbd_device *udev, u_int8_t iface_index,
	      const struct usbd_config *setup);

usbd_status
usbd_interface_count(struct usbd_device *udev, u_int8_t *count);

usbd_status
usbd_transfer_setup(struct usbd_device *udev,
		    u_int8_t iface_index,
		    struct usbd_xfer **pxfer,
		    const struct usbd_config *setup_start,
		    u_int16_t n_setup,
		    void *priv_sc,
		    struct mtx *priv_mtx,
		    struct usbd_memory_wait *priv_wait);

void
usbd_transfer_unsetup(struct usbd_xfer **pxfer, u_int16_t n_setup);

void
usbd_transfer_drain(struct usbd_memory_wait *priv_wait, struct mtx *priv_mtx);

void
usbd_start_hardware(struct usbd_xfer *xfer);

void
usbd_transfer_start_safe(struct usbd_xfer *xfer);

void
usbd_transfer_start(struct usbd_xfer *xfer);

void
usbd_transfer_stop(struct usbd_xfer *xfer);

void
__usbd_callback(struct usbd_xfer *xfer);

void
usbd_do_callback(struct usbd_callback_info *ptr, 
		 struct usbd_callback_info *limit);
void
usbd_transfer_done(struct usbd_xfer *xfer, usbd_status error);

void
usbd_transfer_enqueue(struct usbd_xfer *xfer);

void
usbd_transfer_dequeue(struct usbd_xfer *xfer);

void
usbd_default_callback(struct usbd_xfer *xfer);

usbd_status
usbd_do_request(struct usbd_device *udev, usb_device_request_t *req, void *data);

usbd_status
usbd_do_request_flags(struct usbd_device *udev, usb_device_request_t *req,
		      void *data, u_int32_t flags, int *actlen,
		      u_int32_t timeout);
void
usbd_fill_get_report(usb_device_request_t *req, u_int8_t iface_no, 
		     u_int8_t type, u_int8_t id, u_int16_t size);
void
usbd_fill_set_report(usb_device_request_t *req, u_int8_t iface_no,
		     u_int8_t type, u_int8_t id, u_int16_t size);
void
usbd_clear_stall_tr_setup(struct usbd_xfer *xfer1, 
			  struct usbd_xfer *xfer2);
void
usbd_clear_stall_tr_transferred(struct usbd_xfer *xfer1, 
				struct usbd_xfer *xfer2);
void
usbd_clearstall_callback(struct usbd_xfer *xfer);

void
usbd_do_poll(struct usbd_device *udev);

void
usbd_set_polling(struct usbd_device *udev, int on);

//int usbd_ratecheck(struct timeval *last);

const struct usb_devno *
usb_match_device(const struct usb_devno *tbl, u_int nentries, u_int size,
		 u_int16_t vendor, u_int16_t product);

int
usbd_driver_load(struct module *mod, int what, void *arg);

#ifdef USB_COMPAT_OLD
usbd_status
usbd_transfer(struct usbd_xfer *xfer);

usbd_status
usbd_sync_transfer(struct usbd_xfer *xfer);

void *
usbd_alloc_buffer(struct usbd_xfer *xfer, u_int32_t size);

void
usbd_free_buffer(struct usbd_xfer *xfer);

void
usbd_get_xfer_status(struct usbd_xfer *xfer, void **priv,
		     void **buffer, u_int32_t *count, usbd_status *status);

struct usbd_xfer *
usbd_alloc_xfer(struct usbd_device *dev);

usbd_status 
usbd_free_xfer(struct usbd_xfer *xfer);

usbd_status 
usbd_open_pipe(struct usbd_interface *iface, u_int8_t address,
               u_int8_t flags, struct usbd_pipe **pipe);

usbd_status 
usbd_open_pipe_intr(struct usbd_interface *iface, u_int8_t address,
                    u_int8_t flags, struct usbd_pipe **pipe,
                    void *priv, void *buffer, u_int32_t len,
                    usbd_callback callback, int ival);

usbd_status
usbd_setup_xfer(struct usbd_xfer *xfer, struct usbd_pipe *pipe,
                void *priv, void *buffer, u_int32_t length,
                u_int32_t flags, u_int32_t timeout,
                usbd_callback callback);

usbd_status
usbd_setup_default_xfer(struct usbd_xfer *xfer, struct usbd_device *dev,
                        void *priv, u_int32_t timeout,
                        usb_device_request_t *req, void *buffer,
                        u_int32_t length, u_int16_t flags,
                        usbd_callback callback);

usbd_status
usbd_setup_isoc_xfer(struct usbd_xfer *xfer, struct usbd_pipe *pipe,
                     void *priv, u_int16_t *frlengths, u_int32_t nframes, 
		     u_int16_t flags, usbd_callback callback);

usbd_status
usbd_bulk_transfer(struct usbd_xfer *xfer, struct usbd_pipe *pipe,
                   u_int16_t flags, u_int32_t timeout, void *buf,
                   u_int32_t *size, char *lbl);

#define usbd_intr_transfer usbd_bulk_transfer

usbd_status 
usbd_abort_pipe(struct usbd_pipe *pipe);

usbd_status 
usbd_abort_default_pipe(struct usbd_device *udev);

usbd_status
usbd_close_pipe(struct usbd_pipe *pipe);

usbd_status 
usbd_clear_endpoint_stall(struct usbd_pipe *pipe);

usbd_status 
usbd_clear_endpoint_stall_async(struct usbd_pipe *pipe);

usbd_status 
usbd_endpoint_count(struct usbd_interface *iface, u_int8_t *count);

void
usbd_interface2device_handle(struct usbd_interface *iface,
                             struct usbd_device **udev);

struct usbd_device *
usbd_pipe2device_handle(struct usbd_pipe *pipe);

usbd_status 
usbd_device2interface_handle(struct usbd_device *udev,
                             u_int8_t iface_index, struct usbd_interface **iface);

usb_endpoint_descriptor_t *
usbd_interface2endpoint_descriptor(struct usbd_interface *iface, u_int8_t index);

usb_endpoint_descriptor_t *
usbd_get_endpoint_descriptor(struct usbd_interface *iface, u_int8_t address);

#endif /* USB_COMPAT_OLD */

/* routines from usb_requests.c */

usbd_status
usbreq_reset_port(struct usbd_device *udev, int port, usb_port_status_t *ps);

usbd_status
usbreq_get_desc(struct usbd_device *udev, int type, int index,
		int len, void *desc, int timeout);

usbd_status
usbreq_get_string_any(struct usbd_device *udev, int si, char *buf, int len);

usbd_status
usbreq_get_string_desc(struct usbd_device *udev, int sindex, int langid,
		       usb_string_descriptor_t *sdesc, int *plen);

usbd_status
usbreq_get_config_desc(struct usbd_device *udev, int confidx,
		       usb_config_descriptor_t *d);

usbd_status
usbreq_get_config_desc_full(struct usbd_device *udev, int conf, void *d, int size);

usbd_status
usbreq_get_device_desc(struct usbd_device *udev, usb_device_descriptor_t *d);

usbd_status
usbreq_get_interface(struct usbd_device *udev, u_int8_t iface_index,
		     u_int8_t *aiface);

usbd_status
usbreq_set_interface(struct usbd_device *udev, u_int8_t iface_index,
		     u_int8_t altno);

usbd_status
usbreq_get_device_status(struct usbd_device *udev, usb_status_t *st);

usbd_status
usbreq_get_hub_descriptor(struct usbd_device *udev, usb_hub_descriptor_t *hd);

usbd_status
usbreq_get_hub_status(struct usbd_device *udev, usb_hub_status_t *st);

usbd_status
usbreq_set_address(struct usbd_device *udev, int addr);

usbd_status
usbreq_get_port_status(struct usbd_device *udev, int port, usb_port_status_t *ps);

usbd_status
usbreq_clear_hub_feature(struct usbd_device *udev, int sel);

usbd_status
usbreq_set_hub_feature(struct usbd_device *udev, int sel);

usbd_status
usbreq_clear_port_feature(struct usbd_device *udev, int port, int sel);

usbd_status
usbreq_set_port_feature(struct usbd_device *udev, int port, int sel);

usbd_status
usbreq_set_protocol(struct usbd_device *udev, u_int8_t iface_index,
		    u_int16_t report);

#ifdef USB_COMPAT_OLD
usbd_status
usbreq_set_report_async(struct usbd_device *udev, u_int8_t iface_index,
			u_int8_t type, u_int8_t id, void *data, int len);
#endif

usbd_status
usbreq_set_report(struct usbd_device *udev, u_int8_t iface_index,
		  u_int8_t type, u_int8_t id, void *data, int len);

usbd_status
usbreq_get_report(struct usbd_device *udev, u_int8_t iface_index,
		  u_int8_t type, u_int8_t id, void *data, int len);

usbd_status
usbreq_set_idle(struct usbd_device *udev, u_int8_t iface_index,
		int duration, int id);

usbd_status
usbreq_get_report_descriptor(struct usbd_device *udev, int ifcno,
			     int size, void *d);

usbd_status
usbreq_read_report_desc(struct usbd_device *udev, u_int8_t iface_index,
 			void **descp, int *sizep, usb_malloc_type mem);

usbd_status
usbreq_set_config(struct usbd_device *udev, int conf);

usbd_status
usbreq_get_config(struct usbd_device *udev, u_int8_t *conf);

/**/
#define usbd_get_device_descriptor(udev) (&(udev)->ddesc)
#define usbd_get_config_descriptor(udev) ((udev)->cdesc)
#define usbd_get_interface_descriptor(iface) ((iface)->idesc)
#define usbd_get_interface_altindex(iface) ((iface)->alt_index)
#define usbd_get_quirks(udev) ((udev)->quirks)
#define usbd_get_speed(udev) ((udev)->speed)
#define usbd_get_hid_descriptor usbd_get_hdesc
#define usbd_set_config_no usbd_search_and_set_config

/* helper for computing offsets */
#define POINTER_TO_UNSIGNED(ptr) \
  (((u_int8_t *)(ptr)) - ((u_int8_t *)0))

/* routines from "usb_cdev.c" */

struct usb_cdev;
struct cdev;
struct mtx;

extern int32_t
usb_cdev_sleep(struct usb_cdev *sc, int32_t fflags);

extern void
usb_cdev_wakeup(struct usb_cdev *sc);

extern int32_t
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
		u_int16_t wr_packets);

extern void
usb_cdev_detach(struct usb_cdev *sc);

extern void
usb_cdev_put_data(struct usb_cdev *sc, u_int8_t *buf, u_int32_t len, 
		  u_int8_t what);
extern void
usb_cdev_put_data_error(struct usb_cdev *sc);

extern u_int8_t
usb_cdev_get_data(struct usb_cdev *sc, u_int8_t *buf, u_int32_t len, 
		  u_int32_t *actlen, u_int8_t what);
extern void
usb_cdev_get_data_error(struct usb_cdev *sc);


typedef int32_t (usb_cdev_open_t)(struct usb_cdev *sc, int32_t fflags, 
				  int32_t mode, struct thread *td);
typedef int32_t (usb_cdev_ioctl_t)(struct usb_cdev *sc, u_long cmd, caddr_t addr, 
				   int32_t fflags, struct thread *td);

typedef void (usb_cdev_cmd_t)(struct usb_cdev *sc);

struct usb_cdev {

    struct usbd_ifqueue     sc_rdq_free;
    struct usbd_ifqueue     sc_rdq_used;
    struct usbd_ifqueue     sc_wrq_free;
    struct usbd_ifqueue     sc_wrq_used;
    struct selinfo          sc_read_sel;
    struct selinfo          sc_write_sel;

    /* various pointers */

    void *                  sc_rdq_pointer;
    void *                  sc_wrq_pointer;
    struct mtx *            sc_mtx_ptr;
    void *                  sc_priv_ptr;
#define USB_CDEV_COUNT 4
    struct cdev *           sc_cdev[USB_CDEV_COUNT];
    struct cdev *           sc_last_cdev;
    struct proc *           sc_async_rd; /* process that wants SIGIO */
    struct proc *           sc_async_wr; /* process that wants SIGIO */

    /* multiplexer functions */

    usb_cdev_open_t *       sc_open;
    usb_cdev_ioctl_t *      sc_ioctl;
    usb_cdev_cmd_t *        sc_start_read;
    usb_cdev_cmd_t *        sc_stop_read;
    usb_cdev_cmd_t *        sc_start_write;
    usb_cdev_cmd_t *        sc_stop_write;

    u_int32_t               sc_cur_context;
    u_int32_t               sc_flags;

  /* synchronization flags */

#define USB_CDEV_FLAG_GONE            0x00000001
#define USB_CDEV_FLAG_FLUSHING_WRITE  0x00000002

#define USB_CDEV_FLAG_OPEN_READ       0x00000004
#define USB_CDEV_FLAG_OPEN_WRITE      0x00000008

#define USB_CDEV_FLAG_SLEEP_READ      0x00000010
#define USB_CDEV_FLAG_SLEEP_WRITE     0x00000020

#define USB_CDEV_FLAG_SLEEP_IOCTL_RD  0x00000040
#define USB_CDEV_FLAG_SLEEP_IOCTL_WR  0x00000080

#define USB_CDEV_FLAG_WAKEUP_READ     0x00000100
#define USB_CDEV_FLAG_WAKEUP_WRITE    0x00000200

#define USB_CDEV_FLAG_WAKEUP_IOCTL_RD 0x00000400
#define USB_CDEV_FLAG_WAKEUP_IOCTL_WR 0x00000800

#define USB_CDEV_FLAG_SELECT_READ     0x00001000
#define USB_CDEV_FLAG_SELECT_WRITE    0x00002000

#define USB_CDEV_FLAG_CLOSING_READ    0x00004000
#define USB_CDEV_FLAG_CLOSING_WRITE   0x00008000

#define USB_CDEV_FLAG_ERROR_READ      0x00010000 /* can be set to indicate error */
#define USB_CDEV_FLAG_ERROR_WRITE     0x00020000 /* can be set to indicate error */

  /* other flags */

#define USB_CDEV_FLAG_FWD_SHORT       0x00040000 /* can be set to forward short transfers */
#define USB_CDEV_FLAG_READ_ONLY       0x00080000 /* device is read only */
#define USB_CDEV_FLAG_WRITE_ONLY      0x00100000 /* device is read only */
#define USB_CDEV_FLAG_WAKEUP_RD_IMMED 0x00200000 /* wakeup read thread immediately */
#define USB_CDEV_FLAG_WAKEUP_WR_IMMED 0x00400000 /* wakeup write thread immediately */

    u_int8_t                sc_wakeup_read; /* dummy */
    u_int8_t                sc_wakeup_write; /* dummy */
    u_int8_t                sc_wakeup_flush; /* dummy */
    u_int8_t                sc_wakeup_close_read; /* dummy */
    u_int8_t                sc_wakeup_close_write; /* dummy */
    u_int8_t                sc_wakeup_detach; /* dummy */
    u_int8_t                sc_wakeup_ioctl; /* dummy */
    u_int8_t                sc_wakeup_ioctl_rdwr; /* dummy */

    u_int8_t                sc_first_open; /* set when first device
					    * is being opened
					    */
};

#endif /* _USB_SUBR_H_ */
