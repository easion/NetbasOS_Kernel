#include <jicama/system.h>
#include <jicama/utsname.h>
#include <jicama/process.h>
#include <jicama/console.h>
#include <jicama/devices.h>
#include <assert.h>
#include "at91.h"
#include "at91_usart.h"

__local volatile struct atmel_usart_regs *usarts[AT91_USART_CNT] = {
	(volatile struct atmel_usart_regs *) AT91_USART0_BASE,
	(volatile struct atmel_usart_regs *) AT91_USART1_BASE
};

__local unsigned long calcCD(unsigned long br)
{
	return (UART_CLOCK / br);
}

__local void uart_init(volatile struct atmel_usart_regs *uart)
{
	/* Reset the USART */
	uart->cr = US_TXDIS | US_RXDIS | US_RSTTX | US_RSTRX;
	/* clear Rx receive and Tx sent counters */
	uart->rcr = 0;
	uart->tcr = 0;
	
	/* Set the serial port into a safe sane state */
	uart->mr = US_USCLKS(0) | US_CLK0 | US_CHMODE(0) | US_NBSTOP(0) |
		    US_PAR(4) | US_CHRL(3);

	uart->brgr = calcCD(9600);

	uart->rtor = 20;			// timeout = value * 4 *bit period
	uart->ttgr = 0;				// no guard time
	uart->rcr = 0;
	uart->rpr = 0;
	uart->tcr = 0;
	uart->tpr = 0;
	at91_uart_send_string("hello world!", 14);
}

__local void inline tx_enable(volatile struct atmel_usart_regs *uart)
{
	uart->ier = US_TXEMPTY;
}

__local void inline rx_enable(volatile struct atmel_usart_regs *uart)
{
	uart->ier = US_ENDRX | US_TIMEOUT;
}

__local void inline tx_start(volatile struct atmel_usart_regs *uart, int ints)
{
	if (ints) {
		tx_enable(uart);
	}
	uart->cr = US_TXEN;
}

__local void inline rx_start(volatile struct atmel_usart_regs *uart, int ints)
{
	uart->cr = US_RXEN | US_STTO;
	uart->rtor = 20;
	if (ints) {
		rx_enable(uart);
	}
}

__local void inline rx_disable(volatile struct atmel_usart_regs *uart)
{
	uart->idr = US_ENDRX | US_TIMEOUT;
}

__local void inline rx_stop(volatile struct atmel_usart_regs *uart)
{
	rx_disable(uart);
	uart->rtor = 0;
	uart->rcr = 0;
	uart->cr = US_RXDIS;
}

__local void start_rx(volatile struct atmel_usart_regs *uart, char *rx_buf, int len )
{
	rx_stop(uart);

 	uart->rpr = (unsigned long) rx_buf;
 	uart->rcr = (unsigned long) len;
 	rx_start(uart, 1);
}

__local void xmit_string(volatile struct atmel_usart_regs *uart, char *p, int len)
{
	uart->tcr = 0;
	uart->tpr = (unsigned long) p;
	uart->tcr = (unsigned long) len;
	tx_start(uart, 1);
}

__local void xmit_char(volatile struct atmel_usart_regs *uart, char ch)
{
	xmit_string(uart, &ch, 1);
}

int at91_init_usart()
{
	uart_init( usarts[0] );	
	return 0;
}

void at91_uart_send_string( char *p, int len )
{
	xmit_string( usarts[0], p, len);
}

void at91_uart_put_char( char ch )
{
	xmit_char( usarts[0], ch );
}

__local void _io_read( long * paddr, long * pvalue )
{
	*pvalue = *paddr;
	return;	
}

static int ops_putchar (console_t *con, unsigned char ch, bool vt)
{
	if (vt)
	{
		return -1;
	}
	at91_uart_put_char(ch);
	return 0;
}

void at91_uart_get_char( char *ch )
{
	unsigned long status;

	long tmp;
	start_rx( usarts[0], ch, 1 );
	do
	{
		// must read 0xfffff100, status register to clear all flags !!!
		_io_read( (long*)0xfffff100, &tmp );
		_io_read( (long*)0xfffff108, &tmp );
		status = usarts[0]->rcr;
	}
	while (status > 0);
}

struct tty_ops ser_ops_bak
={
	name:		"atmel uart",
	init:		at91_init_usart,
	putchar:	ops_putchar,
	readbuf:	kb_read,
	char_erase:	0,
	char_attrib:0,
	exit:NULL,
};

