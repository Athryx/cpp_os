global spin_lock
global spin_try_lock

section .text
bits 64
spin_lock:
	cmp byte [rdi], 0
	jnz .spin_loop

	mov al, 1

.retry:
	xchg al, [rdi]
	test al, al
	jnz .spin_loop
	ret

.spin_loop:
	pause
	cmp byte [rdi], 0
	jnz .spin_loop
	jmp .retry

spin_try_lock:
	cmp byte [rdi], 0
	jnz .end_fail

	mov al, 1

	xchg al, [rdi]
	test al, al
	jnz .end_fail

	ret

.end_fail:
	mov al, 0
	ret
