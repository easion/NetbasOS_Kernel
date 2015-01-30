/*
**     (R)Jicama OS
**      I8259a Chip Driver
**     Copyright (C) 2003 DengPingPing
*/

#include <jicama/system.h>
#include <ibm/io.h>
#include <ibm/protect.h>
#include <ibm/traps.h>
#include <amd64/dep.h>


#define NR_IRQS 224
#define NR_IRQ_VECTORS 1024

__local unsigned char m1;
__local unsigned char m2;
unsigned short mrirq; // the irq mask from rmode used for going back to rmode for bios ints


#define		DEF_MODE	(0x0700|0x00)

__local short* vidmem = (short*) 0XB8000;

u32_t irq_table[16];

void do_exception()
{
	 char *p=(char*)0xb8000;

	 *p='E';
	 *(p+2)='X';
	 *(p+4)='C';
	 *(p+6)='-';
	 *(p+8)='H';

  kprintf("do_exception call\n");
	 while (1);
}




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
 
		 for (i = 0; i < 16; i++) 
		 irq_table[i] = (u64_t) spurious_irq;


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


