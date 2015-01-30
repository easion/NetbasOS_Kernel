/*   
 *  at91_aic.h  
 *  this file defines AIC information on AT91X40 
 *  for Atmel AT91 advanced interrupt controller
 *
 *  Bugs report:  li ming  ( lmcs00@mails.tsinghua.edu.cn )
 *
 */

#ifndef __AT91_AIC_H
#define __AT91_AIC_H

/* always include this file for at91 peripherals */
#include "at91.h"


/* here define peripheral-specific hardware datastruct */
#define AIC_BASE		(0xfffff000)

/*
 ******************* COMMON PART ********************
 */
#define AIC_SMR(i)  (AIC_BASE+i*4)
#define AIC_IVR	    (AIC_BASE+0x100)
#define AIC_FVR	    (AIC_BASE+0x104)
#define AIC_ISR	    (AIC_BASE+0x108)
#define AIC_IPR	    (AIC_BASE+0x10C)
#define AIC_IMR	    (AIC_BASE+0x110)
#define AIC_CISR    (AIC_BASE+0x114)
#define AIC_IECR    (AIC_BASE+0x120)
#define AIC_IDCR    (AIC_BASE+0x124)
#define AIC_ICCR    (AIC_BASE+0x128)
#define AIC_ISCR    (AIC_BASE+0x12C)
#define AIC_EOICR   (AIC_BASE+0x130)




void at91_init_aic(void);

#endif// __AT91_AIC_H
