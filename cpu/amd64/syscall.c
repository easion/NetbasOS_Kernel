#include <jicama/system.h>
#include <amd64/ptrace.h>
#include <ibm/regs.h>
#include <amd64/msr.h>
#include <amd64/dep.h>

void syscall32_cpu_init(void);


__asmlink void system_call();

void syscall_init(void)
{
	/* Enable SYSCALL/SYSRET */
	set_efer_flag(0);

	/* 
	 * LSTAR and STAR live in a bit strange symbiosis.
	 * They both write to the same internal register. STAR allows to set CS/DS
	 * but only a 32bit target. LSTAR sets the 64bit rip. 	 
	 */ 
	wrmsrl(MSR_STAR,  ((u64_t)__USER32_CS)<<48  | ((u64_t)__KERNEL_CS)<<32); 
	wrmsrl(MSR_LSTAR, system_call); 

#ifndef CONFIG_IA32_EMULATION   		
	syscall32_cpu_init ();
#endif

	/* Flags to clear on syscall */
	//wrmsrl(MSR_SYSCALL_MASK, EF_TF|EF_DF|EF_IE|0x3000); 
	/* Mask RFLAGS on syscall 
	 * - disable interrupts, until we exchange the stack register
	 *   (mask the IE bit)
	 */
	wrmsrl(MSR_SYSCALL_MASK, 0x200); 
}


 int main_ia32_syscall(void *args)
 {	
	 kprintf("main_ia32_syscall call\n");
	 while (1);	 
 }

 int c_amd64_syscall(u64_t *args)
 {	
	 int i;
	// kprintf("c_amd64_syscall call\n");
	 for (i=0; i<9; i++)
	 {
		 kprintf("c_amd64_syscall arg %d - %d -%x\n", i, args[i]);
	 }
	while (1);	 
 }


/** Dispatch system call */
u64_t c_syscall_handler(u64_t a1, u64_t a2, u64_t a3,
			 u64_t a4, u64_t id)
{
	u64_t rc;

kprintf("id is %d", id);
while (1)
{
}

	
	return rc;
}

