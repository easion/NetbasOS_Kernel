


#include <jicama/system.h>
#include <jicama/process.h>
#include <arch/x86/io.h>
#include <jicama/spin.h>
#include <jicama/module.h>
#include <jicama/linux.h>
#include <arch/x86/traps.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

__asmlink __sighandler_t sigdefaulthandler[NSIG];
__asmlink void SIGRETURN_start_code();
__asmlink void SIGRETURN_end_code();


__local u32_t setulinux_type_sigframe(regs_t * regs, int signo, sigset_t  oldsigmask, __sigrestorer_t restorer)
{
	sigcontext_t * sc;
	sigcontext_t * sc_user;
	u32_t base = (u32_t)current_proc_vir2phys(0);
	const int codelen = (const int)(&SIGRETURN_end_code  - &SIGRETURN_start_code );


	sc = (sigcontext_t *) (regs->user_esp - sizeof(sigcontext_t));
	//kprintf("base=%d,sc=%x\n", base,sc);
	sc_user = current_proc_vir2phys(sc);
	mem_writeable(sc_user, sizeof(sigcontext_t));

	sc_user->signo = signo;
	sc_user->gs = regs->gs;
	sc_user->fs = regs->fs;
	sc_user->es = regs->es;
	sc_user->ds = regs->ds;
	sc_user->edi = regs->edi;
	sc_user->esi = regs->esi;
	sc_user->ebp = regs->ebp;
	sc_user->esp = regs->user_esp;
	sc_user->ebx = regs->ebx;
	sc_user->edx = regs->edx;
	sc_user->ecx = regs->ecx;
	sc_user->eax = regs->eax;	
	sc_user->eip = regs->eip; 
	sc_user->cs = regs->cs;
	sc_user->eflags = regs->eflags;
	sc_user->ss = regs->user_ss;
	sc_user->oldsigmask = oldsigmask; 

	memcpy(sc_user->asmcode, (void*)&SIGRETURN_start_code, codelen);

	sc_user->retaddr = (u32_t)sc + offsetof(sigcontext_t,asmcode);// 

	//kprintf("sc_user->retaddr = %x, %x\n", sc_user->retaddr,restorer);

	return (u32_t)sc;
}


//返回-1表示没有处理
//返回0表示处理了默认信号
//返回1表示处理了用户信号

int process_signal(regs_t * regs,  thread_t *pcurrent)
{
	int signo;
	struct sigaction *sa;
    unsigned eflags;
	u32_t uespsave;
	int ret=-1;

	ASSERT(regs!=NULL);
	ASSERT(pcurrent != NIL_THREAD);
	ASSERT(pcurrent->plink!=NULL);

	if (pcurrent == idle_task){
		//kprintf("can not in idle\n");
		return ret;
	}

	if (!( pcurrent->signal_bits & ~pcurrent->signal_mask)){
		return ret;
	}

	save_eflags(&eflags);

	while ((signo = sigrecv(pcurrent, "process_signal")) > 0) 
	{
		sigdelset(pcurrent, signo);
		//if ((pcurrent->plink == proc[INIT_PROC]) && (signo != SIGCHLD))continue;

		//mask it
		if(sigmasked(pcurrent, signo)==1 
			&&(signo!=SIGKILL&&signo !=SIGSTOP)){
			kprintf("sig is masked %d\n", signo);
			continue;
		}

		sa = &pcurrent->sigaction[(signo)];		
		if (sa->sa_handler == SIG_DFL) {
			ret=0;
			(sigdefaulthandler[signo])(signo);
			continue;
		}

		if(regs->cs != USER_CS)return ret;

		if (sa->sa_handler == SIG_IGN) {
			if (signo != SIGCHLD)
				continue;

			kprintf("signal: wait pid \n");
			while (do_waitpid(-1,(unsigned long *)0,1) > 0)/* reap child *//* do not wait for child to exit */
				/* nothing */;
			continue;
		}

		u32_t entry = (u32_t) sa->sa_handler;

		if (sa->sa_flags & SAONESHOT)
			sa->sa_handler = SIG_DFL;/*change for next time*/

		if (1)//pcurrent->plink->linux_type == 1)
		{
			//linux
			uespsave = setulinux_type_sigframe(regs, signo,pcurrent->signal_mask,sa->restorer);
		}
		else
		{
			//netbas
			#define SIG_FRAME_LEN (20+sizeof(regs_t))
			memcpy(&pcurrent->regs, regs,sizeof(regs_t));

			 uespsave= regs->user_esp-SIG_FRAME_LEN;
			u32_t uesp=(u32_t)proc_vir2phys(pcurrent->plink, uespsave);

			poke32(uesp, sa->restorer); //返回地址
			poke32(uesp+4, signo);
			poke32(uesp+8, signo);
			poke32(uesp+12, pcurrent->signal_mask);

			poke32(uesp+16, regs->user_esp-sizeof(regs_t));
			memcpy((void*)(uesp+20), regs,sizeof(regs_t));

			//kprintf("sig%d top 0x%x - (0x%x-0x%x)\n", signo, uespsave, regs->user_esp,regs->user_ss);
		}

		regs->eflags &= ~EFLAGS_TF;
		regs->user_esp = uespsave; 
		regs->eip = entry;//sa->sa_handler;

		clts();
		save_fpu_state(pcurrent->fpu_state_info);
		ret = 1;
		//kprintf("handler=0x%x, restorer=0x%x\n",(long)entry,(long)sa->restorer);
		break;
	}/*end where*/

	restore_eflags(eflags);
	return ret;
}

