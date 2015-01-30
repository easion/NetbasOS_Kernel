
#include <jicama/system.h>
#include <jicama/process.h>
#include <jicama/msgport.h>
#include <assert.h>
#include <string.h>
#include <jicama/iomux.h>
#define MAX_INFINITE 0xffffffff

struct task_queue
{
  struct task *head;
  struct task *tail;
  thread_t* thread;
  int maxsize;
  int size;
  int flags;
};

struct task_queue sys_task_queue;
static void (*idle_hook)() = NULL;


void set_idle_hook(void (*hook)())
{
	idle_hook = hook;
}

__public void kthread_completion()
{
	thread_t *thread;
	int err = 0;
	
	thread = current_thread();

	syslog(LOG_DEBUG,"kthread_completion() at thread %s ...\n", thread->name);
	kernel_thread_exit(thread , &err);
	panic("oops, kthread_completion ,but it's%d run ...\n", thread->tid);
}


/*
**
*/
__public tid_t new_kernel_thread(char *name, thread_entry_t* entry_ptr, void *arg)
{
	thread_t* new_thread=NULL;
	proc_t *rp;
	unsigned long stack;

	rp = daemon_task->plink;
	stack = get_page();

	if (!stack)
	{
		return -1;
	}

	stack += PAGE_SIZE;

/*
**check args
*/
   ASSERT(entry_ptr != NIL_THREAD);
   ASSERT(rp != NIL_PROC);	

	/*make a thread*/
	new_thread = thread_new( entry_ptr, NULL,arg,stack, rp);

	if (new_thread == NULL){	
		kprintf("new_kernel_thread(): error\n");
		return -1;
	}

	if (!name)
	{
		//线程名为空的情况
		snprintf(new_thread->name,OS_NAME_LENGTH, "%s-%d", "daemon", new_thread->tid);
	}
	else{
		strncpy(new_thread->name, name, OS_NAME_LENGTH);
	}

	//panic("new_thread->name: %s\n",  new_thread->name);
	thread_ready(new_thread);	


	return new_thread->tid;
}

/*
**
*/
__public bool remove_kernel_thread(tid_t tid)
{	
	return TRUE;
}

/*
**
*/
__public bool daemon_thread_setarg(tid_t tid, void *arg)
{
	if (tid > NR_TASK_THREAD)
	{
		return FALSE;
	}

	return TRUE;
}

void kernel_idle(void *arg)
{
	int tasks;

	trace("entry idle proc\n");
	//ASSERT(idle_hook);

	do
	{
		//tasks = runable_proc(true);
		//tasks = runable_thread();
		__asm__ volatile ( "nop" : : : "memory" );
		//enable();
		if(idle_hook)
			(*idle_hook)();
		else
		__asm__ volatile ( "hlt" );

		//if (tasks == 0)	{
		//	continue;
		//}
		
#ifdef __ARM__
		kprintf("idle call schedule tasks=%d ...\n", tasks);
#endif
	}
	while (1);	

}



static void daemon_thread(void *tqarg)
{
	struct task_queue *tq = &sys_task_queue;
	struct task *task;
	taskproc_t proc;
	void *arg;
	unsigned the_eflag;
	thread_t *thread;

  (void)tqarg;

  thread = current_thread();
  while (1)
  {
    // Wait until tasks arrive on the task queue
    while (tq->head == NULL) 
    {
      thread_wait(thread,INFINITE);
      if (tq->head == NULL){
		  //SCHEDEVT('#');
	  }
    }

	save_eflags(&the_eflag);

    // Get next task from task queue
    task = tq->head;
    tq->head = task->next;
    if (tq->tail == task)
		tq->tail = NULL;
    tq->size--;


    // Execute task
    task->flags &= ~TASK_QUEUED;
 	restore_eflags(the_eflag);
   if ((task->flags & TASK_EXECUTING) == 0)
    {
      task->flags |= TASK_EXECUTING;
      proc = task->proc;
      arg = task->arg;
      tq->flags |= TASK_QUEUE_ACTIVE;
  
      proc(arg);

      if (!(tq->flags & TASK_QUEUE_ACTIVE_TASK_INVALID)) 
		  task->flags &= ~TASK_EXECUTING;
      tq->flags &= ~(TASK_QUEUE_ACTIVE | TASK_QUEUE_ACTIVE_TASK_INVALID);
    }
  }
}

int create_sys_task(void)
{
	struct task_queue *tq = &sys_task_queue;

	memset(tq, 0, sizeof(struct task_queue));
	tq->maxsize = MAX_INFINITE;

	//创建内核守护进程
	daemon_task = create_new_process( (unsigned)daemon_thread, get_page()+PAGE_SIZE, "kdaemon");

  //tq->thread = new_kernel_thread("systask",daemon_thread, tq);
	tq->thread =find_thread("kdaemon");
	ASSERT(daemon_task);
	ASSERT(daemon_task->plink->p_asid== MON_PROC);

  return 0;
}

void init_task(struct task *task)
{
  task->proc = NULL;
  task->arg = NULL;
  task->next = NULL;
  task->flags = 0;
}

int queue_task(struct task *task, taskproc_t proc, void *arg)
{
	thread_t *pthread;
	struct task_queue *tq = &sys_task_queue;
	unsigned the_eflag;


	if (!tq) 
	  tq = &sys_task_queue;
	if (task->flags & TASK_QUEUED) 
	  return EBUSY;
	if (tq->maxsize != MAX_INFINITE && tq->size >= tq->maxsize) 
	  return EAGAIN;

	task->proc = proc;
	task->arg = arg;
	task->next = NULL;
	task->flags |= TASK_QUEUED;

	save_eflags(&the_eflag);
	if (tq->tail){
	tq->tail->next = task;
	tq->tail = task;
	}
	else
	tq->head = tq->tail = task;

	tq->size++;
	restore_eflags(the_eflag);


	if ((tq->flags & TASK_QUEUE_ACTIVE) == 0 )
	 // && tq->thread->state == THREAD_STATE_WAITING)
	{
	  //pthread = find_thread_byid(tq->thread);
		thread_ready(tq->thread);
	}

  return 0;
}

