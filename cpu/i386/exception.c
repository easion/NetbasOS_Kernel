
#include <jicama/system.h>
#include <jicama/process.h>
#include <jicama/paging.h>
#include <arch/x86/traps.h>
#include <string.h>


long reset_db6_status(void);

typedef struct 
{
	const char *message;
	int signum;
}trap_msg_t;

char *signame(int signo);
void dump_regs(regs_t *reg);
extern void mdelay(int);
int v86_monitor(regs_t *regs);


// Coprocessor not available exception
// Produced when no coprocessor available or when FPU
// instruction executed with Task Switch bit in cr0 set
// Since coprocessor IS installed, simply clear Task Switch bit in cr0
// and set fpu used and load fpu state
static void fix_fpu_state(void)
{
	clts();
#if	1
	{
	/*not to do more*/
	static proc_t *fpu_prev_used_pid=NULL;
	if(fpu_prev_used_pid==current_proc()) {
		return ;
	}

	load_fpu_state(current_thread()->fpu_state_info);
	fpu_prev_used_pid=current_proc();
	}
#endif
}


extern void save_regs_stat(const regs_t *r);

__public int do_exception(regs_t trap)
{
	int tnr;
	int nr = trap.trap_index;
	unsigned long pc = trap.eip;
	thread_t *rth;

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

 	if ((trap.eflags & EFLAGS_VM) == EFLAGS_VM)
	{
		save_regs_stat(&trap);
		//kprintf("save_regs_stat EFLAGS_VM() eax= %x %x %x \n",trap.ebp,trap.eax,trap.gs);
	     return v86_monitor(&trap);
	}

	//trace("do_exception() with %d\n", nr);

	switch (nr)
	{
	case 1: /*debug*/
	case 3: /*Breakpoint*/
		{
			kprintf("debug exce\n");
			rth = current_thread();

			if(nr == 1)
				rth->debug_regs[6] = reset_db6_status();

			if ((rth->trace_flags & PF_PTRACED))
			{
				//½øÈëµ÷ÊÔ
				rth->trace_regs = &trap;
				rth->exit_code = SIGTRAP;
				kprintf("Breakpoint() called\n");
				thread_wait(rth, INFINITE);
				return 0;
			}
			else{
			}
			break;
		}
	

	case 2:
		kprintf("got spurious NMI\n");
		disable();
		asm("hlt");	
		return 0;
		break;
	case 7:
		//kprintf("fpu exception!\n");
		fix_fpu_state();
		return 0;
		break;
	case 14:
		{
		pte_t cr2_val;
		int error;
		__asmlink int do_swap_page(pte_t error_code,pte_t address);
		__asmlink int do_no_page(pte_t error_code,pte_t vaddr);

		cr2_val = get_cr2();

		if (trap.eax&1)
		{
		/*kprintf("do_swap_page page trap.eax_org %x at addr %x\n",
			trap.eax_org,cr2_val );*/
			error = do_swap_page(trap.eax_org, cr2_val);
			if (error)
			{
				kprintf("do_swap_page error\n");
			}
		}
		else
		{
			error = do_no_page(trap.eax_org, cr2_val);
			//kprintf("do_no_page() eax = %x eax_org %x\n", trap.eax, trap.eax_org);
			if (error)
			{
				kprintf("do_no_page() error eax = %x eax_org %x\n", trap.eax, trap.eax_org);
				//dump_regs(&trap);
			}
		}
		//mdelay(2000);
		//enable();
		if(!error){
		trap.eax = trap.eax_org;
			return 0;
		}
		break;
		}
	default:
		break;		
	}

	if(0){

	void *array[12];
	size_t size;
	char **strings;
	int i;

	size = backtrace (array, 8);
	strings = backtrace_symbols (array, size);

	//backtrace_symbols_fd(array, size,1);
	}

	rth = current_thread();
	//rp = rth->plink;
	kprintf("###%s (Exiting due to signal %s)###\n",
		ex_data[nr].message, signame(ex_data[nr].signum));

	kprintf("Current Task.%d Thread[%s]\n",
		rth->tid,  rth->name);	

	dump_regs(&trap);

	do
	{
		if(trap.cs == USER_CS)
		{
			//dump_user_space_code(rp,0x8048074);
			sendsig(rth, ex_data[nr].signum);	
			break;
		}

		if(nr == 0)
			panic("Exception on idle task!\n");

		kernel_thread_exit(rth, NULL);
	}
	while (0);

	process_signal(&trap,rth);	
	return 0;
 }


