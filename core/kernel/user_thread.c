/*
**     (R)Jicama OS
**     Simple Thread Support
**     Copyright (C) 2003 DengPingPing
*/
#include <jicama/system.h>
#include <jicama/process.h>
#include <jicama/spin.h>
#include <assert.h>
#include <string.h>
#include <jicama/timer.h>
#include <jicama/msgport.h>
static sem_t thread_sock_sem;

struct user_semaphore
{
	int flags;
	char name[256];
	thread_wait_t wait_queue;
	int id;
	int counter;
	LIST_ENTRY(user_semaphore) entries;      /* List. */
};

static int last_semaphore_id;
volatile static LIST_HEAD(, user_semaphore) g_sem_list;

void user_thread_init(void)
{
	LIST_INIT(&g_sem_list);
	last_semaphore_id=0x36;
}

static inline struct user_semaphore* find_semaphore_by_id(int semaphore)
{
	struct user_semaphore		*item;

	LOCK_SCHED(thread_sock_sem);	

	LIST_FOREACH (item,&g_sem_list, entries)
	{
		if (item->id == semaphore)
		{
			UNLOCK_SCHED(thread_sock_sem);	
			return item;
		}
	}	

	UNLOCK_SCHED(thread_sock_sem);	
	return NULL;
}

int destroy_semaphore(void* para)
{
	struct user_semaphore *item = para;
	if (!item)
	{
		return -1;
	}

	thread_wakeup_all(&item->wait_queue,-1);

	kfree(item);
	return 0;
}

int destroy_sem(int semaphore)
{
	struct user_semaphore *item=find_semaphore_by_id(semaphore);
	if (!item)
	{
		return ENOENT;
	}
	LIST_REMOVE(item,entries);
	return destroy_semaphore(item);
}


void * create_semaphore(const char *name, int flags,int init_val)
{
	struct user_semaphore *semaphore;

	semaphore = kcalloc(sizeof(struct user_semaphore));

	if (!semaphore)
	{
		return NULL;
	}

	//kprintf("create_semaphore :init_val %d\n",init_val);

	semaphore->id = last_semaphore_id++;
	semaphore->flags = flags;
	semaphore->counter = init_val;
	strncpy(semaphore->name,name,256);
	thread_waitq_init(&semaphore->wait_queue);


	return semaphore;
}

int create_sem(const char *name, int flags,int init_val)
{
	struct user_semaphore *semaphore;

	semaphore = create_semaphore(name,flags,init_val);
	if (!semaphore)
	{
		return ENOMEM;
	}

	LOCK_SCHED(thread_sock_sem);	

	LIST_INSERT_HEAD(&g_sem_list,semaphore, entries);
	UNLOCK_SCHED(thread_sock_sem);	

	return semaphore->id;
}

#if 0
struct sleep_timer
{
	thread_wait_t *threadq;
	struct timer kt_test;
	int timeout;
	//int runtimes;
};

void sem_lock_timer(void *arg)
{
	struct sleep_timer *stime=arg;

	/*if (stime->runtimes)
	{
	restart_timer(&kt_test, stime->timeout);
	stime->runtimes++;
	}
	else*/if(!stime->timeout){
		stime->timeout=1;
		thread_wakeup(stime->threadq);
		//remove_timer(&stime->kt_test);
	//kprintf("sem_lock_timer timeout...\n");
	}else{
	kprintf("sem_lock_timer restart error...\n");
	}
}
#endif


int lock_semaphore_timeout(void *para,time_t timeout)
{
	int err;
	struct user_semaphore *item=para;

	if (timeout==INFINITE)
	{
		return lock_semaphore(item);
	}


	if (!item)
	{
		kprintf("find_semaphore_by_id error\n");
		return -1;
	}

	KATOMIC_DEC(item->counter,int);
	if (KATOMIC_READ(item->counter,int)>=0)
	{
		//kprintf("find_semaphore_by_id error\n");
		return 0;
	}

	//memset(&lock_timer,0,sizeof(struct sleep_timer));

	//lock_timer.threadq = &item->wait_queue;

	//init_timer(&lock_timer.kt_test,sem_lock_timer,(void*)&lock_timer );
	//install_timer(&lock_timer.kt_test,timeout);

	err = thread_sleep_on_timeout(&item->wait_queue, timeout);
	//±»»½ÐÑ£¬É±ËÀ¼ÆÊ±Æ÷
	//remove_timer(&lock_timer.kt_test);
	if (err)
	{	
		KATOMIC_INC(item->counter,int);
		return -1;//timeout;
	}	

	return 0;
}

int lock_sem_timeout(int semaphore,time_t timeout)
{
	int err;
	//struct sleep_timer lock_timer;
	struct user_semaphore *item;
	item=find_semaphore_by_id(semaphore);
	
	return lock_semaphore_timeout(item,timeout);
}

int trylock_semaphore(void *para)
{
	struct user_semaphore *item=para;

	if (!item)
	{
		return ENOENT;
	}

	if (KATOMIC_READ(item->counter,int)<=0)
	{
		//kprintf("lock_semaphore: sleep %d\n",item->counter);
	return -1;
	}

	KATOMIC_DEC(item->counter,int);

	return 0;
}


int lock_semaphore(void *para)
{
	struct user_semaphore *item=para;

	if (!item)
	{
		return ENOENT;
	}

	if (KATOMIC_READ(item->counter,int)<=0)
	{
		//kprintf("lock_semaphore: sleep %d\n",item->counter);
	thread_sleep_on(&item->wait_queue);
	}

	KATOMIC_DEC(item->counter,int);

	return 0;

}


int lock_sem(int semaphore)
{
	struct user_semaphore *item=find_semaphore_by_id(semaphore);	
	unsigned cur_flag;

	if (!item)
	{
	kprintf("%s: %d called error\n",__FUNCTION__,semaphore);
		return EINVAL;
	}

	return lock_semaphore(item);
}


int unlock_semaphore(void*para)
{
	unsigned cur_flag;

	struct user_semaphore *item=para;

	save_eflags(&cur_flag);	

	thread_wakeup(&item->wait_queue);
	KATOMIC_INC(item->counter,int);
	restore_eflags(cur_flag);
	return 0;
}

int unlock_sem(int semaphore)
{
	struct user_semaphore *item=find_semaphore_by_id(semaphore);	
	unsigned cur_flag;

	if (!item)
	{
	kprintf("%s: %d called error\n",__FUNCTION__,semaphore);
		return EINVAL;
	}

	return unlock_semaphore(item);
}

int unlock_semaphore_ex(void *para,int max)
{
	struct user_semaphore *item=para;
	thread_wakeup_all(&item->wait_queue,max);
	KATOMIC_INC(item->counter,int);
	return 0;
}

int unlock_sem_ex(int semaphore,int max)
{
	struct user_semaphore *item=find_semaphore_by_id(semaphore);	

	if (!item)
	{
	kprintf("%s: %d called error\n",__FUNCTION__,semaphore);
		return EINVAL;
	}
	return unlock_semaphore_ex(item,max);
}

void thread_yield(void)
{
	schedule();
}

int wait_for_thread(int tid,void *ret)
{
	do_waitpid(tid,ret,0);
	return 0;
}


int SysCall( thread_create_semaphore)(regs_t *reg)
{
	void* name = current_proc_vir2phys(reg->ebx);
	int flags = reg->ecx;
	int initval = reg->edx;
	return create_sem(name,flags,initval);
}

int SysCall( thread_delete_semaphore)(regs_t *reg)
{
	int semid=reg->ebx;
	return destroy_sem(semid);
}

int SysCall( thread_lock_semaphore_x)(regs_t *reg)
{
	int semid=reg->ebx;
	time_t timeout=reg->ecx;
	return lock_sem(semid);
}

int SysCall( thread_unlock_semaphore_x)(regs_t *reg)
{
	int semid=reg->ebx;
	return unlock_sem(semid);
}

