/*
**     (R)Jicama OS
**     Program Args Setup
**     Copyright (C) 2003,2004 DengPingPing
*/
#include <jicama/system.h>
#include <jicama/paging.h>
#include <jicama/process.h>
#include <string.h>
#include <assert.h>
static sem_t thread_sock_sem;

/* options for waitpid, WUNTRACED not supported */
/* waitpid ��ѡ�����WUNTRACED δ��֧�� */
#define WNOHANG 1		// ���û��״̬Ҳ��Ҫ���𣬲����̷��ء�
#define WUNTRACED 2		// ����ִֹͣ�е��ӽ���״̬��

#define WIFEXITED(s) (!((s)&0xFF)	// ����ӽ��������˳�����Ϊ�档
#define WIFSTOPPED(s) (((s)&0xFF)==0x7F)	// ����ӽ�����ֹͣ�ţ���Ϊtrue��
#define WEXITSTATUS(s) (((s)>>8)&0xFF)	// �����˳�״̬��
#define WTERMSIG(s) ((s)&0x7F)	// ���ص��½�����ֹ���ź�ֵ���ź�������
#define WSTOPSIG(s) (((s)>>8)&0xFF)	// ���ص��½���ֹͣ���ź�ֵ��
#define WIFSIGNALED(s) (((unsigned int)(s)-1 & 0xFFFF) < 0xFF)	// �������δ��׽���ź�

__public int thread_mark_wait(thread_t *pthread,thread_t *pwait)
{
	if (!pthread)
	{
		return -1;
	}
	pthread->waitpid = 1;
	return 0;
}


//����-1,��ʾû�з���������id,0��ʾ�ҵ�,������ʾ�ɹ�
__local thread_t* wait_child_procs(const thread_t *pproc,int options)
{
	int i;
	int found=0;
	pid_t id=-1;
	thread_t *thread_target;

	ASSERT(pproc);

	LOCK_SCHED(thread_sock_sem);	

	LIST_FOREACH(thread_target,&boot_parameters.thread_list_head,tcb_list)
	{

		ASSERT (thread_target);

		if (thread_target == idle_task)
			continue;

		if (thread_target == daemon_task)
			continue;


		if (thread_target->ptid != pproc->tid)
		{
			continue;
		}

		thread_mark_wait(thread_target,NULL);

		found++;

		if (thread_target->state!=TS_DEAD){
			continue;
		}		
		UNLOCK_SCHED(thread_sock_sem);	
		return thread_target;		
	}

	UNLOCK_SCHED(thread_sock_sem);	

	if (found)
	{
	return (thread_t*)-1;
	}

	return NULL;
}




/*
**do_waitpid system call
	pid<-1 �ȴ�������ʶ����Ϊ pid ����ֵ���κ��ӽ��̡�
    pid=-1 �ȴ��κ��ӽ���,�൱�� wait()��            
    pid=0     �ȴ�������ʶ������Ŀǰ������ͬ���κ��ӽ��̡�       
    pid>0     �ȴ��κ��ӽ���ʶ����Ϊ pid ���ӽ��̡�

*/
pid_t do_waitpid(pid_t pid, unsigned long *state, int options)
{
	int i;
	thread_t *target_thread=NULL;
	thread_t *pthread;

	pthread = current_thread();

loopstart:

	//kprintf("do_waitpid begin %d..\n",pid);

	if (pid==-1){
		target_thread = wait_child_procs(pthread,options);		
	}
	else if(pid>0){
		target_thread = find_thread_byid(pid);

		if (target_thread)
		{
		//���
		thread_mark_wait(target_thread,pthread);
		}
	}
	else{
		//������
		kprintf("pid %d  on group???\n",pid);
		return ECHILD;
	}

	if (!target_thread){	
		trace("pid %d  on not exist???\n",pid);
		return ECHILD;
	}

		//�ȴ�
	if(target_thread ==-1&&options == WNOHANG){
		kprintf("pid %d  exit on flag???\n",pid);
		return 0;
	}

	if (target_thread==-1 || target_thread->state!=TS_DEAD){
		trace("thread_wait %d ..\n",pid);
		thread_wait(pthread, INFINITE);
		goto loopstart;
	}
	else 
	{
		int tid=target_thread->tid;

		ASSERT(THREAD_SPACE(target_thread)->p_asid>=INIT_PROC);

		pthread->sticks += target_thread->sticks;
		if(state){
			*state=target_thread->exit_code;
		}

		if (target_thread->state==TS_DEAD){
			//nop();
			//current_proc()->uticks += target_thread->uticks;	
		}
		else if (target_thread->state==TS_WAITING){
			pthread->sticks += target_thread->sticks;	
			//kprintf("do_waitpid(): task%d TS_WAITING\n",proc_number(current_proc()));
		}

		if (THREAD_SPACE(target_thread)!=THREAD_SPACE(pthread))
		{
		destory_proc_space(THREAD_SPACE(target_thread));		
		//kprintf("destory_child succ\n");
		}

		return tid;
	}


		trace("destory_child ECHILD\n");

	return ECHILD;
}


inline int  thread_find_father(thread_t *child)
{
	int i;
	thread_t *thread_target;

	ASSERT(child != NIL_THREAD);

	LOCK_SCHED(thread_sock_sem);	


	LIST_FOREACH(thread_target,&boot_parameters.thread_list_head,tcb_list)
	{
		ASSERT(thread_target);
		if (thread_target == idle_task)
		{
			continue;
		}

		//�ҵ��˸�����
		if (thread_target->tid == child->ptid){
			UNLOCK_SCHED(thread_sock_sem);	
			return thread_target;
		}
	}

	UNLOCK_SCHED(thread_sock_sem);	
	
	return NULL;
}


/*
** wakeup father process
*/
static int wakeup_father(thread_t *child)
{
	int i;
	thread_t *thread_target;

	ASSERT(child != NIL_THREAD);

	trace("wakeup_father called\n");

	thread_target = thread_find_father(child);

	if (thread_target)
	{
		//�򸸽��̷����˳��ź�
		sendsig((thread_target),SIGCHLD);
	}
	else{
		//����init����
		//��������ź�
		sendsig(( uinit_task), SIGCHLD);
	}

	trace("proc%s  no father(tid%d)!", child->name,  child->ptid);
	return 0;
}


/*
** do stop a task, when task receive a exec
*/
void do_stop_current_thread(int signo,  thread_t *pthread)
{
	int i;
	thread_t *child;

	ASSERT(pthread != NIL_THREAD);

	ASSERT(THREAD_SPACE(pthread)->magic == PROC_MAGIC);

	if (IS_USER_TASK(THREAD_SPACE(pthread)))
	{
		proc_exit(signo,pthread,signo+1);
	}
	else{
		kernel_thread_exit(pthread,NULL);
	}
	return;
}

/*
** do stop a task, when task receive a exec
*/
void do_exit_current_thread(int signo,  thread_t *pthread)
{
	int i;
	thread_t *child;

	ASSERT(pthread != NIL_THREAD);

	ASSERT(THREAD_SPACE(pthread)->magic == PROC_MAGIC);

	if (IS_USER_TASK(THREAD_SPACE(pthread)))
	{
		proc_exit(signo,pthread,-1);
	}
	else{
		kernel_thread_exit(pthread,NULL);
	}
	return;
}

/*
** task exit
*/
__public void proc_exit(int code,  thread_t *pthread, long segv_code)
{
	int i;
	thread_t *tmp_thread;
	int one_space=0;
	int nr = proc_number(pthread->plink);
 	unsigned cur_flag;

	ASSERT(pthread != NIL_THREAD);
	ASSERT(pthread);
	if(!IS_USER_TASK(nr)){
		panic("proc_exit() task %d,%d. error code:%d\n", nr, pthread->tid,code);
	}

	thread_unready(pthread,INFINITE);

	save_eflags(&cur_flag);	

	
	pthread->exit_code = (0);

	if (segv_code==-1){
		pthread->exit_code = code;
	}
	else if (segv_code)
	{
		pthread->exit_code |= ((segv_code-1) & 0177);
		pthread->exit_code |= WAIT_CORE;
	}
	else{
		pthread->exit_code = (code&0377) << 8;
		pthread->exit_code |= 0;
	}
	//pthread->exit_code |= WAIT_CORE;

	pthread->state = TS_DEAD;
	//put_as_id(pthread->plink->p_index);

	tmp_thread = thread_find_father(pthread);

	if (tmp_thread && THREAD_SPACE(tmp_thread)==THREAD_SPACE(pthread))
	{
		one_space=1;
	}

	

	restore_eflags(cur_flag);

	if (!one_space)
	{
		sendfsinfo(nr, nr, FS_EXIT);
	}

	if (!pthread->waitpid&&!one_space)
	{
		//�ͷſռ�
		//sendfsinfo(nr, nr, FS_EXIT);
		//kprintf(__FUNCTION__ "(): destory_proc_space 00 ..\n");
		destory_proc_space(THREAD_SPACE(pthread));	
		//kprintf(__FUNCTION__ "(): destory_proc_space 11 ..\n");
		set_cr3(KERN_PG_DIR_ADDR);
	}
	else{

		wakeup_father(pthread);
	}

	//reset current thread
	schedule();
	return;
}



/*
**send a signo to pid's process
*/
__public int killpid (int pid, int signo)
{
  int i, retval = -1;
  thread_t* pthread;

  LOCK_SCHED(thread_sock_sem);	
  
  //trace("killpid called\n");

  if (pid==0){
	  LIST_FOREACH(pthread,&boot_parameters.thread_list_head,tcb_list)
	  {
		  ASSERT(pthread);
		  if(pthread->ptid==pid){
			  retval=0;
			  sendsig(pthread,signo);
		  }
	  } 

  }
  else if (pid > 0){
	  LIST_FOREACH(pthread,&boot_parameters.thread_list_head,tcb_list)
	  {
		  ASSERT(pthread);
		  if(pthread->tid==pid){
			  //trace("kill thread %s with %d ok\n", pthread->name,signo);
			  sendsig(pthread,signo);
			  retval=0;
		  }
	  }
  }
  else if (pid == -1){ 
	  LIST_FOREACH(pthread,&boot_parameters.thread_list_head,tcb_list)
	  {
		  ASSERT(pthread);
		  if(pthread->tid!=pid){
			  sendsig(pthread,signo);
			  retval=0;
		  }
	  } 

  }

  UNLOCK_SCHED(thread_sock_sem);	
  return retval;
}

