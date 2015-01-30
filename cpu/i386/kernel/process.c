
// --------------------------------------------------------------------------------
//Jicama Operating System
// Copyright (C) 2003  DengPingPing      All rights reserved.  
//---------------------------------------------------------------------------------

#include <jicama/system.h>
#include <jicama/paging.h>
#include <jicama/process.h>
#include <string.h>
#include <assert.h>

/*process entry point*/

__asmlink void tty_server();
/*
u32_t proc_vis_addr(proc_t *rp, size_t pos)
{
	//proc_t *rp;
	register u32_t base;

	//if (!IS_USER_TASK(nr)){return 0;}

	//rp = proc[nr];
	base = proc_phy_addr(rp);
	base+=pos;

	return base;
}
*/

void* current_proc_vir2phys(void* pos)
{
	proc_t *rp = current_proc();

	if (!pos)
	{
		//¿ÕÖ¸Õë
		return NULL;
	}

	ASSERT(rp);
	return (void*)proc_vir2phys(rp, (size_t)pos);
}

#define OUT kprintf

void dump_task (proc_t *drp, thread_t *pthread)
{
	struct tss_struct* tss=&pthread->tss;

	OUT("\nTask %d TSS Debug:",proc_number(drp)); 
	OUT("Eip is %x  CS is %x",(u32_t)tss->eip,(u32_t)tss->cs); 
	OUT("SS is %x \t",(u32_t)tss->ss);
	OUT("Esp is %x ",tss->esp);
	OUT("Cr3 is %x ",(u32_t)tss->cr3);
	OUT("Eflags is %x \t",(u32_t)tss->eflags);
	OUT("SS0 is %x ",(u32_t)tss->ss0);
	OUT("sp0 is %x ",(u32_t)tss->sp0);
	OUT("DS is %x ",(u32_t)tss->ds);
	OUT("ES is %x ",(u32_t) tss->es);
	OUT("EAX is %x ",(u32_t) tss->eax);
	OUT("EBX is %x ",(u32_t) tss->ebx);
	OUT("ECX is %x \t",(u32_t) tss->ecx);
	OUT("EDX is %x ",(u32_t) tss->edx);
	OUT("ldt selector %x ",(u32_t) drp->ldt_sel);
	OUT("ldt base %x \n",(u32_t)get_base((unsigned short)drp->ldt_sel));
}
#undef OUT
