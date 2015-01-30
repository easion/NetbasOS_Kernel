/*
 * (C) Copyright 2002
 * Lineo, Inc <www.lineo.com>
 * Bernhard Kuhn <bkuhn@lineo.com>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Alex Zuepke <azu@sysgo.de>
 *
 * Copyright (C) 1999 2000 2001 Erik Mouw (J.A.K.Mouw@its.tudelft.nl)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <jicama/system.h>
#include <jicama/utsname.h>
#include <jicama/process.h>
#include <jicama/console.h>
#include <jicama/devices.h>
#include "hardware.h"

/* ggi thunder */
const AT91PS_USART us = (AT91PS_USART) AT91C_BASE_DBGU;
#define CONFIG_BAUDRATE 115200

void serial_setbrg (void)
{
	DECLARE_GLOBAL_DATA_PTR;
	int baudrate;

	//if ((baudrate = gd->bd->bi_baudrate) <= 0)
	baudrate = CONFIG_BAUDRATE;
	us->US_BRGR = CFG_AT91C_BRGR_DIVISOR;	/* hardcode so no __divsi3 */
}

__local int serial_init (void)
{
	/* make any port initializations specific to this port */
	*AT91C_PIOA_PDR = AT91C_PA31_DTXD | AT91C_PA30_DRXD;	/* PA 31 & 30 */
	*AT91C_PMC_PCER = 1 << AT91C_ID_SYS;	/* enable clock */
	serial_setbrg ();

	us->US_CR = AT91C_US_RSTRX | AT91C_US_RSTTX;
	us->US_CR = AT91C_US_RXEN | AT91C_US_TXEN;
	us->US_MR =
		(AT91C_US_CLKS_CLOCK | AT91C_US_CHRL_8_BITS |
		 AT91C_US_PAR_NONE | AT91C_US_NBSTOP_1_BIT);
	us->US_IMR = ~0ul;

	serial_puts("hello world!\n");
	return (0);
}

void serial_putc (const char c)
{
	if (c == '\n')
		serial_putc ('\r');
	while ((us->US_CSR & AT91C_US_TXRDY) == 0);
	us->US_THR = c;
}

void serial_puts (const char *s)
{
	while (*s) {
		serial_putc (*s++);
	}
}

int serial_getc (void)
{
	while ((us->US_CSR & AT91C_US_RXRDY) == 0);
	return us->US_RHR;
}

int serial_tstc (void)
{
	return ((us->US_CSR & AT91C_US_RXRDY) == AT91C_US_RXRDY);
}

static int ops_putchar (console_t *con, unsigned char ch, bool vt)
{
	if (vt)
	{
		return -1;
	}
	serial_putc(ch);
	return 0;
}

struct tty_ops ser_ops
={
	name:		"at91rm9200 uart",
	init:		serial_init,
	putchar:	ops_putchar,
	readbuf:	kb_read,
	char_erase:	0,
	char_attrib:0,
	exit:NULL,
};

