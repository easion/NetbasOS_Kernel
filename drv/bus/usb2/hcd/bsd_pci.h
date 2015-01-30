



struct __callout
{
	krnl_timer_t timer;
	 void *mtx;
};

struct selinfo
{
	int a;
};

enum
{
	M_CACHE, 
M_DEVBUF,
M_TEMP, 
		M_ZERO,
		M_USBDEV,
		M_USB,
		M_BUS,
		M_BUS_SC,

};
	#define malloc malloc_static
	#define free free_static
/*
 * flags to malloc
 */
#define	M_WAITOK	0x0000
#define	M_NOWAIT	0x0001
#define M_KERNEL	0x0002

#define	M_MAGIC		877983977	/* time when first defined :-) */


#define	USB_HOST_ALIGN    8		/* bytes, must be power of two */
#define	LIBUSB20_ME_STRUCT_ALIGN sizeof(void *)



#define __FBSDID(X)
#define SYSCTL_INT(X,Y,Z,X1,Y1,Z1,N)
#define SYSCTL_NODE(X,Y,Z,X1,Y1,Z1)
#define MALLOC_DEFINE(X,Y,Z)

enum{
	UQ_SWAP_UNICODE,
 UQ_NO_STRINGS,
UQ_POWER_CLAIM,
		UQ_BUS_POWERED,
};

#define MA_OWNED        9
enum{
	PZERO,
	PRIBIO,
  PCATCH ,
};

enum{
TX_DEF,MTX_RECURSE,MTX_DEF
};

	enum{
		UIO_READ,
			UIO_WRITE,
			UIO_USERSPACE,
	};

#define hz 100
#define curthread current_thread()
#define PWAIT 0
#define SIGIO 0

struct timeval {
	long	tv_sec;		/* seconds */
	long	tv_usec;	/* and microseconds */
};
#define DELAY(N) thread_wait(current_thread(),((N)/100))

#ifndef _TIMESPEC_DECLARED
#define _TIMESPEC_DECLARED
struct timespec {
	time_t	tv_sec;		/* seconds */
	long	tv_nsec;	/* and nanoseconds */
};

#define	TIMEVAL_TO_TIMESPEC(tv, ts)					\
	do {								\
		(ts)->tv_sec = (tv)->tv_sec;				\
		(ts)->tv_nsec = (tv)->tv_usec * 1000;			\
	} while (0)
#define	TIMESPEC_TO_TIMEVAL(tv, ts)					\
	do {								\
		(tv)->tv_sec = (ts)->tv_sec;				\
		(tv)->tv_usec = (ts)->tv_nsec / 1000;			\
	} while (0)

#endif


#define BUS_SPACE_BARRIER_READ 0
#define BUS_SPACE_BARRIER_WRITE 1
#define CALLOUT_RETURNUNLOCKED 1
#define USB_MODE_HOST 1
#define USB_ERR_CANCELLED 1



