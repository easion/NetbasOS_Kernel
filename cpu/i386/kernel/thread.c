
// --------------------------------------------------------------------------------
//Jicama Operating System
// Copyright (C) 2003  DengPingPing      All rights reserved.  
//---------------------------------------------------------------------------------

#include <jicama/system.h>
#include <jicama/paging.h>
#include <jicama/process.h>
#include <string.h>
#include <assert.h>

#define ARGS_OFFSET 8

__asmlink int thread_tss_init(thread_t* pthread, unsigned entry, unsigned stack, proc_t *rp);

/*
**
*/
int SysCall( set_thread_name)(regs_t *reg)
{
	int tid = reg->ecx;
	char *name = (char *)current_proc_vir2phys(reg->ebx);
   
    return set_thread_name( tid,name);
}

/*
**
*/
int SysCall( set_thread_priority)(regs_t *reg)
{
   int tid = reg->ebx;
   int level = reg->ecx;

    set_thread_priority( tid, level);
    return( 0 );
}

int SysCall( thread_join)(regs_t *reg)
{
	tid_t tid = reg->ebx;
	void* ret = current_proc_vir2phys(reg->ecx);

	wait_for_thread(tid,ret);    
    return( 0 );
}

int SysCall( thread_get_info)(regs_t *reg)
{
	tid_t tid = reg->ebx;
	void* ret = current_proc_vir2phys(reg->ecx);

	thread_get_info(tid,ret);    
    return( 0 );
}

int SysCall( thread_yield)(regs_t *reg)
{
	thread_yield();    
    return( 0 );
}

/*
**
*/
int SysCall( thread_cannel)(regs_t *reg)
{
	int thid=reg->ebx ;
	void*ret=(reg->ecx) ;

	thread_exit_byid(thid, ret);
    return( 0 );
}

int SysCall( thread_suspend)(regs_t *reg)
{
	int tid=reg->ebx;
	time_t timeout=reg->ecx;
	thread_t *pthread = find_thread_byid(tid);

	if (!pthread)
	{
		return -1;
	}
	//kprintf("%s: called\n",__FUNCTION__);

	thread_wait(pthread,timeout);
	return 0;
}

int SysCall( thread_resume)(regs_t *reg)
{
	int tid=reg->ebx;
	thread_t *pthread = find_thread_byid(tid);

	if (!pthread)
	{
		return -1;
	}

	//kprintf("%s: called\n",__FUNCTION__);

	thread_ready(pthread);
	return 0;
}

/*
**SysCall( thread_new
*/
int SysCall( thread_new)(regs_t *reg)
{
	thread_t *pcurrent = current_thread();
	proc_t *rp = THREAD_SPACE(pcurrent);
	unsigned args=reg->ebx;
	unsigned s=reg->ecx; //堆栈
	size_t s_size=reg->esi; //堆栈
	void* fn =(void*)reg->edx;  //function entry
	void* thread_exitptr=(void*)reg->edi;

	if (fn == 0 || s == 0){
		kprintf("warn: thread entry or stack is zero\n");
		return -1;
	}

	s=s+s_size-4096;

   if(!IS_USER_TASK(proc_number(rp))) {
		return -1;
	}

	thread_t *pthread = thread_new((void*)fn,thread_exitptr, (void*)args,s,  rp);
	if(!pthread){
	syslog(3, "thread_new() error args=0x%x, s=%x, fn=0x%x,  tcb=%d\n", 
		args, s, fn,  current_proc()->ntcb);
		return -1;
	}

	snprintf(pthread->name, OS_NAME_LENGTH,"%s-%d", "usertask", pthread->tid);

	//设置当前进程为父进程
	 pthread->ptid = pcurrent->tid;


	//thread_ready(pthread);
	return pthread->tid;
}

unsigned save_pagedir(proc_t *rp)
{
	pte_t cr3= get_cr3();
	set_cr3(rp->proc_cr3);
	return cr3;
}

unsigned restore_pagedir(pte_t cr3)
{
	set_cr3(cr3);
}

/*
**arch_thread_create()
*/
__public int  arch_thread_create(thread_t* th_new,
void (*fn)(void *), void (*exit)(void *), 
void *args,  unsigned stack,  proc_t *rp)
{
	int sig_hder;
	thread_t *th_tmp;
	int sc;
	unsigned kthread_stack = proc_phy_addr(rp);
	pte_t cr3= 0;

	ASSERT(rp !=NULL);
	ASSERT(th_new !=NULL);

  
   /*
    * thread's machine state ( registers and stuff )
    */ 
	if(thread_tss_init(th_new,fn, stack, rp)<0)
		return NULL;

	for (sig_hder = 0; sig_hder < NSIG_PROC; sig_hder++)
         bzero((u8_t *)&th_new->sigaction[sig_hder], sizeof(struct sigaction)); 
	cr3 = save_pagedir(rp);

	//程序堆栈位置
	kthread_stack += (th_new->u_stack-ARGS_OFFSET);


	if (!IS_USER_TASK(proc_number(rp)) )
	{
		//保存传递给线程的参数
		poke32(kthread_stack, (unsigned)kthread_completion);
	}
	else{
		poke32(kthread_stack, (unsigned)exit);

		//strcpy((char*)0x80002000,"test");
	//trace ("arch_thread_create kthread_stack = %x %d,%s...\n",kthread_stack,rp->p_asid,(char*)0x80002000);
	}

	poke32(kthread_stack+4, args);

	restore_pagedir(cr3);


	return(0);
}

