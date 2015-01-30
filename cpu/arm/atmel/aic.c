/*   
 *  at91_aic.c
 *  this file implement aic driver on AT91X40 
 *  for Atmel AT91 advanced interrupt controller
 *
 *  Bugs report:  li ming  ( lmcs00@mails.tsinghua.edu.cn )
 *
 */

#include "at91_aic.h"

 /* Internal Sources */
#define LevelSensitive              (0<<5)
#define EdgeTriggered               (1<<5)

 /* External Sources */
#define LowLevel                    (0<<5)
#define NegativeEdge                (1<<5)
#define HighLevel                   (2<<5)
#define PositiveEdge                (3<<5)

static unsigned char eb01_irq_prtable[32] = {
        7 << 5, /* FIQ */
        0 << 5, /* SWIRQ */
        1 << 5, /* US0IRQ */
        1 << 5, /* US1IRQ */
        1 << 5, /* TC0IRQ */
        1 << 5, /* TC1IRQ */
        1 << 5, /* TC2IRQ */
        0 << 5, /* WDIRQ */
        0 << 5, /* PIOAIRQ */
        0 << 5, /* reserved */
        0 << 5, /* reserved */
        0 << 5, /* reserved */
        0 << 5, /* reserved */
        0 << 5, /* reserved */
        0 << 5, /* reserved */
        0 << 5, /* reserved */
        1 << 5, /* IRQ0 */
	0 << 5, /* IRQ1 */
        0 << 5, /* IRQ2 */
};

static unsigned char eb01_irq_type[32] = {
        EdgeTriggered,
        EdgeTriggered,
        EdgeTriggered,
        EdgeTriggered,
        EdgeTriggered,
        EdgeTriggered,
        EdgeTriggered,
        EdgeTriggered,
        EdgeTriggered,
        EdgeTriggered,
        EdgeTriggered,
        EdgeTriggered,
        EdgeTriggered,
        EdgeTriggered,
        EdgeTriggered,
        EdgeTriggered,

        EdgeTriggered,	/* IRQ0 = neg. edge */
        EdgeTriggered,
        EdgeTriggered,
        EdgeTriggered,
        EdgeTriggered,
        EdgeTriggered,
        EdgeTriggered,
        EdgeTriggered,
        EdgeTriggered,
        EdgeTriggered,
        EdgeTriggered,
        EdgeTriggered,
        EdgeTriggered,
        EdgeTriggered,
        EdgeTriggered,
        EdgeTriggered,
};

#define __arch_putb(v,a)	(*(volatile unsigned char *)(a) = (v))
#define __arch_putl(v,a)	(*(volatile unsigned int  *)(a) = (v))

void at91_init_aic(void)
{
        int irqno;

	/* Disable all interrupts */
	__arch_putl(0xFFFFFFFF, AIC_IDCR);

        /* Clear all interrupts	*/
        __arch_putl(0xFFFFFFFF, AIC_ICCR);
        
	for ( irqno = 0 ; irqno < 32 ; irqno++ )
	{
	       __arch_putl(irqno, AIC_EOICR);
	}
        for ( irqno = 0 ; irqno < 32 ; irqno++ )
        {
               __arch_putl((eb01_irq_prtable[irqno] >> 5) | eb01_irq_type[irqno],AIC_SMR(irqno));
													
	}

}