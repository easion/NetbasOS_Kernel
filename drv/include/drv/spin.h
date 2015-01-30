
#ifndef __spinlock_h__
#define __spinlock_h__
#ifdef __cplusplus
extern "C" {
#endif


typedef struct 
{
	volatile unsigned long lock;
	volatile unsigned  flags;
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

static inline void spin_lock( spinlock_t *lock )
{
#ifdef __ARM__
#else
	__asm__ __volatile__(
		"1: lock ; decb %0\n"
		"jge 3f\n"
		"2:\n"
		"cmpb $0, %0\n"
		"rep; nop\n"
		"jle 2b\n"
		"sti\n"
		"jmp 1b\n"
		"3:\n"
		: "=m"(lock->lock) : : "memory");
#endif
}


static inline void spin_unlock( spinlock_t *lock )
{
#ifdef __ARM__
#else
	__asm__ __volatile__(
		"movb $1, %0"
		: "=m"(lock->lock) : : "memory");
#endif		
}

static inline unsigned long	cli( void )
{
	unsigned long ret_val;
#ifdef __ARM__
	#error 111
#else
	__asm__ __volatile__("pushfl\n"
		"popl %0\n"
		"cli"
		: "=a"(ret_val)
		:);
#endif
	return ret_val;
}

static inline void put_cpu_flags (unsigned long	 flags )
{
#ifdef __ARM__
#else
		__asm__ __volatile__("pushl %0\n"
		"popfl"
		:: "m"(flags));
#endif
}

/* Semaphores & IPC */


#define spin_lock_irq(x) \
	spin_lock(x)
#define spin_unlock_irq(x) \
	spin_unlock(x)

#define spinlock_cli( lock, flags ) do{flags = cli(); spin_lock( lock );}while(0)
#define spinunlock_restore( lock, flags ) do{spin_unlock( lock ); put_cpu_flags( flags );}while(0)

static inline void spinlock_init( spinlock_t* plock, const char* pzName )
{
  plock->lock = SPINLOCK_UNLOCKED;
  //plock->sl_nProc   = -1;
  //atomic_set(&plock->sl_nNest, 0);
  //plock->sl_pzName  = pzName;
}

static inline unsigned spinlock_disable( spinlock_t* plock )
{
  unsigned nFlg = cli();
  spin_lock( plock );
  return( nFlg );
}

static inline void spinunlock_enable( spinlock_t* plock, unsigned nFlg )
{
  spin_unlock( plock );
  put_cpu_flags( nFlg );
}

#define spin_lock_irqsave(x, y) \
	spinlock_cli(x, y)
#define spin_unlock_irqrestore(x, y) \
	spinunlock_restore(x, y)

#ifdef __cplusplus
}
#endif


#endif
