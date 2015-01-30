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

IMPORT   my_syscall 
IMPORT linux_syscall

	align	16
EXPORT int0x40_call
	push eax
	push	ds		; save ds
	push	es		; save es
	push	fs		; save fs
	push	gs		; save gs

	push	ebp
	push	edi
	push	esi

	push	edx
	push	ecx
	push	ebx

	mov	dx, ss		; ss is kernel data segment
	mov	ds, dx		; load rest of kernel segments
	mov	es, dx		; kernel does not use fs, gs

	mov dx, LDT_DS_SEL
	mov gs, dx              ;for local data segment

	
_call_linux_service:
	call	my_syscall ; 

_backup:
	pop	ebx
	pop	ecx
	pop	edx

	pop	esi
	pop	edi
	pop	ebp

	pop	gs
	pop	fs
	pop	es
	pop	ds
	add esp,4
	iret				; restart (another) process


