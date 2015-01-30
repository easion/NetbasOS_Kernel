
#ifndef	_TRAPS64_H_
#define	_TRAPS64_H_

#include <string.h>
#define TSS_IOMAP_SIZE	(16*1024+1)	/* 16K for bitmap + 1 terminating byte for convenience */

struct tss {
	u32_t reserve1; /* reserved, ignored */
	u64_t rsp0;/* stack pointer CPL = 0 */
	u64_t rsp1; /* stack pointer CPL = 1 */
	u64_t rsp2;/* stack pointer CPL = 2 */
	u64_t reserve2; /* reserved, ignored */
	u64_t ist1; /* Interrupt stack table 1 */
	u64_t ist2;/* Interrupt stack table 2 */
	u64_t ist3;/* Interrupt stack table 3 */
	u64_t ist4;/* Interrupt stack table 4 */
	u64_t ist5;/* Interrupt stack table 5 */
	u64_t ist6;/* Interrupt stack table 6 */
	u64_t ist7;/* Interrupt stack table 7 */
	u64_t reserve3;/* reserved, ignored */
	u16_t reserve4;/* reserved, ignored */
	u16_t iomap_base;/* io bitmap offset */
	u8_t iomap[TSS_IOMAP_SIZE];
} __attr_packet;
typedef struct tss tss_t;

enum { 
	GATE_INTERRUPT = 0xE, 
	GATE_TRAP = 0xF, 	
	GATE_CALL = 0xC,
}; 	

// 16byte gate
struct gate_struct {          
	u16_t offset_0_15;
	u16_t segment; 
	unsigned ist : 3, zero0 : 5, type : 5, dpl : 2, p : 1;
	u16_t offset_16_31;
	u32_t offset_32_63;
	u32_t zero1; 
} __attr_packet;


static inline void idt_setoffset(struct gate_struct *d, u64_t offset)
{
	/*
	 * Offset is a linear address.
	 */
	d->offset_0_15 = offset & 0xffff;
	d->offset_16_31 = offset >> 16 & 0xffff;
	d->offset_32_63 = offset >> 32;
#if 0
	d->zero0 = 0;
	d->segment = __KERNEL_CS;

	d->p = 1;
	d->type = 0xe;	/* masking interrupt */
#endif
}

#define PTR_LOW(x) ((unsigned long)(x) & 0xFFFF) 
#define PTR_MIDDLE(x) (((unsigned long)(x) >> 16) & 0xFFFF)
#define PTR_HIGH(x) ((unsigned long)(x) >> 32)

enum { 
	DESC_TSS = 0x9,
	DESC_LDT = 0x2,
}; 

// LDT or TSS descriptor in the GDT. 16 bytes.
struct ldttss_desc { 
	u16_t limit0;
	u16_t base0;
	unsigned base1 : 8, type : 5, dpl : 2, p : 1;
	unsigned limit1 : 4, zero0 : 3, g : 1, base2 : 8;
	u32_t base3;
	u32_t zero1; 
} __attr_packet; 

struct desc_ptr {
	u16_t size;
	u64_t address;
} __attr_packet ;

#define load_TR_desc() asm volatile("ltr %w0"::"r" (GDT_ENTRY_TSS*8))
#define load_LDT_desc() asm volatile("lldt %w0"::"r" (GDT_ENTRY_LDT*8))
#define clear_LDT()  asm volatile("lldt %w0"::"r" (0))

/*
 * This is the ldt that every process will get unless we need
 * something other than this.
 */
__asmlink struct gate_struct idt_table64[256]; 

#define __KERNEL_CS 0x0028


#define IA32_SYSCALL_VECTOR	0x80

static inline void _set_gate(void *adr, unsigned type, u64_t func, unsigned dpl, unsigned ist)  
{
	struct gate_struct *s = (struct gate_struct *)adr; 	
	s->segment = __KERNEL_CS;
	s->ist = ist; 
	s->p = 1;
	s->dpl = dpl; 
	s->zero0 = 0;
	s->zero1 = 0; 
	s->type = type; 
	//s->offset_low = PTR_LOW(func); 
	//s->offset_middle = PTR_MIDDLE(func); 
	//s->offset_high = PTR_HIGH(func); 
	idt_setoffset(s, func);
	/* does not need to be atomic because it is only done once at setup time */ 
	//memcpy(adr, &s, 16); 
} 

static inline void set_intr_gate(int nr, void *func) 
{ 
	_set_gate(&idt_table64[nr], GATE_INTERRUPT, (unsigned long) func, 0, 0); 
} 

static inline void set_intr_gate_ist(int nr, void *func, unsigned ist) 
{ 
	_set_gate(&idt_table64[nr], GATE_INTERRUPT, (unsigned long) func, 0, ist); 
} 

static inline void set_system_gate(int nr, void *func) 
{ 
	_set_gate(&idt_table64[nr], GATE_INTERRUPT, (unsigned long) func, 3, 0); 
} 

static inline void set_tssldt_descriptor(void *ptr, unsigned long tss, unsigned type, 
					 unsigned size) 
{ 
	struct ldttss_desc d;
	memset(&d,0,sizeof(d)); 
	d.limit0 = size & 0xFFFF;
	d.base0 = PTR_LOW(tss); 
	d.base1 = PTR_MIDDLE(tss) & 0xFF; 
	d.type = type;
	d.p = 1; 
	d.limit1 = (size >> 16) & 0xF;
	d.base2 = (PTR_MIDDLE(tss) >> 8) & 0xFF; 
	d.base3 = PTR_HIGH(tss); 
	memcpy(ptr, &d, 16); 
}
/*
 * Intel CPU features in CR4
 */
#define X86_CR4_VME		0x0001	/* enable vm86 extensions */
#define X86_CR4_PVI		0x0002	/* virtual interrupts flag enable */
#define X86_CR4_TSD		0x0004	/* disable time stamp at ipl 3 */
#define X86_CR4_DE		0x0008	/* enable debugging extensions */
#define X86_CR4_PSE		0x0010	/* enable page size extensions */
#define X86_CR4_PAE		0x0020	/* enable physical address extensions */
#define X86_CR4_MCE		0x0040	/* Machine check enable */
#define X86_CR4_PGE		0x0080	/* enable global pages */
#define X86_CR4_PCE		0x0100	/* enable performance counters at ipl 3 */
#define X86_CR4_OSFXSR		0x0200	/* enable fast FPU save and restore */
#define X86_CR4_OSXMMEXCPT	0x0400	/* enable unmasked SSE exceptions */

#define __KERNEL_CS 0x28
#define  __USER32_CS 0x23

extern u64_t mmu_cr4_features;

static inline void set_in_cr4 (unsigned long mask)
{
	mmu_cr4_features |= mask;
	__asm__("movq %%cr4,%%rax\n\t"
		"orq %0,%%rax\n\t"
		"movq %%rax,%%cr4\n"
		: : "irg" (mask)
		:"ax");
}

static inline void clear_in_cr4 (unsigned long mask)
{
	mmu_cr4_features &= ~mask;
	__asm__("movq %%cr4,%%rax\n\t"
		"andq %0,%%rax\n\t"
		"movq %%rax,%%cr4\n"
		: : "irg" (~mask)
		:"ax");
}
#endif
