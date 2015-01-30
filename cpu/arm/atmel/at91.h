/*   
 *  at91.h  
 *  this file defines at91 related on AT91X40 
 *  for Atmel AT91 arch
 *
 *  Bugs report:     li ming  ( lmcs00@mails.tsinghua.edu.cn )
 *  Last modified:   2003-02-02 
 *
 */

#ifndef __AT91_H
#define __AT91_H

#define ARM_CLK		(32768000)



#define __arch_putb(v,a)        (*(volatile unsigned char *)(a) = (v))
#define __arch_putl(v,a)        (*(volatile unsigned int  *)(a) = (v))
#define __arch_getb(a)          (*(volatile unsigned char *)(a))
#define __arch_getl(a)          (*(volatile unsigned int  *)(a))
#define outb(v,a)  __arch_putb(v,a)
#define inb(a)     __arch_getb(a)


#define NR_IRQS		24
#define VALID_IRQ(i)	(i<=8 ||(i>=16 && i<NR_IRQS))

#define IRQ_FIQ		0
#define IRQ_SWI		1
#define IRQ_USART0	2
#define IRQ_USART1	3
#define IRQ_TC0		4
#define IRQ_TC1		5
#define IRQ_TC2		6
#define IRQ_WD		7
#define IRQ_PIOA	8

#define IRQ_EXT0	16
#define IRQ_EXT1	17
#define IRQ_EXT2	18


#define NETENABLE 1

#ifdef NETENABLE
#define AT91_NET_BASE    (0xfffa0000)
#define AT91_NET_SIZE    255
#define AT91_NET_IRQNUM  IRQ_EXT0   //16
#endif



#endif //__AT91_H
