

#include <jicama/system.h>
#include <jicama/utsname.h>
#include <jicama/process.h>
#include <jicama/console.h>
#include <jicama/devices.h>
#include <assert.h>
#include <ansi.h>
#include <string.h>
#include "s3c4510.h"

#define	USTAT_RCV_READY         0x20   /* receive data ready */ 
#define	USTAT_TXB_EMPTY         0x40   /* tx buffer empty */

int s3c2410_serial_init(void);



/**
 * This function initializes serial
 */
int s3c2410_serial_init()
{
	u32_t divisor = 0;

	divisor = 0x500; /* 19200 baudrate, 50MHz */

	UARTLCON0 = 0x03;
	UARTCONT0 = 0x09;
	UARTBRD0  = divisor;

	UARTLCON1 = 0x03;
	UARTCONT1 = 0x09;
	UARTBRD1  = divisor;

	for(divisor=0; divisor<100; divisor++);
	//kprintf("hello dpp\n");
	s3c2410_serial_puts("s3c2410_serial_init ok ...\n");
	return 0;
}

/**
 * This function read a character from serial without interrupt enable mode 
 *
 * @return the read char
 */
char s3c2410_serial_getc()
{
	while ((UARTSTAT0 & USTAT_RCV_READY) == 0);

	return UARTRXB0;
}

/**
 * This function will write a character to serial without interrupt enable mode
 *
 * @param c the char to write
 */
void s3c2410_serial_putc(const char c)
{
	/*
		to be polite with serial console add a line feed
		to the carriage return character
	*/
	if (c=='\n')s3c2410_serial_putc('\r');

	/* wait for room in the transmit FIFO */
	while(!(UARTSTAT0 & USTAT_TXB_EMPTY));

	UARTTXH0 = (u8_t)c;
}

static void s3c2410_serial_puts(const char* s)
{
	while (*s)
	{
		s3c2410_serial_putc(*s++);
	}
}

int kb_read(u8_t* buffer, int max_len);

static int ops_putchar (console_t *con, unsigned char ch, bool vt)
{
	if (vt)
	{
		return -1;
	}
	s3c2410_serial_putc(ch);
	return 0;
}

struct tty_ops ser_ops 
={
	name:		"s3c4510 serial",
	init:		s3c2410_serial_init,
	putchar:	ops_putchar,
	readbuf:	kb_read,
	char_erase:	0,
	char_attrib:0,
};

