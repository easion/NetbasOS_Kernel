/*
 * kernel assembly defines
 *
 * $Header$
 * $Log$
 *
 */
#ifndef _KASMDEF_H_
#define _KASMDEF_H_


#define ENTER_KERNEL \
	cld		; \
    pushq		%rsi ; \
    pushq		%rdi		; \
    pushq		%rax		; \
    pushq		%rbx		; \
    pushq		%rcx		; \
    pushq		%rdx		; \
    pushq		%r8		; \
    pushq		%r9		; \
    pushq		%r10		; \
    pushq		%r11		; \
    pushq		%r12		; \
    pushq		%r13		; \
    pushq		%r14		

#define LEAVE_KERNEL \
    popq		%r14		
    popq		%r13		; \
    popq		%r12		; \
    popq		%r11		; \
    popq		%r10		; \
    popq		%r9		; \
    popq		%r8		; \
    popq		%rdx		; \
    popq		%rcx		; \
    popq		%rbx		; \
    popq		%rax		; \
    popq		%rdi		; \
    popq		%rsi 


#define KERNEL_SEGS \
	movl	$0x10, %eax	; \
	movw	%ax, %ds	; \
	movw	%ax, %es	

#endif /* _KASMDEF_H_ */
