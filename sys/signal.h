/* The <signal.h> header defines all the ANSI and POSIX signals.
 * MINIX supports all the signals required by POSIX. They are defined below.
 * Some additional signals are also supported.
 */

#ifndef _SIGNAL_H
#define _SIGNAL_H



/* Here are types that are closely associated with signal handling. */
typedef int sig_atomic_t;

#ifndef _SIGSET_T
#define _SIGSET_T
typedef unsigned long sigset_t;
#endif

#define _NSIG             16	/* number of signals used */

#define SIGHUP             1	/* hangup */
#define SIGINT             2	/* interrupt (DEL) */
#define SIGQUIT            3	/* quit (ASCII FS) */
#define SIGILL             4	/* illegal instruction */
#define SIGTRAP            5	/* trace trap (not reset when caught) */
#define SIGABRT            6	/* IOT instruction */
#define SIGIOT             6	/* SIGABRT for people who speak PDP-11 */
#define SIGUNUSED          7	/* spare code */
#define SIGFPE             8	/* floating point exception */
#define SIGKILL            9	/* kill (cannot be caught or ignored) */
#define SIGUSR1           10	/* user defined signal # 1 */
#define SIGSEGV           11	/* segmentation violation */
#define SIGUSR2           12	/* user defined signal # 2 */
#define SIGPIPE           13	/* write on a pipe with no one to read it */
#define SIGALRM           14	/* alarm clock */
#define SIGTERM           15	/* software termination signal from kill */

#define SIGEMT             7	/* obsolete */
#define SIGBUS            10	/* obsolete */

/* POSIX requires the following signals to be defined, even if they are
 * not supported.  Here are the definitions, but they are not supported.
 */
#define SIGCHLD           17	/* child process terminated or stopped */
#define SIGCONT           18	/* continue if stopped */
#define SIGSTOP           19	/* stop signal */
#define SIGTSTP           20	/* interactive stop signal */
#define SIGTTIN           21	/* background process wants to read */
#define SIGTTOU           22	/* background process wants to write */

/* The sighandler_t type is not allowed unless _POSIX_SOURCE is defined. */

typedef void (*__sighandler_t) (int);
typedef void (*__sigrestorer_t) (void);

/* Macros used as function pointers. */
#define SIG_ERR    ((__sighandler_t) -1)	/* error return */
#define SIG_DFL	   ((__sighandler_t)  0)	/* default signal handling */
#define SIG_IGN	   ((__sighandler_t)  1)	/* ignore signal */
#define SIG_HOLD   ((__sighandler_t)  2)	/* block signal */
#define SIG_CATCH  ((__sighandler_t)  3)	/* catch signal */

//struct sigaction {
//  __sighandler_t sa_handler;	/* SIG_DFL, SIG_IGN, or pointer to function */
//  sigset_t sa_mask;		/* signals to be blocked during handler */
//  int sa_flags;			/* special flags */
//  void(*restorer)(void *);
//};
struct sigaction {
  int sa_flags;
  __sighandler_t sa_handler;
  sigset_t sa_mask;
  __sigrestorer_t restorer;
};
/* Fields for sa_flags. */
#define SA_ONSTACK   0x0001	/* deliver signal on alternate stack */
#define SA_RESETHAND 0x0002	/* reset signal handler when signal caught */
#define SA_NODEFER   0x0004	/* don't block signal while catching it */
#define SA_RESTART   0x0008	/* automatic system call restart */
#define SA_SIGINFO   0x0010	/* extended signal handling */
#define SA_NOCLDWAIT 0x0020	/* don't create zombies */
#define SA_NOCLDSTOP 0x0040	/* don't receive SIGCHLD when child stops */

#define SANOCLDSTOP	1
#define SASTACK		0x08000000
#define SARESTART	0x10000000
#define SAINTERRUPT	0x20000000
#define SANOMASK	0x40000000
#define SAONESHOT	0x80000000

/* POSIX requires these values for use with sigprocmask(2). */
#define SIG_BLOCK          0	/* for blocking signals */
#define SIG_UNBLOCK        1	/* for unblocking signals */
#define SIG_SETMASK        2	/* for setting the signal mask */
#define SIG_INQUIRE        4	/* for internal use only */

__asmlink  int sigaction_setup(int signo, struct sigaction * new_act, struct sigaction * old_act);
__asmlink int signal_setup(int signo, __sighandler_t func,__sigrestorer_t fun2);
__asmlink  int sigreturn(u32_t ebx);
__asmlink u32_t _brk(unsigned brk_addr);

#endif /* _SIGNAL_H */
