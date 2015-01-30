
// ---------------------------------------------------------------------------------------
//Jicama Operating System
// Copyright (C) 2003  DengPingPing      All rights reserved.  
//----------------------------------------------------------------------------------------
#ifndef __PROCESS_H__
#define __PROCESS_H__


#include <jicama/config.h>
//#include <jicama/message.h>
#include <sys/queue.h>
#include <jicama/paging.h>
#include <jicama/fs.h>
#include <jicama/thread.h>
#include <jicama/shm.h>


#define MAX_PID 20000



enum {
	FS_FORK=1,
	FS_EXEC,
	FS_EXIT
};



#define       PROC_MAGIC   (u8_t)0XF8
#define       WAIT_CORE 0200

//task priority define
#define CPU_NO_TIMES      0

#define CPU_THREAD_RROI (HZ/10)

#define inusermode(n) ((n)&3)
#define PNAME_MAXSZ 32
#define NSIG		NSIG_PROC

#define NIL_PROC          	((proc_t *) 0)
#define NIL_THREAD          	((thread_t *) 0)
#define proc_addr(nr) (proc_t *)(proc+(nr))
#define proc_number(RP) (RP)->p_asid
#define ISSU() (current_proc()->uid == 0)


/*env defined*/
#define MAX_EXEC_ARGS                 32
#define MAX_ARG_LEN                   32
/*user program linked address must be zero*/
#define ARG_BUFSZ 128


#define NR_PROC  40 
#define IS_USER_TASK(n) ( (n)>MON_PROC )

#define IDLE_PROC		0 ///this is a kernel process
#define MON_PROC		1
#define	INIT_PROC		2	//shell的进程号
/*
#define foreachthread(node, queue) 						\
for ((node) = (queue)->thread_head; \
     (node) != NIL_THREAD;							\
     (node) = (node)->next_thread)

#define toendthread(node, queue) 						\
for ((node) = (queue)->thread_head; \
     (node)->next_thread != NIL_THREAD;							\
     (node) = (node)->next_thread)
*/
#define NR_TASK_THREAD 8

typedef  int (*jicama_syscall_t) (regs_t *arg);

/*enum{
TS_STOPED = 0, TS_RUNNABLE, TS_BLOCKED, TS_ZOMBIE, TS_RECEIVE
};
*/

typedef enum {
	NO_EXEC, COFF, PE, DJMZ, ELF, ELF_USER_SO, SCRIPT 
} exec_t; // 

struct image_t;

typedef
struct image_queue_t {
	struct image_t *head;
	struct image_t *tail;
} image_queue_t;



typedef struct process
{
	u8_t magic, p_asid;
#ifdef __IA32__
	unsigned *proc_cr3;
	unsigned ldt_sel;
	sys_desc p_ldt[3];	//进程LDT
#endif

	//volatile time_t p_timeout;
	//time_t   sticks;

	uid_t uid;

	//pid_t   pid, ppid,waitpid; //////////process 's parent
	//unsigned priority, exit_code;
	//u8_t p_name[PNAME_MAXSZ];
	unsigned long start_stack, p_bss_code,p_brk_code; //brk, 
	u32_t    stk;        /* stack pointer                    */

	//struct process *prev, *next;
	LIST_ENTRY(process) next_links;

	unsigned int ntcb;
	//thread_t* thread_head;
	TAILQ_HEAD(listhead_thread, tcb) head;

	image_queue_t dynamic_module_list;
	image_queue_t disposable_module_list;
	int imageid_count;
	struct heap_list vm_heap_list;
	char *proc_user_params;
	char *proc_user_env;
}proc_t ;

#define KERNEL_NR_CHAR 64

struct bparam_s
{
  dev_t bp_rootdev;
  dev_t bp_bootdev;
  u32_t bp_ramsize;

  unsigned short bp_processor;
  unsigned char bp_kernel[KERNEL_NR_CHAR];
  unsigned short bp_fs_type;
  unsigned long int bp_switch;

  /*video info*/
    unsigned int lfb_width;
    unsigned int lfb_height;
    unsigned int lfb_bpp;
    unsigned long lfb_ptr;

	unsigned long logo_start;
	unsigned int logo_size;
	unsigned long font_start;
	unsigned int font_size;

  TAILQ_HEAD(listhead_t, tcb) thread_run_head;
 LIST_HEAD(listhead_t2, tcb) thread_list_head;

};



thread_t *main_thread(proc_t *rp);


void wakeup(proc_t *rp);

//////////////#define MAX_WAITED_TASK 10
struct semaphore_s {
	thread_wait_t sem_q;
	semaphore_t	 count;
};

typedef void *sem_t;

#define LOCK_SCHED(s) ((s)=save_eflags(NULL))
#define UNLOCK_SCHED(s) (restore_eflags(s))

void P(struct semaphore_s *s);
void V(struct semaphore_s *s);
void Sinit(struct semaphore_s *s, int cnt);
///////////////////
__asmlink int killpid (int pid, int sig);

extern volatile thread_t *idle_task;
extern volatile thread_t *uinit_task;
extern volatile thread_t *daemon_task;

//proc_t *proc[NR_PROC];
exec_t exec_get_format(void *image,struct filp*fp);
int free_page(unsigned long addr);
void free_pages(void *ptr, int counts);


void* current_proc_vir2phys(void* pos);
void setup_unix_socketcall(int (*fun)(int cmd, void* argp));
__asmlink thread_t *  create_new_process( unsigned entry, unsigned long ustack, char *name);

//extern	proc_t *proc[NR_PROC];

extern proc_t*  current_proc(void); // current_proc() process.
extern thread_t* bill_proc;

unsigned long get_page(void);

 int get_gdt_entry(void);
int exec_file(thread_t *_rp,  char*, char *, char *, char *);


int read_coff(struct filp*  ,  thread_t *, unsigned  *, int);
int read_djmz(struct filp*  ,  thread_t *, unsigned  *);
void exit(int code);

proc_t* ret_proc_entry(void);
int get_as_id(void);
int __switch_to(void);
char *pname_strcpy(char *src_char, const char *add_char);
 int read_elf_static(struct filp*  fp,  thread_t *pthread, unsigned *entry);
void proc_exit(int code,  thread_t *pthread,long );
void make_user_task( int proc_nr, void *entry, char* name);
int elf_check_so(struct filp*  fp);

exec_t pe_check(char *ident);

int user_heap_init(proc_t *rp, unsigned long logical_addr, unsigned int new_heap_size);


int waitpid (pid_t pid, unsigned long *stat_addr, int options);
 thread_t *find_thread_byid(tid_t tid);

pid_t do_wait (pid_t pid, unsigned long *status, int options);

time_t startup_ticks(void);
time_t get_unix_sec(void);
void schedule(void);


/*
**fpu
*/
void	load_fpu_state( void*  );
void	save_fpu_state( void*  );
/*
**
*/
void unlock_schedule(void);
void lock_schedule(int *);
void set_schedule(int );
bool scheduleable(void);

thread_t*  thread_new( void (*fn)(void *),void (*exit)(void *), void *args,  unsigned stack, proc_t *rp);
int  resume_thread( thread_t*);
int  suspend_thread( thread_t*);
void free_pcb(proc_t* rp);
int destory_proc_space(proc_t *rp);
void copy_pagedir(proc_t *current_task, proc_t *new_rp,pte_t from,pte_t to);

//static wait_queue_t g_task_waited;
void destory_child(thread_t *father);
__asmlink  int syscall_setup(int callnr, jicama_syscall_t fun);
__asmlink  int syscall_remove(int callnr);

void map_proc(proc_t *rp);
void unmap_proc(proc_t *rp, bool removedir);
int arch_proc_init(proc_t *rp);
int proc_mem_count(proc_t *rp);

int  arch_thread_create(thread_t* th_new,
void (*fn)(void *), void (*exit)(void *), 
void *args,  unsigned stack,  proc_t *rp);

void destory_arg_space(proc_t *rp);
void free_proc_mem_space(proc_t *rp);


unsigned save_pagedir(proc_t *rp);
unsigned restore_pagedir(pte_t cr3);

int copy_proc_code( proc_t *current_task, proc_t *new_rp);
pid_t do_waitpid(pid_t pid, unsigned long *state, int options);

typedef unsigned dynmodule_id;

typedef struct
{
    int   dy_image_id;  
    char  dy_name[ 256 ];    
    int   dy_num_count; 
    void*dy_entry_point;  
    void* dy_init;   
    void* dy_fini;  
    void* dy_text_addr;   
    void* dy_data_addr;   
    int   dy_text_size;  
    int   dy_data_size;   
    void*dy_ctor_addr;
    int dy_ctor_count;
} dyinfo_t;

int get_dynamic_dependencies(proc_t *rp,dynmodule_id imid, void **info,int init_head);
dynmodule_id load_library(proc_t *rp, char const *path,long flags);
int get_dynamic_module_info(proc_t *rp,dynmodule_id imid, dyinfo_t *info);
dynmodule_id unload_library(proc_t *rp, dynmodule_id imid);
void *dynamic_symbol(proc_t *rp, dynmodule_id imid, char const *symname);

typedef int port_id_t;
port_id_t msgport_get_public_port( int nId, int nFlags,time_t waitsecs );
int msgport_set_public_port( int nId, port_id_t nPort ) ;


void *user_malloc(proc_t *rp, unsigned int size,long flag);
int user_free( proc_t *rp, void *address);
int map_to(u32_t form_addr, u32_t to_addr, int pages);




#endif //__PROCESS_H__
