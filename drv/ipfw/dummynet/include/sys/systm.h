#ifndef _SYS_SYSTM_H_
#define _SYS_SYSTM_H_

#ifndef _WIN32	/* this is the linux version */
/* callout support, in <sys/callout.h> on FreeBSD */
/*
 * callout support on linux module is done using timers
 */
//#include <linux/timer.h>
//#ifdef LINUX_24
//#include <linux/sched.h>        /* jiffies definition is here in 2.4 */
//#endif
//#define callout timer_list
static __inline int
callout_reset(struct callout *co, int ticks, void (*fn)(void *), void *arg)
{
        //co->expires = jiffies + ticks;
        co->handler = (void (*)(unsigned long))fn;
        co->arg = (unsigned long)arg;
		//init_timer(co,fn,arg);
        mod_timer(co,ticks);
        return 0;
}

#define callout_init(co, safe) memset(co,0,sizeof(struct callout))
#define callout_drain(co)       remove_timer(co)
#define callout_stop(co)        remove_timer(co)

#define CALLOUT_ACTIVE          0x0002 /* callout is currently active */
#define CALLOUT_MPSAFE          0x0008 /* callout handler is mp safe */

#else /* _WIN32 */

/* This is the windows part for callout support */
struct callout {
	int dummy;
};
static __inline int
callout_reset(struct callout *co, int ticks, void (*fn)(void *), void *arg)
{
	return 0;
}

#define callout_init(co, safe)
#define callout_drain(co)
#define callout_stop(co)
#endif /* !_WIN32 */


#if 0
/* add out timer to the kernel global timer list */
NTSTATUS 
  IoInitializeTimer(
    IN PDEVICE_OBJECT  DeviceObject,
    IN PIO_TIMER_ROUTINE  TimerRoutine,
    IN PVOID  Context
    );

/* see differences :
IoInitializeDpcRequest
	http://dsrg.mff.cuni.cz/~ceres/sch/osy/text/ch04s01s01.php
	example http://www.beyondlogic.org/interrupts/winnt_isr_dpc.htm
KeInitializeDpc  IRQL: Any level
IoInitializeTimer IRQL: Passive level
KeInitializeTimer */
VOID 
  KeInitializeDpc(
    IN PRKDPC  Dpc,
    IN PKDEFERRED_ROUTINE  DeferredRoutine,
    IN PVOID  DeferredContext
    );
#endif /* commented out */ 

#endif /* _SYS_SYSTM_H_ */
