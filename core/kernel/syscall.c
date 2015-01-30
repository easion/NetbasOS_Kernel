/*
 **     (R)Jicama OS
**      system call
**     Copyright (C) 2003,2004 DengPingPing
*/

#include <jicama/system.h>
#include <jicama/process.h>
#include <jicama/paging.h>
#include <jicama/syscall.h>
#include <jicama/spin.h>
#include <jicama/utsname.h>
#include <assert.h>


#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>



u32_t _brk(unsigned brk_addr)
{
	thread_t *pthread = current_thread();
	proc_t *rp = THREAD_SPACE(pthread);
	//u_stack必须修改到进程空间
	unsigned long limit = main_thread(rp)->u_stack - 16384; //16k for esp used
	unsigned cur_flag;
	
	ASSERT(rp);
	ASSERT(rp->p_brk_code >= 0);

	save_eflags(&cur_flag);
	brk_addr =  pageup(brk_addr);

	if (brk_addr > rp->start_stack || brk_addr < rp->p_bss_code)
	{
		if(brk_addr){
			kprintf("_brk: no memory to brk_addr %p, rp->start_stack on %p-%p\n",
				brk_addr,rp->start_stack, rp->p_bss_code);
		}
		restore_eflags(cur_flag);
		return rp->p_brk_code;
	}

	if (brk_addr<rp->p_brk_code)
	{
		rp->p_brk_code = brk_addr;
		//kprintf("_brk(): error goto addr 0x%x\n", brk_addr);
		restore_eflags(cur_flag);
		return brk_addr;
	}

	if (brk_addr > rp->p_brk_code 
	   && brk_addr < limit){	
		rp->p_brk_code = brk_addr;	
	}

	//kprintf("_brk(): succ goto addr 0x%x\n", rp->p_brk_code);
	
	restore_eflags(cur_flag);
	return rp->p_brk_code;		// 返回进程当前的数据段结尾值。
}


u32_t _sbrk(size_t size)
{
	thread_t *pthread = current_thread();
	proc_t *rp = THREAD_SPACE(pthread);
	unsigned brk_addr = rp->p_brk_code+size;

	brk_addr = _brk(brk_addr);
	return brk_addr;
}

