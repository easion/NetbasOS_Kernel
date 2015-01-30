/* -*-C-*-
 * z8530Lib.c - SCC 8530 library
 *
 *               THIS SOFTWARE IS NOT COPYRIGHTED  
 * 
 * JANZ Computer offers the following for use in the public domain.  
 * JANZ Computer makes no warranty with regard to the software or 
 * it's performance and the user accepts the software "AS IS" with 
 * all faults.
 *
 * JANZ Computer  DISCLAIMS ANY WARRANTIES, EXPRESS OR IMPLIED, WITH REGARD
 * TO THIS SOFTWARE INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * IF THE ABOVE RESTRICTIONS CONFLICT WITH LOCAL LAW, THEN YOU MUST NOT
 * USE THIS SOFTWARE.
 *
 * 1997 (c) JANZ Computer AG
 *
 * $Id: z8530lib.c,v 1.1 2001/05/18 15:54:05 bgat Exp $
 *
 * This package deals with I/O through a 8530 SCC from Zilog. It
 * currently only implements polling mode for UART.
 */

/*
 * $Log: z8530lib.c,v $
 * Revision 1.1  2001/05/18 15:54:05  bgat
 * Added mvme172 target contribution
 *
 * Revision 1.1.1.1  2000/04/06 12:35:37  wolfgang
 * first import to cvs
 *
 * Revision 1.2  1997/11/30 17:29:09  oriada
 * Added deinit8530Uart()
 * Rearranged code in init8530Uart()
 *
 * Revision 1.1  1997/11/24 18:23:29  oriada
 * Initial revision
 *
 */


/***** includes *************************************************************/

#include "z8530.h"
#include "z8530Lib.h"


/***** defines **************************************************************/

#define MAX_DEVS                 8
#define PERMITTED_BAUD_ERROR     25     /* Integers only: 20 -> 0.020 */

/***** typedefs *************************************************************/

typedef unsigned char UCHAR;

/***** globals **************************************************************/


/***** locals ***************************************************************/

static struct {
    UCHAR     *cmd_reg;
    UCHAR     *data_reg;
    int       channel;
} scc_devs[MAX_DEVS];


/***** forward declarations *************************************************/


/* -*-Func-*-
 *
 * init8530Uart - initialize Z8530 Channel for UART mode
 *
 * This functions does all initializing on a Z8530 peripheral chip, to use
 * it as UART afterwards. The parameters are set to 8N1, with a baudrate
 * as specified by <baudRate>.
 *
 * RETURNS:
 * A pos integer as handle for this port on success, or less than zero
 * if the chip couldn't be initialized. This handle needs to be used
 * on subsequent accesses to this port.
 * Negative return values denote the following failure codes:
 * .LS D 5mm
 * .li -1
 * Package cannot handle another chip (no resources left).
 * .li -2
 * Requested baudrate makes unaceptable baud-rate error.
 * .li -3
 * Requested baudrate is too high.
 * .LE
 */
int init8530Uart(
    int           cmdAddr,     /* Addr for Command Register */
    int           dataAddr,    /* Addr for Command Register */
    int           channel,     /* 8530 channel: 0=A, 1=B */
    int           clockRate,   /* BR generator Clock in Hz */
    int           baudRate     /* desired baudrate in bps */
)
{
    int              device;
    volatile int     divisor;      /* don't optimize calculations. */
    volatile int     divisor_rnd;  /* don't optimize calculations. */
    volatile int     abs_error;    /* don't optimize calculations. */
    volatile UCHAR   *cmdReg;
    volatile UCHAR   *dataReg;
    volatile UCHAR   x;
    volatile int     vi;

    /* Search for free slot in <scc_devs> structure */
    for(device=0; device<MAX_DEVS; device++){
	if( scc_devs[device].cmd_reg == 0 )
	    break;
    }

    if( device == MAX_DEVS ){
	return -1;
    }

    scc_devs[device].cmd_reg  = (UCHAR *)cmdAddr;
    scc_devs[device].data_reg = (UCHAR *)dataAddr;
    scc_devs[device].channel  = channel;

    cmdReg = scc_devs[device].cmd_reg;
    dataReg = scc_devs[device].data_reg;


    /*
     * Begin with the actual initialization.
     */

    /* Bring the access state-machine to a known state */
    x = *cmdReg;
    x = *cmdReg;
    x = *cmdReg;
    x = *cmdReg;
    x = *cmdReg;
    
    /* Channel Reset! */
    *cmdReg = Z8530_WR0_SEL_REG9;
    *cmdReg = (channel == Z8530_CHANNEL_A)
	    ? Z8530_WR9_CHA_RST  : Z8530_WR9_CHB_RST ;

    for(vi=0; vi<100; vi++) ;     /* Delay, should be 4 PCLK !?! */

#if 1
    /* Flush the RX fifo !?! */
    while( *cmdReg & Z8530_RR0_RX_READY ){
	x = *dataReg;
    }
#endif
    
    /* Reset all errors */
    *cmdReg = Z8530_WR0_RST_TX_INT;
    *cmdReg = Z8530_WR0_RST_INT;
    *cmdReg = Z8530_WR0_ERR_RST;
    *cmdReg = Z8530_WR0_RST_RX_CRC;
    *cmdReg = Z8530_WR0_RST_TX_CRC;
    *cmdReg = Z8530_WR0_RST_TX_UND;
    *cmdReg = Z8530_WR0_RST_HI_IUS;

    /* Configure but stop baud-rate generator */
    *cmdReg = Z8530_WR0_SEL_REG14;
    *cmdReg = Z8530_WR14_BR_SRC_PCLK;

    /* Configure but disable receiver */
    *cmdReg = Z8530_WR0_SEL_REG3;
    *cmdReg = Z8530_WR3_RX_8BIT;

    /* Configure but disable transmiter */
    *cmdReg = Z8530_WR0_SEL_REG5;
    *cmdReg = Z8530_WR5_TX_8BIT;

    /* Set clocking options: TxC and RxC by baudrate generator,
       TRxC pin is driven with baudrate generator output. */
    *cmdReg = Z8530_WR0_SEL_REG11;
    *cmdReg = Z8530_WR11_TXC_BR + Z8530_WR11_RXC_BR
	    + Z8530_WR11_TRXC_DIR_OUT + Z8530_WR11_TRXC_BR_GEN;

    /* Set modes: x16 clock, 1 stop bit (asychronous mode, no parity. */
    *cmdReg = Z8530_WR0_SEL_REG4;
    *cmdReg = Z8530_WR4_CLOCK_X16 + Z8530_WR4_STOP_1 + Z8530_WR4_PAR_NONE;

    /* Calculate and set baudrate (for x16 mode). We try to be smart
       with rounding.
       Additionally we check for the resulting error, but this code is
       not yet tested very well!
       Scaling makes the following assumptions:
       <clockRate> is >1E6, so that the division by (2*16) does
       not produce an unprecise result. The value is then
       scaled by 100 for better precision after the division with
       the desired baud-rate. */
    divisor      = clockRate / ( 2*16 );  /* Prescaling */
    divisor     *= 100;                   /* For accuray */
    divisor     /= baudRate;              /* Exact divisor: <xxx>.yy */
    divisor_rnd  = (divisor + 50) / 100;  /* Rounded divisor will loose */
    divisor_rnd *= 100;                   /*   two digits!              */

    /* Calculate the tolerated baud-rate error. */
    abs_error    = (((divisor * PERMITTED_BAUD_ERROR) / 100) + 5) / 10;

    /* Check for baud-rate precision violation */
    if( !(    ((divisor+abs_error) >= divisor_rnd)
	   && ((divisor-abs_error) <= divisor_rnd)) )
	return -2;     /* Cannot support this baudrate: Error to high! */

    /* ?!?: Is it correct to subtract "2" in this place, after we have
       calculated the error? */
    divisor = (divisor_rnd/100) - 2;   /* -2 comes from 8530 implementation. */

    if( divisor < 1 )
	return -3;     /* Cannot support this baudrate: Divisor to small! */

    *cmdReg = Z8530_WR0_SEL_REG12;
    *cmdReg = ( divisor     & 0xff);
    *cmdReg = Z8530_WR0_SEL_REG13;
    *cmdReg = ((divisor>>8) & 0xff);

    /* Disable all interrupts */
    *cmdReg = Z8530_WR0_SEL_REG15;
    *cmdReg = Z8530_WR15_BREAK_IE;

    /* Enable receiver */
    *cmdReg = Z8530_WR0_SEL_REG3;
    *cmdReg = Z8530_WR3_RX_8BIT
	    + Z8530_WR3_RX_EN + Z8530_WR3_RX_CRC_EN;

    /* Enable baud-rate generator */
    *cmdReg = Z8530_WR0_SEL_REG14;
    *cmdReg = Z8530_WR14_BR_EN + Z8530_WR14_BR_SRC_PCLK;

    /* Enable transmitter.
       Set RTS and DTR to active state (SPACE: +12V), to indicate
       the other side that we can accept data. */
    *cmdReg = Z8530_WR0_SEL_REG5;
    *cmdReg = Z8530_WR5_TX_8BIT
	    + Z8530_WR5_TX_EN + Z8530_WR5_RTS + Z8530_WR5_DTR;

    return(device);
}


/* -*-Func-*- internal
 *
 * deinit8530Uart - remove z8530 channel from control of this package
 *
 * This function simple removes the specified channel form the device
 * table of this package, so that it's slot can be further reused!
 *
 * The chip itself is left as is, nothing is deinitialzied.
 *
 * RETURNS:
 * Zero on success, less than zero otherwise.
 */
int deinit8530Uart(
    int      dev        /* UART device handle */
)
{
    scc_devs[dev].cmd_reg  = 0;
    scc_devs[dev].data_reg = 0;
    scc_devs[dev].channel  = 0;

    return(0);
}


/* -*-Func-*-
 *
 * tx8530Uart - Send a single character via the UART.
 *
 * This tries to send a character, and will hang util this could
 * be done.
 *
 * RETRUNS:
 * Zero if data was send successfully, <0 if tx8530Uart() returns
 * because of an error.
 */
int tx8530Uart(
    int      dev,       /* UART device handle */
    int      c          /* character to be send to port */
)
{
    UCHAR   *cmdReg  = scc_devs[dev].cmd_reg;
    UCHAR   *dataReg = scc_devs[dev].data_reg;
	UCHAR	stat;
    
	do {
		*cmdReg = 0;
		stat = *cmdReg;
 	} while( (stat & 0x04) == 0 ) ;
    *dataReg = (UCHAR) c;
    return 0;
}


/* -*-Func-*-
 *
 * txStat8530Uart - Probe UART transmitt buffer for availability.
 *
 * RETURNS:
 * Non zero if the transmitt buffer is empty, zero otherwise.
 */
int txStat8530Uart(
    int      dev        /* UART device handle */
)
{
    UCHAR   *cmdReg  = scc_devs[dev].cmd_reg;

    return( (int)(*cmdReg & Z8530_RR0_TX_EMPTY) ) ;
}


/* -*-Func-*-
 *
 * rx8530Uart - Receive a character from the UART
 *
 * With this function a single character is read from the port.
 * It will hang until a character could be read.
 *
 * RETURNS:
 * The character that was read, <0 if rx8530Uart() returns because
 * of an error.
 */
int rx8530Uart(
    int      dev        /* UART device handle */
)
{
    UCHAR   *cmdReg  = scc_devs[dev].cmd_reg;
    UCHAR   *dataReg = scc_devs[dev].data_reg;
    UCHAR   x;

    while( !(*cmdReg & Z8530_RR0_RX_READY) ) ;
    x = *dataReg;
    
    return((int)(x));
}

/* -*-Func-*-
 *
 * rxStat8530Uart - Probe UART for a received character
 *
 * RETURNS:
 * Zero if no character is available at the time, non zero otherwise.
 */
int rxStat8530Uart(
    int      dev        /* UART device handle */
)
{
    volatile UCHAR   *cmdReg  = scc_devs[dev].cmd_reg;
    
    return( (int)(*cmdReg & Z8530_RR0_RX_READY) ) ;
}

