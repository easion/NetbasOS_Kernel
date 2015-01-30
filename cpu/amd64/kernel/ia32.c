
#include <jicama/system.h>
#include <amd64/ptrace.h>
#include <ibm/regs.h>
#include <amd64/msr.h>
#include <amd64/dep.h>



__asmlink void  ia32_sysenter_target();
__asmlink void  ia32_cstar_target();



void syscall32_cpu_init(void)
{
	/* Load these always in case some future AMD CPU supports
	   SYSENTER from compat mode too. */
	checking_wrmsrl(MSR_IA32_SYSENTER_CS, (u64_t)__KERNEL_CS);
	checking_wrmsrl(MSR_IA32_SYSENTER_ESP, 0ULL);
	checking_wrmsrl(MSR_IA32_SYSENTER_EIP, (u64_t)ia32_sysenter_target);

	wrmsrl(MSR_CSTAR, ia32_cstar_target);
}



void cpu_init()
{
	asm volatile("pushfq ; popq %%rax ; btr $14,%%rax ; pushq %%rax ; popfq" ::: "eax");

	syscall_init();

	wrmsrl(MSR_FS_BASE, 0);
	wrmsrl(MSR_KERNEL_GS_BASE, 0);
}

