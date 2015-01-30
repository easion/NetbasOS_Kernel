/*
** Copyright 2003, Travis Geiselbrecht. All rights reserved.
** Distributed under the terms of the NewOS License.
*/
 
#include <drv/defs.h>
#include <drv/drv.h>
#include <drv/sym.h>
#include <drv/ia.h>
#include <drv/pci.h>
#include <drv/errno.h>
#include <linux/module.h>
#include <linux/list.h>

#include <i386/isa_io.h>
#include <bus/pci.h>

#include <usb.h>
#include <usb_hc.h>

#include <string.h>



#include "uhci.h"

#define debug_level_flow 6
#define debug_level_error 10
#define debug_level_info 10

#define DEBUG_MSG_PREFIX "OHCI - "

// stop bandwidth reclamation after (roughly) 50ms
#define IDLE_TIMEOUT  (1000/20)

// Suppress HC interrupt error messages for 5s
#define ERROR_SUPPRESSION_TIME (1000*5)

#define phys_to_virt(oi, phys) (((addr_t)(phys) - (oi)->hcca_phys) + (addr_t)(oi)->hcca)

static uhci_t *devs = NULL;

/* used by userspace UHCI data structure dumper */
uhci_t **uhci_devices = &devs;
static pci_pos_t* g_psPCIBus;


#if 1
static uhci_t *oi_list = NULL;

static uhci_td *allocate_td(uhci_t *oi)
{
	uhci_td *td;

	// pull a td from the freelist
	mutex_lock(&oi->td_freelist_lock);
	td = oi->td_freelist;
	if(td)
		oi->td_freelist = td->next;
	mutex_unlock(&oi->td_freelist_lock);

	if(!td)
		return NULL;

	td->flags = 0;
	td->curr_buf_ptr = 0;
	td->next_td = 0;
	td->buf_end = 0;

	return td;
}

static void free_td(uhci_t *oi, uhci_td *td)
{
	mutex_lock(&oi->td_freelist_lock);
	td->next = oi->td_freelist;
	oi->td_freelist = td;
	mutex_unlock(&oi->td_freelist_lock);
}

static uhci_ed *create_ed(void)
{
	uhci_ed *ed;

	ed = KMALLOC(sizeof(uhci_ed));
	if(!ed)
		return NULL;

	memset(ed, 0, sizeof(uhci_ed));

	//vm_get_page_mapping(vm_get_kernel_aspace_id(), (addr_t)ed, &ed->phys_addr);

	return ed;
}

static void enqueue_ed(uhci_ed *ed)
{
	uhci_queue *queue = ed->queue;
	uhci_t *oi = ed->oi;

	//mutex_lock(&oi->hc_list_lock);

	// stick it in our queue
	ed->prev_ed = NULL;
	ed->next_ed = queue->head;
	if(ed->next_ed != NULL)
		ed->next_ed->prev_ed = ed;
	queue->head = ed;

	// put it in the hardware's queue
	ed->next = *queue->head_phys;
	*queue->head_phys = (uint32_t)ed->phys_addr;

	//mutex_unlock(&oi->hc_list_lock);
}
#endif

static uhci_ed *create_endpoint(uhci_t *oi, usb_endpoint_descriptor *endpoint, int address, bool lowspeed)
{
#if 0

	uhci_ed *ed;
	uhci_td *td;

	ed = create_ed();
	if(!ed)
		return ed;

	// save a pointer to the original usb endpoint structure
	ed->usb_ed = endpoint;
	ed->oi = oi;

	// figure out what queue it should be in
	switch(endpoint->attributes & 0x3) {
		case USB_ENDPOINT_ATTR_CONTROL:
			ed->queue = &oi->control_queue;
			break;
		case USB_ENDPOINT_ATTR_ISO:
			// not supported
			break;
		case USB_ENDPOINT_ATTR_BULK:
			ed->queue = &oi->bulk_queue;
			break;
		case USB_ENDPOINT_ATTR_INT:
			//ed->queue = &oi->interrupt_queue[INT_Q_32MS]; // XXX find the correct queue and rotate through them
			break;
	}

	td = allocate_td(oi);

	// set some hardware flags based on the usb endpoint's data
	ed->flags = (endpoint->max_packet_size << 16) | ((endpoint->endpoint_address & 0xf) << 7) | (address & 0x7f);

	if((endpoint->attributes & 0x3) == USB_ENDPOINT_ATTR_CONTROL) {
		//ed->flags |= OHCI_ED_FLAGS_DIR_TD;
	} else {
		//if(endpoint->endpoint_address & 0x80)
		//	ed->flags |= OHCI_ED_FLAGS_DIR_IN;
		//else
		//	ed->flags |= OHCI_ED_FLAGS_DIR_OUT;
	}
	//if(lowspeed)
		//ed->flags |= OHCI_ED_FLAGS_SPEED;

	// stick the null transfer descriptor in our endpoint descriptors list
	ed->tail_td = td;
	ed->tail = ed->head = td->phys_addr;
	ed->prev_ed = ed->next_ed = NULL;

	// enqueue this descriptor
	enqueue_ed(ed);

	return ed;
#endif
}


static int uhci_create_endpoint(hc_cookie *cookie, hc_endpoint **hc_endpoint,
			usb_endpoint_descriptor *usb_endpoint, int address, bool lowspeed)
{
#if 1
	uhci_t *oi = (uhci_t *)cookie;

	*hc_endpoint = create_endpoint(oi, usb_endpoint, address, lowspeed);
	if(*hc_endpoint == NULL)
		return -1;
#endif
	return 0;
}


static int uhci_enqueue_transfer(hc_cookie *cookie, hc_endpoint *endpoint, usb_hc_transfer *transfer)
{
#if 0
	uhci_t *oi = (uhci_t *)cookie;
	uhci_ed *ed = (uhci_ed *)endpoint;
	usb_endpoint_descriptor *usb_ed = ed->usb_ed;

	switch(usb_ed->attributes & 0x3) {
		case USB_ENDPOINT_ATTR_CONTROL: {
			uhci_td *td0, *td1, *td2, *td3;
			int dir;

			// the direction of the transfer is based off of the first byte in the setup data
			dir = *((uint8 *)transfer->setup_buf) & 0x80 ? 1 : 0;

			mutex_lock(&oi->hc_list_lock);

			SHOW_FLOW(6, "head 0x%x, tail 0x%x", ed->head, ed->tail);

			// allocate the transfer descriptors needed
			td0 = ed->tail_td;
			td2 = allocate_td(oi);
			if(transfer->data_buf != NULL)
				td1 = allocate_td(oi);
			else
				td1 = td2;
			td3 = allocate_td(oi); // this will be the new null descriptor

			// setup descriptor
			//td0->flags = OHCI_TD_FLAGS_CC_NOT | OHCI_TD_FLAGS_DIR_SETUP | OHCI_TD_FLAGS_IRQ_NONE | OHCI_TD_FLAGS_TOGGLE_0;
			//vm_get_page_mapping(vm_get_kernel_aspace_id(), (addr_t)transfer->setup_buf, (addr_t *)&td0->curr_buf_ptr);
			td0->buf_end = td0->curr_buf_ptr + transfer->setup_len - 1;
			td0->next_td = td1->phys_addr; // might be td2
			td0->transfer_head = td0;
			td0->transfer_next = td1;
			td0->usb_transfer = transfer;
			td0->ed = ed;
			td0->last_in_transfer = false;

			SHOW_FLOW(6, "td0 %p (0x%lx) flags 0x%x, curr_buf_ptr 0x%x, buf_end 0x%x, next_td 0x%x",
				td0, td0->phys_addr, td0->flags, td0->curr_buf_ptr, td0->buf_end, td0->next_td);

			if(transfer->data_buf != NULL) {
				// data descriptor
				

				SHOW_FLOW(6, "td1 %p (0x%lx) flags 0x%x, curr_buf_ptr 0x%x, buf_end 0x%x, next_td 0x%x",
					td1, td1->phys_addr, td1->flags, td1->curr_buf_ptr, td1->buf_end, td1->next_td);
			}

			// ack descriptor
			//td2->flags = OHCI_TD_FLAGS_CC_NOT | OHCI_TD_FLAGS_TOGGLE_1 |
			//	(dir ? OHCI_TD_FLAGS_DIR_OUT : OHCI_TD_FLAGS_DIR_IN);
			td2->curr_buf_ptr = 0;
			td2->buf_end = 0;
			td2->next_td = td3->phys_addr;
			td2->transfer_head = td0;
			td2->transfer_next = NULL;
			td2->usb_transfer = transfer;
			td2->ed = ed;
			td2->last_in_transfer = true;

			SHOW_FLOW(6, "td2 %p (0x%lx) flags 0x%x, curr_buf_ptr 0x%x, buf_end 0x%x, next_td 0x%x",
				td2, td2->phys_addr, td2->flags, td2->curr_buf_ptr, td2->buf_end, td2->next_td);

			// next null descriptor is td3, it should be nulled out from allocate_td()
			td3->ed = NULL;
			td3->usb_transfer = NULL;
			td3->transfer_head = td3->transfer_next = NULL;

			SHOW_FLOW(6, "td3 %p (0x%lx) flags 0x%x, curr_buf_ptr 0x%x, buf_end 0x%x, next_td 0x%x",
				td3, td3->phys_addr, td3->flags, td3->curr_buf_ptr, td3->buf_end, td3->next_td);

			ed->tail_td = td3;
			ed->tail = td3->phys_addr;

			oi->regs->command_status = COMMAND_CLF;

			mutex_unlock(&oi->hc_list_lock);
			break;
		}
		case USB_ENDPOINT_ATTR_INT: {
			uhci_td *td0, *td1;

			mutex_lock(&oi->hc_list_lock);

			td0 = ed->tail_td;
			td1 = allocate_td(oi);

			// XXX only deals with non page boundary data transfers
			//td0->flags = OHCI_TD_FLAGS_CC_NOT | OHCI_TD_FLAGS_ROUNDING;
			//vm_get_page_mapping(vm_get_kernel_aspace_id(), (addr_t)transfer->data_buf, (addr_t *)&td0->curr_buf_ptr);
			td0->buf_end = td0->curr_buf_ptr + transfer->data_len - 1;
			td0->next_td = td1->phys_addr;
			td0->transfer_head = td0;
			td0->transfer_next = td1;
			td0->usb_transfer = transfer;
			td0->ed = ed;
			td0->last_in_transfer = true;

			SHOW_FLOW(6, "td0 %p (0x%lx) flags 0x%x, curr_buf_ptr 0x%x, buf_end 0x%x, next_td 0x%x",
				td0, td0->phys_addr, td0->flags, td0->curr_buf_ptr, td0->buf_end, td0->next_td);

			// new tail
			td1->ed = NULL;
			td1->usb_transfer = NULL;
			td1->transfer_head = td1->transfer_next = NULL;

			SHOW_FLOW(6, "td1 %p (0x%lx) flags 0x%x, curr_buf_ptr 0x%x, buf_end 0x%x, next_td 0x%x",
				td1, td1->phys_addr, td1->flags, td1->curr_buf_ptr, td1->buf_end, td1->next_td);

			ed->tail_td = td1;
			ed->tail = td1->phys_addr;

			//oi->regs->command_status = COMMAND_BLF;

			//mutex_unlock(&oi->hc_list_lock);
			break;
		}
		default:
			SHOW_ERROR(0, "unsupported transfer type %d", ed->usb_ed->attributes & 0x3);
			return -2;
	}
#endif
	return 0;
}





static int uhci_init(int (*hc_init_callback)(void *callback_cookie, void *cookie), void *callback_cookie)
{
	int i;
	pci_state_t pinfo;
	uhci_t *oi;
	int count = 0;

	uhci_hcd_init();
	hc_init_callback(callback_cookie, oi);

#if 0
	pci_module_hooks *pci;
	if(module_get(PCI_BUS_MODULE_NAME, 0, (void **)&pci) < 0) {
		SHOW_ERROR0(0, "uhci_detect: no pci bus found..");
		return -1;
	}

	for(i = 0; pci->get_nth_pci_info(i, &pinfo) >= 0; i++) {
		if(pinfo.class_base == OHCI_BASE_CLASS &&
		   pinfo.class_sub == OHCI_SUB_CLASS &&
		   pinfo.class_api == OHCI_INTERFACE) {

			{
				int j;

				for(j=0; j < 6; j++) {
					dprintf(" %d: base 0x%x size 0x%x\n", j, pinfo.u.h0.base_registers[j], pinfo.u.h0.base_register_sizes[j]);
				}
			}

			oi = uhci_init_hc(&pinfo);
			if(oi) {
				// add it to our list of uhci_t hcfs
				oi->next = oi_list;
				oi_list = oi;
				count++;

				// register it with the bus
				hc_init_callback(callback_cookie, oi);
			}
		}
	}
#endif
	//module_put(PCI_BUS_MODULE_NAME);

	return count;
}




static int
uhci_module_init(void)
{
	return 0;
}

static int
uhci_module_uninit(void)
{
	return 0;
}

int uhci_interrupt(int irq, void *dev_id )
{
	uhci_t *s = dev_id;
	unsigned int io_addr = s->io_addr;
	unsigned short status;
	struct list_head *p, *p2;
	int restarts, work_done;
	
	/*
	 * Read the interrupt status, and write it back to clear the
	 * interrupt cause
	 */

	status = inw (io_addr + USBSTS);

	if (!status)		/* shared interrupt, not mine */
		return( 0 );

	dbg("interrupt\n");

	if (status != 1) {
		// Avoid too much error messages at a time
		if ((get_system_time() - s->last_error_time > ERROR_SUPPRESSION_TIME*1000)) {
			dbg("interrupt, status %x, frame# %i\n", status, 
			     UHCI_GET_CURRENT_FRAME(s));
			s->last_error_time = get_system_time();
		}
		
		// remove host controller halted state
		if ((status&0x20) && (s->running)) {
			printk("UHCI: Host controller halted, trying to restart.\n");
			outw (USBCMD_RS | inw(io_addr + USBCMD), io_addr + USBCMD);
		}
		//uhci_show_status (s);
	}
	/*
	 * traverse the list in *reverse* direction, because new entries
	 * may be added at the end.
	 * also, because process_urb may unlink the current urb,
	 * we need to advance the list before
	 * New: check for max. workload and restart count
	 */

	//spinlock (&s->urb_list_lock);

	restarts=0;
	work_done=0;

restart:
	s->unlink_urb_done=0;
	p = s->urb_list.prev;	

	while (p != &s->urb_list && (work_done < 1024)) {
		p2 = p;
		p = p->prev;

		process_urb (s, p2);
		
		work_done++;

		if (s->unlink_urb_done) {
			s->unlink_urb_done=0;
			restarts++;
			
			if (restarts<16)	// avoid endless restarts
				goto restart;
			else 
				break;
		}
	}
	if ((get_system_time() - s->timeout_check) > (1000/30)*1000) 
		uhci_check_timeouts(s);

	clean_descs(s, CLEAN_NOT_FORCED);
	uhci_cleanup_unlink(s, CLEAN_NOT_FORCED);
	uhci_switch_timer_int(s);
							
	//spinunlock (&s->urb_list_lock);
	
	outw (status, io_addr + USBSTS);

	dbg("uhci_interrupt: done\n");
	
	return( 0 );
}

inline uint32_t pci_size(uint32_t base, uint32_t mask)
{
	uint32_t size = base & mask;
	return size & ~(size-1);
}

static uint32_t get_pci_memory_size(const pci_state_t* pcPCIInfo, int nResource)
{
	uint32_t nSize, nBase;
	int nOffset = PCI_BASE_REGISTERS + nResource*4;

	g_psPCIBus->read_pci_config32(pcPCIInfo, nOffset, &nBase);
	g_psPCIBus->write_pci_config32(pcPCIInfo, nOffset,  ~0);

	g_psPCIBus->read_pci_config32(pcPCIInfo, nOffset, &nSize);
	g_psPCIBus->write_pci_config32(pcPCIInfo, nOffset,  nBase);

	if (!nSize || nSize == 0xffffffff) return 0;
	if (nBase == 0xffffffff) nBase = 0;

	if (!(nSize & PCI_ADDRESS_SPACE)) {
		return pci_size(nSize, PCI_ADDRESS_MEMORY_32_MASK);
	} else {
		return pci_size(nSize, PCI_ADDRESS_IO_MASK & 0xffff);
	}
}


static int uhci_pci_probe (pci_state_t* dev )
{
	int i;
	unsigned short cmd_stat;

	if (!dev->irq) {
		printk("UHCI: found UHCI device with no IRQ assigned. check BIOS settings!\n");
		return -ENODEV;
	}
	
	g_psPCIBus->read_pci_config16( dev, PCI_COMMAND, &cmd_stat );
	/* Enable busmaster */
	g_psPCIBus->write_pci_config16( dev, PCI_COMMAND, 
		cmd_stat | PCI_COMMAND_IO | PCI_COMMAND_MASTER );

	/* Search for the IO base address.. */
	for (i = 0; i < 6; i++) {

		unsigned int io_addr =dev->base[i]; // *( &dev.u.h0.nBase0 + i );
		unsigned int io_size = get_pci_memory_size( dev, i );
		
		//if( !( io_addr & PCI_ADDRESS_SPACE ) ) {
		if(  !io_addr  ) {
			continue;
		}

		/* disable legacy emulation */
		g_psPCIBus->write_pci_config16( dev, USBLEGSUP,  0 );

		printk("alloc_uhci @%x-%d\n", io_addr, io_size);
	
		return alloc_uhci(  dev, dev->irq, io_addr & PCI_ADDRESS_IO_MASK, io_size);
	}
	printk("uhci: nodev\n");
	return -ENODEV;
}




static int
uhci_uninit(void *cookie)
{
	// XXX finish
	return -2;
}

static int uhci_destroy_endpoint(hc_cookie *cookie, hc_endpoint *hc_endpoint)
{
	// XXX implement

	return 0;
}


static struct usb_hc_module_hooks uhci_hooks = {
	&uhci_init, // bus initialization
	&uhci_uninit,
	&uhci_create_endpoint,
	&uhci_destroy_endpoint,
	&uhci_enqueue_transfer,
};
static int alloc_uhci (  pci_state_t* dev, int irq, unsigned int io_addr, unsigned int io_size)
{
	uhci_t *s;
	struct usb_hc_module_hooks *bus;
	char buf[8], *bufp = buf;

	sprintf(buf, "%d", irq);

	printk("USB UHCI at I/O 0x%x, IRQ %s\n",
		io_addr, bufp);

	s = kmalloc (sizeof (uhci_t), MEMF_KERNEL);
	if (!s)
		return -1;

	memset (s, 0, sizeof (uhci_t));
	//INIT_LIST_HEAD (&s->free_desc);
	//INIT_LIST_HEAD (&s->urb_list);
	//INIT_LIST_HEAD (&s->urb_unlinked);
	//spinlock_init (&s->urb_list_lock, "urb_list_lock");
	//spinlock_init (&s->qh_lock, "qh_lock");
	//spinlock_init (&s->td_lock, "td_lock");
	//atomic_set(&s->avoid_bulk, 0);
	s->irq = -1;
	s->io_addr = io_addr;
	s->io_size = io_size;
	s->uhci_pci=dev;
	
	bus = usbbus_alloc_bus();
	if (!bus) {
		kfree (s);
		return -1;
	}

	s->bus = bus;
	//bus->AddDevice = uhci_alloc_dev;
	//bus->RemoveDevice = uhci_free_dev;
	//bus->GetFrameNumber = uhci_get_current_frame_number;
	//bus->SubmitPacket = uhci_submit_urb;
	//bus->CancelPacket = uhci_unlink_urb;
	bus->pHCPrivate = s;
	

	/* UHCI specs says devices must have 2 ports, but goes on to say */
	/* they may have more but give no way to determine how many they */
	/* have, so default to 2 */
	/* According to the UHCI spec, Bit 7 is always set to 1. So we try */
	/* to use this to our advantage */

	for (s->maxports = 0; s->maxports < (io_size - 0x10) / 2; s->maxports++) {
		unsigned int portstatus;

		portstatus = inw (io_addr + 0x10 + (s->maxports * 2));
		dbg("port %i, adr %x status %x\n", s->maxports,
			io_addr + 0x10 + (s->maxports * 2), portstatus);
		if (!(portstatus & 0x0080))
			break;
	}
	dbg("Detected %d ports\n", s->maxports);

	/* This is experimental so anything less than 2 or greater than 8 is */
	/*  something weird and we'll ignore it */
	if (s->maxports < 2 || s->maxports > 8) {
		printk("UHCI: Port count misdetected, forcing to 2 ports\n");
		s->maxports = 2;
	}
	
	
	s->rh.numports = s->maxports;
	s->loop_usage=0;
	if (init_skel (s)) {
		usbbus_free_bus (bus);
		kfree(s);
		return -1;
	}

	reset_hc (s);
	usbbus_add_bus( s->bus );

	start_hc (s);

	if (request_irq (dev->irq, uhci_interrupt, NULL, SA_SHIRQ, "usb_uhci", s) < 0) {
		printk("UHCI: request_irq %d failed!\n",irq);
		usbbus_free_bus (bus);
		reset_hc (s);
		cleanup_skel(s);
		kfree(s);
		return -1;
	}

	/* Enable PIRQ */
	g_psPCIBus->write_pci_config16( dev, USBLEGSUP, USBLEGSUP_DEFAULT );

	s->irq = irq;

	if(uhci_start_usb (s) < 0) {
		return -1;
	}
	
	/* Claim device */
	//if( claim_device( nDeviceID, dev.nHandle, "USB UHCI controller", DEVICE_CONTROLLER ) != 0 )
	//	return( -1 );
		
	//set_device_data( dev.nHandle, s );

	//chain new uhci device into global list
	devs=s;

	return 0;
}

static pci_state_t * usb_dev_probe()
{
	int i;
    pci_state_t *usb_device = NULL; 
   usb_device = g_psPCIBus->pci_dev_lookup(PCI_SERIAL_BUS, PCI_USB);
   if(usb_device)  g_psPCIBus->pci_dump(usb_device);	
    return usb_device;
}

static int uhci_hcd_init( )
{
	int i;
	pci_state_t* pci;
	bool found = false;
	struct bus_man *bus;
	
	/* Get busmanagers */
	bus =  busman_get( "PCI32" );

	if (!bus)
	{
		printk("PCI32 BUS Not AVAILABLE\n");
		return -1;
	}
	g_psPCIBus = bus->bus_hooks(0);
	if( g_psPCIBus == NULL )
	{
		return( -1 );
	}
	

	pci = usb_dev_probe();
	if (!pci)
	{
		printk("No USB UHCI controller found\n\n");
	}

	printk("uhci_pci_probe");

	uhci_pci_probe(pci);
#if 0
	
	/* scan all PCI devices */
    for(i = 0 ; g_psPCIBus->get_pci_info( &pci, i ) == 0 ; ++i) {
    	if( pci.nClassApi == 0 && pci.nClassBase == PCI_SERIAL_BUS && pci.nClassSub == PCI_USB )
    	{
    		if( uhci_pci_probe( nDeviceID, pci ) == 0 )
    			found = true;
        }
        
    }
   	if( !found ) {
   		kerndbg( KERN_DEBUG, "No USB UHCI controller found\n" );
   		disable_device( nDeviceID );
   		return( -1 );
   	}
#endif
	return( 0 );
}





#if 0
static module_header uhci_module_header = {
	USB_HC_MODULE_NAME_PREFIX "/uhci_t/v1",
	MODULE_CURR_VERSION,
	0, // dont keep loaded
	&uhci_hooks,
	&uhci_module_init,
	&uhci_module_uninit
};

module_header *modules[]  = {
	&uhci_module_header,
	NULL
};
#endif

