#include <jicama/system.h>
#include <jicama/process.h>
#include <jicama/spin.h>
#include <assert.h>
#include <string.h>



/*
**data struct
*/

__local volatile thread_t* current_tcb = NULL;


CREATE_SPINLOCK( thread_lock_sem );

void set_current_thread(thread_t *pthread)
{

	ASSERT(pthread!=NULL);
	current_tcb = pthread;	
	barrier();
}



/*
**返回当前运行进程的运行线程
*/

__public  thread_t *current_thread()
 {
	thread_t *pthread;
	unsigned cur_flag;

	save_eflags(&cur_flag);

	ASSERT(current_tcb!=NULL);
	pthread = current_tcb;

	restore_eflags(cur_flag);
	return pthread;
 }

proc_t*  current_proc() // current_proc() process.
{
	proc_t *rp;
	unsigned cur_flag;

	save_eflags(&cur_flag);

	ASSERT(current_tcb!=NULL);
	
	ASSERT(current_tcb->plink!=NULL);
	rp = current_tcb->plink;

	restore_eflags(cur_flag);
	return rp;
}

static bool thread_in_queue(thread_wait_t *queue, thread_t *pthread_query)
{
	thread_t *pthread ;//= TAILQ_FIRST(&(queue->head));

	TAILQ_FOREACH(pthread,&(queue->head),wait_lists){
		if (pthread == pthread_query)
		{
			return true;
		}
	}
	
	return false;
}





__public bool thread_waitq_empty(thread_wait_t *queue)
{
	ASSERT(queue != NULL);

	//判断队列是空否
	return TAILQ_EMPTY(&(queue->head));
}

//队列初始化
__public void thread_waitq_init(thread_wait_t *queue)
{
	ASSERT(queue != NULL);
	
	TAILQ_INIT(&(queue->head));
}


__public int thread_wakeup(thread_wait_t *queue)
{
	thread_t *pthread=NULL;
 	unsigned cur_flag;

	ASSERT(queue != NULL);

	if (TAILQ_EMPTY(&(queue->head)))
	{
		//kprintf("thread_wakeup empty queue on %p\n",queue);
		return -1;
	}

	trace("thread_wakeup called\n");

	save_eflags(&cur_flag);	
	//队列的第一个任务
	pthread = TAILQ_FIRST(&(queue->head));	

	if (pthread)
	{
	TAILQ_REMOVE(&(queue->head), pthread, wait_lists);
	TAILQ_ENTRY_INIT(pthread,wait_lists);

	//第一个任务可以被就绪了
	pthread->timeout_signaled = 0;
	}

	restore_eflags(cur_flag);

	//kprintf("thread_wakeup: wake %p %s\n",queue,pthread->name);


	//add to run lists.
	if (pthread)
		thread_ready(pthread);
	//ASSERT(find_thread_on_runlist(pthread));
	return 0;
	//kprintf("thread_wakeup: find_thread_on_runlist %p %s\n",queue,pthread->name);
}

__public int thread_wakeup_all(thread_wait_t *queue, int max)
{
	do
	{
		if (max!=-1 && max--==0)
		{
			break;			
		}
		if (thread_wakeup(queue))
		{
			break;
		}
	}
	while (1);
	return 0;
}


__public int thread_sleep_on_timeout(thread_wait_t *queue,unsigned timeout)
{
	thread_t *prev;
 	unsigned cur_flag;
	thread_t *pthread ;

	pthread = current_thread();

	save_eflags(&cur_flag);	
	thread_unready(pthread,timeout);

	if (!TAILQ_ENTRY_EMPTY(pthread,wait_lists))
	{
		//kprintf("sleep  - %s\n",  pthread->name);
		//goto sched;
	}



	//插入到队列尾部
	TAILQ_INSERT_TAIL(&(queue->head), pthread, wait_lists);

	restore_eflags(cur_flag);

sched:	

	schedule();

	if (pthread->timeout_signaled == -1)
	{
	save_eflags(&cur_flag);	


	//插入到队列尾部
	TAILQ_REMOVE(&(queue->head), pthread, wait_lists);

	restore_eflags(cur_flag);

	//kprintf("thread_sleep_on done %p %s 000\n",queue,pthread->name);
		return -1;
	}
	return 0;
}

__public int thread_sleep_on(thread_wait_t *queue)
{
	return thread_sleep_on_timeout(queue, INFINITE);
}


void switch_to_idle_thread()
{
	thread_t *ithread =  (idle_task);

	
	arch_thread_resume(NULL,ithread);
}


