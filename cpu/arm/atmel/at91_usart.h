/*   
 *  at91_usart.h  
 *  this file defines usart information on AT91X40 
 *  for Atmel AT91 serial driver
 *
 *  Bugs report:  li ming  ( lmcs00@mails.tsinghua.edu.cn )
 *
 */


#ifndef __AT91_USART_H
#define __AT91_USART_H

/*
 ******************* AT91x40xxx ********************
 */

#define ARM_CLK		(32768000)
#define UART_CLOCK	(ARM_CLK/16)

#define AT91_USART_CNT 2

/*#define AT91_USART0_BASE	(0xfffd0000)
#define AT91_USART1_BASE	(0xfffcc000)
*/
#define AT91_USART3_BASE	0xFFFCC000 /*16K */
#define AT91_USART2_BASE	0xFFFC8000 /*16K */
#define AT91_USART1_BASE	0xFFFC4000 /*16K */
#define AT91_USART0_BASE	0xFFFC0000 /*16K */

struct atmel_usart_regs{
	unsigned long cr;		// control 
	unsigned long mr;		// mode
	unsigned long ier;		// interrupt enable
	unsigned long idr;		// interrupt disable
	unsigned long imr;		// interrupt mask
	unsigned long csr;		// channel status
	unsigned long rhr;		// receive holding 
	unsigned long thr;		// tramsmit holding		
	unsigned long brgr;		// baud rate generator		
	unsigned long rtor;		// rx time-out
	unsigned long ttgr;		// tx time-guard
	unsigned long res1;
	unsigned long rpr;		// rx pointer
	unsigned long rcr;		// rx counter
	unsigned long tpr;		// tx pointer
	unsigned long tcr;		// tx counter
};

/*  US control register */
#define US_SENDA	(1<<12)
#define US_STTO		(1<<11)
#define US_STPBRK	(1<<10)
#define US_STTBRK	(1<<9)
#define US_RSTSTA	(1<<8)
#define US_TXDIS	(1<<7)
#define US_TXEN		(1<<6)
#define US_RXDIS	(1<<5)
#define US_RXEN		(1<<4)
#define US_RSTTX	(1<<3)
#define US_RSTRX	(1<<2)

/* US mode register */
#define US_CLK0		(1<<18)
#define US_MODE9	(1<<17)
#define US_CHMODE(x)(x<<14 & 0xc000)
#define US_NBSTOP(x)(x<<12 & 0x3000)
#define US_PAR(x)	(x<<9 & 0xe00)
#define US_SYNC		(1<<8)
#define US_CHRL(x)	(x<<6 & 0xc0)
#define US_USCLKS(x)(x<<4 & 0x30)

/* US interrupts enable/disable/mask and status register */
#define US_DMSI		(1<<10)
#define US_TXEMPTY	(1<<9)
#define US_TIMEOUT	(1<<8)
#define US_PARE		(1<<7)
#define US_FRAME	(1<<6)
#define US_OVRE		(1<<5)
#define US_ENDTX	(1<<4)
#define US_ENDRX	(1<<3)
#define US_RXBRK	(1<<2)
#define US_TXRDY	(1<<1)
#define US_RXRDY	(1)

#define US_ALL_INTS (US_DMSI|US_TXEMPTY|US_TIMEOUT|US_PARE|US_FRAME|US_OVRE|US_ENDTX|US_ENDRX|US_RXBRK|US_TXRDY|US_RXRDY)

#define _INLINE_

int at91_init_usart( void );

void at91_uart_send_string( char *p, int len );

void at91_uart_put_char( char ch );

void at91_uart_get_char( char *ch );


#endif //__AT91_USART_H

