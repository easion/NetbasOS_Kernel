
#include <jicama/system.h>
#include <jicama/process.h>
#include <amd64/dep.h>

__asmlink void divide_error(void);
__asmlink void debug(void);
__asmlink void nmi(void);
__asmlink void int3(void);
__asmlink void overflow(void);
__asmlink void bounds(void);
__asmlink void invalid_op(void);
__asmlink void device_not_avl(void);
__asmlink void double_fault(void);
__asmlink void coprocessor_segment_overrun(void);
__asmlink void invalid_tss(void);
__asmlink void segment_not_present(void);
__asmlink void stack_exception(void);
__asmlink void general_protection(void);
__asmlink void page_fault(void);
__asmlink void copr_error(void);
__asmlink void simd_coprocessor_error(void);
__asmlink void reserved(void);
__asmlink void alignment_check(void);
__asmlink void machine_check(void);
__asmlink void spurious_interrupt_bug(void);
__asmlink void call_debug(void);
__asmlink void syscall_handler(void);
__asmlink  void  llidt(uint16 size, u64_t address);

//struct gate_struct idt_table64[256]; 
void i8259_init();

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

void system_call();
__asmlink void unexpected_interrupt();

extern struct desc_struct cpu_gdt_table[16];
struct desc_ptr idt_descr = { 256 * 16, (u64_t) idt_table64 }; 
#define GDT_SIZE (16 * 8)
u64_t mmu_cr4_features;


/** Load IDTR register from memory.
 *
 * @param idtr_reg Address of memory from where to load IDTR.
 */
static inline void idtr_load(struct desc_ptr *idtr_reg)
{
	__asm__ volatile ("lidtq %0\n" : : "m" (*idtr_reg));
}


void  traps_init(void)
{
	#if 0
	int i=0;

	for (i=0; i<256; i++)
	set_intr_gate(i,&unexpected_interrupt);     
#else
	
	set_intr_gate(0,&divide_error);
	set_intr_gate_ist(1,&debug,4);
	set_intr_gate_ist(2,&nmi,3);
	set_system_gate(3,&int3);
	set_system_gate(4,&overflow);	/* int4-5 can be called from all */
	set_system_gate(5,&bounds);
	set_intr_gate(6,&invalid_op);
	set_intr_gate(7,&device_not_avl);
	set_intr_gate_ist(8,&double_fault, 2);
	set_intr_gate(9,&coprocessor_segment_overrun);
	set_intr_gate(10,&invalid_tss);
	set_intr_gate(11,&segment_not_present);
	set_intr_gate_ist(12,&stack_exception,1);
	set_intr_gate(13,&general_protection);
	set_intr_gate(14,&page_fault);
	set_intr_gate(15,&spurious_interrupt_bug);
	set_intr_gate(16,&copr_error);
	set_intr_gate(17,&alignment_check);
#ifdef CONFIG_X86_MCE
	set_intr_gate_ist(18,&machine_check, 5); 
#endif

	set_intr_gate(19,&simd_coprocessor_error);

	set_intr_gate(0x20 + 0, hwint00);	
	set_intr_gate(0x20 + 1, hwint01);	
	set_intr_gate(0x20 + 2, hwint02);	
	set_intr_gate(0x20 + 3, hwint03);	
	set_intr_gate(0x20 + 4, hwint04);	
	set_intr_gate(0x20 + 5, hwint05);	
	set_intr_gate(0x20 + 6, hwint06);	
	set_intr_gate(0x20 + 7, hwint07);	
	set_intr_gate(0x20 + 8, hwint08);	
	set_intr_gate(0x20 + 9, hwint09);	
	set_intr_gate(0x20 + 10, hwint10);	
	set_intr_gate(0x20 + 11, hwint11);	
	set_intr_gate(0x20 + 12, hwint12);	
	set_intr_gate(0x20 + 13, hwint13);	
	set_intr_gate(0x20 + 14, hwint14);	
	set_intr_gate(0x20 + 15, hwint15);	



	set_system_gate(IA32_SYSCALL_VECTOR, system_call);
#endif
	i8259_init();
	clear_in_cr4(X86_CR4_VME|X86_CR4_PVI|X86_CR4_TSD|X86_CR4_DE);

	asm volatile("movq %%cr4,%0" : "=r" (mmu_cr4_features));
	//cpu_gdt_descr[0].size = GDT_SIZE;
	//cpu_gdt_descr[0].address = (unsigned long)cpu_gdt_table;

	//asm volatile("lgdt %0" :: "m" (cpu_gdt_descr[0]));
	asm volatile("lidtq %0" :: "m" (idt_descr));
	      
	asm volatile("pushfq ; popq %%rax ; btr $14,%%rax ; pushq %%rax ; popfq" ::: "eax");

	kprintf("Load IDT OK\n");

	syscall_test();

	//while (1);
	
}
	typedef struct 
	{
		const char *message;
		int signum;
	}trap_msg_t;

	static trap_msg_t ex_data[] = {
		{"Divide error", SIGFPE},
		{"Debug exception", SIGTRAP},
		{"Nonmaskable interrupt", SIGBUS},
		{"Breakpoint", SIGEMT},
		{"Overflow", SIGFPE},
		{"Bounds check", SIGFPE},
		{"Invalid opcode", SIGILL},
		{"Coprocessor not available", SIGFPE},
		{"Double fault", SIGBUS},
		{"Copressor segment overrun", SIGSEGV},
		{"Invalid TSS", SIGSEGV},
		{"Segment not present", SIGSEGV},
		{"Stack exception", SIGSEGV},
		{"General protection", SIGSEGV},
		{"Page fault", SIGSEGV},
		{"???", SIGILL},
		{"Coprocessor error", SIGFPE},
  	};
