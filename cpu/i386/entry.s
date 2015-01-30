;;
;;
;;  Jicama OS  
;;  Copyright (C) 2003  DengPingPing     All rights reserved.     
;; Original from liza
	bits	32
	SECTION .text


VIDEO_SEL	        EQU	0x38		;video segment
LDT_DS_SEL	EQU	0x17		;
LINUX_SYSCALL_END	EQU	200		;
NR_SIGRETURN 	EQU	119		;
 ENOSYS	EQU	-38	;;/* Function not implemented */


;;KERNEL_CS_SEL           equ         0X08
;;KERNEL_DS_SEL           EQU         0X10
;;KERNEL16_CS_SEL            EQU         0X28
;;KERNEL16_DS_SEL            EQU         0X30

%macro	EXPORT	1
	GLOBAL %1
	%1:
	GLOBAL _%1
	_%1:
%endmacro

%macro	IMPORT 1
%ifdef UNDERBARS
	EXTERN _%1		; GCC for DOS (DJGPP; COFF)
	%define %1 _%1
%else
	EXTERN %1		; GCC for Linux (ELF)
%endif
%endmacro

%macro 	SAVE_HW_REGS 0
	push	ds		; save ds
	push	es		; save es
	push	fs		; save fs
	push	gs		; save gs
	;;pusha			; save "general" registers

	push	eax
	push	ebp
	push	edi
	push	esi

	push	edx
	push	ecx
	push	ebx

	mov	dx, 0x10		; ss is kernel data segment
	mov	ds, dx		; load rest of kernel segments
	mov	es, dx		; kernel does not use fs, gs

%endmacro

%macro 	LOAD_HW_REGS 0
	;;popa			; save "general" registers
	pop	ebx
	pop	ecx
	pop	edx

	pop	esi
	pop	edi
	pop	ebp
	pop	eax


	pop	gs		; save gs
	pop	fs		; save fs
	pop	es		; save es
	pop	ds		; save ds
%endmacro


         
;------------------------------------------------------------------------------------------------
; Note this is a macro, it looks like a subroutine.



IMPORT   start_kernel
IMPORT cur_task_ptr
IMPORT  sys_nosys 
IMPORT   do_system_call 
IMPORT chedule
IMPORT entry_fork_proc
IMPORT do_exec
IMPORT do_clone


ignore_int:
	iret

delay:
	ret

; __public void nop(port_t port);
	align	16
EXPORT	nopcode
	nop
	ret

; Disable CPU interrupts.


		; Default, interrupt return

;------------------------------------------------------------------
;; system call for kernel
	align	16
EXPORT	sys_call
        push eax ;;do not change stack data 
        push 0
        SAVE_HW_REGS

	mov dx, LDT_DS_SEL
	mov gs, dx              ;for local data segment

	call do_system_call ; 
	mov  [esp + 24] ,eax

	;;mov esi, [_cur_task_ptr]
	;;mov eax, [esi + 0]
	;;cmp eax, 1
	;;je __must_reschedule

_ret_from_syscall:
        LOAD_HW_REGS
	add esp, 8
	iret

__must_reschedule:
         ;;push _ret_from_syscall
	 ;;jmp   _entry_fork_proc
;------------------------------------------------------------------


IMPORT   UnixCall_table 
IMPORT sigflush

	align	16
EXPORT linux_compliance_call
	push eax
	push 0
	SAVE_HW_REGS

	;;mov	dx, ss		; ss is kernel data segment
	;;mov	ds, dx		; load rest of kernel segments
	;;mov	es, dx		; kernel does not use fs, gs

	mov dx, LDT_DS_SEL
	mov gs, dx              ;for local data segment

	cmp eax, LINUX_SYSCALL_END ;;确定范围是否越界
	jmp _call_linux_service

	mov eax, ENOSYS ;;不存在的系统调用
	jmp _backup

_call_linux_service:
	call	[UnixCall_table + 4*eax] ; 
	mov  [esp + 24] ,eax
	call sigflush

_backup:
	LOAD_HW_REGS
	add esp,8
	iret				; restart (another) process

 EXPORT SIGRETURN_start_code
	 push eax
	 mov eax, NR_SIGRETURN
	 int 0x80
 EXPORT SIGRETURN_end_code


;  void entry_proc ( unsigned short tss_selector);
;              ip
 EXPORT entry_proc
	mov	eax,[esp+4]
	push	eax
	push 	dword 0
	jmp 	far [esp]
	add	esp, 8
	ret

_context_save:
			xor    eax,eax
			str     ax
			ret




;;------------------------------------------------------------------------------
SECTION .data
       idle_debug	db	'/','-','\','|'

EXPORT g_kvirt_to_phys
	dd 0

;;;;;;;;;;;;;
;;;; TSS ;;;;
;;;;;;;;;;;;;

	ALIGN 4

; most of the TSS is unused
; I need only ESP0, SS0, and the I/O permission bitmap
EXPORT	ktss
	dw 0, 0			; back link
_tss_esp0:	dd 0			; ESP0
	dw 0x10, 0	; SS0, reserved

	dd 0			; ESP1
	dw 0, 0			; SS1, reserved

	dd 0			; ESP2
	dw 0, 0			; SS2, reserved

	dd 0			; CR3
	dd 0, 0			; EIP, EFLAGS
	dd 0, 0, 0, 0		; EAX, ECX, EDX, EBX
	dd 0, 0, 0, 0		; ESP, EBP, ESI, EDI
	dw 0, 0			; ES, reserved
	dw 0, 0			; CS, reserved
	dw 0, 0			; SS, reserved
	dw 0, 0			; DS, reserved
	dw 0, 0			; FS, reserved
	dw 0, 0			; GS, reserved
	dw 0, 0			; LDT, reserved
	dw 0, g_tss_iopb - ktss	; debug, IO permission bitmap base
EXPORT g_tss_iopb

; no I/O permitted
	times 8192 db 0FFh

; The TSS notes in section 12.5.2 of volume 1 of
; "IA-32 Intel Architecture Software Developer's Manual"
; are confusing as hell. I think they mean simply that the IOPB
; must contain an even number of bytes; so pad here if necessary.
	;dw 0FFFFh
EXPORT g_tss_end

