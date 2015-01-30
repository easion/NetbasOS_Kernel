/*   
 *  at91_tc.h  
 *  this file defines TC information on AT91X40 
 *  for Atmel AT91 timer counter
 *
 *  Bugs report:     li ming  ( lmcs00@mails.tsinghua.edu.cn )
 *  Last modified:   2003-02-02 
 *
 */

#ifndef __AT91_TC_H
#define __AT91_TC_H

/* always include this file for at91 peripherals */
#include "at91.h"



/* here define peripheral-specific hardware datastruct */
#define AT91_TC_BASE		(0xfffe0000)

struct at91_timer_channel
{
	unsigned long ccr;				// channel control register		(WO)
	unsigned long cmr;				// channel mode register		(RW)
	unsigned long reserved[2];		
	unsigned long cv;				// counter value				(RW)
	unsigned long ra;				// register A					(RW)
	unsigned long rb;				// register B					(RW)
	unsigned long rc;				// register C					(RW)
	unsigned long sr;				// status register				(RO)
	unsigned long ier;				// interrupt enable register	(WO)
	unsigned long idr;				// interrupt disable register	(WO)
	unsigned long imr;				// interrupt mask register		(RO)
};

struct at91_timers
{
	struct {
		struct at91_timer_channel ch;
		unsigned char padding[0x40-sizeof(struct at91_timer_channel)];
	} chans[3];
	unsigned  long bcr;				// block control register		(WO)
	unsigned  long bmr;				// block mode	 register		(RW)
};

#define ARM_CLK		(32768000)



/* 0=TC0, 1=TC1, 2=TC2 */
#define KERNEL_TIMER 1	

/*  TC control register */
#define TC_SYNC	(1)

/*  TC mode register */
#define TC2XC2S(x)	(x & 0x3)
#define TC1XC1S(x)	(x<<2 & 0xc)
#define TC0XC0S(x)	(x<<4 & 0x30)
#define TCNXCNS(timer,v) ((v) << (timer<<1))

/* TC channel control */
#define TC_CLKEN	(1)			
#define TC_CLKDIS	(1<<1)			
#define TC_SWTRG	(1<<2)			

/* TC interrupts enable/disable/mask and status registers */
#define TC_MTIOB	(1<<18)
#define TC_MTIOA	(1<<17)
#define TC_CLKSTA	(1<<16)

#define TC_ETRGS	(1<<7)
#define TC_LDRBS	(1<<6)
#define TC_LDRAS	(1<<5)
#define TC_CPCS		(1<<4)
#define TC_CPBS		(1<<3)
#define TC_CPAS		(1<<2)
#define TC_LOVRS	(1<<1)
#define TC_COVFS	(1)

#define IRQ_TC0		4
#define IRQ_TC1		5
#define IRQ_TC2		6

#if (KERNEL_TIMER==0)
#   define KERNEL_TIMER_IRQ_NUM IRQ_TC0
#elif (KERNEL_TIMER==1)
#   define KERNEL_TIMER_IRQ_NUM IRQ_TC1
#elif (KERNEL_TIMER==2)
#   define KERNEL_TIMER_IRQ_NUM IRQ_TC2
#else
#error Wierd -- KERNEL_TIMER is not defined or something....
#endif

void at91_init_timer(void);



#endif// __AT91_TC_H