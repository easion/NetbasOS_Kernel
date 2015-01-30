
#include <jicama/process.h>
#include <jicama/devices.h>
#include <jicama/console.h>

#define INTUART0_RX		5
#define UARTSTAT0       (*(volatile unsigned int *)0x3ffd008)
#define UARTRXB0        (*(volatile unsigned int *)0x3ffd010)

#define	USTAT_RCV_READY	0x20   /* receive data ready */
#define	USTAT_TXB_EMPTY 0x40   /* tx buffer empty */

void uart_isr(void *arg, int vector)
{
}


int uart_getch()
{
	int ch;
	while ((UARTSTAT0 & USTAT_RCV_READY) == 0);
	ch = UARTRXB0;
	return ch;
}


int uart_init()
{
	put_irq_handler(INTUART0_RX, uart_isr, NULL);
	return 0;
}


