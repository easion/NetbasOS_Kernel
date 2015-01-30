;;
;;
;;  Jicama OS  0.01
;;  Copyright (C) 2003  DengPingPing     All rights reserved.     
;;

          SECTION .text
          BITS 32

          global  _start, _start_kernel, ;4 init shell
          extern ptl_0,  _main, ;jump to init code
          EXTERN code, bss, end,  gdtr, pae_setup
;=========================================
          align 4
	; these are in the linker script
 
 _start:
        cmp eax, 0x2BADB002 ;test if boot from grub
	je __boot_from_grub

        ;must support a new boot program myself
        ; display a  'J' and freeze
	push edx
	push ecx
	push ebx
	push eax
	;;call _start_kernel_ia32
	mov word [0B8000h], 144ah
	jmp short $
;======================================
_ret_near:
	ret

__boot_from_grub:
	cli
	mov	esp,0x800000

	mov [grub_eax], eax
	mov [grub_ebx], ebx
	

	lgdt [gdtr]
	mov cx,0x10
	mov ds,cx
	mov es,cx
	mov ss,cx
	mov cx,0x20
	mov fs,cx
	mov gs,cx
mov word [0B8000h], 144ch
	jmp 0x08:now_in_prot

now_in_prot:

	push	dword 0
	popf  ;Make sure the eflag register be 0
	mov word [0B8004h], 144fh

	; ebx holds a pointer to
	; the multiboot information block	
	push ebx
	push eax
	mov eax, 0
	cpuid
	push edx

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
	;;mov eax,cr0
	;;or	eax,0x000050021	; AM/WP/NE/PE
	;;mov cr0,eax
	;;db 0xEB,0x00
; Disable Paging
   mov   eax, cr0
   btr   eax, 31         ; CR0.PG=1
   mov   cr0, eax

	;;	Enter to Long Mode
	mov eax, cr4
	bts eax, 5		; PG
	mov cr4, eax

	;;call pae_setup

	;;mov eax,cr0
	;;and eax,0x7FFFFFFF
	;;mov cr0,eax
	;;call _ret_near

	lea eax, [ptl_0]
	mov cr3, eax


	;;mov eax, cr4
	;or eax,0x2A0		;OSFXSR/PGE/PAE
	;mov cr4, eax

 	mov ecx, 0xc0000080
	rdmsr
	;;or eax,0x0901	; NXE/LME/SCE
	bts eax, 8
	wrmsr

	;;	Enter to Long Mode
	mov eax, cr0
	bts eax, 31		; PG
	mov cr0, eax


sign_ok:
	jmp 0x0028:start64

no_long_mode:
       	;;call _start_kernel_ia32		; call C code
	mov word [0B8002h], 1439h
	jmp short $ ;system halted!
;============================================
		ALIGN  4
	grub_jicama:
	dd 0x1BADB002
	dd 0x00010000
	dd 0-0x1BADB002-0x00010000
        ; aout kludge. These must be PHYSICAL addresses
	dd grub_jicama
	dd code
	dd bss
	dd end
	dd _start

	dd 0
	dd 800
	dd 600
	dd 32

        
;;	64bit mode
[bits 64]
extern _amd64_start_kernel
start64:	
	mov word [0B8002h], 144ah
	;;jmp short $
	;;call _amd64_start_kernel
	mov rsp, 0x400000
	mov rax, _amd64_start_kernel
	jmp rax
	;;call 
	mov word [0B8002h], 144ah
	jmp short $
	;;cli 
	;;hlt



SECTION .data


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

grub_eax dd 0
grub_ebx dd 0

;	times 1024 db 0
	times 256 db 0
_strbuff:

