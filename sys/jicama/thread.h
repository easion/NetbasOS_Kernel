
// ---------------------------------------------------------------------------------
//Jicama Operating System
// Copyright (C) 2005  DengPingPing      All rights reserved.  
//-----------------------------------------------------------------------------------
#ifndef __thread_h__
#define __thread_h__

#include <signal.h>
#include <jicama/rlimit.h>


//#define MAX_THREADS  (NR_PROC*3)  /* max # of threads in the system   */
#define MAX_PRIO     (50)  /* max priority		       */	 	
/*
 * thread states
 */
#define TS_READY     (0x01)
#define TS_WAITING   (0x02)
#define TS_DEAD      (0x04)
#define TS_IOPENDING (0xF0)


#define OS_NAME_LENGTH 32

#define       NSIG_PROC 32

#define THREAD_MAGIC 0x2010dead

#define CHECK_THREAD_VAILD(p,val) do{\
	if(((p)->thread_magic) != (THREAD_MAGIC)){\
	kprintf("%s: line%d CHECK_THREAD_VAILD failed\n", __FUNCTION__,__LINE__);\
	return (val);\
	}\
}\
while (0) 

typedef struct tcb {
	tid_t    tid,ptid;        /* thread id                        */
	char name[OS_NAME_LENGTH];
	uint32_t thread_magic;
	void *sleep_timer;


	time_t  sticks,  ticks, waitpid;      /* ticks collected so far           */
	int      prio,timeout_signaled,exit_code;       /* priority                         */


	void* k_stack; //prunklid
	void* u_stack; //

	int u_stack_size,k_stack_size;
	
#ifdef __IA32__
	unsigned th_tss_sel;
	struct tss_struct tss;

	unsigned current_cr2;
	u8_t		fpu_state_info[128];	/* Intel 387 specific FPU state buffer	*/
#endif

	sigset_t signal_bits, signal_mask;
	struct sigaction sigaction[NSIG_PROC];
	struct rlimit rlimit[RLIM_NLIMITS];
	int msgport_id;

	//struct tcb *prev, *next; //为等待队列准备的

	struct process* plink;   /* process link                     */
	//struct tcb* next_thread;    /* scheduler link                   */

	u32_t    state;      /* state: ready, waiting and stuff  */
	//long trace_flags;
	regs_t regs;
	long debug_regs[8];

	long trace_flags;
	regs_t* trace_regs;
	TAILQ_ENTRY(tcb) thread_chain;
	TAILQ_ENTRY(tcb) run_lists;
	TAILQ_ENTRY(tcb) wait_lists;
	LIST_ENTRY(tcb) tcb_list;

}thread_t;

typedef	struct
{
    int 	ti_thread_id,ti_parent_id;   
    char	ti_thread_name[ 32 ];		/* Thread name				*/

    int  	ti_state;		/* Current task state.			*/
    unsigned int	ti_flags;
	int ti_msgport;
   

    int		ti_priority;
    int		ti_dynamic_pri;

    void*	ti_stack;
    unsigned	ti_stack_size;
    void*	ti_kernel_stack;		/* Top of kernel stack						*/
    unsigned int	ti_kernel_stack_size;		/* Size (in bytes) of kernel stack	*/		
     time_t	ti_sys_time;		/*	Micros in user mode		*/
    //bigtime_t	ti_user_time;		/*	Micros in kernal mode		*/
   // bigtime_t	ti_real_time;		/*	Total micros of execution	*/
     time_t	ti_quantum;		/*	Maximum allowed micros of execution before preemption	*/

} thread_info;


#define THREAD_SPACE(n) ((n)->plink)
#define PF_ALIGNWARN	0x00000001	/* Print alignment warning msgs */
					/* Not implemented yet, only for 486*/
#define PF_PTRACED	0x00000010	/* set if ptrace (0) has been called. */
#define PF_TRACESYS	0x00000020	/* tracing system calls */

//
typedef struct
{
	//struct tcb *head, *tail;
	TAILQ_HEAD(listhead_queue, tcb) head;
} thread_wait_t;

int thread_sleep_on(thread_wait_t *queue);
bool thread_waitq_empty(thread_wait_t *queue);
void thread_waitq_init(thread_wait_t *queue);
int thread_wakeup(thread_wait_t *queue);
int thread_wakeup_all(thread_wait_t *queue,int max);

int get_thread_msgport(thread_t *pthread);
int post_thread_message(thread_t *pthread,  void* va, u32_t count);
int get_thread_message(thread_t *pthread, void* kbuf,	u32_t count, unsigned long timeout);

//
__asmlink  thread_t *current_thread(void);

__asmlink  tid_t current_thread_id(char **name);
__asmlink  int set_thread_name(tid_t id, char *name);
 __asmlink  int set_thread_priority(tid_t id, int prio);
__asmlink void kthread_completion(void);

__asmlink void sendsig(thread_t *pthread, int signo);
__asmlink void sigclear(thread_t *pthread);
__asmlink void sendsig(thread_t *pthread, int signo);
__asmlink int sigdelset(thread_t *pthread, int signo);
__asmlink int sigrecv(thread_t *pthread, char*);
__asmlink thread_t *find_thread(const char* tname);



__asmlink  int sigmaskset(thread_t *pthread, int signo);
__asmlink  bool sigmasked(thread_t *pthread, int signo);
__asmlink  int sigmaskdel(thread_t *pthread, int signo);
void do_stop_current_thread(int sig, thread_t *pthread);
int create_thread_message(thread_t *pthread);

__asmlink tid_t new_kernel_thread(char *name, thread_entry_t* pth, void*);
__asmlink bool remove_kernel_thread(tid_t tid);
__asmlink void enable_iop(thread_t *);
__asmlink int do_execve(char * filename,   char ** __argv, char ** __env);




__asmlink char *strncpy_from_user(char *src_char, const char *add_char, size_t nr);
__asmlink int  thread_wait(thread_t *pthread, time_t timeout);
__asmlink int  thread_ready(thread_t *pthread);
__asmlink void  kernel_thread_exit(thread_t *pthread, void *ret);
void set_current_thread(thread_t *pthread);

int process_signal(regs_t * regs,  thread_t *);

int read_elf_dynamic(struct filp*  fp,  thread_t *pthread, void**entry);

int wait_for_thread(int tid,void *ret);
int thread_get_info(tid_t id,thread_info*info);

void  thread_exit_byid(tid_t id, void *ret);
void thread_yield(void);

int thread_sleep_on_timeout(thread_wait_t *queue,unsigned timeout);

int sigaddset(thread_t *pthread, int signo);
int  thread_unready(thread_t *pthread, time_t timeout);

void arch_thread_resume (thread_t *pfrom, thread_t *pto);
void do_exit_current_thread(int signo,  thread_t *pthread);

void arch_thread_free(thread_t* pthread);
void user_thread_init(void);

int do_clone (const thread_t *currentthread, thread_t *newthread,  regs_t *reg);

#endif

