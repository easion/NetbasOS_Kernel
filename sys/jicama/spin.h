
#ifndef __spinlock_h__
#define __spinlock_h__

typedef struct 
{
	volatile unsigned char lock;
	//volatile unsigned  flags;
} spinlock_t;

enum{
 SPINLOCK_LOCKED=		0,
 SPINLOCK_UNLOCKED=	1,
};

#define CREATE_SPINLOCK( name ) \
	static spinlock_t name = { SPINLOCK_UNLOCKED, }

#define CREATE_LOCKED_SPINLOCK( name ) \
	static spinlock_t name = { SPINLOCK_LOCKED, }

#define spin_is_locked( s ) \
	( *(volatile signed char *)(&((s)->lock))<=0 )

#define spin_lock_init( x ) \
	do { *(x) = (spinlock_t){ SPINLOCK_UNLOCKED, }; } while( 0 )


#ifdef __IA32__
#include <arch/x86/spin.h>
#elif defined(__ARM__)  //arm9
#define	spin_unlock(p) 
#define spin_lock(p)
#else

#endif


#endif
