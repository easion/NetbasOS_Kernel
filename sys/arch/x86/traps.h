
#ifndef _TRAP_GATE_H
#define _TRAP_GATE_H

#define ENABLE          0x20	// code used to re-enable after an interrupt 
#define MASTER         0x20	                // 首个中断控制端口 
#define MASTER_CTLMASK     0x21	//不允许中断设置位 setting bits in this port disables ints 
#define SLAVE       0xA0	                        // 第二个中断控制器端口
#define SLAVE_CTLMASK    0xA1	    //不允许中断设置位 setting bits in this port disables ints 
#define IRQ0		0x20

#define     TRAP_GATE                   0x8f00  //(0x80 | 15 )<<8 why? 
#define     INTERRUPT_GATE       0xee00  //0x8e-kernrl, 0xee-user
#define      SYSTEM_GATE             0xef00

extern  gate_desc idt[MAX_IDT];

static inline int idt_set_gate(int nr, u32_t handler, u16_t type)
{
   idt[nr].offset_0 = handler;  //handler function
   idt[nr].selector = 0x08;  //kenel code segment
   idt[nr].type = type;         //descriptor type
   idt[nr].offset_16 = (handler >> 16);
   return 0;
}

#define set_hw_trap_handler(n,addr) \
	idt_set_gate(n, (u32_t)(void*)addr,TRAP_GATE)

#define set_interrupt_gate(n,addr) \
	idt_set_gate(n, (u32_t)(void*)addr,INTERRUPT_GATE)

#define set_syscall_gate(n,addr) \
	idt_set_gate(n, (u32_t)(void*)addr, SYSTEM_GATE)





/* eflags register fields */
#define EFLAGS_CF 0x1 /* carry flag */
#define EFLAGS_PF 0x4 /* parity flag */
#define EFLAGS_AF 0x10 /* auxiliary flag */
#define EFLAGS_ZF 0x40 /* zero flag */
#define EFLAGS_SF 0x80 /* sign flag */
#define EFLAGS_TF 0x100 /* trap flag */
#define EFLAGS_IF 0x200 /* interrupt enable flag */
#define EFLAGS_DF 0x400 
#define EFLAGS_OF 0x800
#define EFLAGS_IOPL	0x00003000
#define EFLAGS_NT 0x4000 /* nested task flag */
#define EFLAGS_RF 0x10000 /* resume flag */
#define EFLAGS_VM 0x20000 /* virtual-8086 mode flag */
#define EFLAGS_AC 0x40000 /* alignment check flag */
#define EFLAGS_VIF 0x80000 /* virtual interrupt flag */
#define EFLAGS_VIP 0x100000 /* virtual interrupt pending */
#define EFLAGS_ID 0x200000 /* identification flag */



void remap_pic(unsigned char v1,unsigned char v2);
void set_irq_mask(unsigned short m);
unsigned short get_irq_mask();

#endif

