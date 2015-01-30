;;
;;
;;  Jicama OS  
;;  Copyright (C) 2003  DengPingPing     All rights reserved.     
;; Original from liza
%if 0
	bits	32
	SECTION .text

          global  _start64,  _kernel64_img;4 init shell
          extern _entry_long_mode, _longmode_setup,  _puts ;jump to init code

no_long_mode:
ret

_start64:
	;;64 bit check
	mov eax, 0x80000000
	cpuid
	cmp eax,	0x80000000
	jbe	no_long_mode
	;;/* Check if long mode is implemented */
	mov	eax, 0x80000001
	cpuid
	bt	edx, 29
	jnc	no_long_mode

amd64:
	cli
	mov eax,cr0
	or	eax,0x000050021	; AM/WP/NE/PE
	mov cr0,eax

	call _entry_long_mode

	;;	signature check
	mov ebx,[_kernel64_img]
	mov eax,[ebx]
	cmp eax,0x464C457F ; ELF
	jz sign_ok
	mov ebx, bad_kernel_image	
	push ebx
	CALL _puts
	add esp,2
	ret

sign_ok:
	jmp 0x0028:start64


;;	64bit mode
[bits 64]
start64:	
	;; 64bit IDT
	lidt [__IDT_64 wrt rip]

	;;	parse ELF program header
	mov rbx,[_kernel64_img  wrt rip]
	lea r15,[rbx]
	mov r8,[r15+0x20]
	movzx r9d,word [r15+0x36]
	movzx r10d,word [r15+0x38]
	lea r14,[r15+r8]

locate_kernel_l00:
	mov eax,[r14]
	cmp eax,byte 0x01 ; PT_LOAD
	jnz locate_kernel_l99

	mov rcx,[r14+0x20]
	mov rdi,[r14+0x10]
	xor eax,eax
	lea ecx,[rcx+0xFFFF]
	and ecx,dword 0xFFFFF000
	shr ecx,3
	rep stosq
	mov ecx,[r14+0x28]
	mov rsi,[r14+0x08]
	mov rdi,[r14+0x10]
	lea ecx,[rcx+7]
	shr ecx,3
	lea rsi,[rsi+r15]
	rep movsq

locate_kernel_l99:
	lea r14,[r14+r9]
	dec r10d
	jnz locate_kernel_l00

	;;	jmp kernel
	lea rdi,[memstat wrt rip]
	;;push rdi
	mov rax,[r15+0x18]
	jmp rax



bad_kernel_image:
	db "error: kernel not ELF format!",0

_base_idt32:
	times 	256 * 8 db 0

__IDT_32:
		dw 0xFFF
		dd _base_idt32

__IDT_64:
		dw 0xFFF
		dd 0x30000

__IDT_RM:
	dw 0x03FF
	dd 0

memstat:
acpiRecMemBase	dd 0
_text_vram		dd 0xB8000
_col			db 0
_row			db 0
_kernel64_img dd 0
memstat_end:

;	times 1024 db 0
	times 256 db 0
_strbuff:

%endif

