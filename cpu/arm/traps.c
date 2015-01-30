
#include <jicama/process.h>

#include "./s3c4510/s3c4510.h"


/**
 * this function will show registers of CPU
 *
 * @param regs the registers point
 */
void arm_show_register (struct arm_register *regs)
{
	kprintf("r00:0x%08x r01:0x%08x r02:0x%08x r03:0x%08x\n", regs->r0, regs->r1, regs->r2, regs->r3);
	kprintf("r04:0x%08x r05:0x%08x r06:0x%08x r07:0x%08x\n", regs->r4, regs->r5, regs->r6, regs->r7);
	kprintf("r08:0x%08x r09:0x%08x r10:0x%08x\n", regs->r8, regs->r9, regs->r10);
	kprintf("fp :0x%08x ip :0x%08x\n", regs->fp, regs->ip);
	kprintf("sp :0x%08x lr :0x%08x pc :0x%08x\n", regs->sp, regs->lr, regs->pc);
	kprintf("cpsr:0x%08x\n", regs->cpsr);
}

/**
 * When ARM7TDMI comes across an instruction which it cannot handle, 
 * it takes the undefined instruction trap.
 *
 * @param regs system registers
 *
 * @note never invoke this function in application
 */
void arm_trap_udef(struct arm_register *regs)
{
    kprintf("undefined instruction Execption:\n\n");
    arm_show_register(regs);
    halt();
}


/**
 * An abort indicates that the current memory access cannot be completed,
 * which occurs during an instruction prefetch.
 *
 * @param regs system registers
 *
 * @note never invoke this function in application
 */
void arm_trap_pabt(struct arm_register *regs)
{
    kprintf("prefetch abort Execption:\n\n");
    arm_show_register(regs);
    halt();
}

/**
 * An abort indicates that the current memory access cannot be completed,
 * which occurs during a data access.
 *
 * @param regs system registers
 *
 * @note never invoke this function in application
 */
void arm_trap_dabt(struct arm_register *regs)
{
    kprintf("Data Abort Execption:\n");
    arm_show_register(regs);
    halt();
}

/**
 * Normally, system will never reach here
 *
 * @param regs system registers
 *
 * @note never invoke this function in application
 */
void arm_trap_resv(struct arm_register *regs)
{
    kprintf("not used Execption:\n\n");
    arm_show_register(regs);
    halt();
}

#define __SKYEYE__
typedef void (*isr_handler_t)(int vector);

extern isr_handler_t irq_table[];
void arm_trap_irq()
{
#ifndef __SKYEYE__
	unsigned long irqno;
	isr_handler_t isr_func;

	irqno = INTOFFSET;

	/* check pending according um_4510b */
	if ( irqno == 0x54 ) return;
	else irqno = irqno >> 2;

	/* get interrupt service routine */
	isr_func = irq_table[irqno];

	/* turn to interrupt service routine */
	isr_func(irqno);

	/* clear pending register */
	INTPEND = 1 << irqno;
#else
	register int offset;
	unsigned long pend;
	isr_handler_t isr_func;

	//puts("entry irq\n");

	pend = INTPEND;

	for (offset = 0; offset < INTGLOBAL; offset ++)
	{
		if (pend & (1 << offset)) break;
	}
	if (offset == INTGLOBAL) return;

	/* get interrupt service routine */
	isr_func = irq_table[offset];

	/* turn to interrupt service routine */
	isr_func(offset);

	/* clear pending register */
	INTPEND = 1 << offset;
	//puts("level irq\n");
#endif 
}

void arm_trap_fiq()
{
    kprintf("fast interrupt request Execption:\n\n");
}

