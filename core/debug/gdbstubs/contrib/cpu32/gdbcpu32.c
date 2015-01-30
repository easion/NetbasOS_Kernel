/***************************************
*                                      *
*   $Workfile:: GDBCPU32.C         $   *
*                                      *
*   $Revision: 1.1 $   *
*   $Modtime:: 12/18/96 9:26a      $   *
*                                      *
*** $NoKeywords: $ ********************/

#include "gdb.h"
#include "gdbcpu32.h"
#include "mc68332.h"

/* note:  "hargs" stands for "hex arguments" */

/*
There are 180 bytes of registers on a 68020 w/68881 (worst case for 68K family)
The D0-D7,A0-A7,PS(status register),PC registers are 4 byte (32 bit)
The registers FP0-FP7 are 12 byte (96 bit) registers
The FPCONTROL, FPSTATUS and FPIADDR registers are 4 byte (32 bit)
Only D0-D7,A0-A7,PS(status register),PC are required for 68K, however GDB
assumes worst case and expects all these registers. Unused registers are
ignored when received and returned to GDB as 0
*/

/* mask for trace bit in status register: */
#define INSTRUCTION_TRACE_MASK 0x8000

#define TRACE_FORMAT_AND_VECTOR_OFFSET 0x2024
#define TRAP0_FORMAT_AND_VECTOR_OFFSET 0x0080
#define TRAP1_FORMAT_AND_VECTOR_OFFSET 0x0084
#define SCI_FORMAT_AND_VECTOR_OFFSET   0x0100

#define SIGTRAP 5

/* serial port portability defines */
#define TDR  (SCDR_TDR)
#define RDR  (SCDR_RDR)
#define TDRE (SCSR->tdre)
#define RDRF (SCSR->rdrf)
#define PER  (SCSR->pf)
#define FER  (SCSR->fe)
#define ORER (SCSR->or)
#define NER  (SCSR->nf) /* noise flag */

/* gdb won't be allowed to read or write to the gdb_register_file array above
element [PC] */
#define VALID_REG_ID(reg_id) ((reg_id >= D0) && (reg_id <= PC))

/* define array indices into gdb_register_file[] */
enum
{
  D0, D1, D2, D3, D4, D5, D6, D7,
  A0, A1, A2, A3, A4, A5, A6,

  A7,
  SP = A7, /* A7 is supervisor stack pointer (SP) */

  PS, /* status register (only lower 16 bits of the long are used) */

  PC, /* program counter */

  /* gdb won't be allowed to read or write to the gdb_register_file array above
  element [PC]; our stub needs extra storage for the format and vector offset
  word, so we'll use the next element for our own internal use */
  FV, /* exception format (4 bits) and vector offset (12 bits) (only lower 16
  bits of the long are used) */
};

static long gdb_register_file[FV - D0 + 1];

/* stuffs a byte out the serial port */
void gdb_putc(char c)
{
  /* wait for the previous byte to go */
  while(!TDRE)
  {
    ;
  }

  /* send the byte */
  TDR = c;
}

/* blocks until a byte is received at the serial port */
char gdb_getc(void)
{
  char c;

  /* clear any detected errors */
  PER = FER = ORER = NER = 0;

  /* wait for a byte */
  while (RDRF == 0)
  {
    ;
  }

  /* got one--return it */
  c = RDR;

  return c;
}

/* retrieves a register value from gdb_register_file. returns the size of the
register, in bytes, or zero if an invalid id is specified (which *will*
happen---gdb.c uses this functionality to tell how many registers we actually
have) */
short gdb_peek_register_file(short id, long *val)
{
  /* all supported registers are longs */
  short retval = sizeof(long);

  if (VALID_REG_ID(id))
  {
    *val = gdb_register_file[id];
  }
  else
  {
    retval = 0;
  }

  return retval;
}

/* stuffs a register value into gdb_register_file. returns the size of the
register, in bytes, or zero if an invalid id is specified */
short gdb_poke_register_file(short id, long val)
{
  /* all our registers are longs */
  short retval = sizeof(long);

  if (VALID_REG_ID(id))
  {
    gdb_register_file[id] = val;
  }
  else
  {
    retval = 0;
  }

  return retval;
}

/* uses the instruction trace feature of the cpu32 to generate an exception
after we execute the next instruction */
void gdb_step(char *hargs)
{
  /* set trace bit in CPU status register so that we get an exception after the
  execution of the next machine language instruction */
  gdb_register_file[PS] |= INSTRUCTION_TRACE_MASK;  /* PS is status register */

  /* other than setting the trace bit, stepping is the same as continuing */
  gdb_continue(hargs);
}


/* continue program execution at addr, or at the current pc if addr == 0 */
void gdb_continue(char *hargs)
{
  long addr = 0;

  /* parse address, if any */
  while (*hargs != '#')
  {
    addr = (addr << 4) + hex_to_long(*hargs++);
  }

  /* if we're stepping/continuing from an address, adjust pc (untested!) */
  if (addr)
  {
    gdb_register_file[PC] = addr;
  }

  /* call function with the foreknowledge that will never return to here!! */
  gdb_return_from_exception();
}

/* kills the current application. simulates a reset by jumping to the address
taken from the reset vector at address 4 */
void gdb_kill(void)
{
  /* return control to stub in boot device */

  /* jump to reset vector contained in boot device */
  asm(" move.l (0x4),%a0");
  asm(" jmp (%a0)");
}

void gdb_cout(const char *buf)
{
  long len = 0;
  char *pCh;

  for (pCh = (char *)buf; *pCh != 0; pCh++)
  {
    len++;
  }

  gdb_console_output(len, buf);
}

void gdb_interrupt_handler(void)
{
  /* other interrupts can still happen, so disable all interrupts levels; the
  current interrupt mask in effect at the time of the gdb interrupt is on the
  stack and will be restored automatically when normal execution resumes */
  DISABLE_INTERRUPTS;

  /* remove the effects of the C function call link.w a6,#0 ; REMOVE THIS IF
  COMPILING USING -fomit-frame-pointer */
  asm(" unlk %a6");

  /*
  the stack currently looks like this:
  status register (16 bits)                <--- stack pointer points here
  program counter (32 bits)                <--- stack pointer + (1 word)
  exception format & vect offset (16 bits) <--- stack pointer + (3 words)
  */

  /* save registers before changing anything else */
  asm(" movem.l %d0-%d7/%a0-%a7,gdb_register_file");

  /* obtain PS (status register) that existed at time of interrupt by examining
  the stack, and save copy of it to gdb_register_file */
  gdb_register_file[PS] = *(unsigned short *)gdb_register_file[SP];

  /* obtain PC (program counter) to return execution to when resume normal
  program execution by examining the stack, and save copy to
  gdb_register_file */
  gdb_register_file[PC] = *(unsigned long *)
    ((unsigned short *)gdb_register_file[SP] + 1);

  /* obtain exception frame format and vector offset by examining the stack,
  and save copy to gdb_register_file */
  gdb_register_file[FV] = *(((unsigned short *)gdb_register_file[SP]) + 3);

  switch (gdb_register_file[FV])
  {
    case TRAP1_FORMAT_AND_VECTOR_OFFSET:
    {
      /* fix up saved stack pointer to position prior to interrupt by
      accounting for size of exception stack frame */
      gdb_register_file[SP] += 4 * sizeof(unsigned short);

      /* we hit a gdb-installed breakpoint and our exception frame has our
      stacked program counter pointing 2 past the trap #1 instruction; need to
      decrement the pc in the gdb register file so that when we return we
      execute the instruction that gdb overwrites the trap #1 with */
      gdb_register_file[PC] -= 2;

      break;
    }

    case TRACE_FORMAT_AND_VECTOR_OFFSET:
    {
      /* fix up saved stack pointer to position prior to interrupt by
      accounting for size of exception stack frame */
      gdb_register_file[SP] += 6 * sizeof(unsigned short);

      /* stop the tracing */
      gdb_register_file[PS] &= ~INSTRUCTION_TRACE_MASK;

      break;
    }

    case SCI_FORMAT_AND_VECTOR_OFFSET:
    {
      /* fix up saved stack pointer to position prior to interrupt by
      accounting for size of exception stack frame */
      gdb_register_file[SP] += 4 * sizeof(unsigned short);

      /* clear the sci receive interrupt flag and put the sci into polled mode
      (rather than interrupt mode) */
      if ((SCCR1->rie == 1) && (SCSR->rdrf == 1))
      {
        /* reading RDR after reading SCSR with RDRF set clears RDRF interrupt
        flag */
        SCDR_RDR;

        /* clear RIE because gdb polls the sci */
        SCCR1->rie = 0;
      }

      break;
    }

    case TRAP0_FORMAT_AND_VECTOR_OFFSET:
    default:
    {
      /* fix up saved stack pointer to position prior to interrupt by
      accounting for size of exception stack frame */
      gdb_register_file[SP] += 4 * sizeof(unsigned short);

      break;
    }
  }

  /* call function with the foreknowledge that will never return to here!! */
  gdb_handle_exception(SIGTRAP);
}

void gdb_return_from_exception(void)
{
  switch (gdb_register_file[FV])
  {
    case TRACE_FORMAT_AND_VECTOR_OFFSET:
    {
      /* fixup stack pointer so that it is correct for rte when it pops
      the stack */
      gdb_register_file[SP] -= 6 * sizeof(unsigned short);

      break;
    }

    case SCI_FORMAT_AND_VECTOR_OFFSET:
    {
      /* fixup stack pointer so that it is correct for rte when it pops
      the stack */
      gdb_register_file[SP] -= 4 * sizeof(unsigned short);

      /* set sci's receive interrupt enable so that it becomes interrupt driven
      again */
      SCCR1->rie = 1;

      break;
    }

    case TRAP1_FORMAT_AND_VECTOR_OFFSET:
    case TRAP0_FORMAT_AND_VECTOR_OFFSET:
    default:
    {
      /* fixup stack pointer so that it is correct for rte when it pops
      the stack */
      gdb_register_file[SP] -= 4 * sizeof(unsigned short);

      break;
    }
  }

  /* copy the PS (status register) stored in gdb_register_file back into the
  stack memory area so that rte will correctly restore the status register when
  the rte is executed */
  *(unsigned short *)gdb_register_file[SP] = gdb_register_file[PS];

  /* copy the PC (program counter) stored in gdb_register_file back into the
  stack memory area so that rte will correctly restore the program counter when
  the rte is executed */
  *(unsigned long *)((unsigned short *)gdb_register_file[SP] + 1) =
    gdb_register_file[PC];

  /* copy the exception format and vector offset stored in gdb_register_file
  back into the stack memory area so that rte will correctly pop the correct
  number of bytes from the stack when the rte is executed */
  *(((unsigned short *)gdb_register_file[SP]) + 3) = gdb_register_file[FV];

  /* restore all data and address registers (including stack pointer) */
  asm(" movem.l gdb_register_file,%d0-%d7/%a0-%a7");

  /* restore program counter and status register as saved on the stack at the
  time of the last gdb interrupt, along with clearing the 8 (trap/sci) or 12
  (trace) byte exception frame */
  asm(" rte");
}

