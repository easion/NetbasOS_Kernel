/*
  Copyright (c) 2001 by      William A. Gatliff
  All rights reserved.      bgat@billgatliff.com

  See the file COPYING for details.

  This file is provided "as-is", and without any express or implied
  warranties, including, without limitation, the implied warranties of
  merchantability and fitness for a particular purpose.

  The author welcomes feedback regarding this file.

  Original CPU32/MC68332 port of gdbstubs contributed by Scott Sumner,
  sasumner@juno.com.  Thanks, Scott!

  Made non-MC68332 specific and generally revised Oct 2001 by Bill
  Gatliff, bgat@billgatliff.com.

  The basic CPU32 stub handling code.  Uses TRACE bits for instruction
  stepping.  Does not yet do much with bus error handling, but it will
  report a bus error as such.

  The exception handler always assumes that there is a twelve word
  exception stack frame present.  This isn't a problem, because the
  stack pointer is always unwound to remove whatever exception frame
  is present before any other processing begins.  A format==0000 stack
  frame is constructed when returning from exceptions.

  A copy of the exception stack frame is stored at the end of
  register_file.


  $Id: cpu32.c,v 1.1 2002/04/08 15:08:20 bgat Exp $

*/

#include "gdb.h"
#include "cpu32.h"



#define SR_T1 0x8000
#define SR_T0 0x4000


typedef enum {
  D0, D1, D2, D3, D4, D5, D6, D7,
  A0, A1, A2, A3, A4, A5, A6, A7,
  FP=A6, SP=A7, SR, PC } register_id_E;

typedef struct {
  /* these are the basic registers that gdb understands; the ones we
     don't send gdb will assume contain zeros */
  long d[8];
  long a[8];
  short sr;
  long pc;

  /* internal register/stack management stuff; not part of the
     register file proper, but this is a good place for it anyway.
     gdb never sees these values */
  short fv;
  long faulted_addr;
  long dbuf;
  long current_pc;
  short itcr;
  short special;
	} register_file_S;

static register_file_S register_file;


/* Retrieves a register value from register_file. returns the size of
   the register, in bytes, or zero if an invalid id is specified,
   which *will* happen---gdb.c uses this functionality to tell how
   many registers we actually have.  */
int gdb_peek_register_file (int id, long *val)
	{
	int retval = sizeof(long);

	switch (id)
		{
		case D0: case D1: case D2: case D3:
		case D4: case D5: case D6: case D7:
		  *val = register_file.d[id - D0];
		  break;

		case A0: case A1: case A2: case A3:
		case A4: case A5: case A6: case A7:
		  *val = register_file.a[id - A0];
		  break;

		case SR:
		  /* sr isn't really 32 bits, but gdb thinks it is */
		  *val = register_file.sr;
		  break;

		case PC:
		  *val = register_file.pc;
		  break;

		default: retval = 0;
		}

	return retval;
	}

/* Stuffs a register value into register_file.  Returns the size of
   the register, in bytes, or zero if an invalid id is specified */
int gdb_poke_register_file (int id, long val)
	{
	int retval = sizeof(long);

	switch (id)
		{
		case D0: case D1: case D2: case D3:
		case D4: case D5: case D6: case D7:
		  register_file.d[id - D0] = val;
		  break;

		case A0: case A1: case A2: case A3:
		case A4: case A5: case A6: case A7:
		  register_file.a[id - A0] = val;
		  break;

		case SR:
		  register_file.sr = (short)val;
		  break;

		case PC:
		  register_file.pc = val;
		  break;

		default: retval = 0;
		}

	return retval;
	}


void gdb_step (long addr)
	{
	register_file.sr |= SR_T1;
	gdb_continue(addr);
	}


void gdb_continue (long addr)
	{
	if (addr) register_file.pc = addr;
	gdb_return_from_exception();
	}


void gdb_monitor_onentry (void){}
void gdb_monitor_onexit (void){}

void gdb_cpu32_cleanup_stack (void)
	{
	int sigval;

	/* undo damage caused by exception stack frame */
	switch (register_file.fv >> 12)
		{
		default:
		case 0:
		  register_file.a[7] += 4 * sizeof(short);
		  break;
		case 2:
		  register_file.a[7] += 6 * sizeof(short);
		  break;
		case 0xc:
		  register_file.a[7] += 12 * sizeof(short);
		  break;
		}

	/* turn of instruction tracing */
	register_file.sr &= ~(SR_T1 | SR_T0);

	/* if gdb sets a breakpoint using trap #1, we have to back up in
	   order to restart the instruction replaced by the breakpoint */
	if(register_file.fv == 0x0084)
		register_file.pc -= 2;

	switch(register_file.fv & 0xfff)
		{
		/* translate the exception number into a gdb signal number. */
		/* TODO: this has not been thoroughly tested. */
		case 2: sigval = GDB_SIGBUS; break;
		case 3: sigval = GDB_SIGSEGV; break;
		case 4: sigval = GDB_SIGILL; break;
		case 5: sigval = GDB_SIGFPE; break;
		case 8: sigval = GDB_SIGILL; break;
		case 6: case 9: case 7:
		default: sigval = GDB_SIGTRAP; break;
		}
  
	gdb_handle_exception(sigval);
	}


__asm__("

.global gdb_interrupt_handler
gdb_interrupt_handler:

   /* disable interrupts */
   ori.w #0x700,%sr

gdb_save_state:

   /* populate register file */
   movem.l %d0-%d7/%a0-%a7,register_file
 
   /* save the exception stack frame */
   moveq.l #12,%d0
   move.l %a7,%a0
   lea register_file+64,%a1

save_exception_frame:
   move.w (%a0)+,(%a1)+
   dbf %d0,save_exception_frame

   bra gdb_cpu32_cleanup_stack
   nop

.global gdb_return_from_exception
gdb_return_from_exception:

   /* always return via a format==0000 frame */
   move.l (register_file+60),%a7
   moveq.l #0,%d0
   move.w %d0,-(%a7)
   lea register_file+70,%a0
   move.w -(%a0),-(%a7)
   move.w -(%a0),-(%a7)
   move.w -(%a0),-(%a7)
   movem.l register_file,%d0-%d7/%a0-%a6
   rte
   nop

.global gdb_kill
gdb_kill:
   move.l (0),%a7
   move.l (4),%a0
   jmp (%a0)

.global gdb_detach
gdb_detach:
   move.l (0),%a7
   move.l (4),%a0
   jmp (%a0)


/* cpu32's don't have caches, do they? */
.global gdb_flush_cache
gdb_flush_cache:
   rts

");

