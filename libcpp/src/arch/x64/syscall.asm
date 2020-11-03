global syscall

section .text
bits 64
syscall:
	push rbx	; save rbx

	mov rbx, rdx	; move arg 2 into place
	mov rdx, rsi	; move arg 1 into place
	mov rsi, rdi	; move options and syscall number into place
	mov rdi, r9	; move arg 5 into place
	mov r9, r8	; move arg 4 into place
	mov r8, rcx	; move arg 3 into place

	test rsi, 0x100000000	; test if extended args is set
	jz .no_eargs

	push r12		; save these regs
	push r13
	push r14
	push r15

	mov r15, [rsp + 0x30]	; move arg 6-10 into place
	mov r14, [rsp + 0x38]
	mov r13, [rsp + 0x40]
	mov r12, [rsp + 0x48]
	mov rax, [rsp + 0x50]

	syscall

	pop r15			; restore these regs
	pop r14
	pop r13
	pop r12

	jmp .end

.no_eargs:
	mov rax, [rsp + 0x10]	; move arg 6 into place

	syscall

.end:
	pop rbx			; restore rbx
	ret
