/* 
  Copyright (c) 2001 by      William A. Gatliff
  All rights reserved.      bgat@billgatliff.com

  See the file COPYING for details.

  This file is provided "as-is", and without any express or implied
  warranties, including, without limitation, the implied warranties of
  merchantability and fitness for a particular purpose.

  The author welcomes feedback regarding this file.

  **

  Generic MC68360 support, generously funded by Michael Dorin,
  dorin@gr-303.com, of EDI Enterprises, Inc.  Thanks, Mike!

  Written Oct 2001 by Bill Gatliff, bgat@billgatliff.com


  SMC1UART - serial i/o on SMC1 in UART mode (polled)


  This stub assumes that DPRBASE == 0xffff0000, and the built-in
  startup code, if used, consumes the first 1024 bytes of the
  dual-port RAM for stack and data storage.  The first part of this
  area is the register file and other data; the last few bytes are
  used for the transmit and receive buffers (one byte each) and the
  buffer descriptors (one each for transmit and receive).  The stack
  starts immediately above the buffer descriptors.

  To build this stub, do something like the following:

  $ m68k-elf-gcc -mcpu32 -Wall -nostartfiles \
      -o gdbstub.out -Wl,--script=cpu32-mc68360.x \
      -DCRT0 -DSMC1UART gdb.c cpu32.c cpu32-mc68360.c
  $ m68k-elf-objcopy -O srec gdbstub.out gdbstub.s19

  You will need a .gdbinit file, as the stub does very little in the
  way of setting up peripheral registers, etc. in the host
  microcontroller.  The file cpu32-mc68360.gdbinit is a start.  Note
  that the setting for $dprbase in your gdbinit must match what the
  stub uses.  The stub isn't yet clever enough to deal with changes to
  MBAR at runtime.

  You will also need to put a statement somewhere that sets up the
  initial stack pointer, frame pointer, and program status register,
  because the stub does not initialize its register file to sane
  values (this is your job, not the stub's).  If the values chosen are
  relatively constant then you can put them in your gdbinit file, or
  just type them in:

  (gdb) set $sp=$fp=0x1000800
  (gdb) set $ps=0x2700

  Since the SMC1UART driver is polled, a ctrl-C from gdb is ignored.
  To fix this, provide an interrupt-driven getc() and putc(), and
  forcibly give control to the exception handler when a ctrl-C is
  received.


  $Id: cpu32-mc68360.c,v 1.1 2002/04/08 15:08:20 bgat Exp $

*/


#include "gdb.h"
#include "cpu32.h"


typedef struct
	{
	  volatile short status;
	  volatile short length;
	  volatile char  *buf;
	} bd_T;


/* Until someone figures out some clever macros, do not change DPRBASE
   without making the corresponding changes in the vector table and
   startup code in the CRT0 block below. */
#define DPRBASE 0xffff0000UL
#define REGB (DPRBASE + 4096)

#define SMC1BASE (DPRBASE + 0xe80)

#define PBPAR   ((volatile long* )(REGB + 0x6bc))
#define PBDIR   ((volatile long* )(REGB + 0x6b8))
#define PBODR   ((volatile short*)(REGB + 0x6c2))
#define SIMODE  ((volatile long* )(REGB + 0x6e0))
#define BRGC1   ((volatile long* )(REGB + 0x5f0))
#define SMCE1   ((volatile char* )(REGB + 0x686))
#define SMCM1   ((volatile char* )(REGB + 0x68a))
#define SMCMR1  ((volatile short*)(REGB + 0x682))
#define CR      ((volatile short*)(REGB + 0x5c0))
#define SDCR    ((volatile short*)(REGB + 0x51e))
#define SYPCR   ((volatile char* )(REGB + 0x22))

#define RBASE   ((volatile short*)(SMCBASE + 0))
#define TBASE   ((volatile short*)(SMCBASE + 2))
#define RFCR    ((volatile char* )(SMCBASE + 4))
#define TFCR    ((volatile char* )(SMCBASE + 5))
#define MRBLR   ((volatile short*)(SMCBASE + 6))
#define RSTATE  ((volatile long* )(SMCBASE + 8))
#define RBPTR   ((volatile short*)(SMCBASE + 0x10))
#define TSTATE  ((volatile long* )(SMCBASE + 0x18))
#define TBPTR   ((volatile short*)(SMCBASE + 0x20))
#define MAX_IDL ((volatile short*)(SMCBASE + 0x28))
#define BRKLN   ((volatile short*)(SMCBASE + 0x2c))
#define BRKEC   ((volatile short*)(SMCBASE + 0x2e))
#define BRKCR   ((volatile short*)(SMCBASE + 0x30))

#define TX_READY        0x8000
#define TX_WRAP         0x2000
#define RX_EMPTY        0x8000
#define RX_WRAP         0x2000

#define BD      ((volatile bd_T* )(DPRBASE + 0x3e0))
#define RXBD    BD
#define TXBD    (BD + 1)
#define RXBUF   ((volatile char* )(BD + 2))
#define TXBUF   ((volatile char* )(RXBUF + 1))



#if defined(SMC1UART)
#define SMCBASE SMC1BASE
#define BRGC    BRGC1
#define SMCE    SMCE1
#define SMCM    SMCM1
#define SMCMR   SMCMR1

#define BAUD_115200 0x10022
#define BAUD_57600  0x10044
#define BAUD_9600   0x10144
#endif

#if !defined(BAUD)
#define BAUD BAUD_115200
#endif



int gdb_putc (int c)
	{
#if defined(SMC1UART)
	while (TXBD->status & TX_READY) {}
	*TXBUF = c;
	TXBD->length = 1;
	TXBD->status |= TX_READY;
#else
#warning Null gdb_putc().
#endif
	return c & 0xff;
	}


int gdb_getc (void)
	{
	char retval = 0;
#if defined(SMC1UART)
	while (RXBD->status & RX_EMPTY) {}
	retval = *RXBUF;
	RXBD->status |= RX_EMPTY;
#else
#warning Null gdb_getc().
#endif
	return retval;
	}


void gdb_startup (void)
	{
#if defined(SMC1UART)
  
	/* see MC68360 QUad Integrated Communications Controller User's
	   Manual, section 7.11.8, "SMC UART Example" for details on this */

	/* TODO: this puts a bunch of crap on the tx line */

	RXBD->status = RX_EMPTY + RX_WRAP;
	RXBD->buf = RXBUF;
	RXBD->length = 0;
	TXBD->status = TX_WRAP;
	TXBD->buf = TXBUF;
	TXBD->length = 0;
  
	*SDCR = 0x740;

	*BRGC = BAUD;

	*PBPAR = 0xcce;
	*PBDIR = 0x3033f;
	*PBODR = 0;

	*SIMODE &= 0xffff0fff;

	*RBASE = (long)RXBD - DPRBASE;
	*TBASE = (long)TXBD - DPRBASE;
  
	*CR = 1; while( *CR & 1 );

	*RFCR = 0x18;
	*TFCR = 0x18;

	*MRBLR = 1;
  
	*MAX_IDL = 0;

	*BRKLN = 0;
	*BRKEC = 0;

	*BRKCR = 0;

	*SMCE = 0xff;
	*SMCM = 0;
  
	*SMCMR = 0x4820;

	*SMCMR = 0x4823;
#else
#warning Null gdb_startup().
#endif
	} 


#if defined(CRT0)


__asm__("

.global gdb_startup
.global gdb_monitor
.global gdb_interrupt_handler

.align 2
.section .vect

.global _vector_table
_vector_table:
  .long 0xffff03dc
  .long _start

  .long gdb_interrupt_handler
  .long gdb_interrupt_handler
  .long gdb_interrupt_handler
  .long gdb_interrupt_handler
  .long gdb_interrupt_handler
  .long gdb_interrupt_handler
  .long gdb_interrupt_handler
  .long gdb_interrupt_handler

  .long gdb_interrupt_handler
  .long gdb_interrupt_handler
  .long gdb_interrupt_handler
  .long gdb_interrupt_handler
  .long gdb_interrupt_handler
  .long gdb_interrupt_handler
  .long gdb_interrupt_handler
  .long gdb_interrupt_handler
  .long gdb_interrupt_handler
  .long gdb_interrupt_handler

  .long gdb_interrupt_handler
  .long gdb_interrupt_handler
  .long gdb_interrupt_handler
  .long gdb_interrupt_handler
  .long gdb_interrupt_handler
  .long gdb_interrupt_handler
  .long gdb_interrupt_handler
  .long gdb_interrupt_handler
  .long gdb_interrupt_handler
  .long gdb_interrupt_handler

  .long gdb_interrupt_handler
  .long gdb_interrupt_handler
  .long gdb_interrupt_handler
  .long gdb_interrupt_handler
  .long gdb_interrupt_handler
  .long gdb_interrupt_handler
  .long gdb_interrupt_handler
  .long gdb_interrupt_handler
  .long gdb_interrupt_handler
  .long gdb_interrupt_handler

  .long gdb_interrupt_handler
  .long gdb_interrupt_handler
  .long gdb_interrupt_handler
  .long gdb_interrupt_handler
  .long gdb_interrupt_handler
  .long gdb_interrupt_handler
  .long gdb_interrupt_handler
  .long gdb_interrupt_handler
  .long gdb_interrupt_handler
  .long gdb_interrupt_handler

  .long gdb_interrupt_handler
  .long gdb_interrupt_handler
  .long gdb_interrupt_handler
  .long gdb_interrupt_handler
  .long gdb_interrupt_handler
  .long gdb_interrupt_handler
  .long gdb_interrupt_handler
  .long gdb_interrupt_handler
  .long gdb_interrupt_handler
  .long gdb_interrupt_handler


.align 2
.section .text

.global _start
_start:

  /* set up MBAR */
  move #7, %d0
  movec %d0, %sfc
  movec %d0, %dfc
  move.l #0xffff0001, %d0
  moves.l %d0, (0x3ff00)

  /* disable SWT */
  move.b #0xc, (0xffff1022)

  /* initialize */
  jsr gdb_startup

  /* drop into the monitor and stay there */
to_monitor:
  pea 5
  jsr gdb_monitor
  bra to_monitor
");

#endif

