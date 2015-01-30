
#include <jicama/system.h>
#include <jicama/process.h>
#include <amd64/dep.h>
#include "kernel/kernel.h"
#include "kernel/x86_64.h"

void amd64_traps_init();

void cpu_init();
void libtest();
void enable();
void	_modf();
unsigned int	iopl(unsigned int lev);
extern struct desc_ptr cpu_gdt_descr[];

void arch_init2()
{
	int a,b=4;
	//a=4+b/0;
	//enable();
	//	iopl(1);

	kprintf("enable()...ok\n");	

	stop();

}

void arch_init3()
{
}

typedef unsigned long long sysarg_t;


extern sysarg_t __syscall(const sysarg_t p1, const sysarg_t p2, 
			  const sysarg_t p3, const sysarg_t p4,
			  const int id);

#define __SYSCALL0(id) __syscall(0, 0, 0, 0, id)
#define __SYSCALL1(id, p1) __syscall(p1, 0, 0, 0, id)
#define __SYSCALL2(id, p1, p2) __syscall(p1, p2, 0, 0, id)
#define __SYSCALL3(id, p1, p2, p3) __syscall(p1,p2,p3, 0, id)
#define __SYSCALL4(id, p1, p2, p3, p4) __syscall(p1,p2,p3,p4,id)

void syscall_test()
{
//iopl(1);
	__SYSCALL0(5);
}


int system_init()
{
	u64_t addr=1;
	 __asm("movq %%cr3, %0" :"=r"(addr));
	//asm volatile("sgdt %0" :: "m" (cpu_gdt_descr[0]));
	//asm volatile("lgdt %0" :: "m" (cpu_gdt_descr[0]));
	kprintf("loading amd64 ok(cr3:0x%x) ...\n", addr);
	
	//stop();
	return 1;
}

void org_puts(char *str)
{
	int i;
		 char *p=(char*)0xb8000;

		 for (i=0; str[i];i++ )
		 {
			*(p+i)=str[i];
		 }

}

static void clean_IOPL_NT_flags(void)
{
	asm
	(
		"pushfq;"
		"pop %%rax;"
		"and $~(0x7000),%%rax;"
		"pushq %%rax;"
		"popfq;"
		:
		:
		:"%rax"
	);
}

/** Disable alignment check
 *
 * Clean AM(18) flag in CR0 register 
 */
static void clean_AM_flag(void)
{
	asm
	(
		"mov %%cr0,%%rax;"
		"and $~(0x40000),%%rax;"
		"mov %%rax,%%cr0;"
		:
		:
		:"%rax"
	);
}



/** Load TR from descriptor table.
 *
 * @param sel Selector specifying descriptor of TSS segment.
 */
static inline void tr_load(u16_t sel)
{
	__asm__ volatile ("ltr %0" : : "r" (sel));
}

__public int _amd64_start_kernel(u64_t pLoad)
{
	int i=0;
	u64_t *ap=(u64_t*)pLoad;


/* Enable No-execute pages */
	set_efer_flag(11);

	dprintf("load gdt\n");

	load_gdt();
	{
	 char *p=(char*)0xb8000;

	 *p='E';
	 *(p+2)='X';
	 *(p+4)='C';
	}


	//traps_init();

      /* Disable I/O on nonprivileged levels
	 * clear the NT(nested-thread) flag 
	 */
	clean_IOPL_NT_flags();
	/* Disable alignment check */
	clean_AM_flag();


	return core_start(pLoad);
}

unsigned int	iopl(unsigned int lev)
{
   int return_value;
    __asm__ volatile ("int $0x80" \
        : "=a" (return_value) \
        : "0" ((long)(102)),"b" ((long)(lev))); \
    return return_value;
}

 
