

#include <amd64/kasmdef.h>

#define MASTER			0x20		//; I/O port for interrupt controller
#define MASTER_MASK  	0x21		//; setting bits in this port disables ints
#define SLAVE                   	0xA0		//; I/O port for second interrupt controller
#define SLAVE_MASK     	0xA1		//; setting bits in this port disables ints
#define ENABLE                	0x20		//; code used to re-enable after an interrupt

 .globl common_interrupt,ia32_sysenter_target,
 .globl  syscall_handler,system_call,ia32_cstar_target,unexpected_interrupt
  .globl    divide_error
 .globl   debug
 .globl   nmi
 .globl   int3
 .globl   overflow
 .globl    bounds
 .globl   invalid_op
 .globl   device_not_avl
 .globl   double_fault
 .globl   coprocessor_segment_overrun
 .globl     invalid_tss
 .globl   segment_not_present
 .globl   stack_exception
 .globl   general_protection
 .globl   copr_error
 .globl    page_fault
 .globl    coprocessor_error
 .globl    spurious_interrupt_bug
 .globl    alignment_check
 .globl    simd_coprocessor_error
  .globl 		hwint00		// Interrupt routine for irq 0 (the clock)
 .globl 		hwint01		// Interrupt routine for irq 1 (keyboard)
 .globl 		hwint02		// Interrupt routine for irq 2 (cascade)
 .globl 		hwint03		// Interrupt routine for irq 3 (second serial)
 .globl 		hwint04		// Interrupt routine for irq 4 (first serial)
 .globl 		hwint05		// Interrupt routine for irq 5 (XT winchester)
 .globl 		hwint06		// Interrupt routine for irq 6 (floppy)
 .globl 		hwint07		// Interrupt routine for irq 7 (printer)
 .globl 		hwint08		// Interrupt routine for irq 8 (realtime clock)
 .globl 		hwint09		// Interrupt routine for irq 9 (irq 2 redirected
 .globl 		hwint10		// Interrupt routine for irq 10
 .globl 		hwint11		// Interrupt routine for irq 11
 .globl 		hwint12		// Interrupt routine for irq 12
 .globl 		hwint13		// Interrupt routine for irq 13  FPU exception)
 .globl 		hwint14		// Interrupt routine for irq 14 (AT winchester)
 .globl 		hwint15		// Interrupt routine for irq 15
.extern main_ia32_syscall,do_exception, c_amd64_syscall
.extern system_call
.extern irq_table

.macro hwint_master nr
	ENTER_KERNEL

	movb	$ENABLE, %al
	movw       $MASTER, %dx
	outb	        %al, %dx

	pushq	\nr		
	movq	\nr, %rax
	call *irq_table(,%rax,8)  # XXX:	 rip relative
	addq	%rsp, 8

	LEAVE_KERNEL
	iret				
.endm

.macro hwint_slave nr
	ENTER_KERNEL

	movb	$ENABLE, %al
	movw       $MASTER, %dx
	outb	        %al, %dx
	jmp 1f
1:
	movw       $SLAVE, %dx
	outb	        %al, %dx

	pushq	\nr		
	movq	\nr, %rax
	call *irq_table(,%rax,8)  # XXX:	 rip relative
	popq	%rcx

	LEAVE_KERNEL
	iret				
.endm

hwint00:		// Interrupt routine for irq 0 (the clock)
	hwint_master 0

	
hwint01:		// Interrupt routine for irq 1 (keyboard)
	hwint_master 1

	
hwint02:		// Interrupt routine for irq 2 (cascade)
	hwint_master 2

	
hwint03:		// Interrupt routine for irq 3 (second serial)
	hwint_master 3

	
hwint04:		// Interrupt routine for irq 4 (first serial)
	hwint_master 4

	
hwint05:		// Interrupt routine for irq 5 (XT winchester)
	hwint_master 5

	
hwint06:		// Interrupt routine for irq 6 (floppy)
	hwint_master 6

	
hwint07:		// Interrupt routine for irq 7 (printer)
	hwint_master 7

//---------------------------------------------------------------------------------------------------
// Each of these entry points is an expansion of the hwint_slave macro
	
hwint08:		// Interrupt routine for irq 8 (realtime clock)
	hwint_slave 8

	
hwint09:		// Interrupt routine for irq 9 (irq 2 redirected
	hwint_slave 9

	
hwint10:		// Interrupt routine for irq 10
	hwint_slave 10

	
hwint11:		// Interrupt routine for irq 11
	hwint_slave 11

	
hwint12:		// Interrupt routine for irq 12
	hwint_slave 12

	
hwint13:		// Interrupt routine for irq 13  FPU exception)
	hwint_slave 13

	
hwint14:		// Interrupt routine for irq 14 (AT winchester)
	hwint_slave 14

	
hwint15:		// Interrupt routine for irq 15
	hwint_slave 15

#define PAGE_SIZE 4096

system_call1:
	# Switch to hidden gs	
	swapgs
	# %gs:0 now points to pointer to stack page
	mov %gs:0, %r10     # We have a ptr to stack page in r10
	addq $PAGE_SIZE-16, %r10 # We need some space to store old %sp
	
	movq %rsp, 0(%r10)  # Save old stack pointer to stack
	movq %r10, %rsp     # Change to new stack
	pushq %rcx          # Return address
	pushq %r11          # Save flags

	# Switch back to remain consistent
	swapgs 

	sti
	movq %r9, %rcx      # Exchange last parameter as a third
	
	call c_syscall_handler
	cli                 # We will be touching stack pointer
		
	popq %r11
	popq %rcx
	movq 0(%rsp), %rsp
	sysretq


ia32_sysenter_target:
     ENTER_KERNEL
    call	main_ia32_syscall
    LEAVE_KERNEL
    iret

system_call:
     ENTER_KERNEL
    call	c_amd64_syscall
    LEAVE_KERNEL
    iret



ia32_cstar_target:
     ENTER_KERNEL
    call	main_ia32_syscall
    LEAVE_KERNEL
    iret

divide_error:
	pushq	$0
	pushq	$0
	jmp	_exception
	
debug:
	pushq	$0
	pushq	$1
	jmp	_exception

nmi:
	pushq	$0
	pushq	$2
	jmp	_exception

int3:
	pushq	$0
	pushq	$3
	jmp	_exception

overflow:
	pushq	$0
	pushq	$4
	jmp	_exception


	
bounds:
	pushq	$0
	pushq	$5
	jmp	_exception


	
invalid_op:
	pushq	$0
	pushq	$6
	jmp	_exception
	
device_not_avl:
	pushq	$0
	pushq	$7
	jmp	_exception
	
double_fault:
	pushq	$8
	jmp	_exception
	
coprocessor_segment_overrun:
	pushq	$0
	pushq	$9
	jmp	_exception
	
invalid_tss:
	pushq	$10
	jmp	_exception

segment_not_present:
	pushq	$11
	jmp	_exception

stack_exception:
	pushq	$12
	jmp	_exception

general_protection:
	pushq	$13
	jmp	_exception

page_fault:
	pushq	$14
	jmp	_exception

copr_error:
	pushq	$0
	pushq	$16
	jmp	_exception

 

spurious_interrupt_bug:
	pushq	$0
	pushq	$15
	jmp	_exception

alignment_check:
	pushq	$0
	pushq	$17
	jmp	_exception

simd_coprocessor_error:
 	pushq	$0
	pushq	$19
	jmp	_exception

_exception:
    ENTER_KERNEL
    call	do_exception
    LEAVE_KERNEL
    iret

unexpected_interrupt:
     ENTER_KERNEL
    call	do_exception
    LEAVE_KERNEL
    iret


