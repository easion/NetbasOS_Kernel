


#include <jicama/system.h>
#include <jicama/process.h>
#include <jicama/spin.h>
#include <jicama/module.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

int sigisignore(int signo);

#define sigisvalid(signo) ((signo)>0  && (signo)<=NSIG)

/* Low bit of signal masks. */
#define SIGBIT_0	((sigset_t) 1)
/* Mask of valid signals (0 - NSIG_PROC).  Assume the shift doesn't overflow. */
#define SIGMASK		((SIGBIT_0 << (NSIG + 1)) - 1)


//nt contain(int a) { return current_thread()->signal_bits & (1UL << a); }
int contain(unsigned long bits, int a) { return bits & (1UL << --a); }
int sig_invalid(unsigned long bits) { return contain(bits, SIGKILL) || contain(bits, SIGSTOP); }

__local int sig_procs;

__local char *__signame[] = {
	"ZERO", "HUP", "INT", "QUIT", "ILL", "TRAP", "IOT", "BUS",
	"FPE", "KILL", "USR1", "SEGV", "USR2", "PIPE", "ALARM", "TERM",
	"STKFLT", "CHLD", "CONT", "STOP", "TSTP", "TTIN", "TTOU", "URG",
	"XCPU", "XFSZ", "VTALRM", "PROF", "WINCH", "IO", "PWR", "31"
};

__public char *signame(int signo)
{
  if (!sigisvalid(signo)) return "???"; 
  return __signame[signo];
}


__public int sigmaskempty(thread_t *pthread) 
{ return pthread->signal_bits == 0; }

void sigclear(thread_t *pthread)
{ 
   ASSERT(pthread != NIL_THREAD);
	pthread->signal_bits = 0; 
	pthread->signal_mask = 0; 
}

__public void maskedsig(sigset_t * masked)
{
	*masked = current_thread()->signal_bits & current_thread()->signal_mask;
}

__public void unmaskedsig(thread_t *pthread, sigset_t * unmasked)
{
   ASSERT(pthread != NIL_THREAD);
	*unmasked = pthread->signal_bits & ~pthread->signal_mask;
}

__public  int sigaddset(thread_t *pthread, int signo)
{
   ASSERT(pthread != NIL_THREAD);

	if (!sigisvalid(signo)) {
		return EINVAL;
	  }

	pthread->signal_bits |= SIGBIT_0 << --signo;
	thread_ready(pthread);
	return 0;
}


__public  int sigmaskset(thread_t *pthread, int signo)
{
   ASSERT(pthread != NIL_THREAD);

	if (!sigisvalid(signo)) {
		return EINVAL;
	  }

	pthread->signal_mask |= SIGBIT_0 << --signo;
	return 0;
}


__public  int sigmaskdel(thread_t *pthread, int signo)
{
   ASSERT(pthread != NIL_THREAD);

	if (!sigisvalid(signo)) {
		return EINVAL;
	}

	pthread->signal_mask &= ~(SIGBIT_0 << --signo);
	return 0;
}


__public  int sigdelset(thread_t *pthread, int signo)
{
   ASSERT(pthread != NIL_THREAD);

	if (!sigisvalid(signo)) {
		return EINVAL;
	}

	pthread->signal_bits &= ~(SIGBIT_0 << --signo);
	return 0;
}

__public void sendsig(thread_t *pthread, int signo)
{
   ASSERT(pthread != NIL_THREAD);

	sigaddset(pthread, signo);
	//kprintf("sig add %d to task%d\n", signo,proc_number(pthread));
	  ++sig_procs;			/* count new process pending */
 }

int signo(sigset_t sig)
{
	int i;

	for (i=1; i<=NSIG; i++)
	{
		if(sig&SIGBIT_0)
		{
			//kprintf("sig%d found!\n",sig);
			return i;
		}
		sig>>=1;
	}
	return 0;
}

__public int sigrecv(thread_t *pthread, char *ch)
{
	register int i;
	sigset_t sig;

	MSGASSERT(pthread != NIL_THREAD, ch);

	unmaskedsig(pthread, &sig);
	return signo(sig);
}


__public int sigemptyset(sigset_t *set)
{
  *set = 0;
  return 0;
}

__public  int sigfillset(sigset_t *set)
{
  *set = SIGMASK;
  return 0;
}

__public  bool sigmasked(thread_t *pthread, int signo)
{
	int ret;
	unsigned cur_flag;
   ASSERT(pthread != NIL_THREAD);

	if (!sigisvalid(signo)) {
		return EINVAL;
	}

	save_eflags(&cur_flag);
	ret = pthread->signal_mask & (SIGBIT_0 << --signo);
	restore_eflags(cur_flag);

	return ret?true:false;
}


__public  int sigpending(sigset_t * set)
{
   ASSERT(set != NULL);
	mem_writeable((u32_t)set, sizeof(sigset_t));
	maskedsig(set);
	return 0;
}

__local void checkpending(struct sigaction * sa, int signo)
{
   ASSERT(sa != NULL);

	if (sa->sa_handler == SIG_IGN) {
		if (signo == SIGCHLD)
			return;
		sigdelset(current_thread(), signo);
		return;
	}
	if (sa->sa_handler == SIG_DFL) {
		if (!sigisignore(signo))
			return;
		sigdelset(current_thread(), signo);
	}
}

__public void sig_dump()
{
	int i;

	kprintf("sigset contain:");
	for ( i = 1; i < 32; i++)
		if (contain(current_thread()->signal_bits, i))
			kprintf("SIG%s  ", signame(i));
	kprintf("\n");
}

__public  int signal_setup(int signo, __sighandler_t func, __sigrestorer_t fuc2)
{
	__sighandler_t ofunc;
	struct sigaction * sa;

	if (!sigisvalid(signo) || signo == SIGKILL || signo == SIGSTOP)
		return EINVAL; 

	sa = &current_thread()->sigaction[(signo)];
	ofunc = sa->sa_handler; /*save old*/
	sa->sa_handler = func;
	sigemptyset(&current_thread()->signal_mask);
	sa->sa_flags = SAONESHOT | SANOMASK;
	sa->sa_mask = 0;
	sa->restorer = fuc2;
	checkpending(sa, signo);
	return (int)ofunc;
}

__public  int sigaction_setup(int signo, struct sigaction * new_act, struct sigaction * old_act)
{
	struct sigaction * sa;

	if (!sigisvalid(signo) || signo == SIGKILL || signo == SIGSTOP)
		return EINVAL; 

	sa = &current_thread()->sigaction[(signo)];

	if (old_act) {
		mem_writeable((u32_t)old_act, sizeof(struct sigaction));
		memcpy(old_act, sa, sizeof(struct sigaction));
	}

	if (!new_act)return 0;

	memcpy(sa, new_act, sizeof(struct sigaction));

	if (new_act->sa_flags & SANOMASK){
		new_act->sa_mask = 0;
		}
	else{
		//if(signo != SIGCHLD)
		//sigaddset(current_thread(), signo);
		}

	checkpending(sa, signo);
	return 0;
}


__local void proc_stop(int signo)
{
	trace("proc_stop call %s\n",signame(signo));
	do_stop_current_thread(signo, current_thread());
}

__local void ignore(int signo)
{
	trace("ignore: (%s)!\n",signame(signo));
}

__local void terminate(int signo)
{
	trace("terminate: (%s)!\n",signame(signo));
	proc_stop(signo);
}

__local void sigwakeup(int signo)
{
	trace("sigwakeup: (%s)!\n",signame(signo));
	do_stop_current_thread(signo, current_thread());
}

__local void coredump(int signo)
{
	//kprintf("coredump: (%s)!\n",signame(signo));
	terminate(signo);
}

__public __sighandler_t sigdefaulthandler[NSIG] = {
ignore,			/* SIGZERO	 	 0 */			terminate,		/* SIGHUP		 1 */
terminate,		/* SIGINT		 2 */			coredump,		/* SIGQUIT		 3 */
coredump,		/* SIGILL		 4 */			coredump,		/* SIGTRAP		 5 */
coredump,		/* SIGABRT SIGIOT 6 */ignore,			/* SIGUNUSED 7 */
coredump,		/* SIGFPE		 8 */		terminate,		/* SIGKILL		 9 */
terminate,		/* SIGUSR1		10 */		coredump,		/* SIGSEGV		11 */
terminate,		/* SIGUSR2		12 */		terminate,		/* SIGPIPE		13 */
sigwakeup,		/* SIGALRM		14 */		terminate,		/* SIGTERM		15 */
terminate,		/* SIGSTKFLT		16 */ignore,			/* SIGCHLD		17 */
ignore,			/* SIGCONT		18 */		proc_stop,			/* SIGSTOP		19 */
proc_stop,			/* SIGTSTP		20 */			proc_stop,			/* SIGTTIN		21 */
proc_stop,			/* SIGTTOU		22 */			terminate,		/* SIGIO SIGPOLL	23 */
ignore,			/* SIGURG		SIGIO */	coredump,		/* SIGXCPU		24 */
coredump,		/* SIGXFSZ		25 */		terminate,		/* SIGVTALRM		26 */
ignore,			/* SIGPROF		27 */		ignore,			/* SIGWINCH		28 */
};

int sigisignore(int signo)
{
	return sigdefaulthandler[signo] == ignore;
}

__sighandler_t sigdefault_ponit(int signo)
{
	if(!sigisvalid(signo))
		return (__sighandler_t)0;
	return sigdefaulthandler[signo];
}

