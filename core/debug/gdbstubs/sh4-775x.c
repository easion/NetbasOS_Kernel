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
*/

/* $Id: sh4-775x.c,v 1.3 2002/04/08 15:08:20 bgat Exp $ */

#include "sh4-775x-rmap.h"
#include "gdb.h"


/* some local definitions to make life easier */
#define SCSMR2    SH4_775X_SCIF_SCSMR2
#define SCBRR2    SH4_775X_SCIF_SCBRR2
#define SCSCR2    SH4_775X_SCIF_SCSCR2
#define SCFTDR2   SH4_775X_SCIF_SCFTDR2
#define SCFSR2    SH4_775X_SCIF_SCFSR2
#define SCFRDR2   SH4_775X_SCIF_SCFRDR2
#define SCFCR2    SH4_775X_SCIF_SCFCR2
#define SCFDR2    SH4_775X_SCIF_SCFDR2
#define SCSPTR2   SH4_775X_SCIF_SCSPTR2
#define SCLSR2    SH4_775X_SCIF_SCLSR2

#define ER        SH4_775X_SCIF_SCFSR2_ER
#define TDFE      SH4_775X_SCIF_SCFSR2_TDFE
#define BRK       SH4_775X_SCIF_SCFSR2_BRK
#define PER       SH4_775X_SCIF_SCFSR2_PER
#define FER       SH4_775X_SCIF_SCFSR2_FER
#define RDF       SH4_775X_SCIF_SCFSR2_RDF

#define ORER      SH4_775X_SCIF_SCLSR2_ORER

#define BARA      SH4_775X_UBC_BARA
#define BAMRA     SH4_775X_UBC_BAMRA
#define BBRA      SH4_775X_UBC_BBRA
#define BASRA     SH4_775X_UBC_BASRA
#define BARB      SH4_775X_UBC_BARB
#define BAMRB     SH4_775X_UBC_BAMRB
#define BBRB      SH4_775X_UBC_BBRB
#define BASRB     SH4_775X_UBC_BASRB
#define BDRB      SH4_775X_UBC_BDRB
#define BDMRB     SH4_775X_UBC_BDMRB
#define BRCR      SH4_775X_UBC_BRCR

#define BASMA     SH4_775X_UBC_BAMRA_BASMA

#define IDA1      SH4_775X_UBC_BBRA_IDA1
#define IDA0      SH4_775X_UBC_BBRA_IDA0
#define RWA1      SH4_775X_UBC_BBRA_RWA1
#define RWA0      SH4_775X_UBC_BBRA_RWA0

#define CMFA      SH4_775X_UBC_BRBR_CMFA
#define CMFB      SH4_775X_UBC_BRBR_CMFB
#define PCBA      SH4_775X_UBC_BRBR_PCBA
#define DBEB      SH4_775X_UBC_BRBR_DBEB
#define PCBB      SH4_775X_UBC_BRBR_PCBB
#define SEQ       SH4_775X_UBC_BRBR_SEQ
#define UBDE      SH4_775X_UBC_BRBR_UBDE

typedef struct {
  unsigned long pr;
  unsigned long gbr;
  unsigned long vbr;
  unsigned long mach;
  unsigned long macl;
  unsigned long r[16];
  unsigned long pc;
  unsigned long sr;
} gdb_register_file_T;

gdb_register_file_T gdb_register_file
__attribute__((section(".regfile")));

/* stuff for stepi */
/* note: for these to work properly, (op) MUST be a long */
#define OPCODE_BT(op)         (((op) & 0xff00) == 0x8900)
#define OPCODE_BF(op)         (((op) & 0xff00) == 0x8b00)
#define OPCODE_BTF_DISP(op)   (((op) & 0x80) ? (((op) | 0xffffff80) << 1) : (((op) & 0x7f ) << 1))
#define OPCODE_BFS(op)        (((op) & 0xff00) == 0x8f00)
#define OPCODE_BTS(op)        (((op) & 0xff00) == 0x8d00)
#define OPCODE_BRA(op)        (((op) & 0xf000) == 0xa000)
#define OPCODE_BRA_DISP(op)   (((op) & 0x800) ? (((op) | 0xfffff800) << 1) : (((op) & 0x7ff) << 1))
#define OPCODE_BRAF(op)       (((op) & 0xf0ff) == 0x0023)
#define OPCODE_BRAF_REG(op)   (((op) & 0x0f00) >> 8)
#define OPCODE_BSR(op)        (((op) & 0xf000) == 0xb000)
#define OPCODE_BSR_DISP(op)   (((op) & 0x800) ? (((op) | 0xfffff800) << 1) : (((op) & 0x7ff) << 1))
#define OPCODE_BSRF(op)       (((op) & 0xf0ff) == 0x0003)
#define OPCODE_BSRF_REG(op)   (((op) >> 8) & 0xf)
#define OPCODE_JMP(op)        (((op) & 0xf0ff) == 0x402b)
#define OPCODE_JMP_REG(op)    (((op) >> 8) & 0xf)
#define OPCODE_JSR(op)        (((op) & 0xf0ff) == 0x400b)
#define OPCODE_JSR_REG(op)    (((op) >> 8) & 0xf)
#define OPCODE_RTS(op)        ((op) == 0xb)
#define SR_T_BIT_MASK         0x1

#define STEP_OPCODE           0xc320



/*
  Stuffs a byte out the serial port.
 */
void gdb_putc ( char c )
{
  /* wait for the previous byte to go */
  while( !( *SCFSR2 & TDFE ) )
    ;

  /* send the byte, clear read flag */
  *SCFTDR2 = c;
  *SCFSR2 &= ~TDFE;

  return;
}


/*
  Blocks until a byte is received at the serial port.
 */
char gdb_getc ( void )
{
  char c;

  /* clear any detected errors */
  *SCFSR2 &= ~( ER | PER | FER );
  *SCLSR2 &= ~( ORER );

  /* wait for a byte */
  while( !( *SCFSR2 & RDF ) )
    ;

  /* got one-- return it */
  c = *SCFRDR2;
  *SCFSR2 &= ~RDF;

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
  /* all our registers are longs */
  short retval = sizeof( long );


  switch( id ) {

  case 0:  case 1:  case 2:  case 3:
  case 4:  case 5:  case 6:  case 7:
  case 8:  case 9:  case 10: case 11:
  case 12: case 13: case 14: case 15:

    *val = gdb_register_file.r[id];
    break;

  case 16:

    *val = gdb_register_file.pc;
    break;

  case 17:

    *val = gdb_register_file.pr;
    break;

  case 18:

    *val = gdb_register_file.gbr;
    break;

  case 19:

    *val = gdb_register_file.vbr;
    break;

  case 20:

    *val = gdb_register_file.mach;
    break;

  case 21:

    *val = gdb_register_file.macl;
    break;

  case 22:

    *val = gdb_register_file.sr;
    break;

  default:
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


  switch( id ) {

  case 0:  case 1:  case 2:  case 3:
  case 4:  case 5:  case 6:  case 7:
  case 8:  case 9:  case 10: case 11:
  case 12: case 13: case 14: case 15:

    gdb_register_file.r[id] = val;
    break;

  case 16:

    gdb_register_file.pc = val;
    break;

  case 17:

    gdb_register_file.pr = val;
    break;

  case 18:

    gdb_register_file.gbr = val;
    break;

  case 19:

    gdb_register_file.vbr = val;
    break;

  case 20:

    gdb_register_file.mach = val;
    break;

  case 21:

    gdb_register_file.macl = val;
    break;

  case 22:

    gdb_register_file.sr = val;
    break;

  default:
    retval = 0;
  }

  return retval;
}


/*
  Analyzes the next instruction, to see where the program
  will go to when it runs.  Returns the destination address.
  
  TODO: support more than just the basic instructions here.
*/
long gdb_get_stepi_dest ( void )
{
  short op = *(short*)gdb_register_file.pc;
  long addr = gdb_register_file.pc + 2;

  /* BT, BT/S (untested!), BF and BF/S (untested!)
     TODO: test delay-slot branches */
  if((( OPCODE_BT( op ) || OPCODE_BTS( op ))
      && ( gdb_register_file.sr & SR_T_BIT_MASK ))
     || (( OPCODE_BF( op ) || OPCODE_BFS( op ))
         && !( gdb_register_file.sr & SR_T_BIT_MASK ))) {
    
    /* we're taking the branch */
    
    /* per 6.12 of the SH1/SH2 programming manual,
       PC+disp is address of the second instruction
       after the branch instruction, so we have to add 4 */
    /* TODO: spend more time understanding this magic */ 
    addr = gdb_register_file.pc + 4 + OPCODE_BTF_DISP( op );
  }
  
  /* BRA */
  else if( OPCODE_BRA( op ))
    addr = gdb_register_file.pc + 4 + OPCODE_BRA_DISP( op );
  
  /* BRAF (untested!)
     TODO: test BRAF */
  else if( OPCODE_BRAF( op ))
    addr = gdb_register_file.pc + 4
      + gdb_register_file.r[OPCODE_BRAF_REG( op )];
  
  /* BSR (untested!)
     TODO: test BSR */
  else if( OPCODE_BSR( op ))
    addr = gdb_register_file.pc + 4 + OPCODE_BSR_DISP( op );
  
  /* BSRF (untested!)
     TODO: test BSRF */
  else if( OPCODE_BSRF( op ))
    addr = gdb_register_file.pc + 4
      + gdb_register_file.r[OPCODE_BSRF_REG( op )];

  /* JMP (untested!)
     TODO: test JMP */
  else if( OPCODE_JMP( op ))
    addr = gdb_register_file.r[OPCODE_JMP_REG( op )];

  /* JSR */
  else if( OPCODE_JSR( op ))
    addr = gdb_register_file.r[OPCODE_JSR_REG( op )];
  
  /* RTS */
  else if( OPCODE_RTS( op ))
    addr = gdb_register_file.pr;

  return addr;
}

#if 1
/* This code is from the existing SH-Linux stub
   (http://www.m17n.org/linux-sh/index.html).

   (BM: It's not pretty, but it works. =)
*/

#define L1_CACHE_BYTES 32
#define CACHE_IC_ADDRESS_ARRAY   0xf0000000
#define CACHE_IC_ENTRY_MASK   0x1fe0

struct __large_struct { unsigned long buf[100]; };
#define __m(x) (*(struct __large_struct *)(x))

static inline void ctrl_outl(unsigned int b, unsigned long addr)
{
  *(volatile unsigned long*)addr = b;
}

/* Write back data caches, and invalidates instruction caches */
void gdb_flush_cache(void *start, void *end)
{
  unsigned long addr, data, v;

  (unsigned long)start &= ~(L1_CACHE_BYTES-1);

  for (v = (unsigned long)start; v < (unsigned long)end; v += L1_CACHE_BYTES)
  {
    /* Write back O Cache */
    asm volatile("ocbwb  %0"
      : /* no output */
      : "m" (__m(v)));

    /* Invalidate I Cache */
    addr = CACHE_IC_ADDRESS_ARRAY |
           (v & CACHE_IC_ENTRY_MASK) | 0x8 /* A-bit */;
    data = (v & 0xfffffc00); /* Valid=0 */
    ctrl_outl(data,addr);
   }
}

#else

/*
  Clear the cache. Assumes the stub normally runs in P1.

  (BM: WARNING! This code does not work! I kept it around for reference,
  in case anyone wants to fix it (hint, hint) since I'd rather use this 
  than the truly horrific code above...)
*/
extern void gdb_flush_cache ( void );
__asm__("

  .global _gdb_flush_cache
_gdb_flush_cache:

  /* save some registers... */
  mov.l r0, @-r15
  mov.l r1, @-r15
  mov.l r2, @-r15 

  /* since we're running in P1, 
     switch to non-cached P2 area */
  mov.l p2_area_target, r0 
  mov.l area_mask, r1
  mov.l p2_area_offset, r2
  and   r1, r0
  or    r2, r0
  jmp   @r0
  nop

p2_area_target:
  /* now that we're in P2, clear the caches 
     by setting the ICI and OCI bits in CCR */
  mov.l ccr_register, r0
  mov.l ccr_mask, r1
  mov.l @r0, r2
  or    r1, r2
  mov.l r2, @r0 
   
  /* perform a P1 area data access (as per section 4.2 
     of the SH4 hardware manual) at least four
     instructions after modifying CCR */
  mov   #0, r0 
  nop
  mov.l area_mask, r1
  mov.l p1_area_offset, r2
  mov.l @r0, r0 

  /* branch to a P1 area at least eight instructions
     after modifying CCR */
  mov.l p1_area_target, r0
  and   r1, r0
  or    r2, r0
  jmp   @r0
  nop

p1_area_target:
  /* we're back in P1, restore everything 
     and exit */
  mov.l @r15+, r2
  mov.l @r15+, r1
  mov.l @r15+, r0 

  rts
  nop

  .align 4
area_mask:
  .long 0x1fffffff
p1_area_offset:
  .long 0x80000000
p2_area_offset:
  .long 0xa0000000
ccr_register:
  .long 0xff00001c
ccr_mask:
  .long 0x00000808
");

#endif


#if 1
/*
  Uses the UBC to generate an exception
  after we execute the next instruction.
*/
void gdb_step ( char *hargs )
{
  long addr = 0;


  /* parse address, if any */
  while( *hargs != '#' )
    addr = ( addr << 4 ) + hex_to_long( *hargs++ );

  /* if we're stepping from an address, adjust pc */
  if( addr ) gdb_register_file.pc = addr;

  /* Use the UBC to break after instruction execution.
     
     This isn't perfect.  In particular, we won't stop when we want
     to if the target instruction disables interrupts (ldc/stc, among
     others). (BM: I'm not sure what happens if the target instruction is 
     in a delay slot.)

     In these cases, we'll execute one additional instruction before
     taking the UBC exception.  That's just how the UBC works, but I
     think this is still a better solution than patching in a TRAP
     opcode, especially if the target is in ROM.  :^)  b.g. */
  
  /* determine where the target instruction will send us to */
  addr = gdb_register_file.pc;


  /* set the breakpoint address in UBC channel A */
  *SH4_775X_UBC_BARA = addr;

  /* zero-out ASID bits, just in case */
  *SH4_775X_UBC_BASRA = 0x00;

  /* ignore BASRA bits in break conditions, don't mask any BARA bits */
  *SH4_775X_UBC_BAMRA = BASMA;

  /* break on r/w, instruction or operand access (ignoring size). */
  *SH4_775X_UBC_BBRA = IDA1 | IDA0 | RWA1 | RWA0;

  /* clear any condition match flags in BRCR */
  *SH4_775X_UBC_BRCR &= ~( CMFA | CMFB );

  /* trigger the break after instruction execution */
  *SH4_775X_UBC_BRCR = PCBA;

  /* must execute at least 11 instructions after changing UBC regs,
     as per section 20.2.1 of the SH7750 hardware manual */
  asm(" nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop");

  /* we're all done now */
  gdb_return_from_exception();

  return;
}

void gdb_undo_step ( void )
{
  /* nothing to do if we use UBC */
}

#else

/*
  Uses a TRAP to generate an exception
  after we execute the next instruction.
*/

short gdb_stepped_opcode;

void gdb_step ( char *hargs )
{
  long addr = 0;


  /* parse address, if any */
  while( *hargs != '#' )
    addr = ( addr << 4 ) + hex_to_long( *hargs++ );

  /* if we're stepping from an address, adjust pc (untested!) */
  /* TODO: test gdb_step when PC is supplied */
  if( addr ) gdb_register_file.pc = addr;

  /* determine where the target instruction will send us to */
  addr = gdb_get_stepi_dest();

  /* replace it */
  gdb_stepped_opcode = *(short*)addr;
  *(short*)addr = STEP_OPCODE;

  /* don't forget to flush the cache */
  gdb_flush_cache(addr, addr+2);

  /* we're all done now */
  gdb_return_from_exception();

  return;
}

void gdb_undo_step ( void )
{
  if( gdb_stepped_opcode ) {

    *(short*)gdb_register_file.pc = gdb_stepped_opcode;
    gdb_stepped_opcode = 0;

    /* don't forget to flush the cache */
    gdb_flush_cache(gdb_register_file.pc, gdb_register_file.pc+2);
  }

  return;
}

#endif


/*
  Continue program execution at addr,
  or at the current pc if addr == 0.
*/
void gdb_continue ( char *hargs )
{
  long addr = 0;


  /* parse address, if any */
  while( *hargs != '#' )
    addr = ( addr << 4 ) + hex_to_long( *hargs++ );

  if( addr )
    gdb_register_file.pc = addr;


  gdb_return_from_exception();

  return;
}


/*
  Generic code to save processor context.
  Expects sigval to be in r4.
*/
__asm__("

save_registers_handle_exception:

  /* find end of gdb_register_file */
  mov.l register_file_end, r0

  /* save registers */
  stc   ssr, r1
  mov.l r1, @r0
  stc.l spc, @-r0
  mov.l r15, @-r0
  mov.l r14, @-r0
  mov.l r13, @-r0
  mov.l r12, @-r0
  mov.l r11, @-r0
  mov.l r10, @-r0
  mov.l r9, @-r0
  mov.l r8, @-r0
  stc.l r7_bank, @-r0
  stc.l r6_bank, @-r0
  stc.l r5_bank, @-r0
  stc.l r4_bank, @-r0
  stc.l r3_bank, @-r0
  stc.l r2_bank, @-r0
  stc.l r1_bank, @-r0
  stc.l r0_bank, @-r0
  sts.l macl, @-r0
  sts.l mach, @-r0
  stc.l vbr, @-r0
  stc.l gbr, @-r0
  sts.l pr, @-r0

  /* call gdb_handle_exception */
  mov.l handle_exception, r0
  jsr @r0
  nop

  .align 4
handle_exception:
  .long _gdb_handle_exception
register_file_end:
  .long _gdb_register_file+88

");


/*
  Generic code to init processor context. 
  Expects initial_r15 parameter to be in r4.
*/
extern void gdb_init_register_file( long initial_r15 );
__asm__("

  .global _gdb_init_register_file
_gdb_init_register_file:

  /* save r0-r1 */
  mov.l r0, @-r15
  mov.l r1, @-r15

  /* find end of gdb_register_file */
  mov.l reg_file_end, r0

  /* initialize sr in register file */
  mov.l initial_sr, r1
  mov.l r1, @r0

  /* initialize ssr */
  ldc   r1, ssr

  /* skip pc in register file */
  add #-4, r0

  /* init r15 in register file */
  mov.l r4, @-r0

  /* skip r14-r0, macl, mach */
  add #-68, r0

  /* save vbr, gbr, pr in register file */
  stc.l vbr, @-r0
  stc.l gbr, @-r0
  sts.l pr, @-r0

  /* restore r0-r1 */
  mov.l @r15+, r1
  mov.l @r15+, r0

  /* return to caller */
  rts
  nop

  .align 4
reg_file_end:
  .long _gdb_register_file+88
  /* BM TODO: check this... 
     do we really want to run debuggees in priviledged mode? */
initial_sr:
  .long 0x40000000
");


/* 
  Generic exception handler. 
  Dispatches to the proper handler.

  Note that the SH4 will automatically inhibit
  further interrupts and exceptions, put us in
  priviledged mode, and switch to register bank 1
  so we don't need to save R0-R7 here.

  TODO: we really should switch back to our own stack.
  Right now this stays on the debuggee's stack...
*/
extern void gdb_exception_dispatch ( void );
__asm__("

  .global _gdb_exception_dispatch
_gdb_exception_dispatch:

  /* read exception event register 
     to see what happened */
  mov.l expevt_register, r0
  mov.l @r0, r1

  /* just keep the exception code, 
     mask off everything else */
  mov.l exception_code_mask, r0
  and   r0, r1

  /* transform exception code to an 
     offset into our exception table.

     code = (code >> 5) * sizeof( void* );
  */
  shlr2 r1
  shlr  r1

  /* jump to appropriate handler */
  mov.l exception_event, r0
  add   r1, r0
  mov.l @r0, r0 
  jmp   @r0
  nop

  .align 4
exception_event:
  .long _exception_event_table
expevt_register:
  .long 0xff000024
exception_code_mask:
  .long 0x00000fff

");


/*
  Unhandled exception isr.

  Not really much we can do here, so
  we just send a SIGUSR to gdb_handle_exception().
*/
extern void gdb_unhandled_isr ( void );
__asm__("

  .global _gdb_unhandled_isr
_gdb_unhandled_isr:

  /* load SIGUSR in r4 */
  mov #30, r4

  /* save registers, call gdb_handle_exception */
  bra save_registers_handle_exception
  nop

");


/*
  Generic TRAPA dispatcher. 

  This allows to handle each trap differently depending
  on the operand.
*/
extern void gdb_trapa_isr ( void );
__asm__ ("

  .global _gdb_trapa_isr
_gdb_trapa_isr:

  /* read trap exception register 
     to see which trap was called */
  mov.l tra_register, r1
  mov.l @r1, r0

  /* mask out uninteresting bits */
  mov.l trap_value_mask, r1
  and   r1, r0
  shlr2 r0

  /* check for TRAPA #32:
     (GDB software breakpoint) */
  cmp/eq #32, r0
  bt    trapa32

  /* TODO: add more handlers here */
 
  /* unknown trap called, 
     call generic handler */
  bra   unknown_trap
  nop

unknown_trap:
  mov.l unknown_trapa_isr, r0
  jmp   @r0
  nop

trapa32:
  mov.l trapa32_isr, r0
  jmp   @r0
  nop

  .align 4
tra_register:
  .long 0xff000020
trap_value_mask:
  .long 0x000003fc

unknown_trapa_isr:
  .long _gdb_unknown_trapa_isr
trapa32_isr:
  .long _gdb_trapa32_isr
");


/*
  Generic TRAPA isr.
  Sends a SIGTRAP to gdb_handle_exception().
*/
extern void gdb_unknown_trapa_isr ( void );
__asm__ ("

  .global _gdb_unknown_trapa_isr
_gdb_unknown_trapa_isr:

  /* load SIGTRAP in r4 */
  mov #5, r4

  /* save registers,
     call gdb_handle_exception */
  bra save_registers_handle_exception
  nop

");


/*
  TRAPA #32 (breakpoint) isr.
  Sends a SIGTRAP to gdb_handle_exception().

  Because we always subtract 2 from the pc
  stacked during exception processing, this
  function won't permit compiled-in breakpoints.
  If you compile a TRAPA #32 into the code, we'll
  loop on it indefinitely.  Use TRAPA #33 instead.
*/
extern void gdb_trapa32_isr ( void );
__asm__ ("

  .global _gdb_trapa32_isr
_gdb_trapa32_isr:

  /* fudge pc, so we re-execute the instruction replaced
     by the trap; this breaks compiled-in breakpoints! */
  stc spc, r0  
  add #-2, r0
  ldc r0, spc

  /* load SIGTRAP in r4 */
  mov #5, r4

  /* save registers,
     call gdb_handle_exception */
  bra save_registers_handle_exception
  nop

");


/*
  Illegal instruction isr.
  Sends a SIGILL to gdb_handle_exception().
*/
extern void gdb_illegalinst_isr ( void );
__asm__ ("

  .global _gdb_illegalinst_isr
_gdb_illegalinst_isr:

  /* load SIGILL in r4 */
  mov #4, r4

  /* save registers,
     call gdb_handle_exception */
  bra save_registers_handle_exception
  nop

");


/*
  Address error isr.
  Sends a SIGSEGV to gdb_handle_exception().
*/
extern void gdb_addresserr_isr ( void );
__asm__ ("

  .global _gdb_addresserr_isr
_gdb_addresserr_isr:

  /* load SIGSEGV in r4 */
  mov #11, r4

  /* save registers,
     call gdb_handle_exception */
  bra save_registers_handle_exception
  nop

");



/*
  UBC isr.

  Turns off UBC, then calls save_registers_handle_exception.
*/
extern void gdb_ubc_isr ( void );
__asm__ ("

  .global _gdb_ubc_isr
_gdb_ubc_isr:

  /* zero BBRA to turn off UBC */
  mov.l ubc_bbra, r0
  mov #0, r1
  mov.w r1, @r0

  /* zero BBRB to turn off UBC */
  mov.l ubc_bbrb, r0
  mov.w r1, @r0

  /* load SIGTRAP in r4 */
  mov #5, r4

  /* save registers,
     call gdb_handle_exception */
  bra save_registers_handle_exception
  nop

  .align 4
ubc_bbra:
  .long 0xff200008
ubc_bbrb:
  .long 0xff200014

");

/*
  Restores registers to the values specified
  in gdb_register_file.
*/
__asm__ ("

  .global _gdb_return_from_exception
_gdb_return_from_exception:

  /* find gdb_register_file */
  mov.l register_file, r0

  /* restore registers */
  lds.l @r0+, pr
  ldc.l @r0+, gbr
  ldc.l @r0+, vbr
  lds.l @r0+, mach
  lds.l @r0+, macl
  ldc.l @r0+, r0_bank
  ldc.l @r0+, r1_bank
  ldc.l @r0+, r2_bank
  ldc.l @r0+, r3_bank
  ldc.l @r0+, r4_bank
  ldc.l @r0+, r5_bank
  ldc.l @r0+, r6_bank
  ldc.l @r0+, r7_bank
  mov.l @r0+, r8
  mov.l @r0+, r9
  mov.l @r0+, r10
  mov.l @r0+, r11
  mov.l @r0+, r12
  mov.l @r0+, r13
  mov.l @r0+, r14
  mov.l @r0+, r15
  ldc.l @r0+, spc
  ldc.l @r0+, ssr

  /* we're done-- return */
  rte
  nop

  .align 4
register_file:
  .long _gdb_register_file

");

