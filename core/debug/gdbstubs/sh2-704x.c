/*
  Copyright (c) 2001 by      William A. Gatliff
  All rights reserved.      bgat@billgatliff.com

  See the file COPYING for details.

  This file is provided "as-is", and without any express
  or implied warranties, including, without limitation,
  the implied warranties of merchantability and fitness
  for a particular purpose.

  The author welcomes feedback regarding this file.
*/

/*
  This is code to support the SH2's 704x family of chips.  This code
  has been tested on the Hitachi 7045EDK and 7043EDK.

  To build this stub, do something like the following:

  $ sh-elf-gcc -Wall -m2 -g -DSH2_7045EDK -DCRT0 -o sh2-stub \
  -Wl,--script=sh2-704x.x -nostartfiles sh2-704x.c gdb.c sh2.c

  To convert the output file to Motorola S-Records, do this:
  
  $ sh-elf-objcopy -O srec sh2-stub sh2-stub.mot

  See README-sh2-7045edk for instructions on burning this stub into
  the 7045EDK's on-chip flash.

  If you are using the 7045EDK, be sure to use the sh2-7045-gdbinit
  file during gdb startup, to configure external RAM and other
  essential peripherals.  This is your job, not the stub's.  See
  sh2-7045edk.gdbinit for a starting point.
*/

/* $Id: sh2-704x.c,v 1.7 2002/04/15 19:37:39 bgat Exp $ */


#include "gdb.h"
#include "sh2.h"


#define SCI_SSR_TDRE 0x80
#define SCI_SSR_RDRF 0x40
#define SCI_SSR_ORER 0x20
#define SCI_SSR_FER  0x10
#define SCI_SSR_PER  0x08
#define SCI_SSR_TEND 0x04
#define SCI_SSR_MPB  0x02
#define SCI_SSR_MPBT 0x01

#define SCI_SMR0 ((volatile char *)0xffff81a0L) 
#define SCI_BRR0 ((volatile char *)0xffff81a1L)
#define SCI_SCR0 ((volatile char *)0xffff81a2L)
#define SCI_TDR0 ((volatile char *)0xffff81a3L)
#define SCI_SSR0 ((volatile char *)0xffff81a4L)
#define SCI_RDR0 ((volatile char *)0xffff81a5L)

#define SCI_SMR1 ((volatile char *)0xffff81b0L) 
#define SCI_BRR1 ((volatile char *)0xffff81b1L)
#define SCI_SCR1 ((volatile char *)0xffff81b2L)
#define SCI_TDR1 ((volatile char *)0xffff81b3L)
#define SCI_SSR1 ((volatile char *)0xffff81b4L)
#define SCI_RDR1 ((volatile char *)0xffff81b5L)

#define PFC_PACRL2 ((volatile short *) 0xffff838eL)
#define PFC_PEIOR  ((volatile short *) 0xffff83b4L)
#define IO_PEDR    ((volatile short *) 0xffff83b0L)

#define TDRE SCI_SSR_TDRE
#define RDRF SCI_SSR_RDRF
#define PER  SCI_SSR_PER
#define FER  SCI_SSR_FER
#define ORER SCI_SSR_ORER

#define PACRL2 0x0145

#define BAUD_115200 7
#define BAUD_57600 14
#define BAUD_9600

#if defined(SH2_7045EDK)
#define SMR SCI_SMR0
#define BRR SCI_BRR0
#define SCR SCI_SCR0
#define TDR SCI_TDR0
#define SSR SCI_SSR0
#define RDR SCI_RDR0
#else
#define SMR SCI_SMR0
#define BRR SCI_BRR0
#define SCR SCI_SCR0
#define TDR SCI_TDR0
#define SSR SCI_SSR0
#define RDR SCI_RDR0
#endif

#if !defined(BAUD)
#define BAUD BAUD_115200
#endif



int gdb_putc (int c)
	{
	while (!(*SSR & TDRE));
	*TDR = c;
	*SSR &= ~TDRE;
	return c & 0xff;
	}

int gdb_getc (void)
	{
	unsigned char c;

	*SSR &= ~( PER | FER | ORER );
	while (!(*SSR & RDRF));
	c = *RDR;
	*SSR &= ~RDRF;
	return c;
	}


void gdb_monitor_onexit (void) {}



void gdb_startup (void)
	{
	/* enable txd/rxd lines */
	*PFC_PACRL2 = PACRL2;
  
  /* enable re + te, no interrupts */
	*SMR = 0;
	*BRR = BAUD;
	*SCR = 0x30;

	/* turn on the 7045EDK's green led */
	*PFC_PEIOR = 0x20;
	*IO_PEDR = 0;

	return;
	}


#if defined(CRT0)

__asm__(

".section .vect\n"
".align 2\n"
".global _vector_table\n"
"_vector_table:\n"

"  .long  _start                /*  0: power-on reset */\n"
"  .long 0xfffffffc\n"
"  .long _start                 /*  2: manual reset */\n"
"  .long 0xfffffffc\n"
"  .long _gdb_illegalinst_isr   /*  4: general illegal instruction */\n"
"  .long _gdb_unhandled_isr     /*  5: (reserved) */\n"
"  .long _gdb_illegalinst_isr   /*  6: slot illegal instruction */\n"
"  .long _gdb_unhandled_isr     /*  7: (reserved) */\n"
"  .long _gdb_unhandled_isr     /*  8: (reserved) */\n"
"  .long _gdb_addresserr_isr    /*  9: CPU address error */\n"
"  .long _gdb_addresserr_isr    /* 10: DMAC/DTC address error */\n"

"  .long _gdb_unhandled_isr     /* 11: NMI */\n"
"  .long _gdb_unhandled_isr     /* 12: UBC */\n"
"  .long _gdb_unhandled_isr     /* 13: (reserved) */\n"
"  .long _gdb_unhandled_isr     /* 14: (reserved) */\n"
"  .long _gdb_unhandled_isr     /* 15: (reserved) */\n"
"  .long _gdb_unhandled_isr     /* 16: (reserved) */\n"
"  .long _gdb_unhandled_isr     /* 17: (reserved) */\n"
"  .long _gdb_unhandled_isr     /* 18: (reserved) */\n"
"  .long _gdb_unhandled_isr     /* 19: (reserved) */\n"
"  .long _gdb_unhandled_isr     /* 20: (reserved) */\n"

"  .long _gdb_unhandled_isr     /* 21: (reserved) */\n"
"  .long _gdb_unhandled_isr     /* 22: (reserved) */\n"
"  .long _gdb_unhandled_isr     /* 23: (reserved) */\n"
"  .long _gdb_unhandled_isr     /* 24: (reserved) */\n"
"  .long _gdb_unhandled_isr     /* 25: (reserved) */\n"
"  .long _gdb_unhandled_isr     /* 26: (reserved) */\n"
"  .long _gdb_unhandled_isr     /* 27: (reserved) */\n"
"  .long _gdb_unhandled_isr     /* 28: (reserved) */\n"
"  .long _gdb_unhandled_isr     /* 29: (reserved) */\n"
"  .long _gdb_unhandled_isr     /* 30: (reserved) */\n"

"  .long _gdb_unhandled_isr     /* 31: (reserved) */\n"
"  .long _gdb_trapa32_isr       /* 32: trap 32 instruction */\n"
"  .long _gdb_trapa33_isr       /* 33: trap 33 instruction */\n"
"  .long _gdb_trapa34_isr       /* 34: trap 34 instruction */\n"
"  .long _gdb_unhandled_isr     /* */\n"
"  .long _gdb_unhandled_isr     /* */\n"
"  .long _gdb_unhandled_isr     /* */\n"
"  .long _gdb_unhandled_isr     /* */\n"
"  .long _gdb_unhandled_isr     /* */\n"

"  .long _gdb_unhandled_isr\n"
"  .long _gdb_unhandled_isr\n"
"  .long _gdb_unhandled_isr\n"
"  .long _gdb_unhandled_isr\n"
"  .long _gdb_unhandled_isr\n"
"  .long _gdb_unhandled_isr\n"
"  .long _gdb_unhandled_isr\n"
"  .long _gdb_unhandled_isr\n"
"  .long _gdb_unhandled_isr\n"
"  .long _gdb_unhandled_isr\n"

"  .long _gdb_unhandled_isr\n"
"  .long _gdb_unhandled_isr\n"
"  .long _gdb_unhandled_isr\n"
"  .long _gdb_unhandled_isr\n"
"  .long _gdb_unhandled_isr\n"
"  .long _gdb_unhandled_isr\n"
"  .long _gdb_unhandled_isr\n"
"  .long _gdb_unhandled_isr\n"
"  .long _gdb_unhandled_isr\n"
"  .long _gdb_unhandled_isr\n"

"  .long _gdb_unhandled_isr\n"
"  .long _gdb_unhandled_isr\n"
"  .long _gdb_unhandled_isr\n"
"  .long _gdb_unhandled_isr\n"

".section .text\n"
".align 2\n"
".global _start\n"
".global start\n"
"start:\n"
"_start:\n"
"  nop\n"
"  mov   #0, r0\n"
"  mov   #1, r1\n"
"  mov   #2, r2\n"
"  mov   #3, r3\n"
"  mov   #4, r4\n"
"  mov   #5, r5\n"
"  mov   #6, r6\n"
"  mov   #7, r7\n"
"  mov   #8, r8\n"
"  mov   #9, r9\n"
"  mov   #10, r10\n"
"  mov   #11, r11\n"
"  mov   #12, r12\n"
"  mov   #13, r13\n"
"  mov   #14, r14\n"
"  mov.l gdbstartup_k, r0\n"
"  jsr @r0\n"
"  nop\n"
"  trapa #32\n"
"  nop\n"
#if 0
"  mov.l gdbmonitor_k, r0\n"
"  mov #5, r4\n"
"  jsr @r0\n"
"  nop\n"
"  bra _start\n"
"  nop\n"
#endif
".align 2\n"
"gdbstartup_k: .long _gdb_startup\n"
"gdbmonitor_k: .long _gdb_monitor\n"

);

#endif
