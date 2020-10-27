%include "asm_def.asm"

global int_sched
global thread_userspace

section .text
bits 64
int_sched:
	int INT_SCHED
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
