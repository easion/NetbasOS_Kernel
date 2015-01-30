
#include <jicama/system.h>
#include <jicama/process.h>
#include <arch/x86/traps.h>
#include <jicama/ipc.h>
#include <jicama/ptrace.h>
#include <string.h>
#include <assert.h>


typedef int (ptrace_func_t)( thread_t *, thread_t *, void *, void * );

struct user
{
	regs_t reg;
	long dbg[8];
};

struct user_fpregs_struct
{
  long int cwd;
  long int swd;
  long int twd;
  long int fip;
  long int fcs;
  long int foo;
  long int fos;
  long int st_space [20];
};

struct user_fpxregs_struct
{
  unsigned short int cwd;
  unsigned short int swd;
  unsigned short int twd;
  unsigned short int fop;
  long int fip;
  long int fcs;
  long int foo;
  long int fos;
  long int mxcsr;
  long int reserved;
  long int st_space[32];   /* 8*16 bytes for each FP-reg = 128 bytes */
  long int xmm_space[32];  /* 8*16 bytes for each XMM-reg = 128 bytes */
  long int padding[56];
};

#define STRUCT_OFFSET(TYPE, MEMBER) ((unsigned long) &((TYPE *) 0)->MEMBER)
#define EFLAG_MASK			0x00044dd5
#define DR7_MASK			0xffff07ff
#define EFL 14

static void untrace( thread_t *pthread )
{
	pthread->debug_regs[7] = 0;
	if(pthread->trace_regs)
		pthread->trace_regs->eflags &= ~EFLAGS_TF;
	pthread->trace_flags=0;
}

static int ptrace_pokeuser(  thread_t *pthread,
                            void *address, void *data_ptr )
{
	long int reg_offset = (long int) address;
	u32_t value = (u32_t) data_ptr;

	if ( ( reg_offset & 3 ) != 0 )
		return EINVAL;

	if ( reg_offset < sizeof( struct user ) )
	{
		if ( reg_offset < STRUCT_OFFSET(struct user, dbg) )
		{
			/* write register */
			void *pDst = ( (void *) pthread->trace_regs ) + reg_offset;

			switch ( reg_offset )
			{
				case STRUCT_OFFSET(regs_t, ds) :
				case STRUCT_OFFSET(regs_t, es) :
				case STRUCT_OFFSET(regs_t, fs) :
				case STRUCT_OFFSET(regs_t, gs) :
				case STRUCT_OFFSET(regs_t, cs) :
				case STRUCT_OFFSET(regs_t, user_ss) :
					*( (u16_t *) pDst ) = (u16_t) ( value & 0xffff );
					break;
				case STRUCT_OFFSET(regs_t, eflags) :
					value &= EFLAG_MASK;
					value |= pthread->trace_regs->eflags &
						~EFLAG_MASK;
					/* fall through */
				default :
					*( (u32_t *) pDst ) = value;
			}
		}
		else
		{
			/* write debug register */
			int reg_idx = ( reg_offset - STRUCT_OFFSET(struct user, dbg) ) / 4;
			kprintf( "writing 0x%X to debug register %d\n", value, reg_idx );

			if ( reg_idx == 7 )
				value &= DR7_MASK;
			else if ( reg_idx < 4 )
			{
				//if ( ( value != 0 ) && ( value < AREA_FIRST_USER_ADDRESS ) )
				//	return EIO;
			}

			pthread->debug_regs[reg_idx] = value;
		}
	}

	return 0;
}

static int ptrace_peekuser(  thread_t *pthread,
                            void *address, void *data_ptr )
{
	long int reg_offset = (long int) address;
	int retval = -EIO;

	if ( ( reg_offset & 3 ) != 0 )
		return EINVAL;

	if ( reg_offset < sizeof( struct user ) )
	{
		if ( reg_offset < STRUCT_OFFSET(struct user, dbg) )
		{
			/* read register */
			reg_offset += (long int) pthread->trace_regs;
			retval = memcpy_to_user( data_ptr, (void *) reg_offset, 4 );
		}
		else
		{
			/* read debug register */
			int reg_idx = ( reg_offset - STRUCT_OFFSET(struct user, dbg) ) / 4;
			kprintf( "reading debug register %d\n", reg_idx );
			retval = memcpy_to_user( data_ptr,
				(void *) &pthread->debug_regs[reg_idx], 4 );
		}
	}

	return retval;
}


__local int continue_with_signal( thread_t *pthread, u32_t signo )
{
	if ( signo > _NSIG )
		return EIO;

	if ( ( signo != 0 ) && ( signo != SIGSTOP ) )
	{
		//atomic_or( &pthread->tr_nPTraceFlags, PT_ALLOW_SIGNAL );
		sigaddset( pthread, signo );
	}

	thread_ready( pthread );
	return 0;
}

__local int ptrace_cont(  thread_t *target_thread,
                        void *address, void *data_ptr )
{

	target_thread->trace_regs->eflags &= ~EFLAGS_TF;
	return continue_with_signal( target_thread, (u32_t) data_ptr );
}

static int ptrace_detach(  thread_t *pthread,
                          void *address, void *data_ptr )
{
	untrace( pthread );
	return ptrace_cont(  pthread, NULL, data_ptr );
}


__local int ptrace_kill(  thread_t *target_thread,
                        void *address, void *data_ptr )
{
	if ( target_thread->state == TS_DEAD )
		return ptrace_detach(  target_thread, NULL, (void *) SIGKILL );
	else
	{
		untrace( target_thread );
		sendsig( target_thread, SIGKILL );
		return 0;
	}
}

__local int ptrace_singlestep(  thread_t *target_thread,void *address, void *data_ptr )
{
	target_thread->trace_regs->eflags |= EFLAGS_TF;
	return continue_with_signal( target_thread, (u32_t) data_ptr );	
}

__local int ptrace_getregs(  thread_t *target_thread,
                           void *address, void *data_ptr )
{
	return memcpy_to_user( data_ptr, (void *) target_thread->trace_regs, sizeof(regs_t) );
}

__local int ptrace_setregs(  thread_t *target_thread,
                           void *address, void *data_ptr )
{
	regs_t regs;

	ASSERT( target_thread->trace_regs);

	if ( memcpy_from_user( &regs, data_ptr, sizeof(regs_t) ) < 0 )
		return EFAULT;

	regs.eflags &= EFLAG_MASK;
	regs.eflags |= target_thread->trace_regs->eflags & ~EFLAG_MASK;

	memcpy( (void *) target_thread->trace_regs, &regs, sizeof(regs_t) );
	return 0;
}

__local int ptrace_getfpregs(thread_t *target_thread,
                             void *address, void *data_ptr )
{
	struct user_fpregs_struct *fpsave;
	ASSERT(sizeof(struct user_fpregs_struct)<=sizeof(target_thread->fpu_state_info));

	fpsave =(struct user_fpregs_struct *)target_thread->fpu_state_info;
	return memcpy_to_user( data_ptr, fpsave,
	                      sizeof( struct user_fpregs_struct ) );
}

__local int ptrace_setfpregs(thread_t *target_thread,
                             void *address, void *data_ptr )
{
	int retval;
	struct user_fpregs_struct *fpsave; 

	ASSERT(sizeof(struct user_fpregs_struct)<=sizeof(target_thread->fpu_state_info));
	
	fpsave=(struct user_fpregs_struct *)target_thread->fpu_state_info;

	retval = memcpy_from_user( fpsave, data_ptr,
	                            sizeof( struct user_fpregs_struct )  );

	return retval;
}

__local int ptrace_traceme( thread_t *target_thread)
{
	/* are we already being traced? */
	if (target_thread->trace_flags & PF_PTRACED)
		return EPERM;

	/* set the ptrace bit in the proccess trace_flags. */
	target_thread->trace_flags |= PF_PTRACED;
	return 0;
}


__local int ptrace_attach( thread_t *child,thread_t *pthread)
{
	if (child == pthread)
		return EPERM;

	//if ((!child->dumpable || (current->uid != child->euid) ||
	//    (current->gid != child->egid)) && !suser())
	//	return EPERM;
	/* the same process cannot be attached many times */
	if (child->trace_flags & PF_PTRACED)
		return EPERM;
	child->trace_flags |= PF_PTRACED;
	if (child->ptid != pthread->tid) {
		//REMOVE_LINKS(child);
		child->ptid = pthread->tid;
		//SET_LINKS(child);
	}
	sendsig((child), SIGSTOP);
	return 0;
}

static int ptrace_peekmem(  thread_t *target_thread,
                           void *address, void *data_ptr )
{
	u32_t value;
	int retval=0;
	unsigned long *ptr;
	long real_addr = address;
	proc_t *rp = target_thread->plink;



	if ((unsigned long)real_addr > USER_SPACE_SIZE)
		return -1;

	ptr = (unsigned long *)proc_vir2phys(rp, (unsigned long)address);
	value = *ptr;

	if ( ptr )
		retval = memcpy_to_user( data_ptr, &value, 4 );

	return retval;	
}

static int ptrace_pokemem(  thread_t *target_thread,
                           void *address, void *data_ptr )
{
	u32_t value;
	int retval=0;
	unsigned long *ptr;
	long real_addr = address;
	proc_t *rp = target_thread->plink;

	if (!rp)
		return -1;

	

	if ((unsigned long)real_addr > USER_SPACE_SIZE)
		return -1;

	ptr = (unsigned long *)proc_vir2phys(rp, (unsigned long)address);

	if ( ptr )
		retval = memcpy_from_user( ptr, data_ptr, 4 );

	return retval;	
}

int UnixCall( ptrace)(long request, long pid, long addr, long data)
{
	thread_t *thread;
	struct user * dummy;
	int i,retval=-1;
	thread_t *currentthread = current_thread();
	//proc_t *current = currentthread->plink;
	regs_t *regs = (regs_t*) &request;

	//没有启用调试选项，防止病毒利用
	if (check_debug()==0)
		return EPERM;

	dummy = NULL;

	kprintf("%s() %d %d %x %x\n", __FUNCTION__, request, pid, addr, data);

	if (request == PTRACE_TRACEME) {
		return ptrace_traceme(currentthread);
	}

	if (pid == 1)		/* you may not mess with init */
		return EPERM;

	if (!(thread = find_thread_byid(pid)))
		return ESRCH;

	//thread = thread->plink;

	if (request == PTRACE_ATTACH) {

		return ptrace_attach(thread, currentthread);
	}

	if (!(thread->trace_flags & PF_PTRACED))
		return ESRCH;

	if (thread->state != TS_DEAD) {
		if (request != PTRACE_KILL)
			return ESRCH;
	}

	if (thread->ptid != currentthread->tid)
		return ESRCH;

	switch (request) {
	/* when I and D space are seperate, these will need to be fixed. */
		case PTRACE_PEEKTEXT: /* read word at location addr. */ 
		case PTRACE_PEEKDATA: {
			unsigned long tmp;
			int res;

			res = ptrace_peekmem((thread),addr,data); //-1;//read_long(child, addr, &tmp);
			if (res < 0)
				return res;
			//res = verify_area(VERIFY_WRITE, (void *) data, sizeof(long));
			//if (!res)
			//	put_fs_long(tmp,(unsigned long *) data);
			return res;
		}

	/* read the word at location addr in the USER area. */
		//case PTRACE_PEEKUSR: 			
		//	return 0;

      /* when I and D space are seperate, this will have to be fixed. */
		case PTRACE_POKETEXT: /* write the word at location addr. */
		case PTRACE_POKEDATA:
			return ptrace_pokemem((thread),addr,data);

		//case PTRACE_POKEUSR: /* write the word at location addr in the USER area */
			
		  //return EIO;

		case PTRACE_SYSCALL: /* continue and stop at next (return from) syscall */
		case PTRACE_CONT: { /* restart after signal. */
			long tmp;

			if ((unsigned long) data > NSIG)
				return EIO;
			if (request == PTRACE_SYSCALL)
				thread->trace_flags |= PF_TRACESYS;
			else
				thread->trace_flags &= ~PF_TRACESYS;

			thread->exit_code = data;
			ptrace_cont((thread),addr,data);
			thread_ready(thread);

	/* make sure the single step bit is not set. */
			//tmp = get_stack_long(child, sizeof(long)*EFL-sizeof(regs_t)) & ~EFLAGS_TF;
			//put_stack_long(child, sizeof(long)*EFL-sizeof(regs_t),tmp);
			return 0;
		}

/*
 * make the child exit.  Best I can do is send it a sigkill. 
 * perhaps it should be put in the status that it want's to 
 * exit.
 */
		case PTRACE_KILL: {
			long tmp;

			//child->status = TASK_RUNNING;
			thread_ready(thread);
			thread->exit_code = SIGKILL;
			ptrace_kill((thread),addr,data);
	/* make sure the single step bit is not set. */
			//tmp = get_stack_long(thread, sizeof(long)*EFL-sizeof(regs_t)) & ~EFLAGS_TF;
			//put_stack_long(thread, sizeof(long)*EFL-sizeof(regs_t),tmp);
			return 0;
		}

		case PTRACE_SINGLESTEP: {  /* set the trap flag. */
			long tmp;

			if ((unsigned long) data > NSIG)
				return EIO;
			thread->trace_flags &= ~PF_TRACESYS;
			//tmp = get_stack_long(thread, sizeof(long)*EFL-sizeof(regs_t)) | EFLAGS_TF;
			//put_stack_long(thread, sizeof(long)*EFL-sizeof(regs_t),tmp);
			//thread->status = TASK_RUNNING;
			thread_ready(thread);
			thread->exit_code = data;
	/* give it a chance to run. */
			return 0;
		}

		case PTRACE_DETACH: { /* detach a process that was attached. */
			long tmp;

			if ((unsigned long) data > NSIG)
				return EIO;
			thread->trace_flags &= ~(PF_PTRACED|PF_TRACESYS);
			//thread->status = TASK_RUNNING;
			thread_ready(thread);
			thread->exit_code = data;
			//REMOVE_LINKS(thread);
			//thread->p_pptr = thread->p_opptr;
			//SET_LINKS(thread);
			/* make sure the single step bit is not set. */
			//tmp = get_stack_long(thread, sizeof(long)*EFL-sizeof(regs_t)) & ~EFLAGS_TF;
			//put_stack_long(thread, sizeof(long)*EFL-sizeof(regs_t),tmp);
			return 0;
		}

		case PTRACE_PEEKUSER:
		{
			retval = ptrace_peekuser((thread),addr,data);
			break;
		}

		case PTRACE_POKEUSER:
		{
			retval = ptrace_pokeuser((thread),addr,data);
			break;
		}

		case PTRACE_GETREGS:
		{
			retval = ptrace_getregs((thread),addr,data);
			break;
		}

		case PTRACE_SETREGS:{
			retval = ptrace_setregs((thread),addr,data);
			break;
		}

		case PTRACE_GETFPREGS:{
			retval = ptrace_getfpregs((thread),addr,data);
			break;
		}

		case PTRACE_SETFPREGS:{
			retval = ptrace_setfpregs((thread),addr,data);
			break;
		}

		case PTRACE_GETFPXREGS:{
			//retval = ptrace_getfpregs((child),addr,data);
			break;
		}

		case PTRACE_SETFPXREGS:{
			//retval = ptrace_setfpregs((child),addr,data);
			break;
		}

		default:
			return EIO;
	}
	return retval;
}

