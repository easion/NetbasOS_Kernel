/*
 GDB stub for 68EZ328 (in emulation mode).
 Designed to be used with William A. Gatliff excellent gdb stub library.

 Copyright (c) 2000 by David Williams.

 Permission to use, copy, modify, and distribute this software for any
 purpose without fee is hereby granted, provided that this entire notice
 is included in all copies of any software which is or includes a copy
 or modification of this software and in all copies of the supporting
 documentation for such software.

 THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
 WARRANTY.  IN PARTICULAR, THE AUTHOR DOES NOT MAKE ANY  REPRESENTATION OR
 WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY OF THIS SOFTWARE OR ITS
 FITNESS FOR ANY PARTICULAR PURPOSE.

 Please report bugs/send useful modifications/send e-postcards to
	David Williams
	email: davidwilliams@ozemail.com.au

 Development History
	12 Feb 2000 - First public release
*/

/*
  This implementation assumes following:

	Application being debugged is always in supervisor mode. User mode is
	currently not supported.

	68000 core only (68020, FPU, CPU32 etc. not currently supported)

	More particularly designed for 68EZ328 (in emulation mode).

	Assumes serial port has already been setup for appropriate baud rate.
	Tested using rate of 115200.

	Application being debugged is in ROM (eg FLASH). This is not a requirement,
	but simply the reason why this stub was written in the first place. The
	standard 68K stub would probably work (once serial port stuff was sorted)
	if application is in RAM.

	This stub supports only a single hardware breakpoint. Which is all you get
	with 68EZ328 emulation mode without using external hardware. If external
	hardware is used then you would need to modify this stub anyway.	This does
	not affect the number of software breakpoints (breakpoints in RAM). GDB
	handles this by simply reading and writing to RAM (to read op-codes and
	replace them with breakpoint instructions).

	I have tried to determine if GDB needs more than one breakpoint when single
	steping through source code. I my limited testing I have not found the GDB
	needs more than one breakpoint which means that the stub can be used	to
	single step code. The limitation of one hardware breakpoint means that you
	can only set a single breakpoint	when running the application from ROM. If
	you attempt to set more than one breakpoint then this stub simply
	overwrites the any previous breakpoint with the new one, such that only
	the last breakpoint set will actually be active. Of course GDB wont know
	this so it will still think you have multiple breakpoints set.

	I have asked on GDB mail list about the number of breakpoints required to
	single step and no one seemed to be	able (or wanted to) answer :(

	This stub has been tested with version 4.17.1 of GDB.

	Have not check out how hardware breakpoints are set and used directly from
	GDB UI. Currently only tested against software breakpoints being fudged by
	this stub setting hardware breakpoints. I think I may need later version
	of GDB to test this against.
*/
#include "gdb.h"

/********************************************

	68EZ328 - periperal register defintions
	---------------------------------------
*********************************************/

/* Base address of all memory mapped internal registers */
#define	EZ328BASE	0xFFFFF000

/* Interrupt Registers */
#define	EZ328_IMR		(*(volatile unsigned long *)(EZ328BASE+0x304))	/* Interrupt Mask Reg */
#define	EZ328_ISR		(*(volatile unsigned long *)(EZ328BASE+0x30C))	/* Interrupt Status Reg */

/* types of interrupts */
#define EZ328_INT_EMIQ	0x00800000
#define EZ328_INT_SAM	0x00400000
#define EZ328_INT_IRQ5	0x00100000
#define EZ328_INT_IRQ6	0x00080000
#define EZ328_INT_IRQ3	0x00040000
#define EZ328_INT_IRQ2	0x00020000
#define EZ328_INT_IRQ1	0x00010000
#define EZ328_INT_INT3	0x00000800
#define EZ328_INT_INT2	0x00000400
#define EZ328_INT_INT1	0x00000200
#define EZ328_INT_INT0	0x00000100
#define EZ328_INT_PWM	0x00000080
#define EZ328_INT_KEYB	0x00000040
#define EZ328_INT_RTC	0x00000010
#define EZ328_INT_WDT	0x00000008
#define EZ328_INT_UART	0x00000004
#define EZ328_INT_TMR	0x00000002
#define EZ328_INT_SPI	0x00000001

/* Port G Registers */
#define	EZ328_PGDIR		(*(volatile unsigned char *)(EZ328BASE+0x430))	/* Direction Reg */
#define	EZ328_PGDATA	(*(volatile unsigned char *)(EZ328BASE+0x431))	/* Data Reg */
#define 	EZ328_PGPUEN	(*(volatile unsigned char *)(EZ328BASE+0x432))	/* Pullup enable Reg */
#define	EZ328_PGSEL		(*(volatile unsigned char *)(EZ328BASE+0x433))	/* Select Reg */

/* UART Registers */
#define	EZ328_URX		(*(volatile unsigned short *)(EZ328BASE+0x904))	/* Rx Reg */
#define	EZ328_UTX		(*(volatile unsigned short *)(EZ328BASE+0x906))	/* Tx Reg */

/* Bits for UART receive register */
#define EZ328_UART_RX_FIFO_FULL		0x8000
#define EZ328_UART_RX_FIFO_HALF		0x4000
#define EZ328_UART_RX_DATA_RDY		0x2000
#define EZ328_UART_RX_OLD_DATA		0x1000
#define EZ328_UART_RX_OVERRUN_ERR	0x0800
#define EZ328_UART_RX_FRAME_ERR		0x0400
#define EZ328_UART_RX_BREAK_ERR		0x0200
#define EZ328_UART_RX_PARITY_ERR		0x0100

/* Bits for UART transmit register */
#define EZ328_UART_TX_FIFO_EMPTY		0x8000
#define EZ328_UART_TX_FIFO_HALF		0x4000
#define EZ328_UART_TX_AVAIL			0x2000
#define EZ328_UART_TX_SEND_BREAK		0x1000
#define EZ328_UART_TX_NOCTS			0x0800
#define EZ328_UART_TX_BUSY				0x0400
#define EZ328_UART_TX_CTS_STATE		0x0200
#define EZ328_UART_TX_CTS_DELTA		0x0100

/* ICEM registers */
#define	EZ328_ICEMACR	(*(volatile unsigned long *)(EZ328BASE+0xD00))	/* ICEM address compare reg */
#define	EZ328_ICEMAMR	(*(volatile unsigned long *)(EZ328BASE+0xD04))	/* ICEM address mask reg */
#define	EZ328_ICEMCCR	(*(volatile unsigned short *)(EZ328BASE+0xD08))	/* ICEM control compare reg */
#define	EZ328_ICEMCMR	(*(volatile unsigned short *)(EZ328BASE+0xD0A))	/* ICEM control mask reg */
#define	EZ328_ICEMCR	(*(volatile unsigned short *)(EZ328BASE+0xD0C))	/* ICEM control reg */
#define	EZ328_ICEMSR	(*(volatile unsigned short *)(EZ328BASE+0xD0E))	/* ICEM status reg */

/* Watchdog Register */
#define	EZ328_WATCHDOG	(*(volatile unsigned short *)(EZ328BASE+0xB0A))

/* Define values for Watchdog register */
#define EZ328_WATCHDOG_ENABLE 0x0001
#define EZ328_WATCHDOG_DISABLE 0x0000

/*********************

	68EZ328 - vectors
	-----------------
**********************/

/* Macro function to set a vector into the vector table */
#define SET_VECTOR(vect,func)  	*(unsigned long*)((unsigned long)vect<<2) = (unsigned long)func

/* Vectors used in this module */
#define TRACE_VECTOR							0x09
#define A_LINE_VECTOR						0x0A
#define TRAP_1_VECTOR						0x21
#define INT_LEVEL_7_VECTOR					0x47

/**************************************

	68EZ328 - CPU register defintions
	---------------------------------
***************************************/

/*
	There are 180 unsigned chars of registers on a 68020 w/68881 (worst case for 68K family)
	The D0-D7,A0-A7,PS,PC registers are 4 unsigned char (32 bit)
	The registers FP0-FP7 are 12 unsigned char (96 bit) registers
	The FPCONTROL, FPSTATUS and FPIADDR registers are 4 unsigned char (32 bit)

	Only D0-D7,A0-A7,PS,PC are required for 68K, however GDB assumes
	worst case and expects all these registers. Unused registers are
	ignored when received and returned to GDB as 0
*/
#define NUMREGBYTES 180
typedef enum
	{
	D0,D1,D2,D3,D4,D5,D6,D7,
   A0,A1,A2,A3,A4,A5,A6,A7,
   PS,PC,
   FP0,FP1,FP2,FP3,FP4,FP5,FP6,FP7,
   FPCONTROL,FPSTATUS,FPIADDR
   } regname;

int CPUregisters[NUMREGBYTES/4];

/* Define trace bit - used in PS register to set single step (trace) mode of CPU */
#define TRACE_BIT 0x8000


/*************************

	Local stack for stub
	--------------------
**************************/
/*
	FIXME: Currently not used as code crashes on switching to local stack
	Reason for needing local stack - Need to consider affect of "inferior" function calls on application stack
	When I really figure out what this really means I may look at fixing this.
*/
#define STACKSIZE 400
int localStack[STACKSIZE/sizeof(int)];
int* localStackPtr = &localStack[STACKSIZE/sizeof(int) - 1];


/*************************

	Useful Macro functions
	----------------------
**************************/

/* Disable all interrupts levels (except level 7 which is NMI) */
#define DISABLE_ALL_INTERRUPTS()	asm("	ori.w #0x0700,%sr");

/* Enable all interrupts levels */
#define ENABLE_ALL_INTERRUPTS()	asm("	andi.w #0xF8FF,%sr");

/* disable further emulator interrupts - esp level 7*/
#define DISABLE_EMU_INTERRUPTS()	asm("	ori.l #0x00800000,0xFFFFF304");

/* cause software breakpoint */
#define BREAKPOINT()	asm(" trap #1")

/*
  Generic code to save processor context.
  Assumes the stack looks like this:

  ....					<--- stack pointer + 6
  pc(32 bits)			<--- stack pointer + 2
  sr(16 bits)			<--- stack pointer points here
*/
#define SAVE_CONTEXT()			asm("
   movem.l	%d0-%d7/%a0-%a7,CPUregisters /* save registers        */
	lea		CPUregisters,%a0   /* get address of registers     */
	move.w	(%sp),66(%a0)		/* Save SR to CPUregisters */
	move.l	2(%sp),68(%a0)		/* Save PC to CPUregisters */
	addi.l	#6,60(%a0)  	 /* fixup saved stack pointer to position prior to interrupt */
/*	move.l	localStackPtr,%sp	*//* switch to local stack - not currently working so commented out! */
");

/*
  Stuffs a unsigned char out the serial port.
 */
void gdb_putc ( char c )
{
	/*
		Cant allow UART interrupts while sending
		EZ328_IMR |= EZ328_INT_UART;
	*/
	asm("	ORI.L %1,%0" : "m=" (EZ328_IMR) : "i" (EZ328_INT_UART));

	/* wait for space in UART tx FIFO */
	while (!(EZ328_UTX & EZ328_UART_TX_AVAIL));

	/* place unsigned char from buffer into tx FIFO - ignoring HW flow control */
	EZ328_UTX = EZ328_UART_TX_NOCTS | (unsigned short)c;

	/*
		Allow UART interrupts now that have finished
		EZ328_IMR &= ~EZ328_INT_UART;
	*/
	asm("	ANDI.L %1,%0" : "m=" (EZ328_IMR) : "i" (~EZ328_INT_UART));

	return;
}


/*
  Blocks until a unsigned char is received at the serial port.
 */
char gdb_getc ( void )
{
	char c;
	unsigned short rxStatusData;

	/*
		Cant allow UART interrupts while polling for receive
		EZ328_IMR |= EZ328_INT_UART;
	*/
	asm("	ORI.L %1,%0" : "m=" (EZ328_IMR) : "i" (EZ328_INT_UART));

	/* Do initial read of uart RX register to determine if any characters have been received */
   rxStatusData = EZ328_URX;

	/* Wait for received data */
	while ( !(rxStatusData & EZ328_UART_RX_DATA_RDY ))
	{
	   rxStatusData = EZ328_URX;

      /* Tickle H/W watchdog timer */
		EZ328_WATCHDOG = EZ328_WATCHDOG_ENABLE;
	}

	/* Assign char received to variable to return - ignore any errors during receive */
	c = (char)rxStatusData;

	/*
		Allow UART interrupts now that have finished
		EZ328_IMR &= ~EZ328_INT_UART;
	*/
	asm("	ANDI.L %1,%0" : "m=" (EZ328_IMR) : "i" (~EZ328_INT_UART));

	return c;
}


/*
  Retrieves a register value from gdb_register_file.
  Returns the size of the register, in bytes,
  or zero if an invalid id is specified (which *will*
  happen--- gdb.c uses this functionality to tell
  how many registers we actually have).
 */
short gdb_peek_register_file ( short id, long *val )
{
	/* all supported registers are longs */
	short retval = sizeof( long );

	if ( (id >= D0) && (id<=PC) )
	{
		*val = CPUregisters[id];
	}
	else
	{
		retval = 0;
	}

  return retval;
}


/*
  Stuffs a register value into gdb_register_file.
  Returns the size of the register, in bytes,
  or zero if an invalid id is specified.
 */
short gdb_poke_register_file ( short id, long val )
{
	/* all our registers are longs */
	short retval = sizeof( long );

	if ( (id >= D0) && (id<=PC) )
	{
		CPUregisters[id] = val;
	}
	else
	{
		retval = 0;
	}

	return retval;
}


/*
  Uses the UBC to generate an exception
  after we execute the next instruction.
*/
void gdb_step ( unsigned long addr )
{

	/* if we're stepping from an address,
     adjust pc (untested!) */
  	/* TODO: test gdb_step when PC is supplied */
	if( addr )
	{
      CPUregisters[PC] = addr;
	}

	/* set Trace bit in CPU status register */
	CPUregisters[PS] |= TRACE_BIT;

	/* we're all done now */
  	gdb_return_from_exception();

  	return;
}


/*
  Continue program execution at addr,
  or at the current pc if addr == 0.
*/
void gdb_continue ( unsigned long addr )
{

  if( addr )
  {
    CPUregisters[PC] = addr;
  }

  /* Clear Trace bit in CPU status register - in case been tracing */
  CPUregisters[PS] &= ~TRACE_BIT;

  gdb_return_from_exception();

  return;
}


/*
  Kills the current application.
  Simulates a reset by jumping to
  the address taken from the reset
  vector at address 0.
 */
void gdb_kill ( void )
{
}

/*
	TRACE/TRAP#1 ISR (Single step/software breakpoint)
*/
asm("
   .global gdb_traceTrap_isr
gdb_traceTrap_isr:
");
DISABLE_EMU_INTERRUPTS();
SAVE_CONTEXT();
asm("
	/* Push SIGTRAP on STACK(local) */
	move.l #5, -(%sp)
	jsr	 gdb_handle_exception	/* never returns to here! */
");
/* Function prototype for above interrupt handler */
void gdb_traceTrap_isr(void);

/* Function prototype for above interrupt handler */
void gdb_int7_isr(void);
void gdb_aline_isr(void);

/*
  Restores registers to the values specified
  in CPUregisters
*/
asm("
   .global gdb_return_from_exception
gdb_return_from_exception:
	lea		CPUregisters,%a0	/* Get starting address of saved registers */
	subi.l   #6,60(%a0)				/* fixup stack pointer for return */
	move.l	60(%a0),%a1				/* Get pointer to top of users stack - should be pointing at stacked SR at time of last interrupt */
	move.w	66(%a0),(%a1)			/* replace stacked copy of SR with copy in CPUregisters */
	move.l	68(%a0),2(%a1)	  		/* replace stacked copy of PC (return address) with copy in CPUregisters */
	movem.l  CPUregisters,%d0-%d7/%a0-%a7	/* restore all data and address registers (inc stack pointer!) */

	andi.l #0xFF7FFFFF,0xFFFFF304 /* IMR - enable emulator interrupts */
 	move.w 0xFFFFFD0E,0xFFFFFD0E	/* ICEMSR - clear status bits */

	rte	  /* pop and go! */
");


void gdb_init(void)
{
	/* Set vector for trace and Trap #1 interrupts */
   SET_VECTOR(TRACE_VECTOR,gdb_traceTrap_isr);
   SET_VECTOR(TRAP_1_VECTOR,gdb_traceTrap_isr);

   /* Vectors for emulation mode */
	SET_VECTOR(INT_LEVEL_7_VECTOR,gdb_int7_isr);
	SET_VECTOR(A_LINE_VECTOR,gdb_aline_isr);


	/* Enable EMUIRQ signal to cause interrupt */
	EZ328_PGSEL &= ~0x04;
	EZ328_PGPUEN |= 0x04;

   EZ328_ICEMSR = EZ328_ICEMSR;	/* ICEMSR - clear status bits to avoid immediate interrupt */

	/* enable emulator breakpoints */
	EZ328_IMR &= ~EZ328_INT_EMIQ;

	BREAKPOINT();	/* Causes control to be passed to GDB stub */
}

int gdb_is_rom_addr(long addr)
{
	return (int)((addr >= 0x800000L) && (addr < 0xC00000L ));
}

void gdb_write_rom(long len,long addr, const char *hargs)
{
  /*
  		Test for special case of attempting to write a trap #1 op-code (0x4E41) to flash memory
		In this case do a hardware breakpoint instead
		Ignore all other cases - ie simply dont write anything.
  */
	if ( len == 2 )
	{
   	if (hexbuf_to_long( sizeof( short ) * 2, hargs ) == 0x4e41L)
		{
      	gdb_set_hw_break(addr);
		}
	}
}

void gdb_set_hw_break(unsigned long addr)
{
	EZ328_ICEMACR = addr;
	EZ328_ICEMAMR = 0x00000000;	/* full address matches only */
	EZ328_ICEMCCR = 0x0003;			/* read instruction breakpoint */
	EZ328_ICEMCMR = 0x0000;			/* Dont mask read/write and data/instruction matching */
	EZ328_ICEMCR = 0x001F;			/* Enable int7 for Bus breakpoint, disable hardmap, Single breakpoint,
												program break, Enable compare */
}
short lastICEMSR;

/*
	HW breakpoint ISR
*/
asm("
   .global gdb_int7_isr
gdb_int7_isr:
");
DISABLE_EMU_INTERRUPTS();
ENABLE_ALL_INTERRUPTS();	/* A bit controversial - should interrupts be enabled after a breakpoint - remove this if not */
SAVE_CONTEXT();
asm("
	move.w 0xFFFFFD0E, %d0	/* read current ICEMSR */
   move.w %d0,lastICEMSR	/* save it  */
	btst	 #1,%d0		/* test for a-line exception - instruction breakpoint */
	beq	 clearStatus
							/* remove a-line exception frame to get at real frame */
	move.w	6(%sp),66(%a0)		/* Save SR to CPUregisters */
	move.l	8(%sp),68(%a0)		/* Save PC to CPUregisters */
	addi.l	#6,60(%a0)  	 /* fixup saved stack pointer to position prior to interrupt */

clearStatus:
 	move.w 0xFFFFFD0E,0xFFFFFD0E	/* ICEMSR - clear status bits */
	andi.w #0xFFFE,0xFFFFFD0C		/* ICEMCR - disable further breakpoint interrupts */

	/* Push SIGTRAP on STACK(local) */
	move.l #5, -(%sp)
	jsr	 gdb_handle_exception /* never returns to here! */
");

/*
	Simply return from a-line exception
	Will get an a-line exception as well as int7 exception for certains types of breakpoints.
	But get an int7 exception for all types of breakpoints, so can simply ignore all a-line exceptions.
*/
asm("
	.global gdb_aline_isr
gdb_aline_isr:
	rte
");

