#include <amd64/kasmdef.h>

.globl    start
.globl    idt_table64
.globl	initial_code
.globl init_rsp

.global rdtsc
.global read_efer_flag
.global set_efer_flag

 

.section K_TEXT_START, "ax"


.code64



rdtsc:
	xorq %rax,%rax
	rdtsc
	ret

set_efer_flag:
	movq $0xc0000080, %rcx
	rdmsr
	btsl %edi, %eax
	wrmsr
	ret
	
read_efer_flag:	
	movq $0xc0000080, %rcx
	rdmsr
	ret 		


.globl __syscall

	
__syscall:
	# rcx gets rewritten on syscall
	movq %rcx, %r9
	syscall
	ret
	
//.end __syscall


//#define GDT_SIZE (16 * 8)
#define PTL_NO_EXEC       (1<<63)
#define PTL_ACCESSED      (1<<5)
#define PTL_CACHE_DISABLE (1<<4)
#define PTL_CACHE_THROUGH (1<<3)
#define PTL_USER          (1<<2)
#define PTL_WRITABLE      (1<<1)
#define PTL_PRESENT       1
#define PTL_2MB_PAGE      (1<<7)
	.data
//.section K_DATA_START, "aw", @progbits
.align 4096

# Identical mapping of first 64MB and the same of -2GB -> 0	
.global ptl_2
ptl_2:	
	.quad 0x0 | (PTL_WRITABLE | PTL_PRESENT | PTL_2MB_PAGE)
	.quad 0x200000 | (PTL_WRITABLE | PTL_PRESENT | PTL_2MB_PAGE)
	.quad 0x400000 | (PTL_WRITABLE | PTL_PRESENT | PTL_2MB_PAGE)
	.quad 0x600000 | (PTL_WRITABLE | PTL_PRESENT | PTL_2MB_PAGE)
	.quad 0x800000 | (PTL_WRITABLE | PTL_PRESENT | PTL_2MB_PAGE)
	.quad 0xa00000 | (PTL_WRITABLE | PTL_PRESENT | PTL_2MB_PAGE)
	.quad 0xc00000 | (PTL_WRITABLE | PTL_PRESENT | PTL_2MB_PAGE)
	.quad 0xe00000 | (PTL_WRITABLE | PTL_PRESENT | PTL_2MB_PAGE)
	.quad 0x1000000 | (PTL_WRITABLE | PTL_PRESENT | PTL_2MB_PAGE)
	.quad 0x1200000 | (PTL_WRITABLE | PTL_PRESENT | PTL_2MB_PAGE)
	.quad 0x1400000 | (PTL_WRITABLE | PTL_PRESENT | PTL_2MB_PAGE)
	.quad 0x1600000 | (PTL_WRITABLE | PTL_PRESENT | PTL_2MB_PAGE)
	.quad 0x1800000 | (PTL_WRITABLE | PTL_PRESENT | PTL_2MB_PAGE)
	.quad 0x1a00000 | (PTL_WRITABLE | PTL_PRESENT | PTL_2MB_PAGE)
	.quad 0x1c00000 | (PTL_WRITABLE | PTL_PRESENT | PTL_2MB_PAGE)
	.quad 0x1e00000 | (PTL_WRITABLE | PTL_PRESENT | PTL_2MB_PAGE)
	.quad 0x2000000 | (PTL_WRITABLE | PTL_PRESENT | PTL_2MB_PAGE)
	.quad 0x2200000 | (PTL_WRITABLE | PTL_PRESENT | PTL_2MB_PAGE)
	.quad 0x2400000 | (PTL_WRITABLE | PTL_PRESENT | PTL_2MB_PAGE)
	.quad 0x2600000 | (PTL_WRITABLE | PTL_PRESENT | PTL_2MB_PAGE)
	.quad 0x2800000 | (PTL_WRITABLE | PTL_PRESENT | PTL_2MB_PAGE)
	.quad 0x2a00000 | (PTL_WRITABLE | PTL_PRESENT | PTL_2MB_PAGE)
	.quad 0x2c00000 | (PTL_WRITABLE | PTL_PRESENT | PTL_2MB_PAGE)
	.quad 0x2e00000 | (PTL_WRITABLE | PTL_PRESENT | PTL_2MB_PAGE)
	.quad 0x3000000 | (PTL_WRITABLE | PTL_PRESENT | PTL_2MB_PAGE)
	.quad 0x3200000 | (PTL_WRITABLE | PTL_PRESENT | PTL_2MB_PAGE)
	.quad 0x3400000 | (PTL_WRITABLE | PTL_PRESENT | PTL_2MB_PAGE)
	.quad 0x3600000 | (PTL_WRITABLE | PTL_PRESENT | PTL_2MB_PAGE)
	.quad 0x3800000 | (PTL_WRITABLE | PTL_PRESENT | PTL_2MB_PAGE)
	.quad 0x3a00000 | (PTL_WRITABLE | PTL_PRESENT | PTL_2MB_PAGE)
	.quad 0x3c00000 | (PTL_WRITABLE | PTL_PRESENT | PTL_2MB_PAGE)
	.quad 0x3e00000 | (PTL_WRITABLE | PTL_PRESENT | PTL_2MB_PAGE)
	
.align 4096
.global ptl_1
ptl_1:
	.quad ptl_2 + (PTL_WRITABLE | PTL_PRESENT)
	.fill 509,8,0
	.quad ptl_2 + (PTL_WRITABLE | PTL_PRESENT)
	.fill 1,8,0
	
.align 4096
.global ptl_0
ptl_0:
	.quad ptl_1 + (PTL_WRITABLE | PTL_PRESENT)
	.fill 255,8,0
	.quad ptl_1 + (PTL_WRITABLE | PTL_PRESENT)
	.fill 254,8,0
	.quad ptl_1 + (PTL_WRITABLE | PTL_PRESENT)


cpu_gdt_table:
	.quad	0x0000000000000000	/* NULL descriptor */
	.quad	0x00cf9a000000ffff	/* __KERNEL32_CS 0x08*/
	.quad	0x00cf92000000ffff	/* __KERNEL_DS  0x10*/


	.quad	0x00cffa000000ffff	/* __USER32_CS 0x18*/
	.quad	0x00cff2000000ffff	/* __USER_DS, __USER32_DS  0x29*/		

	.quad	0x00af9a000000ffff	/* __KERNEL64_CS */
	.quad	0x00affa000000ffff	/* __USER_CS */

	.quad	0x008f9a000000ffff	/* __KERNEL_COMPAT32_CS */	
	.quad	0x00009a000000ffff	/* __KERNEL16_CS - 16bit PM for S3 wakeup. */
					/* base must be patched for real base address. */

	.quad	0,0			/* TSS */
	.quad	0			/* LDT */
	.quad   0,0,0			/* three TLS descriptors */ 
	.quad	0			/* unused now */

gdt_end:	
	/* asm/segment.h:GDT_ENTRIES must match this */	
	/* This should be a multiple of the cache line size */
	/* GDTs of other CPUs: */	
	.fill (GDT_SIZE * 1) - (gdt_end - cpu_gdt_table)

idt_table64:	
	.rept   256
	.quad   0
	.quad 	0
	.endr
