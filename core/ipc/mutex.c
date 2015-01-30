/*
**     (R)Jicama OS
**     messaging subsystem
**     Copyright (C) 2005 DengPingPing
*/
#include <jicama/process.h>
#include <jicama/trace.h>
#include <jicama/spin.h>
#include <jicama/msgport.h>
//#include <string.h>
//#include <errno.h>
#include <assert.h>

#if 0
/*
**信号量初始化函数
*/
__public void sem_init(struct thread_sem *s, int cnt)
{	
	int i;

	ASSERT(s != NULL);

	//有多少可用的资源
	s->count = cnt;	
	thread_waitq_init(&s->threadq);
}

/*
**
*/
void *sem_new( int cnt)
{
	struct thread_sem *s = (struct thread_sem *)kcalloc(sizeof(struct thread_sem));
	if (s == NULL)
	{
		kprintf("sem_new: no memory\n");
		return NULL;
	}
	s->spin.lock = SPINLOCK_UNLOCKED;
	sem_init(s,cnt);
	return s;
}

/*
**
*/
void sem_free(struct thread_sem *s)
{
	ASSERT(s != NULL);
	mm_free(s, sizeof(struct thread_sem));
}

/*
**进入的时候，减可用资源少
*/
int sem_down(struct thread_sem *s, time_t timeout)
{
	unsigned t = timeout;
	int ret = semaphore_down(&s->count);

	ASSERT(s != NULL);

	if (ret<0)
	{
		t = thread_sleep_on_timeout(&s->threadq, timeout);
		if (t<0)
		{
			semaphore_up(&s->count);
		}
	}
	else{
		t = 0;
		//kprintf("oops(%d) down  not sleep ...\n",ret);
	}

	return t;
}

/*
**离开的时候，增加可用资源
*/
int sem_up(struct thread_sem *s)
{
	int i;
	unsigned cur_flag;

	save_eflags(&cur_flag);	

	ASSERT(s != NULL);

	semaphore_up(&s->count);
	thread_wakeup(&s->threadq);	
	restore_eflags(cur_flag);
	return 0;
}

#endif


