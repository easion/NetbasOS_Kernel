
#include <jicama/process.h>
#include <assert.h>
//#include "s3c4510.h"


void arch_thread_free(thread_t* pthread)
{
	ASSERT(pthread != NIL_THREAD);
}


/* Control Bits */

#define IRQ_MASK	(1 << 7)	/* IRQ Disable Bit */
#define FIQ_MASK	(1 << 6)	/* FIQ Disable Bit */
#define THUMB_CODE	(1 << 5)	/* thumb code mode */
#define USER_MODE	(0x10)		/* USER Mode */
#define FIQ_MODE	(0x11)		/* FIQ Mode */
#define IRQ_MODE	(0x12)		/* IRQ Mode */
#define SVC_MODE	(0x13)		/* SVC Mode */
#define ABORT_MODE	(0x17)		/* ABORT Mode */
#define UNDEF_MODE	(0x1B)		/* UNDEF Mode */
#define SYSTEM_MODE	(0x1F)		/* SYSTEM Mode */

void switch_to_thread(thread_t* pthread, thread_t* current);

//进程切换
__noreturn void arch_thread_resume (thread_t *thread_from, 	thread_t *pthread)
{
	extern int sys_irq_nest;
	ASSERT(pthread);
	ASSERT(pthread->k_stack);

	if (!pthread->k_stack)	{
		kprintf("arch_thread_resume no kernel task \n");
		//return ;
	}

	//disable();
	if (!thread_from)	{
		thread_context_switch(NULL,(u32_t)&pthread->k_stack);
	}
	else if (sys_irq_nest == 0)	{
			kprintf("arch_thread_resume 0x%x @%s called \n", 
				*(u32_t*)(pthread->k_stack), pthread->name);
#ifndef USER_THREAD
			thread_context_save((u32_t)&thread_from->k_stack,
				(u32_t)&pthread->k_stack);
#else
	switch_to_thread(thread_from, pthread);
#endif
	}
	else	{
		kprintf("switch in interrupt\n");
		arch_context_switch_interrupt((u32_t)&thread_from->k_stack,
			(u32_t)&pthread->k_stack);
	}
	//enable();
}



void switch_to_initial_thread(thread_t* pthread)
{
    asm("  mov  %%sp, %0\n"
	"  ldmia %%sp!, {%%pc}\n"
	:
	: "r"(pthread->k_stack));
	
}


inline static void thread_stack_push(thread_t *pthread, u32_t value)
{
	(char*)pthread->k_stack -= sizeof(u32_t);
    *(u32_t*)(pthread->k_stack) = value;
}

void create_kernel_stack_frame(thread_t *pthread, 
	void* tentry,void* texit, void* param)
{
	int i;

	thread_stack_push(pthread, tentry); /* pc */
	thread_stack_push(pthread, texit); /* lr */
	for (i=0;i<12 ;i++ ){ /*r1-r12*/
		thread_stack_push(pthread, 0);
	}
	thread_stack_push(pthread, param); /* r0 : argument */
	thread_stack_push(pthread, SVC_MODE|0x40);/* cpsr IRQ, FIQ disable*/
	thread_stack_push(pthread, SVC_MODE|0x40);/* spsr IRQ, FIQ disable*/
}

void switch_to_thread(thread_t* pthread, thread_t* current)
{
   __asm__ __volatile__
	("\n\t/* switch_to_thread(start) */	\n"
	 "	stmdb	sp!, {lr}		\n"
	 "	adr	lr, 1f			\n"
	 "	str	lr, [sp, #-4]!		\n"
	 "	str	sp, [%0]		\n"
	 "	mov	sp, %1			\n"
	 "	ldr	pc, [sp], #4		\n"
	 "1:	ldmia	sp!, {lr}		\n\t"
	 "/* switch_to_thread(end) */		\n"
	 :
	 : "r"(&current->k_stack), "r"(pthread->k_stack)
	 : //"r0", "r1",
	 "r2", "r3", 
	 "r4", "r5", "r6", "r7", 
	 "r8", "r9", "r10", "r11",
	 "r12", "r13", "r14", "memory"
	    );
}


void create_user_stack_frame(thread_t *pthread, u32_t usp, 
u32_t uip, void* entry, void* param)
{
	//#define CONFIG_USERMODE_NOIRQ
    thread_stack_push(pthread, uip);           // user instruction pointer
    thread_stack_push(pthread, 0);		// kernel lr - dummy
    thread_stack_push(pthread, 0);		// user lr - dummy
    thread_stack_push(pthread, usp);    // user stack
#if defined(CONFIG_USERMODE_NOIRQ)
    thread_stack_push(pthread, (IRQ_MASK|FIQ_MASK|USER_MODE));//flags
#else
    thread_stack_push(pthread, (/*IRQ_MASK|FIQ_MASK|*/USER_MODE));// flags
#endif
    thread_stack_push(pthread, (u32_t) param);  // parameter
    thread_stack_push(pthread, (u32_t) entry);  // entry point in kernel
}


__public int  arch_thread_create(thread_t* th_new,
void (*fn)(void *),void (*exit)(void *), 
	void *args,  u32_t stack,  proc_t *rp)
{
	ASSERT(th_new);

	if (IS_USER_TASK(rp->p_index))
	{
		th_new->k_stack = 0;
		//create_user_stack_frame();
		return 0;
	}

	if (stack%4)
	{
		kprintf("%s @ stack %x  ...\n",
			"[arch_thread_create]", stack);
		//stack -= (stack%4);
	}

#ifdef USER_THREAD
	th_new->reg->content[0] = (unsigned long)th_new;//(&task[pid+1]);
	th_new->reg->content[1] = 0x5f;   	/*cpsr*/
	th_new->reg->content[2] = 0x100000-1024; /*usr/sys模式堆栈*/
	th_new->reg->content[3] = SVCMODE;	/*svc模式*/
	th_new->reg->content[18]= (unsigned long)fn;	/*pc*/
	char *sp = mm_malloc(4096);
	create_user_stack_frame(th_new,fn, sp+4096, sp,fn);

#else
	memset((void*)((char*)stack-PAGE_SIZE), '#', PAGE_SIZE);	
	th_new->k_stack = stack ;
	create_kernel_stack_frame(th_new,fn, kthread_completion, args);
#endif
	return 0;
}


__public int do_clone (thread_t *currentthread, thread_t *newthread,  regs_t *reg)
{
	return 0;
}




int arch_proc_init(proc_t *rp, int linux_style)
{
	ASSERT(rp);
	return 0;
}

