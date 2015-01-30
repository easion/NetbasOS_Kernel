/*
**     (R)Jicama OS
**     Real Time Controller
**     Copyright (C) 2003 DengPingPing
*/

#include <jicama/system.h>
#include <jicama/process.h>
#include <jicama/log.h>
#include <jicama/timer.h>
#include <jicama/console.h>
#include <jicama/devices.h>
#include <string.h>
int get_gmtoff(void);

#define MSECTOTICKS(X) ((X)*HZ/1000)

void reset_clock();
unsigned long get_unix_time(void);

__asmlink	bool scheduleable();
__asmlink	void switch_enable();
time_t volatile ticks;
__local  void init_timers();

unsigned long volatile boot_unix_time;
time_t volatile upsince;

void clock_init (void)
{
	upsince = get_unix_time();
	ticks=0;
	init_timers();
	reset_clock();
}


//#define USECS_PER_TICK  (USEC_IN_SEC / TIMER_FREQ)/*Ã¿ÃëµÎ´ðÊý£º10000*/
//#define MSECS_PER_TICK  (1000 / TIMER_FREQ) /*Ã¿µÎ´ðÎ¢ÃëÊý£º10*/
__local void run_timer_list();



void clock_handler(void* arg, int irq, interrupt_regs_t reg)
{
	thread_t *pthread;

	KATOMIC_INC(ticks, time_t);
	if(bill_proc)bill_proc->sticks++;		

	pthread = current_thread();

	if (pthread->ticks > 0){
		pthread->ticks--;
		//return;	
	}

	run_timer_list();
	return 0;
}

time_t get_unix_sec()
{
	time_t cur_sec;
	unsigned cur_flag;

	save_eflags(&cur_flag);
	cur_sec= (upsince+ticks/HZ);
	restore_eflags(cur_flag);
	return cur_sec;
}

time_t startup_ticks()
{
	time_t cur_tiks;
	unsigned cur_flag;

	save_eflags(&cur_flag);
	cur_tiks= ticks;
	restore_eflags(cur_flag);
	return cur_tiks;	
}

#include <jicama/spin.h>

CREATE_SPINLOCK( mdelay_sem );

void mdelay(int msec)
{
	int now;
	unsigned flags;

    spin_lock( &mdelay_sem );
	//save_eflags(&flags);
	enable();

	now = startup_ticks();
    do;
	while (startup_ticks() - now < MSECTOTICKS(msec));
	//restore_eflags(flags); 
	spin_unlock( &mdelay_sem );
}



time_t msec2ticks(time_t msec)
{
	time_t retval = msec * HZ / 1000;
	
	return retval;
}

time_t ticks2msec(time_t _ticks)
{
	time_t retval = _ticks * 1000/ HZ;
	return retval;
}


#define TVN_BITS 6
#define TVR_BITS 8
#define TVN_SIZE (1 << TVN_BITS)
#define TVR_SIZE (1 << TVR_BITS)
#define TVN_MASK (TVN_SIZE - 1)
#define TVR_MASK (TVR_SIZE - 1)

struct timer_vec 
{
  int index;
  struct slink vec[TVN_SIZE];
};

struct timer_vec_root 
{
  int index;
  struct slink vec[TVR_SIZE];
};


__local unsigned int timer_ticks = 0;

__local struct timer_vec tv5;
__local struct timer_vec tv4;
__local struct timer_vec tv3;
__local struct timer_vec tv2;
__local struct timer_vec_root tv1;

__local struct timer_vec * const tvecs[] = 
{
  (struct timer_vec *)&tv1, &tv2, &tv3, &tv4, &tv5
};

#define NOOF_TVECS (sizeof(tvecs) / sizeof(tvecs[0]))

//
// attach_timer
//

__local void attach_timer(struct timer *timer)
{
  unsigned int expires ;
  unsigned int idx ;
  struct slink *vec;
	unsigned fl;

    save_eflags(&fl);

	expires = timer->expires;
	idx = expires - timer_ticks;

  if (idx < TVR_SIZE) 
  {
    int i = expires & TVR_MASK;
    vec = tv1.vec + i;
  } 
  else if (idx < 1 << (TVR_BITS + TVN_BITS)) 
  {
    int i = (expires >> TVR_BITS) & TVN_MASK;
    vec = tv2.vec + i;
  } 
  else if (idx < 1 << (TVR_BITS + 2 * TVN_BITS)) 
  {
    int i = (expires >> (TVR_BITS + TVN_BITS)) & TVN_MASK;
    vec =  tv3.vec + i;
  } 
  else if (idx < 1 << (TVR_BITS + 3 * TVN_BITS)) 
  {
    int i = (expires >> (TVR_BITS + 2 * TVN_BITS)) & TVN_MASK;
    vec = tv4.vec + i;
  } 
  else if ((signed long) idx < 0) 
  {
    // Can happen if you add a timer with expires == timer_ticks, 
    // or you set a timer to go off in the past
    vec = tv1.vec + tv1.index;
  } 
  else
  {
    int i = (expires >> (TVR_BITS + 3 * TVN_BITS)) & TVN_MASK;
    vec = tv5.vec + i;
  } 

  timer->link.next = vec;
  timer->link.prev = vec->prev;
  vec->prev->next = &timer->link;
  vec->prev = &timer->link;

  timer->active = true;

  restore_eflags(fl);
}

//
// detach_timer
//

__local int detach_timer(struct timer *timer)
{
  if (!timer->active) return 0;
	unsigned fl;

  save_eflags(&fl);

  timer->link.next->prev = timer->link.prev;
  timer->link.prev->next = timer->link.next;
  timer->active = false;
  restore_eflags(fl);

  return 1;
}

//
// init_timers
//

__local void init_timers()
{
  int i;

  for (i = 0; i < TVN_SIZE; i++) 
  {
    tv5.vec[i].next = tv5.vec[i].prev = tv5.vec + i;
    tv4.vec[i].next = tv4.vec[i].prev = tv4.vec + i;
    tv3.vec[i].next = tv3.vec[i].prev = tv3.vec + i;
    tv2.vec[i].next = tv2.vec[i].prev = tv2.vec + i;
  }
  for (i = 0; i < TVR_SIZE; i++)
  {
    tv1.vec[i].next = tv1.vec[i].prev = tv1.vec + i;
  }
}

//
// init_timer
//

void init_timer(struct timer *timer, void (*handler)(void *arg), void *arg)
{
  timer->link.next = NULL;
  timer->link.next = NULL;
  timer->expires = 0;
  timer->active = false;
  timer->handler = handler;
  timer->arg = arg;
}

//
// add_timer
//

void install_timer(struct timer *timer, time_t msec)
{
  unsigned int expires = msec2ticks(msec);

  timer->expires = ticks+expires;

  if (timer->active) 
  {
    kprintf("timer: timer is already active\n");
    return;
  }

  attach_timer(timer);
}

//
// del_timer
//

int remove_timer(struct timer *timer)
{
  int rc;

  rc = detach_timer(timer);
  timer->link.next = NULL;
  timer->link.prev = NULL;

  return rc;
}

//
// restart_timer
//

int restart_timer(struct timer *timer, unsigned msec)
{
  int rc;
  unsigned int expires = msec2ticks(msec);

  timer->expires = ticks+expires;
  rc = detach_timer(timer);
  attach_timer(timer);

  return rc;
}

int mod_timer(struct timer *timer, unsigned expires)
{
  int rc;

  timer->expires = ticks+expires;
  rc = detach_timer(timer);
  attach_timer(timer);

  return rc;
}

//
// cascade_timers
//
// Cascade all the timers from tv up one level. We are removing 
// all timers from the list, so we don't  have to detach them 
// individually, just clear the list afterwards.
//

__local void cascade_timers(struct timer_vec *tv)
{
  struct slink *head, *curr, *next;

  head = tv->vec + tv->index;
  curr = head->next;

  while (curr != head) 
  {
    struct timer *timer;

    timer = (struct timer *) curr;

    next = curr->next;
    attach_timer(timer);
    curr = next;
  }

  head->next = head->prev = head;
  tv->index = (tv->index + 1) & TVN_MASK;
}

//
// run_timer_list
//

__local void run_timer_list()
{
  while ((long) (ticks - timer_ticks) >= 0) 
  {
    struct slink *head, *curr;

    if (!tv1.index) 
    {
      int n = 1;
      do { cascade_timers(tvecs[n]); }
	  while (tvecs[n]->index == 1 && ++n < NOOF_TVECS);
    }

    while (1)
    {
      struct timer *timer;
      void (*handler)(void *arg);
      void *arg;

      head = tv1.vec + tv1.index;
      curr = head->next;
      if (curr == head) break;

      timer = (struct timer *) curr;
      handler = timer->handler;
      arg = timer->arg;

      detach_timer(timer);
      timer->link.next = timer->link.prev = NULL;

	  if(handler)
      handler(arg);
    }

    timer_ticks++;
    tv1.index = (tv1.index + 1) & TVR_MASK;
  }
}

/*
**Copyright (C) 2001 - Christopher Giese <geezer@execpc.com>
**http://www.execpc.com/~geezer/os/index.htm#cosmos
*/
__local long days_between_dates(short start_year, unsigned short start_day,
		short end_year, unsigned short end_day)
{
	short fourcents, centuries, fouryears, years;
	long days;

	fourcents = end_year / 400 - start_year / 400;
	centuries = end_year / 100 - start_year / 100 -		fourcents * 4;

   //subtract from 'centuries' the centuries already accounted for by
    //'fourcents' 
	fouryears = end_year / 4 - start_year / 4 -		fourcents * 100 - centuries * 25;

   //subtract from 'fouryears' the fouryears already accounted for by
   //'fourcents' and 'centuries' 
	years = end_year - start_year -		400 * fourcents - 100 * centuries - 4 * fouryears;

   //subtract from 'years' the years already accounted for by
    //'fourcents', 'centuries', and 'fouryears' 
   //add it up: 97 leap days every fourcent 
	days = (365L * 400 + 97) * fourcents;
   //24 leap days every residual century 
	days += (365L * 100 + 24) * centuries;
   //1 leap day every residual fouryear 
	days += (365L * 4 + 1) * fouryears;
   //0 leap days for residual years 
	days += (365L * 1) * years;
   //residual days (need the cast!) 
	days += ((long)end_day - start_day);
   //account for terminal leap year 
	if(end_year % 4 == 0 && end_day >= 60)
	{
		days++;
		if(end_year % 100 == 0)
			days--;
		if(end_year % 400 == 0)
			days++;
	}
   //xxx - what have I wrought? I don't know what's going on here,
   //but the code won't work properly without it 
	if(end_year >= 0)
	{
		days++;
		if(end_year % 4 == 0)
			days--;
		if(end_year % 100 == 0)
			days++;
		if(end_year % 400 == 0)
			days--;
	}
	if(start_year > 0)
		days--;
	return days;
}

/*
**Copyright (C) 2001 - Christopher Giese <geezer@execpc.com>
**http://www.execpc.com/~geezer/os/index.htm#cosmos
*/
//month and date start with 1, not with 0
unsigned long unix_time(unsigned short year, unsigned char month,
		unsigned char date, unsigned char hour, unsigned char min,
		unsigned char sec)
{
	unsigned long ret_val;
	unsigned short day;

	__local const unsigned short days_to_date[12] =
	{
   //jan  feb  mar  apr  may  jun  jul  aug  sep  oct  nov  dec 
		0,
		31,
		31 + 28,
		31 + 28 + 31,
		31 + 28 + 31 + 30,
		31 + 28 + 31 + 30 + 31,
		31 + 28 + 31 + 30 + 31 + 30,
		31 + 28 + 31 + 30 + 31 + 30 + 31,
		31 + 28 + 31 + 30 + 31 + 30 + 31 + 31,
		31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30,
		31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31,
		31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30
	};

   //convert month and year to day-in-year 
	if(month > 11 || date > 30)
		return 0;
	day = date + days_to_date[month];

   //convert to Unix JDN (UJDN) 
	ret_val = days_between_dates(1970, 0, year, day);
   //convert from days to seconds, adding time as you go 
	ret_val *= 24;
	ret_val += hour;
	ret_val *= 60;
	ret_val += min;
	ret_val *= 60;
	ret_val += sec;
	return ret_val;
}

unsigned long unix_time2tm(time_t timeval,struct tm *tm_time)
{

	unsigned long ret_val;
	unsigned short day;

	tm_time->sec  = timeval%60;
	timeval/=60;
	tm_time->min  = timeval%60;
	timeval/=60;
	tm_time->sec  = timeval%24;
	timeval/=24;
	//	
}

#define MINUTE	60
#define HOUR	(MINUTE * 60)
#define DAY	(HOUR * 24)
#define YEAR	(DAY * 365)

/* the months of leap year */
static int month[12] = {
	0,
	DAY * (31),
	DAY * (31 + 29),
	DAY * (31 + 29 + 31),
	DAY * (31 + 29 + 31 + 30),
	DAY * (31 + 29 + 31 + 30 + 31),
	DAY * (31 + 29 + 31 + 30 + 31 + 30),
	DAY * (31 + 29 + 31 + 30 + 31 + 30 + 31),
	DAY * (31 + 29 + 31 + 30 + 31 + 30 + 31 + 31),
	DAY * (31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30),
	DAY * (31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31),
	DAY * (31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30),
};

unsigned long to_unix_time(struct tm * time)
{
	unsigned long ret;
	u32_t year;

	int n=0;

	n= time->__tm_gmtoff;

	if (n<0)
	{
	n = get_gmtoff();
	}


	year = time->year + n * 100 - 1970;

	ret = year * YEAR + DAY * ((year + 1) / 4);	

	ret += month[time->month - 1];

	/* if it's not leap year */
	if(time->month > 2 && ((year + 2) % 4))
		ret -= DAY;


	ret += DAY * (time->day - 1);
	ret += HOUR * time->hour;
	ret += MINUTE * time->min;
	ret += time->sec;

	return ret;
}

unsigned long get_unix_time(void)
{
	struct tm init_time;
	unsigned long now_time = 0;

    memset(&init_time,0,sizeof(struct tm));
    do_gettime(&init_time);

	now_time = to_unix_time(&init_time);

	//kprintf("get_unix_time %d-%d-%d %d:%d:%d\n",init_time.year,init_time.month,
	//	init_time.day, init_time.hour, init_time.min,init_time.sec);

	return now_time;
}


#define	YEAR0		        1900
#define	EPOCH_YR	        1970
#define	SECS_DAY	        (24L * 60L * 60L)
#define	LEAPYEAR(year)	        (!((year) % 4) && (((year) % 100) || !((year) % 400)))
#define	YEARSIZE(year)	        (LEAPYEAR(year) ? 366 : 365)
#define	FIRSTSUNDAY(timp)       (((timp)->yday - (timp)->dayofweek + 420) % 7)
#define	FIRSTDAYOF(timp)        (((timp)->dayofweek - (timp)->yday + 420) % 7)
const int _ytab[2][12] = 
{
  {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
  {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
};

struct tm *_gmtime(const time_t *timer, struct tm *tmbuf)
{
  time_t time = *timer;
  unsigned long dayclock, dayno;
  int year = EPOCH_YR;

  dayclock = (unsigned long) time % SECS_DAY;
  dayno = (unsigned long) time / SECS_DAY;

  tmbuf->sec = dayclock % 60;
  tmbuf->min = (dayclock % 3600) / 60;
  tmbuf->hour = dayclock / 3600;
  tmbuf->dayofweek = (dayno + 4) % 7; // Day 0 was a thursday
  while (dayno >= (unsigned long) YEARSIZE(year)) 
  {
    dayno -= YEARSIZE(year);
    year++;
  }
  tmbuf->year = year - YEAR0;
  tmbuf->yday = dayno;
  tmbuf->month = 0;
  while (dayno >= (unsigned long) _ytab[LEAPYEAR(year)][tmbuf->month]) 
  {
    dayno -= _ytab[LEAPYEAR(year)][tmbuf->month];
    tmbuf->month++;
  }
  tmbuf->yday = dayno + 1;
  tmbuf->isdst = 0;

  return tmbuf;
}


