/*   
 *  at91_tc.c
 *  this file implement tc driver on AT91X40 
 *  for Atmel AT91 timer counter
 *
 *  Bugs report:     li ming  ( lmcs00@mails.tsinghua.edu.cn )
 *  Last modified:   2003-02-02 
 *
 */

#include <jicama/process.h>
#include "at91.h"
#include "at91_tc.h"
#include "at91_aic.h"

__local  void __irq timer_handler(int vector)
{
	thread_t *current= current_thread();

	puts("timer_handler called ...\n");

	//clock tick increase
	clock_handler(NULL,vector);


	if ((current->ticks == 0) && scheduleable())
	{
		 //switch_enable();
		puts("timer_handler ok ...\n");
		schedule();
		return;
	 }

}

void uart_isr()
{
	while (1)
	{
	}
}

void OSISR()
{
	while (1)
	{
	}
}
void reset_clock(void)
{
        register volatile struct at91_timers* tt = (struct at91_timers*) (AT91_TC_BASE);
        register volatile struct at91_timer_channel* tc = &tt->chans[KERNEL_TIMER].ch;
        unsigned long v;


        /* No SYNC */
        tt->bcr = 0;
        /* program NO signal on XC1 */
        v = tt->bmr;
		v &= ~TCNXCNS(KERNEL_TIMER,3);
		v |= TCNXCNS(KERNEL_TIMER,1);
		tt->bmr = v;
	
	/*
	CLKEN: Counter Clock Enable Command (Code Label TC_CLKEN)
	0 = No effect.
	1 = Enables the clock if CLKDIS is not 1.
	CLKDIS: Counter Clock Disable Command (Code Label TC_CLKDIS)
	0 = No effect.
	1 = Disables the clock.
	SWTRG: Software Trigger Command (Code Label TC_SWTRG)
	0 = No effect.
	1 = A software trigger is performed: the counter is reset and clock is started.
	*/
	/* disable the channel */
        tc->ccr = 2;  	// disable counter clock & disable software trigger
	
        /* select ACLK/128 as inupt frequency for TC1 and enable CPCTRG */
   	// 0 1 1 MCK/128 TC_CLKS_MCK128
   	// CPCTRG: RC Compare Trigger Enable (Code Label TC_CPCTRG)
	// 0 = RC Compare has no effect on the counter and its clock.
	// 1 = RC Compare resets the counter and starts the counter clock.
        tc->cmr = 3 | (1 << 14);
	
        tc->idr = ~0ul;  /* disable all interrupt */
	//tc->rc = ((ARM_CLK/128)/HZ - 1);   /* load the count limit into the CR register */
	tc->rc = (60);   /* load the count limit into the CR register */
	tc->ier = TC_CPCS;  /* enable CPCS interrupt */

	/* enable the channel */
	tc->ccr = TC_SWTRG|TC_CLKEN;
	
	install_irqhandler(OSISR);
	at91_mask_ack_irq(KERNEL_TIMER_IRQ_NUM);
	at91_unmask_irq(KERNEL_TIMER_IRQ_NUM);


	put_irq_handler(KERNEL_TIMER_IRQ_NUM,timer_handler, NULL);

	put_irq_handler(2,uart_isr, NULL);
}

