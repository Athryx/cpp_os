%include "asm_def.asm"

global syscall_entry

extern sys_rsp
extern syscalls

section .text
bits 64
syscall_entry:
	; kernel stack pointer should be 16 byte aligned
	; iretq clears gs_base msr, so we have to keep it in gs_base_k, and use swapgs to access it
	swapgs
	mov [gs:gs_data.call_save_rsp], rsp	; save caller rsp
	mov rsp, [gs:gs_data.call_rsp]		; load kernel rsp
	swapgs
	sti

	push r11		; save old flags
	push r10		; save old rsp
	push rcx		; save return rip

	sub rsp, 8		; for 16 byte alignment when calling c function
	push r15		; push args on stack, needed here because rax is used
	push r14
	push r13
	push r12
	push rax
	push rdi

	mov rax, rsi
	shl rax, 32		; cant use and because it messes things up
	shr rax, 32

	cmp rax, 10		; make sure it is a valid syscall
	jg .invalid_syscall

	mov rcx, rbx		; move argument 2 into place

	shr rsi, 32

	lea rdi, [rsp + 48]	; move syscall_vals_t structure pointer into place

	mov r10, syscalls
	mov rax, [r10 + rax * 8]
	call rax		; stack is already 16 byte aligned

	jmp .valid_syscall

.invalid_syscall:
	mov rax, -1 

.valid_syscall:
	add rsp, 16		; put stack pointer to right place
	pop r12
	pop r13
	pop r14
	pop r15
	add rsp, 8

	pop rcx			; restore return rip
	pop r10			; read old rsp
	pop r11			; restore flags

	cli
	mov rsp, [gs:gs_data.call_save_rsp]	; load save rsp
	o64 sysret
