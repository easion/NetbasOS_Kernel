
#define START_INFO_SIZE	1104
#define BOOT_OFFSET		0x0000
#define PAGE_SIZE		4096
#define TEMP_STACK_SIZE 0x1000

.section __xen_guest
	.ascii "GUEST_OS=NetbasOS,"
	.ascii "XEN_VER=xen-3.0,"
	.ascii "HYPERCALL_PAGE=0x0000,"
	.ascii "LOADER=generic,"
	.ascii "FEATURES=writable_page_tables"
	.byte   0

.text

.code32
.align 4
.global _start
	
_start:
	# copy start_info (esi initialized by Xen)
	
	movl $start_info, %edi
	movl $START_INFO_SIZE >> 2, %ecx
	cld
	rep movsb
	
	# switch to temporal kernel stack
	
	movl $kernel_stack, %esp
	
	call arch_pre_main
	call start_kernel								# never returns

	cli
	hlt
.text

.global xen_callback
.global xen_failsafe_callback
.global enable_l_apic_in_msr
.global memcpy
.global memcpy_from_uspace
.global memcpy_from_uspace_failover_address
.global memcpy_to_uspace
.global memcpy_to_uspace_failover_address


xen_callback:
	iret

xen_failsafe_callback:
	iret


kernel_stack_bottom:
	.space TEMP_STACK_SIZE
kernel_stack:

.section K_TEXT_START, "aw", @progbits
.global hypercall_page
.org 0
hypercall_page:
	.space PAGE_SIZE

.global shared_info
.org 0x1000
shared_info:
	.space PAGE_SIZE

.global console_page
.org 0x2000
console_page:
	.space PAGE_SIZE
