#ifndef _USB_FREEBSD_TO_NETBAS_H_
#define	_USB_FREEBSD_TO_NETBAS_H_


#include <drv/defs.h>
#include <drv/drv.h>
#include <drv/sym.h>
#include <drv/ia.h>
#include <drv/pci.h>
#include <drv/errno.h>
#include <drv/timer.h>
#include <drv/spin.h>
#include <drv/console.h>
//#include <linux/module.h>
//#include <linux/list.h>

//#include <linux/syllable.h>
#include <i386/isa_io.h>
#include <bus/pci.h>
#include <assert.h>
//#include <bus/usb_defs.h>
//#include <bus/usb.h>
#include "hcd/bsd_pci.h"
#include <sys/queue.h>
#include <sys/compat_bsd.h>
#include <sys/endian.h>

#define bus_space_barrier(t, h, o, l, f)        \
        ((void)((void)(t), (void)(h), (void)(o), (void)(l), (void)(f)))

struct mtx {
        int     state;
		spinlock_t lock;
		unsigned cpu_flag;
} ;

#define mtx_lock(mp)   if (((struct mtx*)mp)->state != 0)   {\
		   kprintf("%s Line%d mtx_lock (mutex already locked)\n",__FUNCTION__,__LINE__);\
	   }\
        ((struct mtx*)mp)->state = 1;


#define mtx_unlock(mp) {\
	   if (((struct mtx*)mp)->state != 1)   {\
		   kprintf("%s Line%d mtx_unlock (mutex not locked)\n",__FUNCTION__,__LINE__);\
	   }\
        ((struct mtx*)mp)->state = 0;\
}\


#define mtx_assert(mp,b) {\
	   if (((struct mtx*)mp)->state != 1)   {\
		   kprintf("%s Line%d mtx_assert(MA_OWNED) not true\n",__FUNCTION__,__LINE__);\
	   }\
}\



#define __KASSERT(a,g) assert(a)
#define KASSERT(a,g) assert(a)
#define MS_TO_TICKS(ms) (((ms) * hz) / 1000)
#define TICKS_TO_MS(t) (((t) * 1000) / hz)



#define bootverbose 1

//#define  time_second (startup_ticks()/100)
#endif //
