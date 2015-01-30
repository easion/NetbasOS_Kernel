/*
 *  startup.S 
 *  the very beginning code for ARM 7TDMI 
 *
 *  Bugs Report:  li ming (lmcs00@mails.tsinghua.edu.cn)
 *  Release:      2003-02-02 11:05:52 
 */
 

	
.text
	.align 4
	.global _start
	.type _start, function

_start:
	@set up irq stack
	mov 	r0, #0xd2                @ make irq mode with all irqs disabled
	msr 	cpsr, r0
	ldr 	sp, =irq_stack           @ set sp_irq = irq_stack
	
	@set up svc stack
        mov	r0, #0xd3                @ make svc mode with all irqs disabled
	msr	cpsr, r0		 
	ldr	sp, =svc_stack           @ set sp_svc = svc_stack

	@ here irq enabled	
	mov 	r0, #0x13
	msr	cpsr, r0
	b 	arch_startup             @ jump to hello.c

	.align  2
        .global  system_isr
        .type    system_isr,function
system_isr:
	SUB	lr, lr, #4            
	STMFD	sp!, {r0-r12, lr}      @ push r0-r12 register file and lr( pc return address )
        MRS 	r4, spsr        
	STMFD 	sp!, {r4}              @ push current spsr_irq

	BL	do_irq                 @ here do_irq is used to clear some virtual-hardware flags
			
        LDMFD 	sp!, {r4}              @ get cpsr_svc from stack to r4
	MSR     spsr, r4               @ prepare spsr to return svc mode
	LDMFD	sp!, {r0-r12, pc}^     @ recover r0-r12 from stack


.data
	.align  4
irq_stack:
	.word	0
	.space	4096
svc_stack:
	.word	0
	.space	4096
