extern thread_current
global thread_switch_asm
global thread_userspace

struc thread_t
	.stack_pointer resq 1
	.stack_start resq 1
	.stack_size resq 1

	.time resq 1
	.sleep_time resq 1
	.state resb 1
	.addr_space resq 1

	.next resq 1
endstruc

section .text
bits 64
thread_switch_asm:
	; save all registers that are not caller saved except rsp
	push rbx
	push rbp
	push r12
	push r13
	push r14
	push r15

	; save rsp in thread data
	mov [rdi + thread_t.stack_pointer], rsp

	; load new rsp
	mov rsp, [rsi + thread_t.stack_pointer]

	; load new cr4 if neaded
	cmp rdx, 0
	je .no_addr_change
	mov cr3, rdx
.no_addr_change:

	; restore registers
	pop r15
	pop r14
	pop r13
	pop r12
	pop rbp
	pop rbx

	ret
thread_userspace:
	mov ax, 0x1b	; userspace data segment and rpl 3
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax	; ss handled by iretq

	pop rcx		; pop off function to start process in
	mov rax, rsp
	push 0x1b	; userspace data segment and rpl 3
	push rax
	pushf
	push 0x23	; userspace code segment and rpl 3
	push rcx	; rip
	iretq
