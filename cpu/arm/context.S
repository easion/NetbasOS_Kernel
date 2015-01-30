

.globl thread_context_save
.globl thread_context_switch

thread_context_save:
	stmfd	sp!, {lr}		@ push pc (lr should be pushed in place of PC)
	stmfd	sp!, {r0-r12, lr}	@ push lr & register file

	mrs	r4, cpsr
	stmfd	sp!, {r4}		@ push cpsr
	mrs	r4, spsr
	stmfd	sp!, {r4}		@ push spsr

	str	sp, [r0]		@ store sp in preempted tasks TCB
#	mov pc, lr

thread_context_switch:
	ldr	sp, [r1]		@ get new task stack pointer

	ldmfd	sp!, {r4}		@ pop new task spsr
	msr	spsr_cxsf, r4
	ldmfd	sp!, {r4}		@ pop new task cpsr
	msr	cpsr_cxsf, r4

	ldmfd	sp!, {r0-r12, lr, pc}	@ pop new task r0-r12, lr & pc

