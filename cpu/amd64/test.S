
#include <amd64/asm.h>

/* 
 * Interrupt entry/exit.
 *
 * Interrupt entry points save only callee clobbered registers in fast path.
 *	
 * Entry runs with interrupts off.	
 */ 

/* 0(%rsp): interrupt number */ 
	.macro interrupt func

	cld

	;;SAVE_ARGS
	;;leaq -ARGOFFSET(%rsp),%rdi	# arg1 for handler

	;;testl $3,CS(%rdi)
	je 1f
	swapgs	
1:	;;addl $1,%gs:pda_irqcount	# RED-PEN should check preempt count
	;;movq %gs:pda_irqstackptr,%rax
	cmoveq %rax,%rsp							
	pushq %rdi			# save old stack	
	call \func
	.endm

.text
.globl	SYMBOL_NAME(modf)

SYMBOL_NAME_LABEL(modf)
ret


