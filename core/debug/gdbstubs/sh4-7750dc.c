/*
  Copyright (c) 1999 by      William A. Gatliff
  All rights reserved.     bgat@open-widgets.com

  See the file COPYING for details.

  This file is provided "as-is", and without any express
  or implied warranties, including, without limitation,
  the implied warranties of merchantability and fitness
  for a particular purpose.

  The author welcomes feedback regarding this file.

  SH-4 support added by Benoit Miller (fulg@iname.com).

  Support for Sega Dreamcast(tm) added by Benoit Miller.
  "SEGA" and "DREAMCAST" are registered trademarks of Sega
  Enterprises, Ltd.
*/

/* $Id: sh4-7750dc.c,v 1.4 2002/04/08 15:08:20 bgat Exp $ */

#include "sh4-775x-rmap.h"
#include "gdb.h"
#include "sh4-775x.h"

#define SH4_7750DC_BORDERCOLOR   ((volatile long *)0xa05f8040)
#define SH4_7750DC_STARTUP_STACK 0x8d000000
#define SH4_7750DC_BAUDRATE      57600

/* these symbols are created by the linker
   to help us find the text, bss and data
   sections, respectively; they aren't
   actual allocations */
extern char _data_start;
extern char _data_end;
extern const char _text_end;
extern char _bss_start;
extern char _bss_end;
extern char _stack_end;

void set_debug_traps();
int main ( void );

/* our stack */
long stack[400] __attribute__((section(".stack")));

/*
  THE startup function.

  Sets up some registers, then calls cstart().

  Note that this must be the FIRST FUNCTION 
  defined in this file!
*/
extern void start ( void );
__asm__ ("

  .align 4
  .global _start
_start:

  /* pad to stay out of another's delay slot */
  nop

  /* load known values into a few registers (for testing) */
  mov   #0, r0
  mov   #1, r1
  mov   #2, r2
  mov   #3, r3
  mov   #4, r4
  mov   #5, r5
  mov   #6, r6
  mov   #7, r7
  ldc   r0, r0_bank
  ldc   r1, r1_bank
  ldc   r2, r2_bank
  ldc   r3, r3_bank
  ldc   r4, r4_bank
  ldc   r5, r5_bank
  ldc   r6, r6_bank
  ldc   r7, r7_bank
  mov   #8, r8
  mov   #9, r9
  mov   #10, r10
  mov   #11, r11
  mov   #12, r12
  mov   #13, r13
  mov   #14, r14

  /* jump to C */
  bra   _cstart
  nop
");

/*
  Initializes peripheral hardware,
  sets up the C runtime environment,
  then calls main().
 */
void cstart ( void )
{
  register const char *src;
  register char *dst;

  while( 1 ) {

    /* Assume that the Dreamcast bootloader initialized the hardware... */

    /* Initialize SCIF */
    *SH4_775X_SCIF_SCSCR2 = 0x0000;     /* TE=0, RE=0, CKE1=0 */
    *SH4_775X_SCIF_SCFCR2 = 0x0006;     /* TFRST=1, RFRTS=1 */
    *SH4_775X_SCIF_SCSMR2 = 0x0000;     /* CHR=0, PE=0, STOP=0, CKS=00 */
                                        /* 8-N-1, Pphi/1 clock */
    *SH4_775X_SCIF_SCSPTR2 = 0x0080;    /* RTS=1 */
    *SH4_775X_SCIF_SCFCR2 = 0x0000;     /* RTRG=00, TTRG=00 */
                                        /* MCE=0, TFRST=0, RFRST=0, LOOP=0 */

    /* calculate and program SCBRR2 (baudrate). */
    *SH4_775X_SCIF_SCBRR2 = (50000000 / (32 * SH4_7750DC_BAUDRATE)) - 1;
 
    /* wait one bit interval before enabling SCIF */
    *SH4_775X_BSC_RFCR = 0xa400;        /* refresh counter clear */
    while( *SH4_775X_BSC_RFCR < 400 )
      ;

    /* enable SCIF */
    *SH4_775X_SCIF_SCSCR2 = 0x0038;     /* TIE=0, RIE=0, REIE=1, TE=1, RE=1 */

    /* disable this if code doesn't have any
       initialized global data, because we
       currently can't tell automatically if
       it's there or not */

    /* initialize data */
    for( dst = &_data_start, src = &_text_end;
         dst < &_data_end;
         dst++, src++ ) {

      *dst = *src;
    }

    /* zero out bss */
    for( dst = &_bss_start;
    dst < &_bss_end;
    dst++ ) {

      *dst = 0;
    }

    /* initialize VBR and exception handlers */
    set_debug_traps();

    /* initialize register file with sane defaults,
       and a good stack ptr */
    gdb_init_register_file( SH4_7750DC_STARTUP_STACK );

    /* change the screen border color to green 
       to tell the world we're running... */
    *SH4_7750DC_BORDERCOLOR = 0x0000ff00;

    /* call main */
    main();
  }
}

/* constants required above... */
__asm__ ("
  .align 4
Pphi_32rds: 
  .long 50000000/32
Baudrate:
  .long 57600
");


/*
  Gotta have this, because gcc wants it.
  (related to gcc's c++ support)
*/
void __main()
{
}


/*
  Kills the current application.
*/
void gdb_kill ( void )
{
  /* nothing on Dreamcast since there's no Operating System yet! =) */
}


/* simple startup code that just forces us
   to give control to the debugging stub */
int main ( void )
{
  while( 1 )
    gdb_monitor( 5 );

  return 0;
}

/* This code was originally from Dan Potter */

/* This scratch space will be used to form the VBR-based exception
   handler space. Rather than try to fit our code to a certain VMA,
   we will write new code into this space and set the VBR here. Care has
   to be taken to clear the cache (and put this in a non-cacheable area)
   or the handlers won't be seen.

   The weird size is to make sure we get enough space for all three vector
   entry points (0, 0x100, 0x400, 0x600) */
static unsigned short vectable[0x700/2] __attribute__((section(".vect")));

/* We'll use this to align the address later */
static unsigned short *vtaddr;

/* This is kind of a hack.. but GCC/AS aren't set up to handle it the
   way I'd really like, so that's what you get.. =) */
void build_exception_handler(unsigned short *dest, void *target_addr) {
#if 1
  unsigned long ta = (unsigned long)target_addr;
  unsigned char code[] = {
    0x06, 0x2f,           /* mov.l r0, @-r15 */
    0x01, 0xD0,           /* mov.l @(3,pc), r0 */
    0x2B, 0x40,           /* jmp @r0 */
    0xf6, 0x60,           /* mov.l @r15+, r0 */
    ta & 0xFF,            /* Target address, LSB */
    (ta >> 8) & 0xFF,     /* TA, LSB+1 */
    (ta >> 16) & 0xFF,    /* TA, MSB-1 */
    (ta >> 24) & 0xFF     /* TA, MSB */
  };
  /* Four instructions plus one data long */
  memcpy(dest, code, sizeof(code));
#else
  /* Test code: writes code to change the border color */
  unsigned char code[] = {
    0x03, 0xd0,   /* mov.l pc+0x10, r0 */
    0xff, 0xe1,   /* mov #-1, r1 */
    0x12, 0x20,   /* mov.l r1, @r0 */
    0xfe, 0xaf,   /* bra pc+0 */
    0x09, 0x00,   /* nop (observe alignment) */
    0x09, 0x00,   /* nop (observe alignment) */
    0x09, 0x00,   /* nop (observe alignment) */
    0x09, 0x00,   /* nop (observe alignment) */
    0x40, 0x80,   /* border reg address */
    0x5f, 0xa0
  };
  // Four instructions plus one data long
  memcpy(dest, code, sizeof(code));
#endif
}


/* Stub initialization part 1: this is called from set_debug_traps()
   before the VBR is set. This lets us put together an exception handler
   space before the processor is switched over. */
void stub_init_1() {

  /* We're running in P1 (cache), do this in P2 (no cache) */
  unsigned long vttmp = (unsigned long)vectable;

  /* Make sure it's 4-byte aligned */
  vttmp = ((vttmp + 3) & ~3) + 0x20000000;
  vtaddr = (unsigned short*)vttmp;

  /* De-cache the vectable */
  memset(vtaddr, 0, sizeof(vectable));

  /* Build exception handlers for the four states */
  build_exception_handler(vtaddr + 0x000/2, gdb_exception_dispatch);
  build_exception_handler(vtaddr + 0x100/2, gdb_exception_dispatch);
  build_exception_handler(vtaddr + 0x400/2, gdb_exception_dispatch);
  build_exception_handler(vtaddr + 0x600/2, gdb_exception_dispatch);
}

/* Stub initialization part 2: called from set_debug_traps() after the
   VBR is set. Basically just turns on traps. */
extern void stub_init_2();
__asm__ ("

  .global _stub_init_2
_stub_init_2:
  mov.l r1, @-r15

  /* Clear BL to allow traps and interrupts */
  stc   sr, r0
  mov.l Land, r1
  and   r1, r0
  ldc   r0, sr
  mov.l @r15+, r1

  rts
  nop

  .align 4
Land:
  .long 0xefffffff
Lor:
  .long 0x000000f0
");


/* Do all stub initialization */
void set_debug_traps() {

  /* Pre-init */
  stub_init_1();

  /* Setup the VBR */
  __asm__ ("
    /* install out VBR */
    mov.l _vbraddr,r0
    ldc   r0,vbr
    nop
    nop
    nop
  ");

  /* Post-init */
  stub_init_2();
}
/* Asm constant for set_debug_traps() */
__asm__ ("
  .align 4
_vbraddr:
  .long _vectable + 0x20000000
");


/* Exception event dispatch table. 
   Basically a transcript from table 5.2
   in the SH-4 hardware manual. */
const void *exception_event_table[]
__attribute__((section(".except"))) = {

  /* 000: Power-on reset, Hitachi UDI reset */
  start,

  /* 020: Manual reset */
  start,

  /* 040: Instruction TLB miss exception */
  /* 040: Data TLB miss exception (read) */
  gdb_unhandled_isr,

  /* 060: Data TLB miss exception (write) */
  gdb_unhandled_isr,

  /* 080: Initial page write exception */
  gdb_unhandled_isr,

  /* 0A0: Instruction TLB protection violation exception */
  /* 0A0: Data TLB protection violation exception */
  gdb_unhandled_isr,

  /* 0C0: Data TLB protection violation exception */
  gdb_unhandled_isr,

  /* 0E0: Instruction address error */
  /* 0E0: Data address error (read) */
  gdb_addresserr_isr,

  /* 100: Data address error (write) */
  gdb_addresserr_isr,

  /* 120: FPU exception */
  gdb_unhandled_isr,

  /* 140: Instruction TLB multiple-hit exception */
  /* 140: Data TLB multiple-hit exception */
  gdb_unhandled_isr,

  /* 160: Unconditional trap (TRAPA) */
  gdb_trapa_isr,

  /* 180: General illegal instruction exception */
  gdb_illegalinst_isr,

  /* 1A0: Slot illegal instruction exception */
  gdb_illegalinst_isr,

  /* 1C0: Nonmaskable interrupt */
  gdb_unhandled_isr,

  /* 1E0: User break before instruction execution */
  /* 1E0: User break after instruction execution */
  gdb_ubc_isr,

  /* 200-3C0: External interrupts */
  gdb_unhandled_isr,
  gdb_unhandled_isr,
  gdb_unhandled_isr,
  gdb_unhandled_isr,
  gdb_unhandled_isr,
  gdb_unhandled_isr,
  gdb_unhandled_isr,
  gdb_unhandled_isr,
  gdb_unhandled_isr,
  gdb_unhandled_isr,
  gdb_unhandled_isr,
  gdb_unhandled_isr,
  gdb_unhandled_isr,
  gdb_unhandled_isr,
  gdb_unhandled_isr,

  /* 3E0: (undocumented) */
  gdb_unhandled_isr,

  /* 400-760: Peripheral module interrupt */
  gdb_unhandled_isr,
  gdb_unhandled_isr,
  gdb_unhandled_isr,
  gdb_unhandled_isr,
  gdb_unhandled_isr,
  gdb_unhandled_isr,
  gdb_unhandled_isr,
  gdb_unhandled_isr,
  gdb_unhandled_isr,
  gdb_unhandled_isr,
  gdb_unhandled_isr,
  gdb_unhandled_isr,
  gdb_unhandled_isr,
  gdb_unhandled_isr,
  gdb_unhandled_isr,    /* 5C0: (undocumented) */
  gdb_unhandled_isr,    /* 5E0: (undocumented) */
  gdb_unhandled_isr,
  gdb_unhandled_isr,
  gdb_unhandled_isr,
  gdb_unhandled_isr,
  gdb_unhandled_isr,
  gdb_unhandled_isr,
  gdb_unhandled_isr,
  gdb_unhandled_isr,    /* 6E0: (undocumented) */
  gdb_unhandled_isr,
  gdb_unhandled_isr,
  gdb_unhandled_isr,
  gdb_unhandled_isr,

  /* 780-7E0: (undocumented) */
  gdb_unhandled_isr,
  gdb_unhandled_isr,
  gdb_unhandled_isr,
  gdb_unhandled_isr,

  /* 800: General FPU disable exception */
  gdb_unhandled_isr,

  /* 820: Slot FPU disable exception */
  gdb_unhandled_isr,
};

