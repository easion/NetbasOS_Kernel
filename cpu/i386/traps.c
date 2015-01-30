/*
**     (R)Jicama OS
**      ISR Handler
**     Copyright (C) 2003 DengPingPing
*/

#include <jicama/system.h>
#include <arch/x86/protect.h>
#include <arch/x86/traps.h>
#include <jicama/syscall.h>
int linux_kernel_init(void);

/*
** system call function
*/
__asmlink void sys_call(void);
int put_syscall(int nr, void(*fn)());
void linux_compliance_call (void);

/*
** exception handler function
*/
__asmlink void divide_error (void);
__asmlink void debug (void); 
__asmlink void nmi (void); 
__asmlink void xint3(void); 
__asmlink void overflow (void); 
__asmlink void bounds (void); //5

__asmlink void invalid_op (void); 
__asmlink void device_not_avl (void); 
__asmlink void double_fault (void); 
__asmlink void coprocessor_segment_overrun (void); 
__asmlink void invalid_tss (void); 
__asmlink void segment_not_present (void); 
__asmlink void stack_exception (void); 
__asmlink void general_protection (void); 
__asmlink void page_fault (void); 
__asmlink void copr_error (void); 
__asmlink void pagefault (void); 

void dos_kernel_init(void);


__local  void reserved(void)
{
	trace ("Error:This IDT is Reserved!\n");
	return;
}


void traps_init(void)
{
	int i;
	
	set_hw_trap_handler(0, divide_error);
	set_hw_trap_handler(1, debug);
	set_hw_trap_handler(2, nmi);
	set_hw_trap_handler(3, xint3);
	set_hw_trap_handler(4, overflow);
	set_hw_trap_handler(5, bounds);

	set_hw_trap_handler(6, invalid_op);
	set_hw_trap_handler(7, device_not_avl);
	set_hw_trap_handler(8, double_fault);
	set_hw_trap_handler(9, coprocessor_segment_overrun); //协处理器段超出限度
	set_hw_trap_handler(10, invalid_tss);
	set_hw_trap_handler(11, segment_not_present);
	set_hw_trap_handler(12, stack_exception); //堆栈故障
	set_hw_trap_handler(13, general_protection);	
	set_hw_trap_handler(14, pagefault);
	set_hw_trap_handler(15, copr_error); //协处理器故障
    
	for(i = 16; i < 32; i++)
		set_hw_trap_handler(i, reserved);

	dos_kernel_init();
	linux_kernel_init();

	/*syscall*/
	put_syscall(SYSVEC, sys_call );
	/*Compliance with unix*/ 
	put_syscall(UNIXVEC, linux_compliance_call);

	return;
}

int put_syscall(int nr, void(*fn)())
{
	if (nr<0||nr>255)
		return -1;
	set_syscall_gate(nr, fn); 
	return 0;
}

int put_interrupt(int nr, void(*fn)())
{
	if (nr<0||nr>255)
		return -1;
	set_interrupt_gate(nr, fn); 
	return 0;
}


