
// -----------------------------------------------------------------------------------------
//Jicama Operating System
// Copyright (C) 2003  DengPingPing      All rights reserved.  
//------------------------------------------------------------------------------------------

#include <type.h>
#include <arch/x86/regs.h>
#include <jicama/devices.h>
#include <arch/x86/io.h>
#include <arch/x86/clock.h>

struct tm *_gmtime(const time_t *timer, struct tm *tmbuf);

__local char * months[12] = { "January", "February", "March", "April", "May",
  "June", "July", "August", "September", "October",
  "November", "December"  };

__local  char * weekdays[7] = {  "Sunday" ,"Monday", "Tuesday", "Wednesday",
  "Thursday", "Friday", "Saturday"};

void get_time( struct tm *time)
{
	time->sec = cmos_read(T_SECOND);
	time->min = cmos_read(T_MINUTE);
	time->hour= cmos_read(T_HOUR);

	time->hour = BCD2BIN(time->hour);
	time->min = BCD2BIN(time->min);
	time->sec = BCD2BIN(time->sec);

}

int get_gmtoff(void)
{
	int n;

	n=cmos_read(T_CENTURY);

	n = BCD2BIN(n);

	return n;
}

 void get_date( struct tm *time)
{
	time->__tm_gmtoff = cmos_read(T_CENTURY);

	time->dayofweek = cmos_read(T_DAYOFWEEK);
	time->day = cmos_read(T_DAYOFMONTH);
	time->month = cmos_read(T_MONTH);
	time->year = cmos_read(T_YEAR);

	time->dayofweek =  BCD2BIN(time->dayofweek);
	time->day =  BCD2BIN(time->day);
	time->month =  BCD2BIN(time->month);
	time->year = BCD2BIN(time->year);
	time->__tm_gmtoff = BCD2BIN(time->__tm_gmtoff);
	return;
}



//#define	INT_FREQ		1000

/* Countdown value to load into PIT timer, rounded to nearest integer. */
//static const uint32_t LATCH = ( ( TIMER_FREQ + INT_FREQ/2 ) / INT_FREQ );


static void init_timer0( void )
{
 	outb(TIMER_MODE, 0x34); /////0x43 0x36
	outb(TIMER0, LATCH&255); ////////0x40 0x
	outb(TIMER0, LATCH >> 8);
}

static void init_timer2( void )
{
	outb(  TIMER_MODE,0xb4 );
	outb(  TIMER2,0 );
	outb(  TIMER2,0 );
}

void reset_clock()
{
 	outb(0x61, inb( 0x61 ) | 1 );

	init_timer0();
	init_timer2();

	put_irq_handler(0, (void*)&clock_handler,NULL,"RTC");
}





 void do_gettime(struct tm*  toget)
{
 	 get_date(toget);
	 get_time(toget);
}



void write_cmos_reg(int reg, unsigned char val)
{
  outb(0x70, reg);
  outb(0x71, val);
}

__local void write_bcd_cmos_reg(int reg, unsigned char val)
{
  write_cmos_reg(reg, (unsigned char) (((val / 10) << 4) + (val % 10)));
}

__local inline void set_cmos_time(struct tm *tm)
{
  write_bcd_cmos_reg(0x09, (unsigned char) (tm->year % 100));
  write_bcd_cmos_reg(0x32, (unsigned char) ((tm->year + 1900) / 100));
  write_bcd_cmos_reg(0x08, (unsigned char) (tm->month + 1));
  write_bcd_cmos_reg(0x07, (unsigned char) (tm->yday));
  write_bcd_cmos_reg(0x04, (unsigned char) (tm->hour));
  write_bcd_cmos_reg(0x02, (unsigned char) (tm->min));
  write_bcd_cmos_reg(0x00, (unsigned char) (tm->sec));
}


void set_time(struct timeval *tv)
{
  struct tm tm;

  //upsince += (tv->tv_sec - systemclock.tv_sec);
  //systemclock.tv_usec = tv->tv_usec;
  //systemclock.tv_sec = tv->tv_sec;
  _gmtime((time_t *)&tv->tv_sec, &tm);
  set_cmos_time(&tm);
}



#define CLOCK_TICK_RATE	1193180 /* Underlying HZ */
#define CLOCK_TICK_FACTOR	20	/* Factor of both 1000000 and CLOCK_TICK_RATE */

long tick = 1000000 / HZ;               /* timer interrupt period */
#define TICK_SIZE tick
#define CT_TO_SECS(x)	((x) / HZ)
#define CT_TO_USECS(x)	(((x) % HZ) * 1000000/HZ)

__local inline unsigned long do_gettimeoffset(void)
{
	int count;
	unsigned long offset = 0;

	/* timer count may underflow right here */
	outb(0x00, 0x43);	/* latch the count ASAP */
	count = inb(0x40);	/* read the latched count */
	count |= inb(0x40) << 8;
	/* we know probability of underflow is always MUCH less than 1% */
	if (count > (LATCH - LATCH/100)) {
		/* check for pending timer interrupt */
		outb(0x0a, 0x20);
		if (inb(0x20) & 1)
			offset = TICK_SIZE;
	}
	count = ((LATCH-1) - count) * TICK_SIZE;
	count = (count + LATCH/2) / LATCH;
	return offset + count;
}


