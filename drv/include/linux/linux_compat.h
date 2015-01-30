
#ifndef	__F_SYLLABLE_LINUX_COMPAT_H__
#define	__F_SYLLABLE_LINUX_COMPAT_H__


#ifdef __cplusplus
extern "C" {
#endif

/* It would be better if these could be mapped to kerndbg macros */
#ifndef NO_DEBUG_STUBS
# ifndef KERN_DEBUG
#  define KERN_DEBUG
# endif
# ifndef KERN_INFO
#  define KERN_INFO
# endif
# ifndef KERN_ERR
#  define KERN_ERR
# endif
# ifndef KERN_WARNING
#  define KERN_WARNING
# endif
# ifndef KERN_NOTICE
#  define KERN_NOTICE
# endif
# ifndef KERN_ALERT
#  define KERN_ALERT
# endif
# ifndef KERN_CRIT
#  define KERN_CRIT
# endif
#endif

#include <net/pkt.h>
#include <drv/pci.h>
static inline int wait_event_interruptible_timeout(
	wait_queue_head_t *queue, int done, int args)
{
	return thread_sleep_on(queue);
}

static inline void wake_up_interruptible(wait_queue_head_t *queue)
{
	thread_wakeup(queue);
}

static inline void init_waitqueue_head(wait_queue_head_t *queue)
{
	thread_waitq_init(queue);
}

static inline void * ioremap( unsigned long phys_addr, unsigned long size )
{
	void* hArea;
	void *pAddr = NULL;

	//hArea = create_area( "mmio", &pAddr, size, size, AREA_KERNEL|AREA_ANY_ADDRESS|AREA_FULL_ACCESS, AREA_NO_LOCK );
	//remap_area( hArea, (void *) ( phys_addr & PCI_ADDRESS_IO_MASK ) );

	return pAddr;
}



/* Are these valid? */
static inline void * request_region(unsigned long start, unsigned long len, char *name)
{
	return (void *)1;
}

static inline void release_region(unsigned long start, unsigned long len)
{
	/* EMPTY */
}


#if 0

/* PCI devices */
static inline void* pci_alloc_consistent( PCI_Info_s *psDev, size_t nSize, dma_addr_t *hDma )
{
	void *p = kmalloc( nSize, MEMF_KERNEL );
	*hDma = (uint32)p;
	return p;
}

#define pci_free_consistent( cookie, size, ptr, dma_ptr ) \
	kfree( ptr )
#define pci_map_single( cookie, address, size, dir ) \
	virt_to_bus( address )
#define pci_unmap_single( cookie, address, size, dir ) \
	{/* EMPTY */}

/* psBus must be available & be a valid PCI bus instance */
#define pci_read_config_byte(dev, address, val) \
	val = psBus->read_pci_config((dev)->nBus, (dev)->nDevice, (dev)->nFunction, address, sizeof(val))
#define pci_read_config_word(dev, address, val) \
	val = psBus->read_pci_config((dev)->nBus, (dev)->nDevice, (dev)->nFunction, address, sizeof(val))
#define pci_read_config_dword(dev, address, val) \
	val = psBus->read_pci_config((dev)->nBus, (dev)->nDevice, (dev)->nFunction, address, sizeof(val))
#define pci_write_config_byte(dev, address, val) \
	psBus->write_pci_config((dev)->nBus, (dev)->nDevice, (dev)->nFunction, address, sizeof(val), (uint32)val)
#define pci_write_config_word(dev, address, val) \
	psBus->write_pci_config((dev)->nBus, (dev)->nDevice, (dev)->nFunction, address, sizeof(val), (uint32)val)
#define pci_write_config_dword(dev, address, val) \
	psBus->write_pci_config((dev)->nBus, (dev)->nDevice, (dev)->nFunction, address, sizeof(val), (uint32)val)

#define pci_set_master(dev) \
	psBus->enable_pci_master((dev)->nBus, (dev)->nDevice, (dev)->nFunction)

#define pci_find_capability(dev, cap) \
	psBus->get_pci_capability((dev)->nBus, (dev)->nDevice, (dev)->nFunction, cap)

static inline PCI_Info_s * pci_find_device( int vendor_id, int device_id, PCI_Info_s * pci_dev )
{
	int i;
	static PCI_Info_s  sInfo;
	PCI_bus_s *psBus = get_busmanager( PCI_BUS_NAME, PCI_BUS_VERSION );
	if( psBus == NULL )
		return NULL;

	for( i = 0 ; psBus->get_pci_info( &sInfo, i ) == 0 ; ++i )
		if ( sInfo.nVendorID == vendor_id && sInfo.nDeviceID == device_id)
			return &sInfo;

	return NULL;
}
#define PCI_ANY_ID (~0)
#define PCIBIOS_MAX_LATENCY 255
#define PCI_CLASS_REVISION 0x08

struct pci_device_id
{
	unsigned int vendor_id, device_id;
	unsigned int subvendor, subdevice;
	unsigned int class, class_mask;
	unsigned long driver_data;
};

#define	disable_irq	disable_irq_nosync
#endif

/* 
static inline packet_t* alloc_pkt_buffer(int nSize)
	{
	return new_pkt2(nSize,NULL,NULL);
	}
//Net 

static inline packet_t* alloc_pkt_buffer_aligned(int nSize)
{
	packet_t *psBuffer;

	psBuffer = alloc_pkt_buffer( nSize+15 );
	if( psBuffer )
		psBuffer->data = ALIGN16( psBuffer->data );

	return psBuffer;
}
*/

static inline pci_state_t * pci_find_device( int vendor_id, int device_id )
{
	int i;
	//static PCI_Info_s  sInfo;
	struct bus_man *bus;
	
	/* Get busmanagers */
	bus = busman_get( "PCI32" );

	if (!bus)
	{
		printk("PCI32 BUS Not AVAILABLE\n");
		return -1;
	}
	//g_psBus = bus->bus_hooks(0);

	//for( i = 0 ; psBus->get_pci_info( &sInfo, i ) == 0 ; ++i )
		//if ( sInfo.nVendorID == vendor_id && sInfo.nDeviceID == device_id)
			//return &sInfo;

	return NULL;
}

#define IFHWADDRLEN 6
#define MAX_ADDR_LEN IFHWADDRLEN

#define IFF_RUNNING 0x40

/* The following macros assume that dev has the fields tbusy, start & flags */
#define netif_wake_queue(dev) \
	do { clear_bit(0, (void*)&dev->tbusy); } while(0)
#define netif_start_queue(dev) \
	clear_bit(0, (void*)&dev->tbusy)
#define netif_start_tx_queue(dev) \
	do { (dev)->tbusy = 0; dev->start = 1; } while(0)
#define netif_stop_tx_queue(dev) \
	do { (dev)->tbusy = 1; dev->start = 0; } while(0)
#define netif_stop_queue(dev) \
	set_bit(0, (void*)&dev->tbusy)
#define netif_queue_paused(dev) \
	((dev)->tbusy != 0)
#define netif_pause_tx_queue(dev) \
	(test_and_set_bit(0, (void*)&dev->tbusy))
#define netif_unpause_tx_queue(dev) \
	do { clear_bit(0, (void*)&dev->tbusy); } while(0)
#define netif_resume_tx_queue(dev) \
	do { clear_bit(0, (void*)&dev->tbusy); } while(0)
#define netif_running(dev) \
	((dev)->start != 0)
#define netif_mark_up(dev) \
	do { (dev)->start = 1; } while (0)
#define netif_mark_down(dev) \
	do { (dev)->start = 0; } while (0)
#define netif_queue_stopped(dev) \
	((dev)->tbusy)
#define netif_link_down(dev) \
	(dev)->flags &= ~ IFF_RUNNING
#define netif_link_up(dev) \
	(dev)->flags |= IFF_RUNNING
#define netif_carrier_on(dev) \
	netif_link_up(dev)
#define netif_carrier_off(dev) \
	netif_link_down(dev)
#define netif_carrier_ok(dev) \
	((dev)->flags & IFF_RUNNING)

enum{
	SA_SHIRQ,
};




/** Intel x86 has 4K pages */
#define PAGE_SHIFT	12
#define PAGE_SIZE	(1UL << PAGE_SHIFT)
#define PAGE_MASK	(~(PAGE_SIZE-1))

/* to align the pointer to the (next) page boundary */
#define PAGE_ALIGN(addr)	(((addr)+PAGE_SIZE-1)&PAGE_MASK)
#define PAGE_NR(addr)		(((unsigned long)(addr)) >> PAGE_SHIFT)

static inline int request_irq( int nIrqNum, void(* pTopHandler)(), void* pBottomHandler,
		 u32 nFlags, const char* pzDevName, void* pData )
{
	return put_irq_handler(nIrqNum, (u32_t)pTopHandler, pData,pzDevName);
}

static inline int release_irq( int nIrqNum, void *nHandle )
{
	return free_irq_handler(nIrqNum,nHandle);
}

static inline void udelay(unsigned long usec)
{
	thread_wait(current_thread(), usec/1000);
}

#include <drv/spin.h>
//查找线程
#define find_task_by_pid find_thread_byid 
//查找进程
//#define find_task_by_pid find_proc_bypid
static inline int copy_to_user(char *a,char *b, int c) 
{
	if (!a||!b){
		return -1;
	}
	memcpy(a,b,c);
	return 0;
}

#ifdef __cplusplus
}
#endif

#endif	/* __F_SYLLABLE_LINUX_COMPAT_H__ */

