
// ---------------------------------------------------------------------------------
//Jicama Operating System
// Copyright (C) 2003  DengPingPing      All rights reserved.  
//------------------------------------------------------------------------------------

#include <type.h>
#include <arch/x86/io.h>
#include <jicama/system.h>
#include <jicama/paging.h>
#include <jicama/process.h>
#include <jicama/syscall.h>
#include <string.h>
#include <assert.h>

__asmlink int build_user_ldt_space(proc_t *new_rp);

/*
** 
*/

/*
** 
*/
__public void fill_tss(struct tss_struct *ptss, regs_t *_reg, pte_t *cr3)
{
	ptss->backlink = 0;
	//ptss->sp0 = kstack;
	ptss->ss0 = 0x10; //for kernel use.
	ptss->cr3 = cr3; //;
	ptss->eip = _reg->eip;
	ptss->eflags = _reg->eflags;
	ptss->eax = 0;  /*return 0, if entryed new task.*/
	ptss->ecx = _reg->ecx;
	ptss->edx = _reg->edx;
	ptss->ebx = _reg->ebx;
	ptss->esp = _reg->user_esp;
	ptss->ebp = _reg->ebp;
	ptss->esi = _reg->esi;
	ptss->edi = _reg->edi;
	ptss->es = _reg->es & 0xffff;
	ptss->cs = _reg->cs & 0xffff;
	ptss->ss = _reg->user_ss & 0xffff;
	ptss->ds = _reg->ds & 0xffff;
	ptss->fs = _reg->fs & 0xffff;
	ptss->gs = _reg->gs & 0xffff;
}

/*
** 
*/
__public void save_tss(regs_t *_reg, struct tss_struct *ptss)
{
	_reg->eip=ptss->eip  ;
	_reg->eflags=ptss->eflags ;
	_reg->eax=ptss->eax ;  /*return 0, if entryed new task.*/
	_reg->ecx=ptss->ecx ;
	_reg->edx=ptss->edx ;
	_reg->ebx=ptss->ebx ;
	_reg->user_esp=ptss->esp;
	_reg->ebp=ptss->ebp;
	_reg->esi=ptss->esi;
	_reg->edi=ptss->edi ;
	_reg->es=ptss->es & 0xffff;
	_reg->cs=ptss->cs & 0xffff;
	_reg->user_ss=ptss->ss & 0xffff;
	_reg->ds =ptss->ds& 0xffff;
	_reg->fs=ptss->fs & 0xffff;
	_reg->gs =ptss->gs& 0xffff;
}


/*
** 
*/
__public int do_clone (const thread_t *currentthread, thread_t *newthread,  regs_t *reg)
{
	proc_t *new_rp = newthread->plink;
	proc_t *current_task = currentthread->plink;
	int nr = new_rp->p_asid;

	ASSERT(currentthread);
	ASSERT(reg);

	if (!IS_USER_TASK(nr))
	{
		kprintf("fork() Not from user Task ?\n");
	fork_err: 
	    kprintf ("do_clone(): FAILED");
		return EAGAIN;
	}

	fill_tss(&newthread->tss, reg,new_rp->proc_cr3); 
	sigclear(newthread);

	memcpy(newthread->debug_regs,currentthread->debug_regs,sizeof(currentthread->debug_regs));

	 if (new_rp == currentthread->plink)
	 {
		 //在同一空间的克隆
		 return currentthread->tid;
	 }

	//new_rp->prev = NIL_PROC;
	//new_rp->next = NIL_PROC;

	//pname_strcpy((char *)new_rp->p_name, (char *)current_task->p_name);
	pname_strcpy((char *)newthread->name, (char *)currentthread->name);
	new_rp->p_bss_code = current_task->p_bss_code;
	new_rp->p_brk_code = current_task->p_brk_code;
	new_rp->start_stack = current_task->start_stack;
    newthread->sticks= 0; /*run time count*/

	/*set uid*/
	new_rp->uid = current_task->uid;
	newthread->ptid = currentthread->tid;

	//current thread
	// new_rp->ntcb= current_task->ntcb; /*ntcb count*/

	sendfsinfo(proc_number(current_task), nr, FS_FORK);

	if(copy_proc_code (current_task, new_rp) != OK){
		kprintf("copy_proc_code error ..\n");
		goto fork_err;
	}


 	return newthread->tid;
}

