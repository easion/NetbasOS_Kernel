/*
 * crt0.S -- startup file for m68k-coff
 *
 * Copyright (c) 1995, 1996, 1998 Cygnus Support
 *
 * The authors hereby grant permission to use, copy, modify, distribute,
 * and license this software and its documentation for any purpose, provided
 * that existing copyright notices are retained in all copies and that this
 * notice is included verbatim in any distributions. No written agreement,
 * license, or royalty fee is required for any of the authorized uses.
 * Modifications to this software may be copyrighted by their authors
 * and need not follow the licensing terms described here, provided that
 * the new terms are clearly indicated on the first page of each file where
 * they apply.
 */
#include "asm.h"

	.title "crt0.S for m68k-elf"

/* These are defined in C code. */
	.global SYM (main)
	.global __main

/*
 * These values are set in the linker script, so they must be
 * explicitly named here without SYM.
 */
	.extern _stack_top
	.extern _bss_start
	.extern _bss_end

/*
 * set things up so the application will run. This *must* be called start.
 */
	.global SYM (gcc_start)


SYM (gcc_start):
/*#	move.w	#0x2000,sr */
	movea.l	IMM(_stack_top), sp

/*
 * zero out the bss section.
 */
	movea.l	IMM(_bss_start), a0
	movea.l	IMM(_bss_end), a1
1:
	move.l	#0,(a0)+
	cmp.l	a0,a1
	bhi		1b

/* call the main C function */
	move.l	IMM(0),d0
	jsr	SYM (main)

/* jump back to the 162bug monito */

exit:
   jsr     SYM (exit)

__main:
		rts



