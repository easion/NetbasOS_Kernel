
	extern _arch_start_kernel,vm32backup
	;;global	_amd64_start_kernel

[bits 64]
[section .text]

_amd64_start_kernel:
	push rbx
	;;call _arch_start_kernel
	call _vm32_func 
	;;call vm32backup
	jmp $

_vm32_func:
	mov eax,[rdx]
	;;cmp al,vm32_init_driver32
	jz _init_driver32
	;;cmp al,vm32_get_api32
	;;jz _get_api32
	;;cmp al,vm32_get_api64
	;;jz _get_api64
	xor eax,eax
	ret

_api_gate3264_thunk:
	mov edx,edx
	mov eax,eax
	cmp eax,[(_api_table-4) wrt rip]
	jae ag_catch
	push rsi
	push rdi
	mov rcx,rdx
	mov rdi,rdx
	mov r15,[_api_table+rax*8]
	call [_api_table+rax*8]
	pop rdi
	pop rsi
	retf
ag_catch:
	or eax,byte -1
	retf

_init_driver32:
	push r15
	push r14
	push r13
	push r12
	push r11
	push rdi
	push rsi
	push rbp
	push rbx
	mov edx,[rdx+8]
	lea eax,[_init_driver32_return wrt rip]
	mov ecx,[_api_gate32_ptr wrt rip]
	push rax
	mov [rsp+4],cs
	mov rax,[_init_driver32_thunk_ptr wrt rip]
	push rax
	retf
_init_driver32_return:
	pop rbx
	pop rbp
	pop rsi
	pop rdi
	pop r11
	pop r12
	pop r13
	pop r14
	pop r15
	ret


[bits 32]
[section .text]

align 16

_api_gate32_ptr				dd _api_gate32
_api_gate32_super:
	jmp [_api_gate32_ptr]

_api_gate32:
	or eax,eax
	jz _api_gate32_stack
	lea edx,[esp+4]
_api_gate32_go:
	push ebp
	mov ebp,esp
	and esp,byte 0xF0
	call far [_api_gate3264_thunk_ptr]
	leave
_create_thread_thunk:
	ret

_api_gate32_stack:
	lea edx,[edx+4]
	mov eax,[edx-4]
	jmp _api_gate32_go

startvm32 :
	mov word [0B8000h], 144ah
	push ecx
	call edx
	pop ecx
	retf

_init_driver32_thunk:
	mov word [0B8000h], 144ah
	retf


[bits 64]
	global mainCRTStartup
mainCRTStartup:
	push rbx
	ret

align 16
[section .data]

_api_gate					dq 0

_api_table_max				dq 0
_api_table					times 100 dq 0

;;_create_thread_thunk_ptr	dd _create_thread_thunk,0x20
_api_gate3264_thunk_ptr	dd _api_gate3264_thunk,0x28
_init_driver32_thunk_ptr	dd _init_driver32_thunk,0x08

_ebss:


