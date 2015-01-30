/*
**     (R)Jicama OS
**      ISR Handler
**     Copyright (C) 2003 DengPingPing
*/

#include <jicama/system.h>
#include <jicama/process.h>

#define FPU_CONTEXT_SIZE 108

#define TSS_USED		0x8000
#define FPU_USED                0x4000

/* Clear Task Switched Flag! Used for FPU preemtion */
__public void clts(void)
{
	asm volatile ("clts"); 
}

/*
    FPU context switch management functions!
    FPU management exported at kernel layer to allow the use
    of floating point in kernel primitives; this turns to be
    useful for bandwidth reservation or guarantee!
*/

/* FPU lazy state save handling.. */
__public void save_fpu_state(void *t)
{
	asm volatile (	"fnsave (%%eax)\n"
			"fwait\n"
			:
			: "a" ((unsigned long)(t))
			: "memory" );
}

/*this function works well*/
__public void load_fpu_state(void *t)
{
   	asm volatile (	"frstor (%%eax)\n"
				"fwait\n"
				:
				: "a" ((unsigned long)(t))
				: "memory" );
}

__public void reset_fpu(void)
{ 
	asm volatile ("finit");
}

__public void init_fpu(void)
{
    asm volatile ("movl %cr0,%eax");
    asm volatile ("orl  $34,%eax");
    asm volatile ("movl %eax,%cr0");
    asm volatile ("finit");
}


