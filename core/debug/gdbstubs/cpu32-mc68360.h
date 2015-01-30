/* 
  Copyright (c) 2001 by      William A. Gatliff
  All rights reserved.      bgat@billgatliff.com

  See the file COPYING for details.

  This file is provided "as-is", and without any express or implied
  warranties, including, without limitation, the implied warranties of
  merchantability and fitness for a particular purpose.

  The author welcomes feedback regarding this file.

  **

  Generic MC68360 support, generously funded by Michael Dorin of EDI
  Enterprises, Inc.  TODO: email  Thanks, Mike!

  ARCH=CPU32 TARGET=MC68360 IO=SMC1UART - a 68360 using SMC1 in UART mode
  ARCH=CPU32 TARGET=MC68360 IO=???      - add one here

  **

  Members of the CPU32 family are so consistent, it's tough to
  discriminate between them for purposes of gdbstubs organization.
  This file is "specific" to the '360 because it's all I have to test
  with; it is likely that it would work on other, similar chips as
  well.  When this is confirmed, update this comment and consider
  renaming this file.

*/


/*
** How internal Buffer Descriptor RAM is used
** Note:
**    Max of 256 Buffer descriptors.
*/

#define	NUM_SCC_RX_BD			16
#define	NUM_SCC_TX_BD			16

#define	SCC1_RX_BD_BASE			 0*8	/* 16 Descrs  0..15	*/
#define	SCC1_TX_BD_BASE			16*8	/* 16 Descrs 16..31	*/

#define	SCC2_RX_BD_BASE			32*8	/* 16 Descrs 32..47	*/
#define	SCC2_TX_BD_BASE			48*8	/* 16 Descrs 48..63	*/

#define	SCC3_RX_BD_BASE			64*8	/* 16 Descrs 64..79	*/
#define	SCC3_TX_BD_BASE			80*8	/* 16 Descrs 80..95	*/

#define	SCC4_RX_BD_BASE			96*8	/* 16 Descrs 96..111	*/
#define	SCC4_TX_BD_BASE			112*8	/* 16 Descrs 112..127	*/

#define	NUM_SMC_1_RX_BD			16
#define	NUM_SMC_1_TX_BD			16
#define	SMC_1_RX_BD_BASE		128*8	/* 16 Descrs 128..143	*/
#define	SMC_1_TX_BD_BASE		144*8	/* 16 Descrs 144..159	*/

#define	NUM_SMC_2_RX_BD			16
#define	NUM_SMC_2_TX_BD			16
#define	SMC_2_RX_BD_BASE		160*8	/* 16 Descrs 160..175	*/
#define	SMC_2_TX_BD_BASE		176*8	/* 16 Descrs 176..191	*/

#define	SPI_RX_BD_BASE			192*8	/* 1 Descr  192		*/
#define	SPI_TX_BD_BASE			193*8	/* 1 Descr  193		*/



/*****************************************************************
        Base Address Register (address 0x2ff00)
*****************************************************************/

#define WRITE_BAR(x)    (*((long *)0x2ff00) = x)


/*****************************************************************
        Command Register
*****************************************************************/

/*
**      bit fields within command register 
*/
#define SOFTWARE_RESET          0x8000
#define CMD_OPCODE                      0x0f00
#define CMD_CHANNEL                     0x00f0
#define CMD_FLAG                        0x0001

/*
**      general command opcodes 
*/
#define INIT_RXTX_PARAMS        0x0000
#define INIT_RX_PARAMS          0x0100
#define INIT_TX_PARAMS          0x0200
#define ENTER_HUNT_MODE         0x0300
#define STOP_TX                         0x0400
#define GR_STOP_TX                      0x0500
#define RESTART_TX                      0x0600
#define CLOSE_RX_BD                     0x0700
#define SET_ENET_GROUP          0x0800
#define RESET_ENET_GROUP        0x0900

/*
**      SMC (GCI) commands 
*/
#define GCI_ABORT                       0x0400
#define GCI_TIMEOUT                     0x0500

/*
**      Timer commands 
*/
#define SET_TIMER                       0x0800


/*
****************************************************************
**      General SCC mode register (GSMR)
****************************************************************
*/
/*
**      GSMRA
**              SCC modes
*/
#define HDLC_PORT               0x0
#define HDLC_BUS                0x1
#define APPLE_TALK              0x2
#define SS_NO7                  0x3
#define UART                    0x4
#define PROFI_BUS               0x5
#define ASYNC_HDLC              0x6
#define V14                             0x7
#define BISYNC_PORT             0x8
#define DDCMP_PORT              0x9
#define ETHERNET_PORT   0xc

/*
** diagnostics bits
*/
#define NORMAL                  (0<<6)  /* normal operation with CTS and CD */
/* automatically controlled by SCC */
#define LOOPBACK                (1<<6)  /* connect tx to rx */
#define SERIAL_ECHO             (2<<6)  /* connect rx to tx */
#define LOOPBACK_ECHO   (3<<6)  /* loopback and echo */

/*
**      common mode register bits 
*/
#define MODE            0x0000000f

/*
**      GSMRB 
*/
#define TCRC    0x00008000
#define TRX             0x00001000
#define TTX             0x00000800

/*
**      hdlc mode register
*/
#define CRC32           0x0800


/*
**      ethernet mode register 
*/
#define IAM             0x1000  /* individual address mode */
#define PRO             0x0200  /* promiscuous */
#define BRO             0x0100  /* broadcast address */

/*
** tx parity (SCC asynchronous)
*/
#define ODD_PARITY      (0<<14)
#define EVEN_PARITY     (1<<14)
#define LOW_PARITY      (2<<14)
#define HIGH_PARITY     (3<<14)

/*
** UART options
*/
#define UOPT            0x0c00
#define OPT_NORMAL      (0<<10)
#define OPT_MULTD       (1<<10) /* multi-drop, no auto */
#define OPT_DDCMP       (2<<10) /* ddcmp */
#define OPT_AUTO        (3<<10) /* mutli-drop with automatic */
/* address recognition */
#define UART_FRZ        0x0200


/*
**      bisync mode register 
*/
#define BI_CRC          0x0800  /* BI_CRC set than LRC is used */
#define BI_BCS          0x0200
#define BI_TRANSP       0x0100


/*
****************************************************************
**      TODR (Transmit on demand) Register
****************************************************************
*/
#define TODR_TOD        0x8000  /* Transmit on demand */

/*
****************************************************************
**      SMC mode register
****************************************************************
*/

/*
** defines for SMC modes
*/
#define SMC_MODE                0x0030  /* SMC MODE */
#define SMC_GCI_MODE    0x0000  /* SMC MODE: GCI or SCIT support */
#define SMC_UART                0x0020  /* SMC MODE: UART */
#define SMC_TRANSPARENT 0x0030  /* SMC MODE: Transparent */

/*
***************************************************************
**      SPI registers
***************************************************************
*/

#define MULTI_MASTER_ERR        ABORT /* Multi master error event */


/*
****************************************************************
**      interrupt registers
****************************************************************
*/

#define INTR_PIO_PC0    0x80000000      /* parallel i/o C bit 0 */
#define INTR_SCC1               0x40000000      /* SCC port 1 */
#define INTR_SCC2               0x20000000      /* SCC port 2 */
#define INTR_SCC3               0x10000000      /* SCC port 3 */
#define INTR_SCC4               0x08000000      /* SCC port 4 */
#define INTR_PIO_PC1    0x04000000      /* parallel i/o C bit 1 */
#define INTR_TIMER1             0x02000000      /* timer 1 */
#define INTR_PIO_PC2    0x01000000      /* parallel i/o C bit 2 */
#define INTR_PIO_PC3    0x00800000      /* parallel i/o C bit 3 */
#define INTR_SDMA_BERR  0x00400000      /* SDMA channel bus error */
#define INTR_DMA1               0x00200000      /* idma 1 */
#define INTR_DMA2               0x00100000      /* idma 2 */
#define INTR_TIMER2             0x00040000      /* timer 2 */
#define INTR_CP_TIMER   0x00020000      /* CP timer */
#define INTR_PIP_STATUS 0x00010000      /* PIP status */
#define INTR_PIO_PC4    0x00008000      /* parallel i/o C bit 4 */
#define INTR_PIO_PC5    0x00004000      /* parallel i/o C bit 5 */
#define INTR_TIMER3             0x00001000      /* timer 3 */
#define INTR_PIO_PC6    0x00000800      /* parallel i/o C bit 6 */
#define INTR_PIO_PC7    0x00000400      /* parallel i/o C bit 7 */
#define INTR_PIO_PC8    0x00000200      /* parallel i/o C bit 8 */
#define INTR_TIMER4             0x00000080      /* timer 4 */
#define INTR_PIO_PC9    0x00000040      /* parallel i/o C bit 9 */
#define INTR_SPI                0x00000020      /* SPI */
#define INTR_SMC1               0x00000010      /* SMC 1 */
#define INTR_SMC2               0x00000008      /* SMC 2 */
#define INTR_PIO_PC10   0x00000004      /* parallel i/o C bit 10 */
#define INTR_PIO_PC11   0x00000002      /* parallel i/o C bit 11 */
#define INTR_ERR                0x00000001      /* error */



/*
****************************************************************
**        Spi register setting    
****************************************************************
*/

#define NORMAL_MODE     0x18
#define SPI_START       0x80 /* STR bit in SPCOM, start transmit */
#define CLEAR_SPIE      0xff
#define SPI_MODE                0x50 /* Cp spi mode for init Rx Tx */
 
/*
**      event bits in SPIE 
*/
#define MME             0x20
#define TXE             0x10
#define BSY             0x04
#define TXB             0x02
#define RXB             0x01
 
/*
** transmit BD's status (For Spi)
*/
#define T_L             0x0800 /* Last bit of frame */
 
/*
****************************************************************
**      chip select option register
****************************************************************
*/
#define DTACK           0xe000
#define ADR_MASK        0x1ffc
#define RW_MASK         0x0002
#define FC_MASK         0x0001


/*
****************************************************************
**         Port B Pin Assignment 
****************************************************************
*/
#define PB10TO17        0x0003FC00
#define PB1TO3          0x0000000E
#define PB0             0x00000001

/*
*******************************************************************************
**
** Definitions of the parameter area RAM.
**
*******************************************************************************
*/

/*
** HDLC parameter RAM
**      Note that different structures are overlaid
**      at the same offsets for the different modes
**      of operation.
*/
typedef struct 
{
  /*
  ** SCC parameter RAM
  */
  U16   rbase;          /* RX BD base address */
  U16   tbase;          /* TX BD base address */
  U8    rfcr;           /* Rx function code */
  U8    tfcr;           /* Tx function code */
  U16   mrblr;          /* Rx buffer length */
  U32   rstate;         /* Rx internal state */
  U32   rptr;           /* Rx internal data pointer */
  U16   rbptr;          /* rb BD Pointer */
  U16   rcount;         /* Rx internal byte count */
  U32   rtemp;          /* Rx temp */
  U32   tstate;         /* Tx internal state */
  U32   tptr;           /* Tx internal data pointer */
  U16   tbptr;          /* Tx BD pointer */
  U16   tcount;         /* Tx byte count */
  U32   ttemp;          /* Tx temp */
  U32   rcrc;           /* temp receive CRC */
  U32   tcrc;           /* temp transmit CRC */

  /*
        ** HDLC specific parameter RAM
        */
  U8    RESERVED1[4];   /* Reserved area */
  U32   c_mask;         /* CRC constant */
  U32   c_pres;         /* CRC preset */
  U16   disfc;          /* discarded frame counter */
  U16   crcec;          /* CRC error counter */
  U16   abtsc;          /* abort sequence counter */
  U16   nmarc;          /* nonmatching address rx cnt */
  U16   retrc;          /* frame retransmission cnt */
  U16   mflr;           /* maximum frame length reg */
  U16   max_cnt;        /* maximum length counter */
  U16   rfthr;          /* received frames threshold */
  U16   rfcnt;          /* received frames count */
  U16   hmask;          /* user defined frm addr mask */
  U16   haddr1; /* user defined frm address 1 */
  U16   haddr2; /* user defined frm address 2 */
  U16   haddr3; /* user defined frm address 3 */
  U16   haddr4; /* user defined frm address 4 */
  U16   tmp;    /* temp */
  U16   tmp_mb; /* temp */
} HDLC_PRAM;


/*
**      UART parameter RAM
*/
typedef struct 
{
  /*
  ** SCC parameter RAM
  */
  U16   rbase;          /* RX BD base address */
  U16   tbase;          /* TX BD base address */
  U8    rfcr;           /* Rx function code */
  U8    tfcr;           /* Tx function code */
  U16   mrblr;          /* Rx buffer length */
  U32   rstate;         /* Rx internal state */
  U32   rptr;           /* Rx internal data pointer */
  U16   rbptr;          /* rb BD Pointer */
  U16   rcount;         /* Rx internal byte count */
  U32   rx_temp;        /* Rx temp */
  U32   tstate;         /* Tx internal state */
  U32   tptr;           /* Tx internal data pointer */
  U16   tbptr;          /* Tx BD pointer */
  U16   tcount;         /* Tx byte count */
  U32   ttemp;          /* Tx temp */
  U32   rcrc;           /* temp receive CRC */
  U32   tcrc;           /* temp transmit CRC */

  /*
        ** UART specific parameter RAM
        */
  U8    RESERVED1[8];   /* Reserved area */
  U16   max_idl;        /* maximum idle characters */
  U16   idlc;           /* rx idle counter (internal) */
  U16   brkcr;          /* break count register */

  U16   parec;          /* Rx parity error counter */
  U16   frmer;          /* Rx framing error counter */
  U16   nosec;          /* Rx noise counter */
  U16   brkec;          /* Rx break character counter */
  U16   brkln;          /* Reaceive break length */

  U16   uaddr1;         /* address character 1 */
  U16   uaddr2;         /* address character 2 */
  U16   rtemp;          /* temp storage */
  U16   toseq;          /* Tx out of sequence char */
  U16   cc[8];          /* Rx control characters */
  U16   rccm;           /* Rx control char mask */
  U16   rccr;           /* Rx control char register */
  U16   rlbc;           /* Receive last break char */
} UART_PRAM;

/*
** bits in uart control characters table
*/
#define CC_INVALID      0x8000          /* control character is valid */
#define CC_REJ          0x4000          /* don't store char in buffer */
#define CC_CHAR         0x00ff          /* control character */



/*
**      BISYNC parameter RAM
*/

typedef struct 
{
  /*
  ** SCC parameter RAM
  */
  U16   rbase;          /* RX BD base address */
  U16   tbase;          /* TX BD base address */
  U8    rfcr;           /* Rx function code */
  U8    tfcr;           /* Tx function code */
  U16   mrblr;          /* Rx buffer length */
  U32   rstate;         /* Rx internal state */
  U32   rptr;           /* Rx internal data pointer */
  U16   rbptr;          /* rb BD Pointer */
  U16   rcount;         /* Rx internal byte count */
  U32   rtemp;          /* Rx temp */
  U32   tstate;         /* Tx internal state */
  U32   tptr;           /* Tx internal data pointer */
  U16   tbptr;          /* Tx BD pointer */
  U16   tcount;         /* Tx byte count */
  U32   ttemp;          /* Tx temp */
  U32   rcrc;           /* temp receive CRC */
  U32   tcrc;           /* temp transmit CRC */

  /*
        ** BISYNC specific parameter RAM
        */
  U8    RESERVED1[4];   /* Reserved area */
  U32   crcc;           /* CRC Constant Temp Value */
  U16   prcrc;          /* Preset Receiver CRC-16/LRC */
  U16   ptcrc;          /* Preset Transmitter CRC-16/LRC */
  U16   parec;          /* Receive Parity Error Counter */
  U16   bsync;          /* BISYNC SYNC Character */
  U16   bdle;           /* BISYNC DLE Character */
  U16   cc[8];          /* Rx control characters */
  U16   rccm;           /* Receive Control Character Mask */
} BISYNC_PRAM;

/*
**      IOM2 parameter RAM
**      (overlaid on tx bd[5] of SCC channel[2])
*/
typedef struct 
{
  U16   ci_data;        /* ci data */
  U16   monitor_data;   /* monitor data */
  U16   tstate;         /* transmitter state */
  U16   rstate;         /* receiver state */
} IOM2_PRAM;

/*
**      SPI/SMC parameter RAM
**      (overlaid on tx bd[6,7] of SCC channel[2])
*/
typedef struct 
{
  U16   rbase;          /* Rx BD Base Address */
  U16   tbase;          /* Tx BD Base Address */
  U8    rfcr;           /* Rx function code */
  U8    tfcr;           /* Tx function code */
  U16   mrblr;          /* Rx buffer length */
  U32   rstate;         /* Rx internal state */
  U32   rptr;           /* Rx internal data pointer */
  U16   rbptr;          /* rb BD Pointer */
  U16   rcount;         /* Rx internal byte count */
  U32   rtemp;          /* Rx temp */
  U32   tstate;         /* Tx internal state */
  U32   tptr;           /* Tx internal data pointer */
  U16   tbptr;          /* Tx BD pointer */
  U16   tcount;         /* Tx byte count */
  U32   ttemp;          /* Tx temp */
}SPI_PRAM;

#define SPI_R   0x8000          /* Ready bit in BD */

/*
**      SMC UART parameter RAM
*/
typedef struct 
{
  U16   rbase;          /* Rx BD Base Address */
  U16   tbase;          /* Tx BD Base Address */
  U8    rfcr;           /* Rx function code */
  U8    tfcr;           /* Tx function code */
  U16   mrblr;          /* Rx buffer length */
  U32   rstate;         /* Rx internal state */
  U32   rptr;           /* Rx internal data pointer */
  U16   rbptr;          /* rb BD Pointer */
  U16   rcount;         /* Rx internal byte count */
  U32   rtemp;          /* Rx temp */
  U32   tstate;         /* Tx internal state */
  U32   tptr;           /* Tx internal data pointer */
  U16   tbptr;          /* Tx BD pointer */
  U16   tcount;         /* Tx byte count */
  U32   ttemp;          /* Tx temp */
  U16   max_idl;        /* Maximum IDLE Characters */
  U16   idlc;           /* Temporary IDLE Counter */
  U16   brkln;          /* Last Rx Break Length */
  U16   brkec;          /* Rx Break Condition Counter */
  U16   brkcr;          /* Break Count Register (Tx) */
  U16   r_mask;         /* Temporary bit mask */
} SMC_UART_PRAM;

/*
**      SMC Transparent parameter RAM
*/
typedef struct 
{
  U16   rbase;          /* Rx BD Base Address */
  U16   tbase;          /* Tx BD Base Address */
  U8    rfcr;           /* Rx function code */
  U8    tfcr;           /* Tx function code */
  U16   mrblr;          /* Rx buffer length */
  U32   rstate;         /* Rx internal state */
  U32   rptr;           /* Rx internal data pointer */
  U16   rbptr;          /* rb BD Pointer */
  U16   rcount;         /* Rx internal byte count */
  U32   rtemp;          /* Rx temp */
  U32   tstate;         /* Tx internal state */
  U32   tptr;           /* Tx internal data pointer */
  U16   tbptr;          /* Tx BD pointer */
  U16   tcount;         /* Tx byte count */
  U32   ttemp;          /* Tx temp */
  U16  reserved[5];     /* Reserved */
} SMC_TRNSP_PRAM;

typedef struct 
{
  U16   ibase;          /* IDMA BD Base Address */
  U16   ibptr;  /* IDMA buffer descriptor pointer */
  U32   istate; /* IDMA internal state */
  U32   itemp;  /* IDMA temp */
} IDMA_PRAM;

/*
** ETHERNET parameter RAM
*/
typedef struct 
{
  /*
  ** SCC parameter RAM
  */
  U16   rbase;          /* RX BD base address */
  U16   tbase;          /* TX BD base address */
  U8    rfcr;           /* Rx function code */
  U8    tfcr;           /* Tx function code */
  U16   mrblr;          /* Rx buffer length */
  U32   rstate;         /* Rx internal state */
  U32   rptr;           /* Rx internal data pointer */
  U16   rbptr;          /* rb BD Pointer */
  U16   rcount;         /* Rx internal byte count */
  U32   rtemp;          /* Rx temp */
  U32   tstate;         /* Tx internal state */
  U32   tptr;           /* Tx internal data pointer */
  U16   tbptr;          /* Tx BD pointer */
  U16   tcount;         /* Tx byte count */
  U32   ttemp;          /* Tx temp */
  U32   rcrc;           /* temp receive CRC */
  U32   tcrc;           /* temp transmit CRC */

  /*
        ** ETHERNET specific parameter RAM
        */
  U32   c_pres;         /* preset CRC */
  U32   c_mask;         /* constant mask for CRC */
  U32   crcec;          /* CRC error counter */
  U32   alec;           /* alighnment error counter */
  U32   disfc;          /* discard frame counter */
  U16   pads;           /* short frame PAD characters */
  U16   ret_lim;        /* retry limit threshold */
  U16   ret_cnt;        /* retry limit counter */
  U16   mflr;           /* maximum frame length reg */
  U16   minflr;         /* minimum frame length reg */
  U16   maxd1;          /* maximum DMA1 length reg */
  U16   maxd2;          /* maximum DMA2 length reg */
  U16   maxd;           /* rx max DMA */
  U16   dma_cnt;        /* rx dma counter */
  U16   max_b;          /* max bd byte count */
  U16   gaddr1;         /* group address filter 1 */
  U16   gaddr2;         /* group address filter 2 */
  U16   gaddr3;         /* group address filter 3 */
  U16   gaddr4;         /* group address filter 4 */
  U32   tbuf0_data0;    /* save area 0 - current frm */
  U32   tbuf0_data1;    /* save area 1 - current frm */
  U32   tbuf0_rba0;
  U32   tbuf0_crc;
  U16   tbuf0_bcnt;
  U16   paddr_h;        /* physical address (MSB) */
  U16   paddr_m;        /* physical address */
  U16   paddr_l;        /* physical address (LSB) */
  U16   p_per;          /* persistence */
  U16   rfbd_ptr;       /* rx first bd pointer */
  U16   tfbd_ptr;       /* tx first bd pointer */
  U16   tlbd_ptr;       /* tx last bd pointer */
  U32   tbuf1_data0;    /* save area 0 - next frame */
  U32   tbuf1_data1;    /* save area 1 - next frame */
  U32   tbuf1_rba0;
  U32   tbuf1_crc;
  U16   tbuf1_bcnt;
  U16   tx_len;         /* tx frame length counter */
  U16   iaddr1;         /* individual address filter 1*/
  U16   iaddr2;         /* individual address filter 2*/
  U16   iaddr3;         /* individual address filter 3*/
  U16   iaddr4;         /* individual address filter 4*/
  U16   boff_cnt;       /* back-off counter */
  U16   taddr_h;        /* temp address (MSB) */
  U16   taddr_m;        /* temp address */
  U16   taddr_l;        /* temp address (LSB) */
} ETHERNET_PRAM;

/*
** TRANSPARENT mode parameter RAM
*/
typedef struct 
{
  /*
  ** SCC parameter RAM
  */
  U16   rbase;          /* RX BD base address */
  U16   tbase;          /* TX BD base address */
  U8    rfcr;           /* Rx function code */
  U8    tfcr;           /* Tx function code */
  U16   mrblr;          /* Rx buffer length */
  U32   rstate;         /* Rx internal state */
  U32   rptr;           /* Rx internal data pointer */
  U16   rbptr;          /* rb BD Pointer */
  U16   rcount;         /* Rx internal byte count */
  U32   rtemp;          /* Rx temp */
  U32   tstate;         /* Tx internal state */
  U32   tptr;           /* Tx internal data pointer */
  U16   tbptr;          /* Tx BD pointer */
  U16   tcount;         /* Tx byte count */
  U32   ttemp;          /* Tx temp */
  U32   rcrc;           /* temp receive CRC */
  U32   tcrc;           /* temp transmit CRC */

  /*
        ** TRANSPARENT specific parameter RAM
        */
  U32   crc_p;          /* CRC Preset */
  U32   crc_c;          /* CRC constant */
} TRANSPARENT_PRAM;

/*
** RISC timers parameter RAM
*/
typedef struct 
{
  /*
  ** RISC timers parameter RAM
  */
  U16   tm_base;        /* RISC timer table base adr */
  U16   tm_ptr;         /* RISC timer table pointer */
  U16   r_tmr;          /* RISC timer mode register */
  U16   r_tmv;          /* RISC timer valid register */
  U32   tm_cmd;         /* RISC timer cmd register */
  U32   tm_cnt;         /* RISC timer internal cnt */
} TIMER_PRAM;

/*
*******************************************************************************
** Definitions of the QUICC memory structures
*******************************************************************************
*/
 

/*
**      Buffer Descriptors 
*/
typedef struct
{
  volatile U16  status;
  volatile U16  length;
  volatile U8           *buf;
} QUICC_BD;

/*
** transmit BD's status (common to all modes)
*/
#define TX_READY        0x8000          /* ready bit */
#define TX_WRAP         0x2000          /* wrap bit */
#define TX_INT          0x1000          /* interrupt on completion */
#define TX_LAST         0x0800          /* Last in frame        */
#define TX_CRC          0x0400          /* HDLC tx CRC          */
#define TX_CM           0x0200          /* Tx Continuous */

/*
** receive BD's status (common to all modes)
*/
#define RX_EMPTY        0x8000          /* buffer empty */
#define RX_WRAP         0x2000          /* wrap bit */
#define RX_INT          0x1000          /* interrupt on reception */


#define SCC1_BASE       0x0000
#define MISC_BASE       0x00b0
#define SCC2_BASE       0x0100
#define SPI_BASE        0x0180
#define TIMER_BASE      0x01b0
#define SCC3_BASE       0x0200
#define IDMA1_BASE      0x0270
#define SMC1_BASE       0x0280
#define SCC4_BASE       0x0300
#define IDMA2_BASE      0x0370
#define SMC2_BASE       0x0380

/*
** internal ram
*/
typedef struct 
{
  /*
**************************************
**      BASE + 0x000: user data memory 
**      Rev C+ has an extra 256 bytes
**************************************
        */
#ifdef REV_AB
  volatile U8   bd[0x700];                              /* Buffer Descriptors   */
  volatile U8   RESERVED1[0x500];               /* Reserved area                */
#else
  volatile U8   bd[0x800];                              /* Buffer Descriptors   */
  volatile U8   RESERVED1[0x400];               /* Reserved area                */
#endif

  /*
        **************************************
        **      BASE + 0xc00: PARAMETER RAM 
        **************************************
        */
  U8 pram[0x400];

  /*
        **************************************
        **      BASE + 0x1000: INTERNAL REGISTERS 
        **************************************
        */

        /*
        **      SIM 
        */
  volatile U32  sim_mcr;                        /* module configuration reg */
  volatile U16  sim_simtr;                      /* module test register */
  volatile U8           RESERVED2[0x2];         /* Reserved area */
  volatile U8           sim_avr;                        /* auto vector reg */
  volatile U8           sim_rsr;                        /* reset status reg */
  volatile U8           RESERVED3[0x2];         /* Reserved area */
  volatile U8           sim_clkocr;                     /* CLCO control register */
  volatile U8           RESERVED62[0x3];        /* Reserved area */
  volatile U16  sim_pllcr;                      /* PLL control register */
  volatile U8           RESERVED63[0x2];        /* Reserved area */
  volatile U16  sim_cdvcr;                      /* Clock devider control register */
  volatile U16  sim_pepar;                      /* Port E pin assignment register */
  volatile U8           RESERVED64[0xa];        /* Reserved area */
  volatile U8           sim_sypcr;                      /* system protection control */
  volatile U8           sim_swiv;                       /* software interrupt vector */
  volatile U8           RESERVED6[0x2];         /* Reserved area */
  volatile U16  sim_picr;                       /* periodic interrupt control reg */
  volatile U8           RESERVED7[0x2];         /* Reserved area */
  volatile U16  sim_pitr;                       /* periodic interrupt timing reg */
  volatile U8           RESERVED8[0x3];         /* Reserved area */
  volatile U8           sim_swsr;                       /* software service */
  volatile U32  sim_bkar;                       /* breakpoint address register*/
  volatile U32  sim_bkcr;                       /* breakpoint control register*/
  volatile U8           RESERVED10[0x8];        /* Reserved area */

        /*
        **      MEMC 
        */
  volatile U32  memc_gmr;                       /* Global memory register */
  volatile U16  memc_mstat;                     /* MEMC status register */
  volatile U8           RESERVED11[0xa];        /* Reserved area */
  volatile U32  memc_br0;                       /* base register 0 */
  volatile U32  memc_or0;                       /* option register 0 */
  volatile U8           RESERVED12[0x8];        /* Reserved area */
  volatile U32  memc_br1;                       /* base register 1 */
  volatile U32  memc_or1;                       /* option register 1 */
  volatile U8           RESERVED13[0x8];        /* Reserved area */
  volatile U32  memc_br2;                       /* base register 2 */
  volatile U32  memc_or2;                       /* option register 2 */
  volatile U8           RESERVED14[0x8];        /* Reserved area */
  volatile U32  memc_br3;                       /* base register 3 */
  volatile U32  memc_or3;                       /* option register 3 */
  volatile U8           RESERVED15[0x8];        /* Reserved area */
  volatile U32  memc_br4;                       /* base register 3 */
  volatile U32  memc_or4;                       /* option register 3 */
  volatile U8           RESERVED16[0x8];        /* Reserved area */
  volatile U32  memc_br5;                       /* base register 3 */
  volatile U32  memc_or5;                       /* option register 3 */
  volatile U8           RESERVED17[0x8];        /* Reserved area */
  volatile U32  memc_br6;                       /* base register 3 */
  volatile U32  memc_or6;                       /* option register 3 */
  volatile U8           RESERVED18[0x8];        /* Reserved area */
  volatile U32  memc_br7;                       /* base register 3 */
  volatile U32  memc_or7;                       /* option register 3 */
  volatile U8           RESERVED9[0x28];        /* Reserved area */

  /*
        **      TEST 
        */
  volatile U16  test_tstmra;            /* master shift a */
  volatile U16  test_tstmrb;            /* master shift b */
  volatile U16  test_tstsc;                     /* shift count */
  volatile U16  test_tstrc;                     /* repetition counter */
  volatile U16  test_creg;                      /* control */
  volatile U16  test_dreg;                      /* destributed register */
  volatile U8           RESERVED58[0x404];      /* Reserved area */

  /*
        **      IDMA1 
        */
  volatile U16  idma_iccr;                      /* channel configuration reg */
  volatile U8           RESERVED19[0x2];        /* Reserved area */
  volatile U16  idma1_cmr;                      /* dma mode reg */
  volatile U8           RESERVED68[0x2];        /* Reserved area */
  volatile U32  idma1_sapr;                     /* dma source addr ptr */
  volatile U32  idma1_dapr;                     /* dma destination addr ptr */
  volatile U32  idma1_bcr;                      /* dma byte count reg */
  volatile U8           idma1_fcr;                      /* function code reg */
  volatile U8           RESERVED20;                     /* Reserved area */
  volatile U8           idma1_cmar;                     /* channel mask reg */
  volatile U8           RESERVED21;                     /* Reserved area */
  volatile U8           idma1_csr;                      /* channel status reg */
  volatile U8           RESERVED22[0x3];        /* Reserved area */

  /*
        **      SDMA 
        */
  volatile U8           sdma_sdsr;                      /* status reg */
  volatile U8           RESERVED23;                     /* Reserved area */
  volatile U16  sdma_sdcr;                      /* configuration reg */
  volatile U32  sdma_sdar;                      /* address reg */

        /*
        **      IDMA2 
        */
  volatile U8           RESERVED69[0x2];        /* Reserved area */
  volatile U16  idma2_cmr;                      /* dma mode reg */
  volatile U32  idma2_sapr;                     /* dma source addr ptr */
  volatile U32  idma2_dapr;                     /* dma destination addr ptr */
  volatile U32  idma2_bcr;                      /* dma byte count reg */
  volatile U8           idma2_fcr;                      /* function code reg */
  volatile U8           RESERVED24;                     /* Reserved area */
  volatile U8           idma2_cmar;                     /* channel mask reg */
  volatile U8           RESERVED25;                     /* Reserved area */
  volatile U8           idma2_csr;                      /* channel status reg */
  volatile U8           RESERVED26[0x7];        /* Reserved area */

  /*
        **      Interrupt Controller 
        */
  volatile U32  intr_cicr;                      /* CP interrupt configuration reg*/
  volatile U32  intr_cipr;                      /* CP interrupt pending reg */
  volatile U32  intr_cimr;                      /* CP interrupt mask reg */
  volatile U32  intr_cisr;                      /* CP interrupt in service reg*/

        /*
        **      Parallel I/O 
        */
  volatile U16  pio_padir;                      /* port A data direction reg */
  volatile U16  pio_papar;                      /* port A pin assignment reg */
  volatile U16  pio_paodr;                      /* port A open drain reg */
  volatile U16  pio_padat;                      /* port A data register */
  volatile U8           RESERVED28[0x8];        /* Reserved area */
  volatile U16  pio_pcdir;                      /* port C data direction reg */
  volatile U16  pio_pcpar;                      /* port C pin assignment reg */
  volatile U16  pio_pcso;                       /* port C special options */
  volatile U16  pio_pcdat;                      /* port C data register */
  volatile U16  pio_pcint;                      /* port C interrupt cntrl reg */
  volatile U8           RESERVED29[0x16];       /* Reserved area */

  /*
        **      Timer 
        */
  volatile U16  timer_tgcr;                     /* timer global configuration  reg */
  volatile U8           RESERVED30[0xe];        /* Reserved area */
  volatile U16  timer_tmr1;                     /* timer 1 mode reg */
  volatile U16  timer_tmr2;                     /* timer 2 mode reg */
  volatile U16  timer_trr1;                     /* timer 1 referance reg */
  volatile U16  timer_trr2;                     /* timer 2 referance reg */
  volatile U16  timer_tcr1;                     /* timer 1 capture reg */
  volatile U16  timer_tcr2;                     /* timer 2 capture reg */
  volatile U16  timer_tcn1;                     /* timer 1 counter reg */
  volatile U16  timer_tcn2;                     /* timer 2 counter reg */
  volatile U16  timer_tmr3;                     /* timer 3 mode reg */
  volatile U16  timer_tmr4;                     /* timer 4 mode reg */
  volatile U16  timer_trr3;                     /* timer 3 referance reg */
  volatile U16  timer_trr4;                     /* timer 4 referance reg */
  volatile U16  timer_tcr3;                     /* timer 3 capture reg */
  volatile U16  timer_tcr4;                     /* timer 4 capture reg */
  volatile U16  timer_tcn3;                     /* timer 3 counter reg */
  volatile U16  timer_tcn4;                     /* timer 4 counter reg */
  volatile U16  timer_ter1;                     /* timer 1 event reg */
  volatile U16  timer_ter2;                     /* timer 2 event reg */
  volatile U16  timer_ter3;                     /* timer 3 event reg */
  volatile U16  timer_ter4;                     /* timer 4 event reg */
  volatile U8           RESERVED34[0x8];        /* Reserved area */

  /*
        **      CP 
        */
  volatile U16  cp_cr;                          /* command register */
  volatile U8           RESERVED35[0x2];        /* Reserved area */
  volatile U16  cp_rccr;                        /* main configuration reg */
  volatile U8           RESERVED37;                     /* Reserved area */
  volatile U8           cp_rmds;                        /* development support status reg */
  volatile U32  cp_rmdr;                        /* development support control reg */
  volatile U16  cp_cpcr1;                       /* CP Control Register 1 */
  volatile U16  cp_cpcr2;                       /* CP Control Register 2 */
  volatile U16  cp_cpcr3;                       /* CP Control Register 3 */
  volatile U16  cp_cpcr4;                       /* CP Control Register 4 */
  volatile U8           RESERVED59[0x2];        /* Reserved area */
  volatile U16  cp_rter;                        /* RISC timers event reg */
  volatile U8           RESERVED38[0x2];        /* Reserved area */
  volatile U16  cp_rtmr;                        /* RISC timers mask reg */
  volatile U8           RESERVED39[0x14];       /* Reserved area */

  /*
        **      BRG
        */
  volatile U32  brgc1;          /* BRG1 configuration reg */
  volatile U32  brgc2;          /* BRG2 configuration reg */
  volatile U32  brgc3;          /* BRG3 configuration reg */
  volatile U32  brgc4;          /* BRG4 configuration reg */

        /*
        **      SCC registers 
        */
  struct scc_regs
  {
    volatile U32        gsmra;                          /* SCC general mode reg */
    volatile U32        gsmrb;                          /* SCC general mode reg */
    volatile U16        psmr;                           /* protocol specific mode reg */
    volatile U8         RESERVED42[0x2];        /* Reserved area */
    volatile U16        todr;                           /* SCC transmit on demand */
    volatile U16        dsr;                            /* SCC data sync reg */
    volatile U16        scce;                           /* SCC event reg */
    volatile U8         RESERVED43[0x2];        /* Reserved area */
    volatile U16        sccm;                           /* SCC mask reg */
    volatile U8         RESERVED44[0x1];        /* Reserved area */
    volatile U8         sccs;                           /* SCC status reg */
    volatile U8         RESERVED45[0x8];        /* Reserved area */
  } scc[4];

  /*
        ** SMC 
        */
  struct smc_regs
  {
    volatile U8         RESERVED46[0x2];        /* Reserved area */
    volatile U16        smcmr;                          /* SMC mode reg */
    volatile U8         RESERVED60[0x2];        /* Reserved area */
    volatile U8         smce;                           /* SMC event reg */
    volatile U8         RESERVED47[0x3];        /* Reserved area */
    volatile U8         smcm;                           /* SMC mask reg */
    volatile U8         RESERVED48[0x5];        /* Reserved area */
  } smc[2];

  /*
        **      SPI 
        */
  volatile U16  spi_spmode;                     /* SPI mode reg */
  volatile U8           RESERVED51[0x4];        /* Reserved area */
  volatile U8           spi_spie;                       /* SPI event reg */
  volatile U8           RESERVED52[0x3];        /* Reserved area */
  volatile U8           spi_spim;                       /* SPI mask reg */
  volatile U8           RESERVED53[0x2];        /* Reserved area */
  volatile U8           spi_spcom;                      /* SPI command reg */
  volatile U8           RESERVED54[0x4];        /* Reserved area */

        /*
        **      PIP 
        */
  volatile U16  pip_pipc;                       /* pip configuration reg */
  volatile U8           RESERVED65[0x2];        /* Reserved area */
  volatile U16  pip_ptpr;                       /* pip timing parameters reg */
  volatile U32  pip_pbdir;                      /* port b data direction reg */
  volatile U32  pip_pbpar;                      /* port b pin assignment reg */
  volatile U32  pip_pbodr;                      /* port b open drain reg */
  volatile U32  pip_pbdat;                      /* port b data reg */
  volatile U8           RESERVED71[0x18];       /* Reserved area */

        /*
        **      Serial Interface
        */
  volatile U32  si_simode;                      /* SI mode register */
  volatile U8           si_sigmr;                       /* SI global mode register */
  volatile U8           RESERVED55;                     /* Reserved area */
  volatile U8           si_sistr;                       /* SI status register */
  volatile U8           si_sicmr;                       /* SI command register */
  volatile U8           RESERVED56[0x4];        /* Reserved area */
  volatile U32  si_sicr;                        /* SI clock routing */
  volatile U32  si_sirp;                        /* SI ram pointers */
  volatile U8           RESERVED57[0xc];        /* Reserved area */
  volatile U16  si_siram[0x80];         /* SI routing ram */
} QUICC;

