/* -*-C-*-
 * z8530.h - Register definitions for Zilog 8530 SCC
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
 * $Id: z8530.h,v 1.1 2001/05/18 15:54:05 bgat Exp $
 */

/*
 * $Log: z8530.h,v $
 * Revision 1.1  2001/05/18 15:54:05  bgat
 * Added mvme172 target contribution
 *
 * Revision 1.1.1.1  2000/04/06 12:35:37  wolfgang
 * first import to cvs
 *
 * Revision 1.2  1997/11/30 17:30:23  oriada
 * Changed the names of some macros, to be more meaningful
 * Added documentation to a lot of macros
 *
 * Revision 1.1  1997/11/24 18:23:47  oriada
 * Initial revision
 *
 */


#ifndef z8530_DEFINED
#define z8530_DEFINED
#if __cplusplus
extern "C" {
#endif


/***** defines **************************************************************/

/*
 * WR0: Command Register, Pointer Register
 */
#define Z8530_WR0_SEL_REG0	0x00    /* r/w */
#define Z8530_WR0_SEL_REG1      0x01    /* r/w */
#define Z8530_WR0_SEL_REG2      0x02    /* r/w */
#define Z8530_WR0_SEL_REG3      0x03    /* r/w */
#define Z8530_WR0_SEL_REG4      0x04    /* w */
#define Z8530_WR0_SEL_REG5      0x05    /* w */
#define Z8530_WR0_SEL_REG6      0x06    /* w */
#define Z8530_WR0_SEL_REG7      0x07    /* w */
#define Z8530_WR0_SEL_REG8      0x08    /* r/w */
#define Z8530_WR0_SEL_REG9      0x09    /* w */
#define Z8530_WR0_SEL_REG10     0x0a    /* r/w */
#define Z8530_WR0_SEL_REG11     0x0b    /* w */
#define Z8530_WR0_SEL_REG12     0x0c    /* r/w */
#define Z8530_WR0_SEL_REG13     0x0d    /* r/w */
#define Z8530_WR0_SEL_REG14     0x0e    /* w */
#define Z8530_WR0_SEL_REG15     0x0f    /* r/w */
#define Z8530_WR0_NULL_CODE     0x00
#define Z8530_WR0_RST_INT       0x10
#define Z8530_WR0_SEND_ABORT    0x18
#define Z8530_WR0_EN_INT_RX     0x20
#define Z8530_WR0_RST_TX_INT    0x28
#define Z8530_WR0_ERR_RST       0x30
#define Z8530_WR0_RST_HI_IUS    0x38
#define Z8530_WR0_RST_RX_CRC    0x40
#define Z8530_WR0_RST_TX_CRC    0x80
#define Z8530_WR0_RST_TX_UND    0xc0


/*
 * WR1: transmit/receive interrupt, data transfer mode
 */
#define Z8530_WR1_EXT_INT_EN    0x01    /* Enable external/status interrupt */
#define Z8530_WR1_TX_INT_EN     0x02    /* Enable transmit interrupt */
#define Z8530_WR1_PAR           0x03    /* Parity is special condition */
#define Z8530_WR1_RX_INT_DIS    0x00    /* Disable Rx interrupt */
#define Z8530_WR1_RX_INT_FIRST  0x08    /* En. Rx int for first char */
#define Z8530_WR1_RX_INT_EN     0x10    /* En. Rx int for all chars */
#define Z8530_WR1_RX_INT_SPEC   0x18    /* En. Rx int on special cond. only */

    
/*
 * WR3: receive parameters and control
 */
#define Z8530_WR3_RX_EN         0x01    /* Enable receiver */
#define Z8530_WR3_ADR_SEARCH    0x04    /* Address Seach mode (SDLC) */
#define Z8530_WR3_RX_CRC_EN     0x08    /* Rx CRC enable */
#define Z8530_WR3_RX_5BIT       0x00
#define Z8530_WR3_RX_7BIT       0x40
#define Z8530_WR3_RX_6BIT       0x80
#define Z8530_WR3_RX_8BIT       0xc0

    
/*
 * WR4: transmit/receive misc. parameters and modes
 */
#define Z8530_WR4_PAR_EVEN      0x03    /* Enable even parity */
#define Z8530_WR4_PAR_ODD       0x01    /* Enable odd parity */
#define Z8530_WR4_PAR_NONE      0x00    /* Disable parity */
#define Z8530_WR4_SYNC_MODE     0x00    /* Synchronous Mode */
#define Z8530_WR4_STOP_1        0x04    /* Asynchr. mode: 1 stop bit */
#define Z8530_WR4_STOP_2        0x0c    /* Asynchr. mode: 2 stop bit */
#define Z8530_WR4_CLOCK_X1      0x00    /* X1  clock mode */
#define Z8530_WR4_CLOCK_X16     0x40    /* X16 clock mode */
#define Z8530_WR4_CLOCK_X32     0x80    /* X32 clock mode */
#define Z8530_WR4_CLOCK_X64     0xc0    /* X64 clock mode */

    
/*
 * WR5: transmit parameters and control
 */
#define Z8530_WR5_TX_CRC_EN     0x01    /* Transmitter CRC enable */
#define Z8530_WR5_RTS           0x02    /* RTS control bit */
#define Z8530_WR5_RTS_LOW       0x02    /* Set RTS pin to low state */
#define Z8530_WR5_RTS_HIGH      0x00    /* Set RTS pin to high state */
#define Z8530_WR5_TX_CRC_SDLC   0x00    /* CRC uses SDLC polynominal */
#define Z8530_WR5_TX_CRC_CRC16  0x04    /* CRC uses CRC-16 polynominal */
#define Z8530_WR5_TX_EN         0x08    /* Enable Transmitter */
#define Z8530_WR5_SEND_BREAK    0x10    /* Send break conditions */
#define Z8530_WR5_TX_5BIT       0x00
#define Z8530_WR5_TX_6BIT       0x40
#define Z8530_WR5_TX_7BIT       0x20
#define Z8530_WR5_TX_8BIT       0x60
#define Z8530_WR5_DTR           0x80    /* DTR control bit */
#define Z8530_WR5_DTR_LOW       0x80    /* Set DTR pin to low state */
#define Z8530_WR5_DTR_HIGH      0x00    /* Set DTR pin to high state */

    
/*
 * WR9: misc. transmit/receive control bits
 */
#define Z8530_WR9_VIS           0x01    /* Vector includes status */
#define Z8530_WR9_NV            0x02    /* Non vectored interrupt */
#define Z8530_WR9_DLC           0x04    /* Disable lower chain: IEO set low */
#define Z8530_WR9_MIE           0x08    /* Master interrupt enable */
#define Z8530_WR9_VIS_LOW       0x00    /* Status in bits 3..1 of vector */
#define Z8530_WR9_VIS_HIGH      0x10    /* Status in bits 4..6 of vector */
#define Z8530_WR9_SOFT_INTACK   0x20    /* Enable software int-ack [!NMOS] */
#define Z8530_WR9_NO_RST        0x00
#define Z8530_WR9_CHB_RST       0x40    /* Reset B channel */
#define Z8530_WR9_CHA_RST       0x80    /* Reset A channel */
#define Z8530_WR9_HRD_RST       0xc0    /* Hardware reset */

/*
 * WR10: misc. transmit/receive control bits
 */

    
/*
 * WR11: clock mode control
 */
#define Z8530_WR11_TRXC_XTAL    0x00    /* TRxC pin = XTAL output */
#define Z8530_WR11_TRXC_TXC     0x01    /* TRxC pin = Transmit clock */
#define Z8530_WR11_TRXC_BR_GEN  0x02    /* TRxC pin = BR-generator output */
#define Z8530_WR11_TRXC_DPLL    0x03    /* TRxC pin = DPLL output */
#define Z8530_WR11_TRXC_DIR_IN  0x00    /* TRxC pin is input */
#define Z8530_WR11_TRXC_DIR_OUT 0x04    /* TRxC pin is output */
#define Z8530_WR11_TXC_RTXC     0x00    /* Transmit clock from RTxC pin */
#define Z8530_WR11_TXC_TRxC     0x08    /* Transmit clock from TRxC pin */
#define Z8530_WR11_TXC_BR       0x10    /* Transmit clock from BR-gen */
#define Z8530_WR11_TXC_DPLL     0x18    /* Transmit clock from DPLL */
#define Z8530_WR11_RXC_RTXC     0x00    /* Receive clock from RTxC pin */
#define Z8530_WR11_RXC_TRxC     0x20    /* Receive clock from TRxC pin */
#define Z8530_WR11_RXC_BR       0x40    /* Receive clock from BR-gen */
#define Z8530_WR11_RXC_DPLL     0x60    /* Receive clock from DPLL */
#define Z8530_WR11_RTXC_XTAL    0x80    /* RTxC pin has an quarz connected */

    
/*
 * WR14: misc. control bits
 */
#define Z8530_WR14_BR_EN        0x01    /* Enable BR generator */
#define Z8530_WR14_BR_SRC_RTXC  0x00    /* BR generator driver by RTXC */
#define Z8530_WR14_BR_SRC_PCLK  0x02    /* BR generator driven by PCLK */
#define Z8530_WR14_DTR_FUNC     0x04    /* DTR pin has request functionality */
#define Z8530_WR14_AUTO_ECHO    0x08    /* Enable auto echo mode: RxD->TxD */
#define Z8530_WR14_EN_LOOPBACK  0x10    /* Enable loopback mode: TxD->RxD */
#define Z8530_WR14_DPLL_SEARCH  0x20    /* Enter DPLL search mode */
#define Z8530_WR14_DPLL_RESET   0x40    /* Reset missing clock */
#define Z8530_WR14_DPLL_DISABLE 0x60    /* Disable DPLL */
#define Z8530_WR14_DPLL_SRC_BR  0x80    /* DPLL source = BR generator */
#define Z8530_WR14_DPLL_SRC_RTXC 0xa0   /* DPLL source = RTxC pin */
#define Z8530_WR14_DPLL_FM      0xc0    /* DPLL in FM mode (or Manchester) */
#define Z8530_WR14_DPLL_NRZI    0xc0    /* DPLL in NRZI mode */

    
/*
 * WR15: interrupt enable
 */
#define Z8530_WR15_ZERO_CNT_IE  0x02    /* Zero count interrupt enable */
#define Z8530_WR15_DCD_IE       0x08    /* DCD interrupt enable */
#define Z8530_WR15_SYNC_IE      0x10    /* Sync/Hunt interrupt enable */
#define Z8530_WR15_CTS_IE       0x20    /* CTS interrupt enable */
#define Z8530_WR15_TX_UND_IE    0x40    /* Tx underrun interrupt enable */
#define Z8530_WR15_BREAK_IE     0x80    /* Break/Abort interrupt enable */


/*
 * RR0: transmit/receive buffer status, external status
 */
#define Z8530_RR0_RX_READY      0x01    /* Receive character available */
#define Z8530_RR0_TX_EMPTY      0x04    /* Transmit buffer empty */
#define Z8530_RR0_DCD           0x08    /* DCD pin state bit */
#define Z8530_RR0_CTS           0x20    /* CTS pin state bit */
#define Z8530_RR0_TX_UNDERRUN   0x40    /* Transmit underrun */
#define Z8530_RR0_BREAK         0x80    /* Break/Abort detected */
    
/*
 * RR1: 
 */
#define Z8530_RR1_ALL_SEND      0x01    /* All chars sent */
#define Z8530_RR1_PAR_ERR       0x10    /* Parity error */
#define Z8530_RR1_RX_OVERRUN    0x20    /* Receive overrun */
#define Z8530_RR1_FRAME_ERR     0x40    /* Framing Error */
#define Z8530_RR1_CRC_ERR       0x40    /* CRC Error */
#define Z8530_RR1_EOF           0x80    /* End of frame (SDLC) */
    
    
/***** typedefs *************************************************************/

    
/***** function declarations  ***********************************************/

extern int init8530Uart(int, int, int, int, int);
extern int tx8530Uart(int, int);
extern int txStat8530Uart(int);
extern int rx8530Uart(int);
extern int rxStat8530Uart(int);


#if __cplusplus
}
#endif
#endif /* z8530_DEFINED */
