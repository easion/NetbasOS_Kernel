/*
**     (R)Jicama OS
**     Simple Thread Support
**     Copyright (C) 2003 DengPingPing
*/
#include <jicama/system.h>
#include <jicama/process.h>
#include <jicama/spin.h>
#include <jicama/msgport.h>
#include <assert.h>
#include <string.h>

bool remove_thread(proc_t *rp,  thread_t *pdest);
int thread_free(thread_t* pthread);

__local unsigned int next_pid=0x80; //make sure pid not zero
static volatile TAILQ_HEAD(, tcb) thread_run_list;
static sem_t thread_sock_sem;

/*
** thread_init
*/

__public void thread_init()
{
	int i;


	trace ("msg init\n");
	thread_msg_init();
	user_thread_init();

	 TAILQ_INIT(&thread_run_list);
	 LIST_INIT(&boot_parameters.thread_list_head);
}


/*
**
*/
__local int get_pid(void)
{
	next_pid++;

	if (next_pid < MAX_PID)
		return next_pid;
	else{
		next_pid = 1;
		return next_pid;
	}
}



inline thread_t *find_thread_byid(tid_t id)
{
	thread_t* pthread;

	LOCK_SCHED(thread_sock_sem);

	//thread
   LIST_FOREACH(pthread,&boot_parameters.thread_list_head,tcb_list) {
		ASSERT(pthread);
	   if(pthread->tid== id){
		   UNLOCK_SCHED(thread_sock_sem);
		   return pthread;
	   }
   }

   UNLOCK_SCHED(thread_sock_sem);
   //ASSERT(id>0);
   return NIL_THREAD;
}



int thread_get_info(tid_t id,thread_info*info)
{
	thread_t *pthread = find_thread_byid(id);
	if (!pthread)
	{
		return -1;
	}

	info->ti_thread_id = pthread->tid;
	info->ti_parent_id = pthread->ptid;
	info->ti_msgport = pthread->msgport_id;
	info->ti_state =  pthread->state;
	info->ti_flags =  pthread->trace_flags;
	info->ti_priority = pthread->prio;
	info->ti_quantum =  pthread->ticks;
	info->ti_sys_time =  pthread->sticks;

	info->ti_stack_size = pthread->u_stack_size;
	info->ti_kernel_stack_size = pthread->k_stack_size;

	info->ti_stack = pthread->u_stack;
	info->ti_kernel_stack = pthread->k_stack;
	strncpy(info->ti_thread_name,pthread->name,32);

	return 0;
}




thread_t *find_thread(const char* tname)
{
   u32_t i;
   thread_t* pthread ;

   LOCK_SCHED(thread_sock_sem);	

  LIST_FOREACH(pthread,&boot_parameters.thread_list_head,tcb_list) {
		ASSERT(pthread);


	  if (strcmp(pthread->name, tname)==0) {
		  UNLOCK_SCHED(thread_sock_sem);	
		  return pthread;
	  }
      //pthread++;
   }

   UNLOCK_SCHED(thread_sock_sem);	
   return (NIL_THREAD);
}

/*
** alloc thread space
*/

__public thread_t* alloc_tcb()
{
   thread_t* th_new ;

  th_new = kcalloc(sizeof(thread_t));
	if(!th_new)return NIL_THREAD;

	th_new->tid = get_pid();
	LOCK_SCHED(thread_sock_sem);	
	LIST_INSERT_HEAD(&boot_parameters.thread_list_head,th_new, tcb_list);
	UNLOCK_SCHED(thread_sock_sem);	
	return (th_new);
  
}

__public void free_tcb(thread_t* pthread)
{
	ASSERT(pthread != NIL_PROC);

	LOCK_SCHED(thread_sock_sem);	
	//ASSERT(find_thread_byid(pthread->tid));

	LIST_REMOVE(pthread, tcb_list);
	UNLOCK_SCHED(thread_sock_sem);	
	
	//panic("free_tcb %d\n",pthread->tid);
}

int find_thread_on_runlist(thread_t* id)
{
	int ret=FALSE;
	volatile thread_t *pthread=NULL;	
	int i=0;

	LOCK_SCHED(thread_sock_sem);	

	if (TAILQ_EMPTY(&thread_run_list)){
		UNLOCK_SCHED(thread_sock_sem);	
		return FALSE;
	}

	//kprintf("find_thread_on_runlist %p %p\n",CIRCLEQ_FIRST(&thread_run_list),CIRCLEQ_LAST(&thread_run_list));

	TAILQ_FOREACH(pthread,&thread_run_list,run_lists)
	{
		 barrier();
		i++;
		ASSERT(pthread);
		//kprintf("find_thread_on_runlist: %s %p loop%d\n",pthread->name,pthread,i);
		if(pthread->tid == id->tid){
			//trace("find_thread_on_runlist: %d %d, %p %p\n",pthread->tid, id->tid,pthread,id);
			ret = TRUE;
			break;
		}
	}

	UNLOCK_SCHED(thread_sock_sem);	

   return ret;
}

/*
**makesure thread runable
*/
__public int  thread_ready(thread_t *pthread)
{
	unsigned cur_flag;

	if (pthread == NIL_THREAD)
	{
		return -1;
	}

	CHECK_THREAD_VAILD(pthread,-1);

	if (!TAILQ_ENTRY_EMPTY(pthread,run_lists)){
		return -1;
	}

	if (pthread->state == TS_DEAD)
	{
		kprintf("thread_ready TS_DEAD thread\n");
		return -1;
	}

	ASSERT(THREAD_SPACE(pthread) != NULL);
	
	save_eflags(&cur_flag);

	//ASSERT(pthread->tid!= FREETID);

	trace("thread_ready: %s @ %s\n",
		pthread->name, "");

	//makesure thread can  be schedule
	pthread ->ticks = MAX(4, pthread->ticks);
	pthread ->state = TS_READY;

	//pthread->timeout_signaled = 0;

	//kprintf(__FUNCTION__"-----%s-----\n",pthread->name);
	
	TAILQ_INSERT_TAIL(&thread_run_list, pthread, run_lists);
	
	 barrier();
	
	ASSERT(pthread);
	restore_eflags(cur_flag);

	return 0;

}
#include <jicama/timer.h>

struct alarm_timer
{
	thread_t *thread;
	struct timer timer;
	void *arg;
};


static void free_sleep_timer(thread_t *pthread, int sig)
{
	int i;
	struct alarm_timer *at = pthread->sleep_timer;

	if (!at)
	{
		return;
	}

	pthread->timeout_signaled = sig;//自然超时

	remove_timer(&at->timer);
	kfree((u32_t)at);

	pthread->sleep_timer=NULL;
}

static void thread_sleep_timer(void *arg)
{
	thread_ready(arg);
	free_sleep_timer(arg,1);

	return ;
}

/*
**move thread
*/
__public int  thread_unready(thread_t *pthread, time_t timeout)
{
	unsigned cur_flag;
	int count;
	time_t s_ticks=INFINITE;
	struct alarm_timer *at;
	int rc;

   ASSERT(pthread != NIL_THREAD);
   ASSERT(THREAD_SPACE(pthread) != NULL);
   CHECK_THREAD_VAILD(pthread,-1);

   if(pthread == idle_task){
		panic("thread_wait: with idle(%s)\n", pthread->name);
	   return -2;
   }

	save_eflags(&cur_flag);

	free_sleep_timer(pthread,0);

	if(timeout==INFINITE){
		s_ticks = INFINITE;
	}  
   else{
		s_ticks = msec2ticks(timeout);
   }

   //kprintf("thread_unready remove %s ---\n",pthread->name);

	pthread->state = TS_WAITING;
	pthread ->ticks = MAX(4, pthread ->ticks);

	if (!TAILQ_ENTRY_EMPTY(pthread,run_lists)){
	TAILQ_REMOVE(&thread_run_list, pthread, run_lists);
	}

	TAILQ_ENTRY_INIT(pthread,run_lists);

	
	pthread->timeout_signaled = -1;

	//kprintf("thread_unready() sleep %s...\n",pthread->name);
	 barrier();

	 if(timeout!=INFINITE){
		at = (void*)kmalloc(sizeof (struct alarm_timer),0);

		if (!at)
		{
			restore_eflags(cur_flag);
			return -1;
		}

		at->thread = pthread;


		 init_timer(&at->timer, thread_sleep_timer, pthread);
		//at->timer.expires = startup_ticks() + msec2ticks(msecs);
		mod_timer(&at->timer,s_ticks);
		pthread->sleep_timer = at;
	 }
	 else{
		 //pthread ->ticks=CPU_NO_TIMES;
	 }


	restore_eflags(cur_flag);

	return 0;
}

__public int  thread_wait(thread_t *pthread, time_t timeout)
{
	int err;

	CHECK_THREAD_VAILD(pthread,-1);

	err=thread_unready(pthread,timeout);

	if (err)
	{
	kprintf("thread_wait thread_unready failed\n");
		return err;
	}


	schedule();
	if (pthread->timeout_signaled == -1)
	{
		return -1;
	}
	return 0;
}



/*
**free thread
*/


__public void  kernel_thread_exit(thread_t *pthread, void *ret)
{
   u32_t i;
   int error;
   unsigned cur_flag;

   kprintf("kernel_thread_exit start\n");

   ASSERT(pthread);
   
	//free memory and tss
   thread_free(pthread);  
 
	schedule();
	panic("oops! kill %s ok!\n", current_thread()->name);
	return;
}



__public void  thread_exit_byid(tid_t id, void *ret)
{
	thread_t *pthread = find_thread_byid(id);
	if (!pthread)
	{
		return ;
	}

	do_exit_current_thread(ret,pthread);
}

/*
**提高进程的优先级别
*/

__public int thread_prio_nice(tid_t tid, int newprio)
{
	thread_t *t = find_thread_byid(tid);

	CHECK_THREAD_VAILD(t,-1);

   if( newprio > MAX_PRIO)
	return (0);

   if(!t)return 0;

   t->prio = newprio;
   return (1);
}

/*
**
*/

__public  tid_t current_thread_id(char **name)
 {
	tid_t id;
	thread_t *pthread = current_thread();

	id = pthread->tid;
	if(name)
		*name = pthread->name;

	return id;
 }

__public  int set_thread_name(tid_t id, char *name)
 {
   thread_t* thread = find_thread_byid(id);

   if (!thread)  {
	   return -1;
   }
   CHECK_THREAD_VAILD(thread,-1);

	if(name)
	strncpy( thread->name, name, OS_NAME_LENGTH);
	return 0;
 }

 __public  int set_thread_priority(tid_t id, int prio)
 {
   thread_t* thread = find_thread_byid(id);

   if (!thread)  {
	   return;
   }

	thread->prio += prio;
	return ;
 }



/*
**返回当前运行进程的首个（主）线程
*/
__public thread_t *main_thread(proc_t *rp)
 {
	thread_t *pthread;
	unsigned cur_flag;

	ASSERT(rp!=NULL);

	save_eflags(&cur_flag);
	pthread = TAILQ_FIRST(&rp->head);
	ASSERT(pthread);
	restore_eflags(cur_flag);
	return pthread;
 }

int thread_free(thread_t* pthread)
{
	//kprintf("thread %s free ...\n",pthread->name);
	ASSERT(pthread != NIL_THREAD);

	CHECK_THREAD_VAILD(pthread,-1);

	free_sleep_timer(pthread,2);

	thread_unready(pthread,INFINITE);
	remove_thread(THREAD_SPACE(pthread),pthread);

	msgport_destroy(pthread->msgport_id);
	pthread->state = TS_DEAD;
	free_tcb(pthread);

	arch_thread_free(pthread);
	return 0;
}



int destory_proc_space(proc_t *rp)
{
	thread_t *pthread;
	thread_t *header;
	int nr=proc_number(rp);

	ASSERT(rp !=NULL);

		/*free args memory*/
	 destory_arg_space(rp);

	header=TAILQ_FIRST(&rp->head);

	while (header != NULL) {
		pthread =  TAILQ_NEXT(header, thread_chain);		
		thread_free(header);
		kfree(header);
		header = pthread;
	}

	if(IS_USER_TASK((nr))){		
		free_proc_mem_space(rp);	
	}


	TAILQ_INIT(&rp->head);	
	free_pcb(rp);
	return 0;
}

bool add_thread(proc_t *rp,  thread_t *pthread)
{
	thread_t *tempthread;

	ASSERT(rp != NULL);
	ASSERT(pthread != NIL_THREAD);

	LOCK_SCHED(thread_sock_sem);	

	if (!TAILQ_ENTRY_EMPTY(pthread,thread_chain))
	{
		kprintf("add_thread: warn added?\n");
	}

	TAILQ_INSERT_TAIL(&rp->head, pthread, thread_chain);
	rp->ntcb ++;
	UNLOCK_SCHED(thread_sock_sem);	



	return true; 
}

bool remove_thread(proc_t *rp,  thread_t *pdest)
{
	 thread_t *pthread;

	 if (!rp->ntcb) {
		 return false;
	 }

	 CHECK_THREAD_VAILD(pdest,-1);

	 LOCK_SCHED(thread_sock_sem);	

	 TAILQ_REMOVE(&rp->head, pdest, thread_chain);  

	//降低进程的优先级别
	//rp->priority -= pdest->prio;
	rp->ntcb --;
	UNLOCK_SCHED(thread_sock_sem);	
	return true;
}




/*
**thread_new()
*/
__public thread_t*  thread_new(void (*fn)(void *), void (*exit)(void *), 
	void *args,  unsigned stack,  proc_t *rp)
{
	int sig_hder;
	thread_t *th_tmp;
	int sc;
	int i;
	thread_t* th_new = alloc_tcb();

	ASSERT(rp !=NULL);

  if(th_new == NIL_THREAD ) {
	kprintf("thread_new(): no tcb\n");
	return (th_new);
   }

 	lock_schedule(&sc);
	//kprintf("th_new->tid=%d @ %s\n", th_new->tid, rp->p_name);

   /*
    * thread properties
    */
	th_new->state = TS_IOPENDING;
	th_new->prio = CPU_THREAD_RROI;
	th_new->ticks = th_new->prio;

	//TAILQ_INIT();
	TAILQ_ENTRY_INIT(th_new,thread_chain);
	TAILQ_ENTRY_INIT(th_new,run_lists);
	TAILQ_ENTRY_INIT(th_new,wait_lists);
	
	th_new->plink = rp;

	for (i=0; i<RLIM_NLIMITS; i++)
	{
		th_new->rlimit[i].rlim_cur = RLIM_INFINITY;
		th_new->rlimit[i].rlim_max = RLIM_INFINITY;
	}

	strncpy(th_new->name, "unamed_thread", OS_NAME_LENGTH);
	add_thread(rp, th_new); 
	sigclear(th_new);
	th_new->msgport_id = create_msgport(NULL,th_new);
	th_new->thread_magic = THREAD_MAGIC ;

   	if(arch_thread_create(th_new,fn,exit, args,stack,rp)){
		kprintf("arch_thread_create(): failed on %s\n", "unamed_thread");
		thread_free(th_new);
		kfree(th_new);
		return NIL_THREAD;
	}


	if (th_new->msgport_id<0)
	{
		kprintf("create_msgport error %x\n",th_new->msgport_id);
	}
	
	set_schedule(sc);
	return(th_new);
}

CREATE_SPINLOCK( sched_lock );

/*add signal check*/
/*not task switch*/
void schedule()
{
	int i=0;
	time_t  run_tims = CPU_NO_TIMES;
	thread_t *	pick_thread;
	thread_t *cur_thread;
	thread_t *pthread;
	unsigned cur_flag;

	
	save_eflags(&cur_flag);
	spin_lock( &sched_lock );


	cur_thread = current_thread();
	run_tims = 0;
	pick_thread = NIL_THREAD;
/*
**find a process,
*/
	while (1)
	{
		if (TAILQ_EMPTY(&thread_run_list))
		{
			//kprintf("&thread_run_list empty\n");
			break;
		}


	TAILQ_FOREACH(pthread,&thread_run_list,run_lists) {
		ASSERT(pthread);
		ASSERT(pthread!=idle_task);

		if (i++>100)
		{
			panic("TAILQ_FOREACH schedule error\n");
		}
	
		ASSERT(pthread->tid>0);
		if(pthread->state != TS_READY){
			continue;
		}
		if(pthread->ticks > run_tims){
			pick_thread = pthread;
		}
		run_tims = MAX(pthread->ticks,run_tims);
	}


	if(run_tims != 0)	{
		/*ok, we find a runable task*/
		break;
	}

	TAILQ_FOREACH(pthread,&thread_run_list,run_lists) {
		
		ASSERT(pthread);
		ASSERT(pthread->tid>0);
		
		if(pthread->state != TS_DEAD)
			pthread->ticks= pthread->ticks/2+pthread->prio;
		}
	}


/*
**select a process
*/
	if(pick_thread == NIL_THREAD){
		pick_thread = (idle_task);
		//kprintf("null proc here\n");
	}

	ASSERT(pick_thread->plink);

	bill_proc = pick_thread;

/*
**current thread sched
*/
	if(pick_thread == cur_thread ){
		spin_unlock( &sched_lock );
		restore_eflags(cur_flag);
		return ;
	}

	ASSERT(pick_thread);
	ASSERT(pick_thread->tid<MAX_PID);

	//kprintf("switch to %s ...\n", pick_thread->name);
	//set_current_thread(pick_thread); //设置为当前进程

	spin_unlock( &sched_lock );
	restore_eflags(cur_flag);


	arch_thread_resume(cur_thread, pick_thread);
	return ;
}




