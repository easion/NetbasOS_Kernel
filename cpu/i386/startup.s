;;
;;
;;  Jicama OS  0.01
;;  Copyright (C) 2003  DengPingPing     All rights reserved.     
;;
          SECTION .text
          BITS 32

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

          IMPORT start_kernel 
          IMPORT core_start ;jump to init code
;=========================================
          align 4
          IMPORT code
          IMPORT  bss
          IMPORT  end
	; these are in the linker script
 
EXPORT start
 	fninit
    cmp eax, 0x2BADB002 ;test if boot from grub
	je __boot_from_grub

        ;must support a new boot program myself
        ; display a  'J' and freeze
	push edx
	push ecx
	push ebx
	push eax
	call core_start
	mov word [0B8000h], 144ah
	jmp short $

;======================================
__boot_from_grub:
	;;mov	esp,boot_stack	
	;;mov	esp,0x400000	
	
	push	dword 0
	popf  ;Make sure the eflag register be 0

	; ebx holds a pointer to
	; the multiboot information block	
	push ebx
	push eax
	mov eax, 0
	cpuid
	push edx
       	call start_kernel		; call C code
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
	dd start

	dd 0
	dd 800
	dd 600
	dd 32
	
	;============================================

SECTION .data

	
	bits 32       
  
	

EXPORT gdt_16bit_cs
	dw 0FFFFh
	dw 0			; base; gets set above
	db 0
	db 9Ah			; present, ring 0, code, non-conforming, readable
	db 0			; 16-bit
	db 0

EXPORT gdt_16bit_ds
	dw 0FFFFh
	dw 0			; base; gets set above
	db 0
	db 92h			; present, ring 0, data, expand-up, writable
	db 0
	db 0

EXPORT gdt_kernel_code
dw	0xFFFF	
dw	0
dw	0x9200
dw	0x00CF
EXPORT gdt_kernel_data
dw	0xFFFF	
dw	0
dw	0x9A00
dw	0x00CF


;==============================================
SECTION .bss
	align	16



	ALIGNB  4
%IFDEF DEBUG
	RESB	4096
%ELSE
	RESB	1024
%ENDIF
boot_stack:

