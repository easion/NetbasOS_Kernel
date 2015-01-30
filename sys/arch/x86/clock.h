/* 
*Jicama OS  
 * Copyright (C) 2001-2003  DengPingPing      All rights reserved.   
 */
#ifndef CLOCK_H
#define CLOCK_H

#include <jicama/system.h>

#define PCR		0x65	/* Planar Control Register */
#define PORT_B          0x61	/* I/O port for 8255 port B (kbd, beeper...) */
#define TIMER0          0x40	/* I/O port for timer channel 0 */
#define TIMER2          0x42	/* I/O port for timer channel 2 */
#define TIMER_MODE      0x43	/* I/O port for timer mode control */

//#define COUNTER_FREQ (2*TIMER_FREQ)
#define LATCH_COUNT     0x00

#define TIMER_FREQ  1193182L
#define LATCH		(TIMER_FREQ / HZ)
#define TIMER_COUNT ((unsigned) (TIMER_FREQ/HZ))

#define T_SECOND		0
#define T_SEC_ALRAM	1
#define T_MINUTE		2
#define T_MIN_ALRAM	3
#define T_HOUR		4
#define T_HOUR_ALRAM		5
#define T_DAYOFWEEK		6
#define T_DAYOFMONTH	7
#define T_MONTH		8
#define T_YEAR		9
#define T_CENTURY		0x32

// void time(void);
 //void date(void);

#endif



