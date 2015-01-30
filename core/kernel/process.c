
// --------------------------------------------------------------------------------
//Jicama Operating System
// Copyright (C) 2003  DengPingPing      All rights reserved.  
//---------------------------------------------------------------------------------

#include <jicama/system.h>
#include <jicama/paging.h>
#include <jicama/process.h>
#include <jicama/fs.h>
#include <string.h>
#include <ansi.h>
#include <assert.h>
extern void thread_init();

__local volatile LIST_HEAD(listhead_p2, process) process_list_head;

volatile thread_t *idle_task;
volatile thread_t *uinit_task;
volatile thread_t *daemon_task;
__asmlink char *init_cmd;
__asmlink void kernel_idle();
__asmlink void daemon(void *arg);
void daemon_thread(void *arg);
#define MAX_ASID 255
static char as_space_map[MAX_ASID];
static sem_t thread_sock_sem;


/*only used get entry for user task*/
__public int get_as_id(void)
{
	int i;
	char bit;

	for (i = 0; i < MAX_ASID; ++i) {
		if(!as_space_map[i]){
			as_space_map[i] = 1;
			return i ;
		}
	}

	trace ("get_as_id(): no user process entry");
	return EAGAIN;
}


__public int put_as_id(unsigned int asid)
{
	if (asid>MAX_ASID)
	{
		return -1;
	}
	if (asid<INIT_PROC)
	{
		kprintf("put_as_id error\n");
		return -1;
	}
	as_space_map[asid] = 0;
	return 0;
}



char *pname_strcpy(char *src_char, const char *add_char)
{
	int i = 0;
	char *ret = src_char;

	while (*add_char != '\0' && i<OS_NAME_LENGTH&&*add_char!=' '){
		*src_char++ = *add_char++;
		i++;
	}

	*src_char++ = '\0';
	return ret;
}


/*
** alloc process space
*/

__local proc_t* alloc_pcb()
{
	proc_t* ret;
	proc_t* rp = kcalloc(sizeof(proc_t));

	if(!rp)
		return NIL_PROC;

	rp->magic = PROC_MAGIC;
	rp->p_asid = get_as_id();

	ret = rp;

	LOCK_SCHED(thread_sock_sem);	

	LIST_INSERT_HEAD(&process_list_head, rp, next_links);
	UNLOCK_SCHED(thread_sock_sem);	

	return (ret);
}
/*
** free proc
*/

__public void free_pcb(proc_t* rp)
{
	//int i=rp->p_asid;

	ASSERT(rp != NIL_PROC);
	LOCK_SCHED(thread_sock_sem);	
	LIST_REMOVE( rp, next_links);
	UNLOCK_SCHED(thread_sock_sem);	
	rp->magic = 0;
	kfree(rp);
}

thread_t * create_new_process(  unsigned entry, unsigned long ustack, char *name)
{
	proc_t *rp;
	thread_t *thnew;

	//get pcb block
	rp = alloc_pcb();

	if (!rp)
	{
		kprintf("alloc_pcb error\n");
		return NULL;
	}

	rp->p_bss_code = (long)entry;
	rp->p_brk_code=0;

	/*thread count*/
	rp->ntcb = 0;


	/*ticks count*/
	//rp->sticks = 0;
	TAILQ_INIT(&rp->head);

	if(arch_proc_init(rp))
		goto error;

	thnew = thread_new((void *)entry,NULL, NULL,ustack, rp);

	strncpy(thnew->name, name, OS_NAME_LENGTH);

	if(thnew==NULL)
		goto error;
	return thnew;

error:
	free_pcb(rp);
	return NULL;
}



/*
**进程初始化
*/
__public void process_init (void)
{
	int i;

	 LIST_INIT(&process_list_head);

	thread_init();

	uinit_task = NULL;

	for (i = 0; i <MAX_ASID; i++) {
		as_space_map[i] = 0;
	}


	trace("process_init():idle at 0x%x\n", (unsigned)kernel_idle);

    //make  idle and init process's tss .
	idle_task = create_new_process( (unsigned)kernel_idle, get_page()+PAGE_SIZE,"idle");
	ASSERT(idle_task);
	ASSERT(idle_task->plink->p_asid == IDLE_PROC);


	create_sys_task();



	set_current_thread((idle_task));
	return;
}


/*
** makesure init can be select to run
*/
__public void proc_setup()
{
	int i=0;
	char initfile[FPATH_LEN];
	char initarg[FPATH_LEN];
	thread_t *init_thread;
	unsigned pgdir;

	//创建初始化进程
	uinit_task = create_new_process(NULL, USER_STACK_ADDR_END, "Init");
	ASSERT(uinit_task);
	ASSERT(uinit_task->plink->p_asid == INIT_PROC);




	/*make runable server kernel process*/
	if(!init_cmd)init_cmd="/system/init";


	 memset(initfile,0, FPATH_LEN);

	 while (init_cmd[i] &&!isspace(init_cmd[i]))
		 i++;

	 if (i==0 || i > FPATH_LEN) {
		 panic("init file error");
	 }

	 strncpy( initfile, init_cmd, i);

	 while (isspace(init_cmd[i])) {
		 i++;
	 }

	(uinit_task)->tid = 1;

	init_thread = (uinit_task);
	sigclear(init_thread); /*clean signal*/

	 *(u16_t*)initarg = strlen(init_cmd);

	strncpy( &initarg[2], initfile, FPATH_LEN-2);

	pgdir = save_pagedir(uinit_task->plink);


	if(exec_file(init_thread, initfile,NULL,initarg, (char *)0) != OK){
		kprintf("proc_setup():%s not found !\n",init_cmd);
		set_current_thread(init_thread);
		destory_proc_space(THREAD_SPACE(init_thread));
	}
	else{		
	}

	restore_pagedir(pgdir);

	//unready
	init_thread->state = TS_WAITING;
	return;
}




__public char * thread_human(u32_t state)
{
	switch (state)
	{
	case TS_READY:
		return "RUNNING";
	case TS_IOPENDING:
		return "IOPEND";
	case TS_DEAD:
		return "DEAD";
	case TS_WAITING:
		return "WAIT";
	default:
		break;
	}
	return "OTHER???";
}



//转储所有的任务消息
__public int procdump(char * buf, int size)
{
	int len=0;
	thread_t *pthread;
	char tmp[1024];

	strcpy(buf, "PID  Prio PPID UID  UTICKS   MEM STAT   COMMAND\n");

	LOCK_SCHED(thread_sock_sem);	

	
	LIST_FOREACH(pthread,&boot_parameters.thread_list_head,tcb_list)
	{		
		len += sprintf(tmp, "%04d  %04d %04d %04d %08x 0.%02d %-06s %s\n", 
			pthread->tid,pthread->prio, pthread->ptid,THREAD_SPACE(pthread)->uid,
			pthread->sticks,proc_mem_count(THREAD_SPACE(pthread)), thread_human(pthread->state),  pthread->name);

		strcat(buf, tmp);
		/*kprintf( "\ttid%d\tsigmask:0x%x\tprio:%d\t0x%x\t%s\t%s\n", 
									pthread->tid,  pthread->signal_mask, pthread->prio, 
			 thread_human(pthread->state),  pthread->name);*/
	}

	UNLOCK_SCHED(thread_sock_sem);	

	//sprintf(tmp, "\tTask num:%d\tused:%d\tfreeslot:%d\n", 
	//							NR_PROC, NR_PROC-freeproc, freeproc);
	//strcat(buf, tmp);
	return len;
}


