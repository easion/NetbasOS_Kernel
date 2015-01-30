#include <jicama/system.h>
#include <jicama/process.h>


void set_debug_regs( long regs[8] )
{
	__asm("movl %0, %%eax\n\t"
	"movl	%%eax, %%db0\n\t"
	"movl %1, %%eax\n\t"
	"movl	%%eax, %%db1\n\t"
	"movl %2, %%eax\n\t"
	"movl	%%eax, %%db2\n\t"
	"movl %3, %%eax\n\t"
	"movl	%%eax, %%db3\n\t"
	"movl %4, %%eax\n\t"
	"movl	%%eax, %%db7\n\t"
	::"r"(regs[0]), "r"(regs[1]), "r"(regs[2]), "r"(regs[3]), "r"(regs[7]));
}

long reset_db7_status()
{
	long status=0;

	__asm("movl %%db7, %0\n\t"
	"movl	%1, %%db7\n\t"
	:"=r"(status):"a"(0));
	return status;
}

long reset_db6_status(void)
{
	long status=0;

	__asm("movl %%db6, %0\n\t"
	"movl	%1, %%db6\n\t"
	:"=r"(status):"a"(0));
	return status;
}



