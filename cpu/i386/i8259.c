/*
**     (R)Jicama OS
**      I8259a Chip Driver
**     Copyright (C) 2003 DengPingPing
*/

#include <jicama/system.h>
#include <arch/x86/io.h>
#include <arch/x86/protect.h>
#include <arch/x86/traps.h>

__local unsigned char m1;
__local unsigned char m2;
unsigned short mrirq; // the irq mask from rmode used for going back to rmode for bios ints
#define NR_IRQS 16

__asmlink u32_t irq_table[NR_IRQS];
__asmlink void* irq_arg_table[NR_IRQS];

__asmlink void hwint00 (void);
__asmlink void hwint01 (void);
__asmlink void hwint02 (void);
__asmlink void hwint03 (void); 
__asmlink void hwint04 (void) ;
__asmlink void hwint05 (void);
__asmlink void hwint06 (void); 
__asmlink void hwint07 (void) ;
__asmlink void hwint08 (void) ;
__asmlink void hwint09 (void) ;
__asmlink void hwint10 (void);
__asmlink void hwint11 (void);
__asmlink void hwint12 (void);
__asmlink void hwint13 (void);
__asmlink void hwint14 (void);
__asmlink void hwint15 (void);

int irq_table_init(void);


 void en_irq (const unsigned int nr)
{
	unsigned char value;
	unsigned int port;

	port = nr > 8 ? SLAVE_CTLMASK : MASTER_CTLMASK;

	value = inb (port);  
	value &= ~(1 << nr);
	outb (port, value);
}

 void dis_irq (const unsigned int nr) 
{
  unsigned char value;
  unsigned int port;

  port = nr > 8 ? SLAVE_CTLMASK : MASTER_CTLMASK;
  value = inb (port);
  value |= 1 << nr;
  outb (port, value);
}



void remap_pic(unsigned char v1,unsigned char v2)
{
	  //  ICW1  
    outb (MASTER, 0x11);//  Reset PIc Master port A  
    outb (SLAVE, 0x11);//  Reset PIC Slave port A  

    //  ICW2  
    outb (MASTER_CTLMASK, v1);//  Master offset of 0x20 in the IDT  
    outb (SLAVE_CTLMASK, v2);//  Master offset of 0x28 in the IDT  

    //  ICW3  
    outb (MASTER_CTLMASK, 0x04);//  Slaves attached to IR line 2  
    outb (SLAVE_CTLMASK, 0x02);//  This slave in IR line 2 of master  

    //  ICW4  
    outb (MASTER_CTLMASK, m1);//  Set non-buffered  
    outb (SLAVE_CTLMASK, m2);//  Set non-buffered  
}

void i8259_init ()
  { 
	  int i;

	disable();
	mrirq=((unsigned short)inb(0xA1)<<8)|(unsigned short)inb(0x21);

	m1 = m2 = 0x01;
	remap_pic(0x20, 0x28);

	irq_table_init();

	set_interrupt_gate(0x20 + 0, hwint00);	
	set_interrupt_gate(0x20 + 1, hwint01);	
	set_interrupt_gate(0x20 + 2, hwint02);	
	set_interrupt_gate(0x20 + 3, hwint03);	
	set_interrupt_gate(0x20 + 4, hwint04);	
	set_interrupt_gate(0x20 + 5, hwint05);	
	set_interrupt_gate(0x20 + 6, hwint06);	
	set_interrupt_gate(0x20 + 7, hwint07);	
	set_interrupt_gate(0x20 + 8, hwint08);	
	set_interrupt_gate(0x20 + 9, hwint09);	
	set_interrupt_gate(0x20 + 10, hwint10);	
	set_interrupt_gate(0x20 + 11, hwint11);	
	set_interrupt_gate(0x20 + 12, hwint12);	
	set_interrupt_gate(0x20 + 13, hwint13);	
	set_interrupt_gate(0x20 + 14, hwint14);	
	set_interrupt_gate(0x20 + 15, hwint15);	
 }

void mask_irq(int n)
{
	if(n>7) {
		n-=8;
		m2|=(1<<n);
		outb(SLAVE_CTLMASK,m2);
	} else {
		m1|=(1<<n);
		outb(MASTER_CTLMASK,m1);
	}
}

void unmask_irq(int n)
{
	if(n>7) {
		n-=8;
		m2&=~(1<<n);
		outb(SLAVE_CTLMASK,m2);
	} else {
		m1&=~(1<<n);
		outb(MASTER_CTLMASK,m1);
	}
}

unsigned short get_irq_mask()
{
	unsigned short ret;

	ret=0;
	ret|=m2;
	ret<<=8;
	ret|=m1;

	return ret;
}

void set_irq_mask(unsigned short m)
{
	m2=(m>>8)&0xFF;
	m1=m&0xFF;

	outb(SLAVE_CTLMASK,m2);
	outb(MASTER_CTLMASK,m1);
}
