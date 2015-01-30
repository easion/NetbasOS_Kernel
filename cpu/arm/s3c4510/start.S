
#define CONFIG_STACKSIZE 8192

.section .init, "ax"
.code 32
.globl _start
_start:
	b	reset
	ldr	pc, _vector_undef
	ldr	pc, _vector_swi
	ldr	pc, _vector_pabt
	ldr	pc, _vector_dabt
	ldr	pc, _vector_resv
	ldr	pc, _vector_irq
	ldr	pc, _vector_fiq

_vector_undef:	.word vector_undef
_vector_swi:	.word vector_swi
_vector_pabt:	.word vector_pabt
_vector_dabt:	.word vector_dabt
_vector_resv:	.word vector_resv
_vector_irq:	.word vector_irq
_vector_fiq:	.word vector_fiq

	.balignl 16,0xdeadbeef

_TEXT_BASE:
	.word	TEXT_BASE

/*
 * rtthread kernel start and end
 * which are defined in linker script
 */
.globl _rtthread_start
_rtthread_start:.word _start
.globl _rtthread_end
_rtthread_end:	.word  _end

/*
 * rtthread bss start and end
 * which are defined in linker script
 */
.globl _bss_start
_bss_start:	.word __bss_start
.globl _bss_end
_bss_end:	.word __bss_end

/* IRQ stack memory (calculated at run-time) */
.globl IRQ_STACK_START
IRQ_STACK_START:.word _irq_stack_start + 1024
.globl FIQ_STACK_START
FIQ_STACK_START:.word _fiq_stack_start + 1024
.globl UNDEFINED_STACK_START
UNDEFINED_STACK_START:.word _undefined_stack_start + 512
.globl ABORT_STACK_START
ABORT_STACK_START:.word _abort_stack_start + 512

#define INTMSK 0x3ff4008

/* the system entry */
reset:
	/* set the cpu to SVC32 mode */
	mrs	r0,cpsr
	bic	r0,r0,#0x1f
	orr	r0,r0,#0x13
	msr	cpsr,r0

	/* mask all IRQs by clearing all bits in the INTMRs */
	ldr	r1, =INTMSK
	ldr	r0, =0xffffffff
	str	r0, [r1]

	/* set interrupt vector */
	ldr	r0, _TEXT_BASE		/* source address 0x8000 */
	mov	r1, #0x00			/* target address 0x00   */
	add	r2, r0, #0x20		/* size, 32bytes         */

copy_loop:
	ldmia	r0!, {r3-r10}	/* copy from source address [r0]    */
	stmia	r1!, {r3-r10}	/* copy to   target address [r1]    */
	cmp	r0, r2				/* until source end addreee [r2]    */
	ble	copy_loop

	/* set interrupt vector
	bl copy_vector */

	/* setup stack */
	bl	stack_setup

	/* start RT-Thread Kernel */
	ldr	pc, _arch_startup

_arch_startup: .word arch_startup

/*
 *************************************************************************
 *
 * Interrupt handling
 *
 *************************************************************************
 */

@
@ IRQ stack frame.
@
#define S_FRAME_SIZE	72

#define S_OLD_R0	68
#define S_PSR		64
#define S_PC		60
#define S_LR		56
#define S_SP		52
#define S_IP		48
#define S_FP		44
#define S_R10		40
#define S_R9		36
#define S_R8		32
#define S_R7		28
#define S_R6		24
#define S_R5		20
#define S_R4		16
#define S_R3		12
#define S_R2		8
#define S_R1		4
#define S_R0		0

#define USERMODE	0x10
#define FIQMODE		0x11
#define IRQMODE		0x12
#define SVCMODE		0x13
#define ABORTMODE	0x17
#define UNDEFMODE	0x1b
#define MODEMASK	0x1f
#define NOINT		0xc0

/*
 * use bad_save_user_regs for abort/prefetch/undef/swi ...
 */
	.macro	bad_save_user_regs
	sub	sp, sp, #S_FRAME_SIZE
	stmia	sp, {r0 - r12}		@ Calling r0-r12
	add     r8, sp, #S_PC

	ldr	r2, _rtthread_end
	add	r2, r2, #CONFIG_STACKSIZE
	sub	r2, r2, #8
	ldmia	r2, {r2 - r4}		@ get pc, cpsr, old_r0
	add	r0, sp, #S_FRAME_SIZE	@ restore sp_SVC

	add	r5, sp, #S_SP
	mov	r1, lr
	stmia	r5, {r0 - r4}		@ save sp_SVC, lr_SVC, pc, cpsr, old_r
	mov	r0, sp
	.endm

	.macro	get_bad_stack
	ldr	r13, _rtthread_end		@ setup our mode stack
	add	r13, r13, #CONFIG_STACKSIZE	@ resides at top of normal stack
	sub	r13, r13, #8

	str	lr, [r13]				@ save caller lr / spsr
	mrs	lr, spsr
	str     lr, [r13, #4]

	mov	r13, #SVCMODE			@ prepare SVC-Mode
	msr	spsr_c, r13
	mov	lr, pc
	movs	pc, lr
	.endm

/* exception handlers */
	.align  5
vector_undef:
	get_bad_stack
	bad_save_user_regs
	bl	arm_trap_udef

	.align	5
vector_swi:
	get_bad_stack
	bad_save_user_regs
	bl 	arm_trap_swi

	.align	5
vector_pabt:
	get_bad_stack
	bad_save_user_regs
	bl 	arm_trap_pabt

	.align	5
vector_dabt:
	get_bad_stack
	bad_save_user_regs
	bl 	arm_trap_dabt

	.align	5
vector_resv:
	get_bad_stack
	bad_save_user_regs
	bl 	arm_trap_resv

.globl sys_irq_enter
.globl sys_irq_leave
.globl sys_switch_interrput_flag
.globl sys_irq_from_thread
.globl sys_irq_to_thread
vector_irq:
	stmfd	sp!, {r0-r12,lr}
	bl	sys_irq_enter
	bl	arm_trap_irq
	bl	sys_irq_leave

	/* if sys_switch_interrput_flag set, jump to _interrupt_thread_switch and don't return */
	ldr	r0, =sys_switch_interrput_flag
	ldr	r1, [r0]
	cmp	r1, #1
	beq	_interrupt_thread_switch

	ldmfd	sp!, {r0-r12,lr}
	subs	pc, lr, #4

	.align	5
vector_fiq:
	stmfd	sp!,{r0-r7,lr}
	bl 	arm_trap_fiq
	ldmfd	sp!,{r0-r7,lr}
	subs	pc,lr,#4

_interrupt_thread_switch:
	mov	r1,  #0				@ clear sys_switch_interrput_flag
	str	r1,  [r0]

	ldmfd	sp!, {r0-r12,lr}@ reload saved registers
	stmfd	sp!, {r0-r3}	@ save r0-r3
	mov	r1,  sp
	add	sp,  sp, #16		@ restore sp
	sub	r2,  lr, #4			@ save old task's pc to r2

	mrs	r3,  spsr			@ disable interrupt
	orr	r0,  r3, #NOINT
	msr	spsr_c, r0

	ldr	r0,  =.+8			@ switch to interrupted task's stack
	movs	pc,  r0

	stmfd	sp!, {r2}		@ push old task's pc
	stmfd	sp!, {r4-r12,lr}@ push old task's lr,r12-r4
	mov	r4,  r1				@ Special optimised code below
	mov	r5,  r3
	ldmfd	r4!, {r0-r3}
	stmfd	sp!, {r0-r3}	@ push old task's r3-r0
	stmfd	sp!, {r5}		@ push old task's psr
	mrs	r4,  spsr
	stmfd	sp!, {r4}		@ push old task's spsr

	ldr	r4,  =sys_irq_from_thread
	ldr	r5,  [r4]
	str	sp,  [r5]			@ store sp in preempted tasks's TCB

	ldr	r6,  =sys_irq_to_thread
	ldr	r6,  [r6]
	ldr	sp,  [r6]			@ get new task's stack pointer

	ldmfd	sp!, {r4}		@ pop new task's spsr
	msr	SPSR_cxsf, r4
	ldmfd	sp!, {r4}		@ pop new task's psr
	msr	CPSR_cxsf, r4

	ldmfd	sp!, {r0-r12,lr,pc}	@ pop new task's r0-r12,lr & pc

stack_setup:
	mrs	r0, cpsr
	bic	r0, r0, #MODEMASK
	orr	r1, r0, #UNDEFMODE|NOINT
	msr	cpsr_cxsf, r1		@ undef mode
	ldr	sp, UNDEFINED_STACK_START

	orr	r1,r0,#ABORTMODE|NOINT
	msr	cpsr_cxsf,r1		@ abort mode
	ldr	sp, ABORT_STACK_START

	orr	r1,r0,#IRQMODE|NOINT
	msr	cpsr_cxsf,r1		@ IRQ mode
	ldr	sp, IRQ_STACK_START

	orr	r1,r0,#FIQMODE|NOINT
	msr	cpsr_cxsf,r1		@ FIQ mode
	ldr	sp, FIQ_STACK_START

	bic	r0,r0,#MODEMASK
	orr	r1,r0,#SVCMODE|NOINT
	msr	cpsr_cxsf,r1		@ SVC mode

	ldr	r0, _rtthread_end
	add	sp, r0, #CONFIG_STACKSIZE

	/* USER mode is not initialized. */

	mov	pc,lr				@ The LR register may be not valid for the mode changes.

/*
 * void arch_context_switch_interrupt(rt_uint32 from, rt_uint32 to);
 *在中断上下文之间切换
 */
.globl sys_switch_interrput_flag
.globl sys_irq_from_thread
.globl sys_irq_to_thread
.globl arch_context_switch_interrupt
arch_context_switch_interrupt:
	ldr r2, =sys_switch_interrput_flag
	ldr r3, [r2]
	cmp r3, #1
	beq _reswitch
	mov r3, #1				@ set sys_switch_interrput_flag to 1
	str r3, [r2]
	ldr r2, =sys_irq_from_thread	@ set sys_irq_from_thread
	str r0, [r2]
_reswitch:
	ldr r2, =sys_irq_to_thread		@ set sys_irq_to_thread
	str r1, [r2]
	mov pc, lr
