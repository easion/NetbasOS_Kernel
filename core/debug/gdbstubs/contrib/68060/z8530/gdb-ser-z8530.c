#include "gdb.h"
#include "z8530Lib.h"
#include "sercfg.h"
/******************************************************************************
 * This file must define the following:
 * void gdb_putc ( char c );
 * char gdb_getc ( void );
 * int  init_gdb_serio(long Baud)
 *****************************************************************************/
static int serDev;

void gdb_putc ( char c )
{
	tx8530Uart(serDev,c);
}

char gdb_getc ( void )
{
	return rx8530Uart(serDev);
}


int  init_gdb_serio(long Baud)
{
	/* init MVME172 serial interface */
	serDev = init8530Uart(SCC_B_CTRL, /* cmd reg.  */
					      SCC_B_DATA,	/* data reg. */
					1,			/* channel b */
				 	SCC_CLOCK,	/* clock rate of chip */
					Baud);		/* wanted baud rate */
    return serDev;
}

/******************************************************************************
 *
 * SOME Print functions
 *
 *****************************************************************************/


void putDebugChar(
	int	c
)
{
	tx8530Uart(serDev,c);
}

int getDebugChar(void)
{
	return rx8530Uart(serDev);
}
