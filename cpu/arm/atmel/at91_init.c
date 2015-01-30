#include <jicama/system.h>
#include <jicama/utsname.h>
#include <jicama/process.h>
#include <jicama/console.h>
#include <jicama/devices.h>
#include <assert.h>
/* always include this file for at91 peripherals */
#include "at91.h"


#include "at91_aic.h"
#include "at91_tc.h"
#include "at91_usart.h"
#include "AT91RM9200.h"


extern unsigned long irq_table[32];

void do_irq( void );

void at91_mask_irq(unsigned int irq)
{
	unsigned long mask = 1 << (irq);
	__arch_putl(mask, AIC_IDCR);		// AIC Interrupt Disable Command Register
}	

void at91_unmask_irq(unsigned int irq)
{
	unsigned long mask = 1 << (irq);
	__arch_putl(mask, AIC_IECR);		// AIC Interrupt Enable Command Register
}

void at91_mask_ack_irq(unsigned int irq)
{
	at91_mask_irq(irq);
        __arch_putl(0, AIC_EOICR);     /* value=don't care */
}
/* we always count down the max. */
#define TIMER_LOAD_VAL 0xffff

/* macro to read the 16 bit timer */
#define READ_TIMER (tmr->TC_CV)
AT91PS_TC tmr;

static unsigned long timestamp;
static unsigned long lastinc;

static int interrupt_init (void)
{

    tmr = AT91C_BASE_TC0;

    /* enables TC1.0 clock */
    *AT91C_PMC_PCER = 1 << AT91C_ID_TC0;  /* enable clock */

    *AT91C_TCB0_BCR = 0;
    *AT91C_TCB0_BMR = AT91C_TCB_TC0XC0S_NONE | AT91C_TCB_TC1XC1S_NONE | AT91C_TCB_TC2XC2S_NONE;
    tmr->TC_CCR = AT91C_TC_CLKDIS;
    tmr->TC_CMR = AT91C_TC_TIMER_DIV1_CLOCK;  /* set to MCLK/2 */

    tmr->TC_IDR = ~0ul;
    tmr->TC_RC = TIMER_LOAD_VAL;
    lastinc = TIMER_LOAD_VAL;
    tmr->TC_CCR = AT91C_TC_SWTRG | AT91C_TC_CLKEN;
    timestamp = 0;
    return (0);
}

static int board_init (void)
{
	//DECLARE_GLOBAL_DATA_PTR;

	/* Correct IRDA resistor problem */
	/* Set PA23_TXD in Output */
	(AT91PS_PIO) AT91C_BASE_PIOA->PIO_OER = AT91C_PA23_TXD2;
}

void arm_board_init()
{
	board_init();
	interrupt_init();
	// here Must put at91_init_usart() at the very beginning of init !
	at91_init_aic();	
}

//yangye
//use this function to install your OSISR as the IRQ mode handler
//and your OSISR must call do_irq to deal with different irqs
void install_irqhandler(void * isr)
{
	/* ARM irq exception vector addr is 0x00000018  */
	unsigned int * irq_vec_addr = ( unsigned int * ) 0x18;
	/* this is isr entry address, could be another address like 0x3c, 0x58... */
	unsigned int * isr_entry_addr = ( unsigned int * ) 0x38;

	unsigned int instruction;
	
	/* set the ISR entry at 0x38  */
	*isr_entry_addr = (unsigned int)isr;
	
	/* make an instruction: it is machine-code for "ldr  pc, [pc, #(38-18-8)]"  */
	instruction = ((unsigned int) isr_entry_addr  - (unsigned int)irq_vec_addr - 0x08) | 0xe59ff000;
	
	/* set this instruction at 0x18  */
	*irq_vec_addr = instruction;	
}

void do_irq1(void)
{
	int irq_num;
	void (*isr)(void);
	//chy 2003-02-16 : IVR could have some irqs at the same time, so should check it untile IVR=0 	
	//irqnum is the highest irq number 
label_again:
	irq_num = __arch_getl(AIC_IVR);
	

	if(irq_num!=0){
		//close this interrupt
		at91_mask_irq(irq_num);
		
		isr = (void *)irq_table[irq_num];
		if( isr != 0 )
			isr();

		if(irq_num == KERNEL_TIMER_IRQ_NUM){
			//do some extra work for skyeye timer interrupt
			register volatile struct at91_timers* tt = (struct at91_timers*) (AT91_TC_BASE);
			register volatile struct at91_timer_channel* tc = &tt->chans[KERNEL_TIMER].ch;
			/* clear TC Status Register to continue interrupt */
			unsigned long tmp = tc->sr; // only read status register, must do!
			tmp = tmp;			// just use it to avoid compiler warnings!
		}
		//reenable this interrupt
		at91_unmask_irq(irq_num);
		 /* clear AIC : indicate the end of interrupts */
		__arch_putl(irq_num,AIC_EOICR);  // only write AIC_EOICR
	        //chy 2003-02-16 : IVR could have some irqs at the same time, so should check it untile IVR=0 	
	        goto label_again;
	   }
}




void en_irq(unsigned int irq)
{
	at91_unmask_irq(irq);
}

void dis_irq(unsigned int irq)
{
	at91_mask_irq(irq);
}


