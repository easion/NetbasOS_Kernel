
#include <jicama/process.h>

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

void enable()
{
	__asm__(			
	"ldmia	sp!,	{r0}\n"
	"tst	r0,	#0x80\n"
	"mrs	r0,	cpsr\n"
	"biceq	r0, r0, 	#0x80\n"
	"msr	cpsr_c,	r0\n"
	:::"r0"
	);
}

void disable()
{
	__asm__(			
	"mrs	r0,	cpsr\n"
	"stmdb	sp!,	{r0}\n"
	"orr	r0, r0, 	#0x80\n"
	"msr	cpsr_c,	r0\n"
	:::"r0"
	);
}

unsigned save_eflags(unsigned *flags)
{
    register unsigned foo=0;
	disable();
	return foo;    
}

void restore_eflags(unsigned flags)
{
	enable();
	return ;   
}

void nopcode(void)
{
	asm("nop");
}



void halt()
{
	kprintf("[ARM] halt called");
	do
	{
		/**/
	}
	while (1);
}

u32_t cur_addr_insystem(size_t addr)
{	
	return addr;
}


inline void system_sleep()
{
    /* we have no sleep, haven't we? */
    register u32_t foo, foo2;
    __asm__ __volatile__ (
	"/* enable_interrupts(start) */	\n"
	"	mrs	%0, cpsr	\n"
	"	mov	%1, %2		\n"
	"	msr	cpsr, %1	\n"
	"	nop			\n"
	"	nop			\n"
	"	nop			\n"
	"	nop			\n"
	"	nop			\n"
	"	nop			\n"
	"	nop			\n"
	"	nop			\n"
	"	nop			\n"
	"	nop			\n"
	"	nop			\n"
	"	nop			\n"
	"	nop			\n"
	"	nop			\n"
	"	nop			\n"
	"	msr	cpsr, %0	\n"
	: "=r" (foo), "=r" (foo2)
	: "i" (SVC_MODE));
}

