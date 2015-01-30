/*
** Copyright 2003, Travis Geiselbrecht. All rights reserved.
** Distributed under the terms of the NewOS License.
*/
#ifndef __BUS_UHCI_H
#define __BUS_UHCI_H

typedef volatile unsigned int       vuint32_t;
//typedef unsigned int                 uint32_t;
typedef volatile int                 vint32_t;
//typedef int                           int32_t;
typedef volatile unsigned short     vuint16_t;
//typedef unsigned short               uint16_t;
typedef volatile short               vint16_t;
//typedef short                         int16_t;
typedef volatile unsigned char       vuint8_t;
//typedef unsigned char                 uint8_t;
typedef volatile char                 vint8_t;
//typedef char                           int8_t;
typedef unsigned long addr_t;
typedef int64_t bigtime_t;
typedef uint64_t vnode_id;
typedef int region_id;      // vm region id
typedef int aspace_id;      // address space id
typedef int thread_id;      // thread id
typedef int proc_id;        // process id
typedef int pgrp_id;        // process group id
typedef int sess_id;        // session id
typedef int sem_id;         // semaphore id
typedef int port_id;        // ipc port id
typedef int image_id;       // binary image id


/* Command register */
#define USBCMD		0
#define   USBCMD_RS		0x0001	/* Run/Stop */
#define   USBCMD_HCRESET	0x0002	/* Host reset */
#define   USBCMD_GRESET		0x0004	/* Global reset */
#define   USBCMD_EGSM		0x0008	/* Global Suspend Mode */
#define   USBCMD_FGR		0x0010	/* Force Global Resume */
#define   USBCMD_SWDBG		0x0020	/* SW Debug mode */
#define   USBCMD_CF		0x0040	/* Config Flag (sw only) */
#define   USBCMD_MAXP		0x0080	/* Max Packet (0 = 32, 1 = 64) */

/* Status register */
#define USBSTS		2
#define   USBSTS_USBINT		0x0001	/* Interrupt due to IOC */
#define   USBSTS_ERROR		0x0002	/* Interrupt due to error */
#define   USBSTS_RD		0x0004	/* Resume Detect */
#define   USBSTS_HSE		0x0008	/* Host System Error - basically PCI problems */
#define   USBSTS_HCPE		0x0010	/* Host Controller Process Error - the scripts were buggy */
#define   USBSTS_HCH		0x0020	/* HC Halted */

/* Interrupt enable register */
#define USBINTR		4
#define   USBINTR_TIMEOUT	0x0001	/* Timeout/CRC error enable */
#define   USBINTR_RESUME	0x0002	/* Resume interrupt enable */
#define   USBINTR_IOC		0x0004	/* Interrupt On Complete enable */
#define   USBINTR_SP		0x0008	/* Short packet interrupt enable */

#define USBFRNUM	6
#define USBFLBASEADD	8
#define USBSOF		12

/* USB port status and control registers */
#define USBPORTSC1	16
#define USBPORTSC2	18
#define   USBPORTSC_CCS		0x0001	/* Current Connect Status ("device present") */
#define   USBPORTSC_CSC		0x0002	/* Connect Status Change */
#define   USBPORTSC_PE		0x0004	/* Port Enable */
#define   USBPORTSC_PEC		0x0008	/* Port Enable Change */
#define   USBPORTSC_LS		0x0030	/* Line Status */
#define   USBPORTSC_RD		0x0040	/* Resume Detect */
#define   USBPORTSC_LSDA	0x0100	/* Low Speed Device Attached */
#define   USBPORTSC_PR		0x0200	/* Port Reset */
#define   USBPORTSC_SUSP	0x1000	/* Suspend */

/* Legacy support register */
#define USBLEGSUP 0xc0
#define USBLEGSUP_DEFAULT 0x2000	/* only PIRQ enable set */

#define UHCI_NULL_DATA_SIZE	0x7ff	/* for UHCI controller TD */
#define UHCI_PID 		0xff	/* PID MASK */

#define UHCI_PTR_BITS		0x000F
#define UHCI_PTR_TERM		0x0001
#define UHCI_PTR_QH		0x0002
#define UHCI_PTR_DEPTH		0x0004

#define UHCI_NUMFRAMES		1024	/* in the frame list [array] */
#define UHCI_MAX_SOF_NUMBER	2047	/* in an SOF packet */
#define CAN_SCHEDULE_FRAMES	1000	/* how far future frames can be scheduled */

/*
 * for TD <status>:
 */
#define TD_CTRL_SPD		(1 << 29)	/* Short Packet Detect */
#define TD_CTRL_C_ERR_MASK	(3 << 27)	/* Error Counter bits */
#define TD_CTRL_LS		(1 << 26)	/* Low Speed Device */
#define TD_CTRL_IOS		(1 << 25)	/* Isochronous Select */
#define TD_CTRL_IOC		(1 << 24)	/* Interrupt on Complete */
#define TD_CTRL_ACTIVE		(1 << 23)	/* TD Active */
#define TD_CTRL_STALLED		(1 << 22)	/* TD Stalled */
#define TD_CTRL_DBUFERR		(1 << 21)	/* Data Buffer Error */
#define TD_CTRL_BABBLE		(1 << 20)	/* Babble Detected */
#define TD_CTRL_NAK		(1 << 19)	/* NAK Received */
#define TD_CTRL_CRCTIMEO	(1 << 18)	/* CRC/Time Out Error */
#define TD_CTRL_BITSTUFF	(1 << 17)	/* Bit Stuff Error */
#define TD_CTRL_ACTLEN_MASK	0x7ff	/* actual length, encoded as n - 1 */

#define TD_CTRL_ANY_ERROR	(TD_CTRL_STALLED | TD_CTRL_DBUFERR | \
				 TD_CTRL_BABBLE | TD_CTRL_CRCTIME | TD_CTRL_BITSTUFF)

#define uhci_status_bits(ctrl_sts)	(ctrl_sts & 0xFE0000)
#define uhci_actual_length(ctrl_sts)	((ctrl_sts + 1) & TD_CTRL_ACTLEN_MASK)	/* 1-based */
#define uhci_ptr_to_virt(x)	bus_to_virt(x & ~UHCI_PTR_BITS)

/*
 * for TD <flags>:
 */
#define UHCI_TD_REMOVE		0x0001	/* Remove when done */

/*
 * for TD <info>: (a.k.a. Token)
 */
#define TD_TOKEN_TOGGLE		19

#define uhci_maxlen(token)	((token) >> 21)
#define uhci_toggle(token)	(((token) >> TD_TOKEN_TOGGLE) & 1)
#define uhci_endpoint(token)	(((token) >> 15) & 0xf)
#define uhci_devaddr(token)	(((token) >> 8) & 0x7f)
#define uhci_devep(token)	(((token) >> 8) & 0x7ff)
#define uhci_packetid(token)	((token) & 0xff)
#define uhci_packetout(token)	(uhci_packetid(token) != USB_PID_IN)
#define uhci_packetin(token)	(uhci_packetid(token) == USB_PID_IN)

/* ------------------------------------------------------------------------------------
   New TD/QH-structures
   ------------------------------------------------------------------------------------ */
typedef enum {
	TD_TYPE, QH_TYPE
} uhci_desc_type_t;

// uhci transfer descriptor, page 20
typedef struct uhci_td {
	// hardware portion
	vuint32_t flags;
	vuint32_t curr_buf_ptr;
	vuint32_t next_td;
	vuint32_t buf_end;

	// software portion
	struct uhci_td *next;
	struct uhci_td *transfer_head; // points to the first uhci_td in this transfer
	struct uhci_td *transfer_next; // next one in the transfer
	struct uhci_ed *ed; // current endpoint descriptor this is part of

	addr_t phys_addr; // physical address of this descriptor
	bool last_in_transfer;
	usb_hc_transfer *usb_transfer;

	uint32_t pad[1]; // pads it out to a multiple of 32
} uhci_td;

typedef struct uhci_ed uhci_ed;
// a queue of endpoints
typedef struct uhci_queue {
	uhci_ed *head;
	vuint32_t *head_phys;
} uhci_queue;
// uhci endpoint descriptor, page 16
struct uhci_ed {
	// hardware portion
	vuint32_t flags;
	vuint32_t tail;
	vuint32_t head;
	vuint32_t next;

	// software portion
	addr_t phys_addr; // physical address of this endpoint
	struct uhci *oi; // remember what hc it belongs to

	struct uhci_queue *queue; // what queue it's present in
	struct uhci_ed *prev_ed;  // prev & next pointers in queue it's in
	struct uhci_ed *next_ed;

	struct uhci_td *tail_td; // last td in the list, virtual address

	usb_endpoint_descriptor *usb_ed;

	uint32_t pad[1]; // pads it out to a multiple of 32
} ;


typedef struct {
	uint32_t link;
	uint32_t status;
	uint32_t info;
	uint32_t buffer;
} uhci_td_t, *puhci_td_t;

typedef struct {
	uint32_t head;
	uint32_t element;		/* Queue element pointer */
} uhci_qh_t, *puhci_qh_t;

typedef struct {
	union {
		uhci_td_t td;
		uhci_qh_t qh;
	} hw;
	uhci_desc_type_t type;
	uint32_t dma_addr;
	struct list_head horizontal;
	struct list_head vertical;
	struct list_head desc_list;
	int last_used;
	void* pReal;
} uhci_desc_t, *puhci_desc_t;

typedef struct {
	struct list_head desc_list;	// list pointer to all corresponding TDs/QHs associated with this request
	uint32_t setup_packet_dma;
	uint32_t transfer_buffer_dma;
	unsigned long started;
	//USB_packet_s *next_queued_urb;         // next queued urb for this EP
	//USB_packet_s *prev_queued_urb;
	uhci_desc_t *bottom_qh;
	uhci_desc_t *next_qh;       	// next helper QH
	char use_loop;
	char flags;
} urb_priv_t, *purb_priv_t;

struct virt_root_hub {
	int devnum;		/* Address of Root Hub endpoint */
	void *urb;
	void *int_addr;
	int send;
	int interval;
	int numports;
	int c_p_r[8];
	//ktimer_t rh_int_timer;
};

typedef struct uhci {
	int irq;
	unsigned int io_addr;
	unsigned int io_size;
	unsigned int maxports;
	int running;

	int apm_state;

	struct uhci *next;	// chain of uhci device contexts

	struct list_head urb_list;	// list of all pending urbs

	//SpinLock_s urb_list_lock;	// lock to keep consistency 

	int unlink_urb_done;
	//atomic_t avoid_bulk;
	
	struct usb_hc_module_hooks *bus;	// our bus

	uint32_t* framelist;
	void* framelist_real;
	uhci_desc_t **iso_td;
	uhci_desc_t *int_chain[8];
	uhci_desc_t *ls_control_chain;
	uhci_desc_t *control_chain;
	uhci_desc_t *bulk_chain;
	uhci_desc_t *chain_end;
	uhci_desc_t *td1ms;
	uhci_desc_t *td32ms;
	struct list_head free_desc;
	//SpinLock_s qh_lock;
	//SpinLock_s td_lock;
	struct virt_root_hub rh;	//private data of the virtual root hub
	int loop_usage;            // URBs using bandwidth reclamation

	//struct list_head urb_unlinked;	// list of all unlinked  urbs
	long timeout_check;
	int timeout_urbs;
	pci_state_t* uhci_pci;
	uhci_td *td_freelist;
	void* td_freelist_lock;	
	bigtime_t last_error_time;          // last error output in uhci_interrupt()
	int last_frame;
} uhci_t, *puhci_t;


#define MAKE_TD_ADDR(a) ((a)->dma_addr&~UHCI_PTR_QH)
#define MAKE_QH_ADDR(a) ((a)->dma_addr|UHCI_PTR_QH)
#define UHCI_GET_CURRENT_FRAME(uhci) (inw ((uhci)->io_addr + USBFRNUM))

#define CLEAN_TRANSFER_NO_DELETION 0
#define CLEAN_TRANSFER_REGULAR 1
#define CLEAN_TRANSFER_DELETION_MARK 2

#define CLEAN_NOT_FORCED 0
#define CLEAN_FORCED 1

#define PROCESS_ISO_REGULAR 0
#define PROCESS_ISO_FORCE 1

#define UNLINK_ASYNC_STORE_URB 0
#define UNLINK_ASYNC_DONT_STORE 1

#define is_td_active(desc) (desc->hw.td.status & cpu_to_le32(TD_CTRL_ACTIVE))

#define set_qh_head(desc,val) (desc)->hw.qh.head=cpu_to_le32(val)
#define set_qh_element(desc,val) (desc)->hw.qh.element=cpu_to_le32(val)
#define set_td_link(desc,val) (desc)->hw.td.link=cpu_to_le32(val)
#define set_td_ioc(desc) (desc)->hw.td.status |= cpu_to_le32(TD_CTRL_IOC)
#define clr_td_ioc(desc) (desc)->hw.td.status &= cpu_to_le32(~TD_CTRL_IOC)


/* ------------------------------------------------------------------------------------ 
   Virtual Root HUB 
   ------------------------------------------------------------------------------------ */
/* destination of request */
#define RH_INTERFACE               0x01
#define RH_ENDPOINT                0x02
#define RH_OTHER                   0x03

#define RH_CLASS                   0x20
#define RH_VENDOR                  0x40

/* Requests: bRequest << 8 | bmRequestType */
#define RH_GET_STATUS           0x0080
#define RH_CLEAR_FEATURE        0x0100
#define RH_SET_FEATURE          0x0300
#define RH_SET_ADDRESS			0x0500
#define RH_GET_DESCRIPTOR		0x0680
#define RH_SET_DESCRIPTOR       0x0700
#define RH_GET_CONFIGURATION	0x0880
#define RH_SET_CONFIGURATION	0x0900
#define RH_GET_STATE            0x0280
#define RH_GET_INTERFACE        0x0A80
#define RH_SET_INTERFACE        0x0B00
#define RH_SYNC_FRAME           0x0C80
/* Our Vendor Specific Request */
#define RH_SET_EP               0x2000


/* Hub port features */
#define RH_PORT_CONNECTION         0x00
#define RH_PORT_ENABLE             0x01
#define RH_PORT_SUSPEND            0x02
#define RH_PORT_OVER_CURRENT       0x03
#define RH_PORT_RESET              0x04
#define RH_PORT_POWER              0x08
#define RH_PORT_LOW_SPEED          0x09
#define RH_C_PORT_CONNECTION       0x10
#define RH_C_PORT_ENABLE           0x11
#define RH_C_PORT_SUSPEND          0x12
#define RH_C_PORT_OVER_CURRENT     0x13
#define RH_C_PORT_RESET            0x14

/* Hub features */
#define RH_C_HUB_LOCAL_POWER       0x00
#define RH_C_HUB_OVER_CURRENT      0x01

#define RH_DEVICE_REMOTE_WAKEUP    0x00
#define RH_ENDPOINT_STALL          0x01

/* Our Vendor Specific feature */
#define RH_REMOVE_EP               0x00


#define RH_ACK                     0x01
#define RH_REQ_ERR                 -1
#define RH_NACK                    0x00

#endif
