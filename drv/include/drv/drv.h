

#ifndef DRV_H
#define DRV_H
#ifdef __cplusplus
extern "C" {
#endif

#define __NETBAS__ 1
#define __KERNEL__ 1


#define __SYSTEM__
#include <drv/defs.h>
#include <drv/sym.h>

#include <drv/ansi.h>
#include <drv/user.h>

#define __user
#define __exit
#define __init

#define	peekb(x)	(*(unsigned char *)(x))
#define	peekw(x)	(*(unsigned short *)(x))
#define	peekl(x)	(*(unsigned long *)(x))

#define	pokeb(x,n)	(*(unsigned char *)(x)=(n))
#define	pokew(x,n)	(*(unsigned short *)(x)=(n))
#define	pokel(x,n)	(*(unsigned long *)(x)=(n))

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#define DEV_FD0   0x200
#define DEV_HD0   0x300
#define DEV_RAM   0x100
#define DEV_SCSI  0x700	/* Atari TT only */


#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))

#define	snooze(N) //thread_wait(current_thread(), N+10)
//#define	snooze(N) mdelay((N))


/*TIME STRUCT*/

struct tm
{
  int sec;			// Seconds after the minute [0, 59]
  int min;			// Minutes after the hour [0, 59]
  int hour;			// Hours since midnight [0, 23]

  int day;			// Day of the month [1, 31]
  int month;			// Months since January [0, 11]
  int year;			// Years since 1900
  int dayofweek;			// Days since Sunday [0, 6]
  int yday;			// Days since January 1 [0, 365]

  int isdst;			// Daylight Saving Time flag
  char *__tm_zone;
  int __tm_gmtoff;
};

#ifdef __cplusplus
}
#endif
#endif


