/*
    mcore.c - Architecture-specific code for a gdb stub for M*Core

    Author:  Brian LaPonsey
             Motorola, East Kilbride

    Copyright (c) Motorola, Inc., 2002
    ALL RIGHTS RESERVED

    You are hereby granted a copyright license to use, modify, and
    distribute the SOFTWARE so long as this entire notice is retained
    without alteration in any modified and/or redistributed versions,
    and that such modified versions are clearly identified as such.
    No licenses are granted by implication, estoppel or otherwise under
    any patents or trademarks of Motorola, Inc.

    The SOFTWARE is provided on an "AS IS" basis and without warranty.
    To the maximum extent permitted by applicable law, MOTOROLA DISCLAIMS
    ALL WARRANTIES WHETHER EXPRESS OR IMPLIED, INCLUDING IMPLIED
    WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR
    PURPOSE AND ANY WARRANTY AGAINST INFRINGEMENT WITH
    REGARD TO THE SOFTWARE (INCLUDING ANY MODIFIED VERSIONS
    THEREOF) AND ANY ACCOMPANYING WRITTEN MATERIALS.

    To the maximum extent permitted by applicable law, IN NO EVENT SHALL
    MOTOROLA BE LIABLE FOR ANY DAMAGES WHATSOEVER
    (INCLUDING WITHOUT LIMITATION, DAMAGES FOR LOSS OF
    BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF BUSINESS
    INFORMATION, OR OTHER PECUNIARY LOSS) ARISING OF THE USE OR
    INABILITY TO USE THE SOFTWARE.   Motorola assumes no responsibility
    for the maintenance and support of the SOFTWARE.
*/

#include "mcore.h"
#include "mmc2107.h"
#include <string.h>

#define NUM_VECS 64 /* does not use the entire 128-word vector table */
#define NUM_REGS 46 /* see the README for a discussion of this value */

#define VALID_REG_ID(reg_id) ((reg_id >= 0) && (reg_id <= (NUM_REGS-1)))

/* SYNCR Multiplication Factor Divider field sets clock frequency */
/* default value gives 32MHz fSYS from a 4MHz reference clock */
#if !defined SYNCR_MFD
#define SYNCR_MFD 0x6000
#endif

/* Processor Status Register - Instruction Trace Mode bit mask */
#define mITM 0x00004000

/* array indices into reg_file[] */
enum
{ _r0,   _r1,   _r2,   _r3,   _r4,   _r5,   _r6,   _r7,
  _r8,   _r9,   _r10,  _r11,  _r12,  _r13,  _r14,  _r15,
  _ar0,  _ar1,  _ar2,  _ar3,  _ar4,  _ar5,  _ar6,  _ar7,
  _ar8,  _ar9,  _ar10, _ar11, _ar12, _ar13, _ar14, _ar15,
  _psr,  _vbr,  _epsr, _fpsr, _epc,  _fpc,  _ss0,  _ss1,
  _ss2,  _ss3,  _ss4,  _gcr,  _gsr,  _pc
};

/* reserve stack space  */
unsigned char stub_stack[512] __attribute__ ((section(".stub_stack")));

/* storage space for memory image of mcore register set */
static long reg_file[NUM_REGS];

/* received character and its flag must be visible to ISRs */
static char GDBch;
static unsigned long GotGDBch=FALSE;

const char hextab[] = "0123456789abcdef";

/* lookup table to translate mcore vector into gdb signal */
/* messages provide feedback about actual mcore exception */

#if defined ENABLE_VERBOSE_SIGNAL_MSG
#define ENABLE_SIGNAL_MSG
#endif

#if !defined ENABLE_SIGNAL_MSG
/* don't waste string space if signal messages are disabled */

const unsigned char sig_tab[] = {
  GDB_SIGKILL, GDB_SIGBUS,  GDB_SIGBUS,  GDB_SIGFPE,
  GDB_SIGILL,  GDB_SIGILL,  GDB_SIGTRAP, GDB_SIGTRAP,
  GDB_SIGABRT, GDB_SIGKILL, GDB_SIGINT,  GDB_SIGINT,
  GDB_SIGSYS,  GDB_SIGSYS,  GDB_SIGSYS,  GDB_SIGSYS,
  GDB_SIGTRAP, GDB_SIGTRAP, GDB_SIGTRAP, GDB_SIGTRAP,
  GDB_SIGSYS,  GDB_SIGSYS,  GDB_SIGSYS,  GDB_SIGSYS,
  GDB_SIGSYS,  GDB_SIGSYS,  GDB_SIGSYS,  GDB_SIGSYS,
  GDB_SIGSYS,  GDB_SIGSYS,  GDB_SIGSYS,  GDB_SIGSYS,
  GDB_SIG32,   GDB_SIG33,   GDB_SIG34,   GDB_SIG35,
  GDB_SIG36,   GDB_SIG37,   GDB_SIG38,   GDB_SIG39,
  GDB_SIG40,   GDB_SIG41,   GDB_SIG42,   GDB_SIG43,
  GDB_SIG44,   GDB_SIG45,   GDB_SIG46,   GDB_SIG47,
  GDB_SIG48,   GDB_SIG49,   GDB_SIG50,   GDB_SIG51,
  GDB_SIG52,   GDB_SIG53,   GDB_SIG54,   GDB_SIG55,
  GDB_SIG56,   GDB_SIG57,   GDB_SIG58,   GDB_SIG59,
  GDB_SIG60,   GDB_SIG61,   GDB_SIG62,   GDB_SIG63
};

#else /* defined ENABLE_SIGNAL_MSG */

typedef struct {
  unsigned char signal;
  char *msg;
} sig_tab_t;

const sig_tab_t sig_tab[] = {
  { GDB_SIGKILL, "Hardware Reset"      },
  { GDB_SIGBUS,  "Misaligned access"   },
  { GDB_SIGBUS,  "Access error"        },
  { GDB_SIGFPE,  "Divide by zero"      },
  { GDB_SIGILL,  "Illegal instruction" },
  { GDB_SIGILL,  "Privilege violation" },
  { GDB_SIGTRAP, "Tracepoint"          },
  { GDB_SIGTRAP, "Breakpoint"          },
  { GDB_SIGABRT, "Unrecoverable error" },
  { GDB_SIGKILL, "Software reset"      },
  { GDB_SIGINT,  "Normal autovector"   },
  { GDB_SIGINT,  "Fast autovector"     },
  { GDB_SIGSYS,  "Hardware accelerator"},
  { GDB_SIGSYS,  "" },
  { GDB_SIGSYS,  "" },
  { GDB_SIGSYS,  "" },
  { GDB_SIGTRAP, "Trap 0 instruction"  },
  { GDB_SIGTRAP, "Trap 1 instruction"  },
  { GDB_SIGTRAP, "Trap 2 instruction"  },
  { GDB_SIGTRAP, "Trap 3 instruction"  },
  { GDB_SIGSYS,  "" },
  { GDB_SIGSYS,  "" },
  { GDB_SIGSYS,  "" },
  { GDB_SIGSYS,  "" },
  { GDB_SIGSYS,  "" },
  { GDB_SIGSYS,  "" },
  { GDB_SIGSYS,  "" },
  { GDB_SIGSYS,  "" },
  { GDB_SIGSYS,  "" },
  { GDB_SIGSYS,  "" },
  { GDB_SIGSYS,  "" },
  { GDB_SIGSYS,  "" },
  { GDB_SIG32,   "Priority 0 vectored interrupt"  },
  { GDB_SIG33,   "Priority 1 vectored interrupt"  },
  { GDB_SIG34,   "Priority 2 vectored interrupt"  },
  { GDB_SIG35,   "Priority 3 vectored interrupt"  },
  { GDB_SIG36,   "Priority 4 vectored interrupt"  },
  { GDB_SIG37,   "Priority 5 vectored interrupt"  },
  { GDB_SIG38,   "Priority 6 vectored interrupt"  },
  { GDB_SIG39,   "Priority 7 vectored interrupt"  },
  { GDB_SIG40,   "Priority 8 vectored interrupt"  },
  { GDB_SIG41,   "Priority 9 vectored interrupt"  },
  { GDB_SIG42,   "Priority 10 vectored interrupt" },
  { GDB_SIG43,   "Priority 11 vectored interrupt" },
  { GDB_SIG44,   "Priority 12 vectored interrupt" },
  { GDB_SIG45,   "Priority 13 vectored interrupt" },
  { GDB_SIG46,   "Priority 14 vectored interrupt" },
  { GDB_SIG47,   "Priority 15 vectored interrupt" },
  { GDB_SIG48,   "Priority 16 vectored interrupt" },
  { GDB_SIG49,   "Priority 17 vectored interrupt" },
  { GDB_SIG50,   "Priority 18 vectored interrupt" },
  { GDB_SIG51,   "Priority 19 vectored interrupt" },
  { GDB_SIG52,   "Priority 20 vectored interrupt" },
  { GDB_SIG53,   "Priority 21 vectored interrupt" },
  { GDB_SIG54,   "Priority 22 vectored interrupt" },
  { GDB_SIG55,   "Priority 23 vectored interrupt" },
  { GDB_SIG56,   "Priority 24 vectored interrupt" },
  { GDB_SIG57,   "Priority 25 vectored interrupt" },
  { GDB_SIG58,   "Priority 26 vectored interrupt" },
  { GDB_SIG59,   "Priority 27 vectored interrupt" },
  { GDB_SIG60,   "Priority 28 vectored interrupt" },
  { GDB_SIG61,   "Priority 29 vectored interrupt" },
  { GDB_SIG62,   "Priority 30 vectored interrupt" },
  { GDB_SIG63,   "Priority 31 vectored interrupt" }
};
#endif /* !defined ENABLE_SIGNAL_MSG */


/* _stub_start

   Program entry point
   Initializes stack, C runtime environment and hardware
   Calls main()
*/
void
_stub_start( void )
{
  /* disable interrupts */
  __asm__("mfcr  r1, psr");
  __asm__("bclri r1, 4");
  __asm__("bclri r1, 6");
  __asm__("bseti r1, 8");
  __asm__("mtcr  r1, psr");

  /* initialize vector base register */
  __asm__("lrw   r1, vec_tab");
  __asm__("mtcr  r1, vbr");

  /* initialize stack pointer */
  __asm__("lrw   r1, _stub_stack");
  __asm__("mov   r0,  r1");

  /* CPU hardware initialization */
  rSYNCR = SYNCR_MFD; /* need 32MHz for SCI baud rate */
  rWCR   = 0x000E;    /* turn off watchdog timer */

#if defined CSCR0	 
  rCSCR0 = CSCR0;	  /* Chip select initialization     */
#endif				  /*                                */
					  /* Used when building a gdb stub  */
#if defined CSCR1	  /* that uses external RAM memory  */
  rCSCR1 = CSCR1;	  /* resources at startup.  If the  */
#endif				  /* stub needs only internal RAM,  */
					  /* these chip selects should be   */
#if defined CSCR2	  /* configured by gdb after stub   */
  rCSCR2 = CSCR2;	  /* communications are established */
#endif				  /* using the .gdbinit executable. */
					  /*                                */
#if defined CSCR3	  /* See the Makefile and .gdbinit  */
  rCSCR3 = CSCR3;     /* for examples.                  */
#endif			

  /* zero the .bss data space */
  __asm__("lrw   r1, _stub_bss_start");
  __asm__("lrw   r2, _stub_bss_end");
  __asm__("cmphs r1, r2");
  __asm__("bt    lab1");
  __asm__("movi  r3, 0");

  __asm__("lab0:");
  __asm__("st.b  r3, (r1,0)");
  __asm__("addi  r1, 1");
  __asm__("cmphs r1, r2");
  __asm__("bf    lab0");

  /* byte-copy data from ROM to RAM */
  __asm__("lab1:");
  __asm__("lrw   r1, _stub_data_start");
  __asm__("lrw   r2, _stub_data_end");
  __asm__("cmphs r1, r2");
  __asm__("bt    lab3");
  __asm__("lrw   r3, _stub_data_ROM");

  __asm__("lab2:");
  __asm__("ld.b  r4, (r3, 0)");
  __asm__("st.b  r4, (r1, 0)");
  __asm__("addi  r3, 1");
  __asm__("addi  r1, 1");
  __asm__("cmphs r1, r2");
  __asm__("bf    lab2");
  __asm__("lab3:");

  /* copy the vector table from flash to RAM
     The stub doesn't need this, but applications
     have to be able to install their own vectors. */

  /* r1 is source (vector table load memory address) */
  /* r2 is dest (vector table virtual memory adress) */
  /* r3 is count (64 words -- don't need the whole table) */

  __asm__("lrw   r1, vector_LMA");
  __asm__("mfcr  r2, vbr");

  /* don't move anything if user wants vectors in flash */
  __asm__("cmpne r1, r2");
  __asm__("bf    lab5"); 

  /* Initialize counter to copy 64 words from vector table */
  __asm__("movi  r3, 64");

  /* main copy loop -- move vector table into RAM */
  __asm__("lab4:");
  __asm__("ld.w  r4, (r1, 0)");
  __asm__("st.w  r4, (r2, 0)");
  __asm__("addi  r1, 4");
  __asm__("addi  r2, 4");
  __asm__("decne r3");
  __asm__("bt lab4");
  __asm__("lab5:");

  /* SCI setup for stub's communications */
  rSCI1BD  = 0x0012; /* SCI1 Baud Rate Register (115.2K) */
  rSCI1CR2 = 0x2C;   /* SCI1 Control Register 2 */

  /* Interrupt controller initialization */
  rICR     = 0x0000; /* use vectored interrupts */
  rPLSR8   = 31;     /* make SCI1 RDRF (source 8) priority-31 */
  rFIER   |= 1<<31;  /* enable priority-31's as fast ints */

  /* set fast interrupt enable bit in PSR */
  __asm__("mfcr  r1, psr");
  __asm__("bseti r1, 4");
  __asm__("mtcr  r1, psr");

  /* jump to _start routine
     This might be the entry point of your application,
     or just the entry point of the resident stub.
  */
  _start();
  _stub_exit();
}


/* _stub_exit

   Uses a trap to stop execution instead of a breakpoint
   because the signal() routine will step over a trap.
   If you continue after the trap, execution will resume
   at _stub_start
*/
void
_stub_exit( void )
{
  __asm__("trap 0");
  __asm__("jmpi _stub_start");
}


/* gdb_putc

   Send a byte out the SCI port
*/
int
gdb_putc ( char c )
{
  /* wait for the previous byte to go */
  while(!(rSCI1SR1 & mTDRE)) { ; }

  /* send the byte */
  rSCI1DRL = c;

  /* retval is used for checksum by some callers in gdb.c */
  return (c & 0xff);
}


/* gdb_getc

   Get a byte from the SCI port.
*/
char
gdb_getc ( void )
{
  while(!(GotGDBch)) { ; }  /* wait for a byte to appear */
  GotGDBch = FALSE;         /* clear flag for next byte */
  return GDBch;             /* global storage for received char */
}


/* isr_RDRF1 (Receive Data Register Full)

   Fast interrupt service routine to receive one char and set flag.
   Forces a trace exception if a break (Ctrl-C) is received.  Uses 
   the alternate registers, so the stack is not used or modified.
*/
void
isr_RDRF1 ( void ) /* __attribute__ ((naked)) */
{
  rSCI1SR1;                 /* must read SCI1SR1 to clear RDRF */
  GDBch = rSCI1DRL;         /* store GDB char and clear RDRF */

  if ( '\3' == GDBch )      /* watch for a Ctrl-C character */
  {
    __asm__("mfcr  r6, fpsr"); /* set up to pass control to gdb */
    __asm__("bseti r6, 14  "); /* set the FPSR trace mode bit */
    __asm__("mtcr  r6, fpsr"); /* we'll get a trace after rfi */
  }
  else
  {
    GotGDBch = TRUE;        /* raise flag */
  }

  __asm__("rfi");           /* return from fast interrupt */
}


/* gdb_peek_register_file

  Retrieves a register value from reg_file.
  Returns the size of the register, in bytes,
  or zero if an invalid id is specified.
*/
int
gdb_peek_register_file (int id, long *val)
{
  /* all mcore registers are longs */
  int retval = sizeof(long);

  if (VALID_REG_ID(id))
  {
    *val = reg_file[id];
  }
  else
  {
    retval = 0;
  }

  return retval;
}


/* gdb_poke_register_file

  Stuffs a register value into reg_file.
  Returns the size of the register, in bytes,
  or zero if an invalid id is specified.
*/
int
gdb_poke_register_file ( int id, long val )
{
  /* all mcore registers are longs */
  int retval = sizeof(long);

  if (VALID_REG_ID(id))
  {
    reg_file[id] = val;
    if (id == _pc)          /* if gdb wants to modify the  */
    {                       /* pc, then also put new addr  */
	  reg_file[_epc] = val; /* into the pc shadow register */
    }                       /* so the change will persist  */
  }                         /* after the exception returns */
  else
  {
    retval = 0;
  }

  return retval;
}


/* gdb_step

   Implements the gdb "step instruction" (stepi) command.
   Sets the instruction trace mode bit in the reg_file image
   of the epsr.  After gdb_catch_exception returns, the next
   instruction will cause a trace exception.
*/
void
gdb_step ( long addr )
{

  if (addr)
    reg_file[_epc] = addr;

  /* set trace mode so we get a trace exception after rte */
  reg_file[_epsr] |= mITM;

  return;
}


/* gdb_continue

   Continue execution at addr, or at current pc if addr == 0.
   Clears the instruction trace mode bit in the reg_file image
   of the epsr.  When the gdb_catch_exception ISR returns, program
   execution continues normally.
*/
void
gdb_continue ( long addr )
{
  if (addr)
    reg_file[_epc] = addr;

  /* clear instruction trace mode (bit 14 in PSR) */
  reg_file[_epsr] &= ~mITM;

  return;
}


/* gdb_detach

   Works pretty much the same way as gdb_continue,
   except you can't pass it an "address to detach from"
*/
void
gdb_detach ( void )
{
  /* clear instruction trace mode (bit 14 in PSR) */
  reg_file[_epsr] &= ~mITM;

  return;
}


/* gdb_kill

   kills the current application.  Simulates a reset by jumping to
   the program entry point.  All context and stack info are lost.
*/
void
gdb_kill ( void )
{
  __asm__("jmpi _stub_start");
}


/* gdb_flush_cache
   gdb_monitor_onentry
   gdb_monitor_onexit
   gdb_return_from_exception

   These functions don't do anything except keep gdb.c happy
*/
void
gdb_flush_cache ( void* cache_start, int cache_len )
{
}

void
gdb_monitor_onentry ( void )
{
}

void
gdb_monitor_onexit ( void )
{
}

void
gdb_return_from_exception ( void )
{
}


/* gdb_catch_exception

   Interrupt service routine to catch exceptions intended for the stub.

   1) saves machine state into register file image so gdb can see it
   2) calls signal() function
   3) completely restores context that existed before the exception

   Corrupts the SS0, SS1 and AR0 registers
*/
void
gdb_catch_exception ( void ) /* __attribute__ ((naked)) */
{
  __asm__("mtcr  r0, ss0         "); /* save user's stack pointer    */
  __asm__("mtcr  r1, ss1         "); /* need r1, save it on ss1      */
  __asm__("lrw   r1, reg_file    "); /* storage for user registers   */
  __asm__("stw   r0, (r1,0)      "); /* image user's stack pointer   */
  __asm__("mov   r0, r1          "); /* move storage pointer to r0   */
  __asm__("mfcr  r1, ss1         "); /* restore r1                   */
  __asm__("addi  r0, 4           "); /* increment past r0            */
  __asm__("stm   r1-r15, (r0)    "); /* image user regs r1-r15       */

  __asm__("mfcr  r0, psr         "); /* get current psr              */
  __asm__("bseti r0, 1           "); /* set the alternate file bit   */
  __asm__("mtcr  r0, psr         "); /* now using the alt reg's      */

  __asm__("mtcr  r1, ss1         "); /* need ar1, save it on ss1     */
  __asm__("lrw   r1, reg_file+64 "); /* storage for alternate reg's  */
  __asm__("stw   r0, (r1,0)      "); /* save alternate reg ar0       */
  __asm__("mov   r0, r1          "); /* move storage pointer to ar0  */
  __asm__("mfcr  r1, ss1         "); /* restore ar1                  */
  __asm__("addi  r0, 4           "); /* increment past ar0           */
  __asm__("stm   r1-r15, (r0)    "); /* copy alternate regs ar1-ar15 */

  __asm__("mfcr  r0, psr         "); /* current psr into ar1         */
  __asm__("bclri r0, 1           "); /* clear the alternate file bit */
  __asm__("mtcr  r0, psr         "); /* now using the regular reg's  */

  __asm__("mfcr  r2, epsr        "); /* psr before present exception */
  __asm__("mfcr  r3, vbr         "); /* vector base reg              */
  __asm__("mfcr  r4, epsr        "); /* norm exc PSR shadow reg      */
  __asm__("mfcr  r5, fpsr        "); /* fast exc PSR shadow reg      */
  __asm__("mfcr  r6, epc         "); /* norm exc PC shadow reg       */
  __asm__("mfcr  r7, fpc         "); /* fast exc PC shadow reg       */
  __asm__("mfcr  r8, ss0         "); /* ss0 holds stack pointer      */
  __asm__("mfcr  r9, ss1         "); /* supervisor scratch reg 1     */
  __asm__("mfcr  r10, ss2        "); /* supervisor scratch reg 2     */
  __asm__("mfcr  r11, ss3        "); /* supervisor scratch reg 3     */
  __asm__("mfcr  r12, ss4        "); /* supervisor scratch reg 4     */
  __asm__("mfcr  r13, gcr        "); /* global control reg           */
  __asm__("mfcr  r14, gsr        "); /* global status reg            */
  __asm__("mfcr  r15, epc        "); /* PC before present exception  */

  __asm__("lrw   r1, reg_file+128"); /* addr of control reg storage  */
  __asm__("mov   r0, r1          "); /* load r0 for block save inst  */
  __asm__("stm   r2-r15, (r0)    "); /* copy 13 control regs + pc    */

  __asm__("mfcr  r2, psr         "); /* get current psr into r2      */
  __asm__("lsli  r2, 9           "); /* isolate vector in r2         */
  __asm__("lsri  r2, 25          "); /* r2 is first function arg     */
  __asm__("lrw   r1, _stub_stack "); /* get stub's stack pointer     */
  __asm__("mov   r0, r1          "); /* restore stub's stack pointer */
  __asm__("jsri  signal          "); /* signal gdb; get instructions */

  __asm__("lrw   r1, reg_file+132"); /* address of VBR in reg file   */
  __asm__("mov   r0, r1          "); /* load r0 for block save inst  */
  __asm__("ldm   r4-r15, (r0)    "); /* retreive control regs        */
  __asm__("mtcr  r4, vbr         "); /* restore vector base reg      */
  __asm__("mtcr  r5, epsr        "); /* norm exc PSR shadow reg      */
  __asm__("mtcr  r6, fpsr        "); /* fast exc PSR shadow reg      */
  __asm__("mtcr  r7, epc         "); /* norm exc PC shadow reg       */
  __asm__("mtcr  r8, fpc         "); /* fast exc PC shadow reg       */
                                     /* don't touch r0 (SP) in ss0   */
  __asm__("mtcr  r10, ss1        "); /* supervisor scratch reg 1     */
  __asm__("mtcr  r11, ss2        "); /* supervisor scratch reg 2     */
  __asm__("mtcr  r12, ss3        "); /* supervisor scratch reg 3     */
  __asm__("mtcr  r13, ss4        "); /* supervisor scratch reg 4     */
  __asm__("mtcr  r14, gcr        "); /* global control reg           */
  __asm__("mtcr  r15, gsr        "); /* global status reg            */

  __asm__("mfcr  r0, psr         "); /* get current psr              */
  __asm__("bseti r0, 1           "); /* set the alternate file bit   */
  __asm__("mtcr  r0, psr         "); /* now using the alt registers  */

  __asm__("lrw   r1, reg_file+64 "); /* storage for alternate reg's  */
  __asm__("mov   r0, r1          "); /* move storage pointer to ar0  */
  __asm__("addi  r0, 4           "); /* inc past where ar0 was saved */
  __asm__("ldm   r1-r15, (r0)    "); /* restore alt regs ar1-ar15    */

  __asm__("mfcr  r0, psr         "); /* get current psr              */
  __asm__("bclri r0, 1           "); /* clear the alternate file bit */
  __asm__("mtcr  r0, psr         "); /* now using normal user reg's  */

  __asm__("lrw   r1, reg_file    "); /* storage area for user reg's  */
  __asm__("mov   r0, r1          "); /* move storage pointer to r0   */
  __asm__("addi  r0, 4           "); /* inc past where r0 was saved  */
  __asm__("ldm   r1-r15, (r0)    "); /* restore user regs r1-r15     */
  __asm__("mfcr  r0, ss0         "); /* restore user's stack pointer */
  __asm__("rte                   "); /* rte restores PC & PSR        */
}


/* signal

   Table-lookup prints a message to the console identifying
   the exception, translates the mcore vector into a signal
   that gdb can understand, and calls gdb_handle_exception()
   to request instructions from gdb.
*/
void
signal (int vector)
{
  char buf[2];

  buf[0] = hextab[(vector >> 4) & 0xf];
  buf[1] = hextab[(vector) & 0xf];

  /* for divide-by-zero and traps, step over the instruction */
  if ((vector ==  3) || (vector == 16) || \
      (vector == 17) || (vector == 18) || \
      (vector == 19))
  {
    reg_file[_epc] += 2;
  }

#if defined ENABLE_SIGNAL_MSG
  /* print message to gdb console identifying the actual mcore vector */

#if !defined ENABLE_VERBOSE_SIGNAL_MSG
  /* silence all breakpoint and trace messages unless in VERBOSE mode */
  if ((vector < 6) || (vector > 7))
  {
#endif

    gdb_console_output (27, "\nStub message: Exception 0x");
    gdb_console_output (2, buf);
    gdb_console_output (2, ", ");
    gdb_console_output (strlen(sig_tab[vector].msg), sig_tab[vector].msg);
    gdb_console_output (1, "\n");

#if !defined ENABLE_VERBOSE_SIGNAL_MSG
  }
#endif

#endif /* defined ENABLE_SIGNAL_MSG */

  /* call the exception handler to request instructions from gdb */
#if defined ENABLE_SIGNAL_MSG
  gdb_handle_exception((int)sig_tab[vector].signal);
#else
  gdb_handle_exception((int)sig_tab[vector]);
#endif
}
