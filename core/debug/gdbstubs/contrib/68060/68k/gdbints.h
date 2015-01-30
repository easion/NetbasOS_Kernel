/*
Since gcc doesn't directly support interrupt service routines (it won't push/
 pop registers and return with an rte instruction), we have to give it some
 help by providing an assembly language wrapper function, defined with the
 ISR_ASSEMBLY_WRAPPER macro.  This macro, which takes the C function name for
 the isr as an argument, creates an assembly language function of the same
 name as the C function except for the addition of a leading underscore.
 The assembly language version gets put in the vector table and runs when
 interrupt processing is needed.  It does the needed register pushes, calls
 the C function for the usual isr functionality, then does the register pops
 and returns with the required rte instruction.

 Here's an example usage:
  ISR_ASSEMBLY_WRAPPER(sci_interrupt_handler);

 And here's an example of the macro expansion (the resulting assembly code):
   .global sci_interrupt_handler        ; so jsr can "see" the C function call
   .global _sci_interrupt_handler       ; assembly language "function prototype"
 _sci_interrupt_handler:                ; label that gets put in vector table
   movem.l %d0-%d7/%a0-%a6,-(%a7)       ; push registers on stack
   jsr sci_interrupt_handler            ; call the C function (the real isr)
   movem.l (%sp)+,%d0-%d7/%a0-%a6       ; pop registers from stack
   rte                                  ; return from exception instruction
*/

/* helper macros: */
#define CONCATX(a,b) a ## b
#define CONCAT(a,b) CONCATX(a,b)
#define QUOTE_LITERAL(q) #q
#define QUOTE_DEREFERENCE(q) QUOTE_LITERAL(q)

#define REAL_ISR_PREFIX _
#define ISR_PREFIX(isr) CONCAT(REAL_ISR_PREFIX, isr)
#define QUOTED_REAL_ISR_PREFIX QUOTE_DEREFERENCE(REAL_ISR_PREFIX)

#define ISR_ASSEMBLY_WRAPPER(isr) \
  asm( \
  " .global " QUOTE_LITERAL(isr) "\n" \
  " .global _" QUOTE_LITERAL(isr) "\n" \
  QUOTED_REAL_ISR_PREFIX QUOTE_LITERAL(isr) ":\n" \
  " movem.l %d0-%d7/%a0-%a6,-(%a7)\n" \
  " jsr " QUOTE_LITERAL(isr) "\n" \
  " movem.l (%sp)+,%d0-%d7/%a0-%a6\n" \
  " rte" \
  );

/* ------------------------------------------------------------------------- */

#define ENABLE_ALL_ENABLED_INTERRUPTS asm(" andi.w #0xf8ff,%sr")
#define DISABLE_INTERRUPTS asm(" ori.w #0x0700,%sr")

void exceptionHandler(
	int		_vec,
	void* 	_pFunc
);
