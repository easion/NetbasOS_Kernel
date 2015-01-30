
#include <jicama/process.h>
#include "s3c4510.h"

__asmlink u32_t irq_table[NR_IRQS];
__asmlink void* irq_arg_table[NR_IRQS];
u32_t sys_irq_from_thread, sys_irq_to_thread;
u32_t sys_switch_interrput_flag = 0;
extern int sys_irq_nest;


void arm_board_init()
{
	register int i;

	/* all interrupt disabled include global bit */
    INTMASK = 0x3fffff;

	/* all clear pending */
	INTPEND = 0x1fffff;

	/* all=IRQ mode */
	INTMODE = 0x0;

	sys_irq_from_thread = 0;
	sys_irq_to_thread = 0;
	sys_irq_nest = 0;
	
}

void arm_interrupt_mask(int vector)
{
	INTMASK |= 1 << vector;
}

void arm_interrupt_umask(int vector)
{
	INTMASK &= ~(1 << vector);
}

void en_irq (const unsigned int nr)
{
	arm_interrupt_umask(nr);
}

void dis_irq (const unsigned int nr) 
{
  arm_interrupt_mask(nr);
}



#define DATA_COUNT	0x7a120

__local  void __irq timer_handler(int vector)
{
	thread_t *current= current_thread();


	/* reset TDATA0 */
	TDATA0 = DATA_COUNT;

	//clock tick increase
	clock_handler(NULL,vector);

	if ((current->ticks == 0) && scheduleable())
	{
		 //switch_enable();
		//puts("timer_handler schedule ...\n");
		schedule();
		return;
	 }
	//puts("timer_handler ok ...\n");

}

void reset_clock()
{
	/* set timer0 register */
	TDATA0	= DATA_COUNT;
	TCNT0	= 0x0;
	TMOD	= 0x3;

	/* install interrupt handler */
	put_irq_handler(INTTIMER0, timer_handler, NULL);
	//en_irq(INTTIMER0);
	kprintf("put_irq_handler ok\n");
}



