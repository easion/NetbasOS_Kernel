
#include <arch/x86/io.h>
#include <jicama/paging.h>
#include <jicama/process.h>
#include <errno.h>
#include <string.h>
#include <arch/x86/traps.h>
#include <assert.h>
#include <jicama/linux.h>

__asmlink  int i386_ioperm(unsigned long from, unsigned long num, int turn_on);

	/*Compliance with unix*/ 		
int linux_kernel_init()
{
		return 0;
}


 int UnixCall( ioperm)( unsigned long from, unsigned long num, int turn_on)
{
	return i386_ioperm(from, num, turn_on);
}



int UnixCall( clone )(unsigned int flag )
{
	int err;
	regs_t * regs;
	regs = (regs_t*) &flag;
	unsigned long newsp =regs->ecx ; 
	//regs_t netbas_reg;
	thread_t *pthread = NULL;
	proc_t *rp = current_proc();

	//conv_from_linux_regs(&netbas_reg, regs);

	kprintf("dump %s(): ebx=%x\n", __FUNCTION__, regs->ebx);//标志
	kprintf("ecx=%x\n", regs->ecx); //堆栈地址
	kprintf("edx=%x\n", regs->edx); //
	kprintf("esi=%x\n", regs->esi); //
	kprintf("edi=%x\n", regs->edi); //

	//建立一个新的线程体	
	pthread = thread_new((void*)regs->eip, NULL,(void*)NULL,newsp,  rp);
	if(!pthread)return -1;

	regs->user_esp = newsp;

	//kprintf("call do_clone\n");

	err = do_clone (current_thread(), pthread,  regs);

	//conv_to_linux_regs( regs,&netbas_reg);
	//panic("call do_clone ok\n");

	return pthread->tid;

	//panic(__FUNCTION__);
	//return 0;
}

int UnixCall( iopl)(unsigned int level )
{
	regs_t * regs;

	regs = (regs_t*) &level;

	if (level > 3)
		return EINVAL;

	enable_iop(current_thread());
	regs->eflags = (regs->eflags & 0xffffcfff) | (level << 12);
	return 0;
}

int UnixCall( sigreturn)(u32_t ebx)
{
	int ret;
	thread_t *thread = current_thread();
	sigcontext_t * sc;
	sigcontext_t * sc_user;
	regs_t * regs;

	regs = (regs_t*) &ebx;
	//kprintf("sigreturn called at 0x%x\n",  regs->user_esp);

	sc = (sigcontext_t*) regs->user_esp;
	sc_user = current_proc_vir2phys(sc);
	mem_writeable((void*) sc_user, sizeof(sigcontext_t));

	sigdelset(thread, SIGKILL);
	sigdelset(thread, SIGSTOP);
	thread->signal_mask = sc_user->oldsigmask;

	regs->cs = USER_CS;
	regs->user_ss = USER_DS;
	regs->ds = regs->es = regs->fs = regs->gs = USER_DS;

	regs->ebx = sc_user->ebx; /* may not work */
	regs->ecx = sc_user->ecx;
	regs->edx = sc_user->edx;
	regs->esi = sc_user->esi;
	regs->edi = sc_user->edi;
	regs->user_esp = sc_user->esp;
	regs->ebp = sc_user->ebp;
	regs->eip = sc_user->eip;
	regs->eflags = sc_user->eflags;
	ret = sc_user->eax;
	//regs->error_code = NOTSYSCALLFRAME;

	//kprintf("sigreturn called ok\n");
	load_fpu_state(thread->fpu_state_info);
	return ret;
}

