;;
;;
;;  Jicama OS  
;;  Copyright (C) 2003  DengPingPing     All rights reserved.     
;;
	SECTION .text		
	bits    32	

	extern  soft_irq

VIDEO_SEL	        EQU	0x38		;video segment
;;LDT_DS_SEL	EQU	0x17		;

;;KERNEL_CS_SEL           EQU         0X08
;KERNEL_DS_SEL           EQU         0X10
;KERNEL16_CS_SEL            EQU         0X28
;KERNEL16_DS_SEL            EQU         0X30

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

MASTER		EQU	0x20		; I/O port for interrupt controller
MASTER_MASK  EQU	0x21		; setting bits in this port disables ints
SLAVE                   EQU	0xA0		; I/O port for second interrupt controller
SLAVE_MASK     EQU	0xA1		; setting bits in this port disables ints
ENABLE                EQU	0x20		; code used to re-enable after an interrupt

%macro 	hwint_master 1
        push eax ;;do not change stack data 
        push 0
	SAVE_HW_REGS			; save interrupted process state

	;;call _entry_isr

	mov	al, ENABLE
	out	MASTER, al		; reenable master 8259

	push	dword %1		; irq
	;;push	dword	[irq_arg_table + 4*%1]	; eax = (*irq_table[irq])(irq)
	call	intr_handler	; eax = (*irq_table[irq])(irq)
	;;add	esp, 4

	mov ebx, [esp + 60] 
	cmp ebx, 0x0f ;;user thread
	jz do_soft_isr
	cmp ebx, 0x38 ;;kernel thread
	jz do_soft_isr

	add	esp, 4
	LOAD_HW_REGS
	add	esp, 8
	iret				; restart (another) process
%endmacro
;------------------------------------------------------------------------------------------------

%macro 	hwint_slave 1
         push eax ;;do not change stack data 
	 push 0
	SAVE_HW_REGS			; save interrupted process state
	;;call _entry_isr

	mov	al, ENABLE
	out	MASTER, al		; reenable master 8259
	jmp	%%delay			; delay
%%delay
	out	SLAVE, al		; reenable slave 8259

	push	dword %1		; irq
	;;push	dword	[irq_arg_table + 4*%1]
	call	intr_handler	; eax = (*irq_table[irq])(irq)
	;;pop	ecx
	;;add	esp, 4

	mov ebx, [esp + 60] 
	cmp ebx, 0x0f ;;user thread
	jz do_soft_isr
	cmp ebx, 0x38 ;;kernel thread
	jz do_soft_isr

	add	esp, 4
	LOAD_HW_REGS
	add	esp, 8
	iret				; restart (another) process
%endmacro
;------------------------------------------------------------------------------------------------------
; Each of these entry points is an expansion of the hwint_master macro


IMPORT	intr_handler
IMPORT	irq_arg_table
IMPORT spurious_irq
IMPORT do_exception
IMPORT vm86_backup

do_soft_isr:
	call soft_irq
	add	esp, 4
	LOAD_HW_REGS
	add	esp, 8
	iret				; restart (another) process

	align	16
EXPORT hwint00		; Interrupt routine for irq 0 (the clock)
	hwint_master 0

	align	16
EXPORT hwint01		; Interrupt routine for irq 1 (keyboard)
	hwint_master 1

	align	16
EXPORT hwint02		; Interrupt routine for irq 2 (cascade)
	hwint_master 2

	align	16
EXPORT hwint03		; Interrupt routine for irq 3 (second serial)
	hwint_master 3

	align	16
EXPORT hwint04		; Interrupt routine for irq 4 (first serial)
	hwint_master 4

	align	16
EXPORT hwint05		; Interrupt routine for irq 5 (XT winchester)
	hwint_master 5

	align	16
EXPORT  hwint06		; Interrupt routine for irq 6 (floppy)
	hwint_master 6

	align	16
EXPORT  hwint07		; Interrupt routine for irq 7 (printer)
	hwint_master 7

;---------------------------------------------------------------------------------------------------
; Each of these entry points is an expansion of the hwint_slave macro
	align	16
EXPORT  hwint08		; Interrupt routine for irq 8 (realtime clock)
	hwint_slave 8

	align	16
EXPORT  hwint09		; Interrupt routine for irq 9 (irq 2 redirected
	hwint_slave 9

	align	16
EXPORT  hwint10		; Interrupt routine for irq 10
	hwint_slave 10

	align	16
EXPORT  hwint11		; Interrupt routine for irq 11
	hwint_slave 11

	align	16
EXPORT  hwint12		; Interrupt routine for irq 12
	hwint_slave 12

	align	16
EXPORT  hwint13		; Interrupt routine for irq 13  FPU exception)
	hwint_slave 13

	align	16
EXPORT  hwint14		; Interrupt routine for irq 14 (AT winchester)
	hwint_slave 14

	align	16
EXPORT  hwint15		; Interrupt routine for irq 15
	hwint_slave 15

;----------------------------------------------------------------------------------
	
EXPORT  divide_error
	push	dword  0
	push	dword 	0
	jmp	_exception
	
EXPORT  debug
	push	dword  0
	push	dword	1
	jmp	_exception

EXPORT  nmi
	push	dword  0
	push	dword	2
	jmp	_exception

EXPORT  xint3
	push	dword  0
	push	dword	3
	jmp	_exception

EXPORT  overflow
	push	dword  0
	push	dword	4
	jmp	_exception


	
EXPORT  bounds
	push	dword  0
	push	dword	5
	jmp	_exception


	
EXPORT  invalid_op
	push	dword  0
	push	dword	6
	jmp	_exception
	
EXPORT  device_not_avl
	push	dword  0
	push	dword	7
	jmp	_exception
	
EXPORT  double_fault
	push	dword	8
	jmp	_exception
	
EXPORT  coprocessor_segment_overrun
	push	dword  0
	push	dword	9
	jmp	_exception
	
EXPORT  invalid_tss
	nop
	nop
	push	dword	10
	jmp	_exception

EXPORT  segment_not_present
	nop
	nop
	push	dword	11
	jmp	_exception

EXPORT  stack_exception
	nop
	nop
	push	dword	12
	jmp	_exception

EXPORT  general_protection
	nop
	nop
	push	dword 	13
	jmp	_exception

EXPORT  pagefault
	nop
	nop
	cli
	xchg	eax,[esp]
	push	dword	14
	jmp	_exception


EXPORT  copr_error
	push	dword  0
	push	dword	16
	jmp	_exception


	
_exception:
        SAVE_HW_REGS
	call	do_exception	;to call exception Handler

	;;or eax,eax
	cmp eax,0
	jne chuck_regs
         LOAD_HW_REGS
	add	esp, 8
	;;pop eax
	iret

IMPORT   return32bit

chuck_regs:
         LOAD_HW_REGS
	;;add esp,68+8
; restore C registers and return to pmode kernel
	call return32bit
	;;ret
	add	esp,8
	;;pop eax
	iret
