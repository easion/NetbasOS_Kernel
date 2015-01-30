
#ifndef _SYLLABLE_MODULE_H
#define _SYLLABLE_MODULE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <drv/timer.h>
#include <net/pkt.h>

#define INFINITE_TIMEOUT		(9223372036854775807LL)
enum device_type {
	DEVICE_UNKNOWN,
	DEVICE_SYSTEM,
	DEVICE_CONTROLLER,
	DEVICE_VIDEO,
	DEVICE_AUDIO,
	DEVICE_NET,
	DEVICE_PORT,
	DEVICE_INPUT,
	DEVICE_DRIVE,
	DEVICE_PROCESSOR
};
/*enum debug_level{
	KERN_DEBUG_LOW,
	KERN_DEBUG,
	KERN_INFO,
	KERN_WARNING,
	KERN_FATAL,
	KERN_PANIC
};
*/
#ifndef DEBUG_LIMIT
#define DEBUG_LIMIT	KERN_INFO	/* Default debug level */
#endif

typedef long long int64;
typedef long  int32;
typedef short  int16;
typedef char  int8;
typedef unsigned int  uint;

typedef unsigned long long uint64;
typedef unsigned long  uint32;
typedef unsigned short  uint16;
typedef unsigned char  uint8;
typedef unsigned short  kdev_t;
typedef void *sem_id;
typedef  long bigtime_t;
typedef uint8 wchar_t;

typedef pci_state_t* PCI_Info_s;

typedef  int status_t;
typedef  void* SysCallRegs_s;
typedef  spinlock_t SpinLock_s;

#define kerndbg(level, format, argc...) kprintf(format, ##argc)
#define get_system_time() startup_ticks()
#define spinunlock spin_unlock
#define spinlock spin_lock


static inline void		release_device( int nHandle )
{
}
static inline void		release_devices( int nDeviceID )
{
}

static inline int			register_device( const char* pzName, const char* pzBus )
{
	return 0;
}

static inline void		unregister_device( int nHandle )
{
	return ;
}

#define spinlock spin_lock
#define spinunlock spin_unlock

//#define wakeup_sem(X,Y) up(X)
//#define sleep_on_sem(X,Y) down((X),(Y))

static inline void	LOCK(void* sem)
{
	down(sem,1000);
}


inline static status_t	claim_device( int nDeviceID, 
int nHandle, const char* pzName, int eType )
{
	return 0;
}

//#define	snooze(N) thread_wait(current_thread(), N)
#define	TRY_LOCK LOCK
#define UNLOCK( sem )	up( (sem) )
typedef void* ktimer_t;
typedef void timer_callback( void* pData );

static inline ktimer_t create_timer( void )
{
	return (void*)kmalloc(sizeof(krnl_timer_t),0);
}

static inline void start_timer( ktimer_t hTimer, timer_callback* pfCallback, 
void* pData, bigtime_t nPeriode, bool bOneShot )
{
	init_timer(hTimer, pfCallback, pData);
	install_timer(hTimer,nPeriode);
}

static inline void delete_timer( ktimer_t hTimer )
{
	remove_timer(hTimer);
	kfree(hTimer);
}

struct net_device
{

	/*
	 * This is the first field of the "visible" part of this structure
	 * (i.e. as seen by users in the "Space.c" file).  It is the name
	 * the interface.
	 */
	char			*name;

	/*
	 *	I/O specific fields
	 *	FIXME: Merge these and struct ifmap into one
	 */
	unsigned long		base_addr;	/* device I/O address	*/
	unsigned int		irq;		/* device IRQ number	*/
	
	/* Low-level status flags. */
	volatile unsigned char	start;		/* start an operation	*/
	/*
	 * These two are just single-bit flags, but due to atomicity
	 * reasons they have to be inside a "unsigned long". However,
	 * they should be inside the SAME unsigned long instead of
	 * this wasteful use of memory..
	 */
	unsigned long		interrupt;	/* bitops.. */
	unsigned long		tbusy;		/* transmitter busy */
	
	struct device		*next;
	
	/*
	 * This marks the end of the "visible" part of the structure. All
	 * fields hereafter are internal to the system, and may change at
	 * will (read: may be cleaned up at will).
	 */

	/* These may be needed for future network-power-down code. */
	unsigned long		trans_start;	/* Time (in jiffies) of last Tx	*/
	
	unsigned short		flags;	/* interface flags (a la BSD)	*/
	void			*priv;	/* pointer to private data	*/
	
	/* Interface address info. */
//	unsigned char		broadcast[MAX_ADDR_LEN];	/* hw bcast add	*/
	unsigned char		pad;		/* make dev_addr aligned to 8 bytes */
	unsigned char		dev_addr[MAX_ADDR_LEN];	/* hw address	*/
//	unsigned char		addr_len;	/* hardware address length	*/

	struct dev_mc_list	*mc_list;	/* Multicast mac addresses	*/
	int			mc_count;	/* Number of installed mcasts	*/
//	int			promiscuity;
//	int			allmulti;
    
	/* For load balancing driver pair support */
  

	//packetqueue_t*   packet_queue;
	volatile bool run_timer;
	int	  timer_thread;
	int irq_handle; /* IRQ handler handle */
};


#ifdef __cplusplus
}
#endif
#endif /* _SYLLABLE_MODULE_H */
