global sys_hi

section .text
bits 64
sys_hi:
	push rbx
	mov rsi, 0
	syscall
	pop rbx
	ret
