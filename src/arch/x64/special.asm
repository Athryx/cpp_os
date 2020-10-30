global asm_load_gdt
global asm_load_idt
global asm_load_tss
global rdmsr
global wrmsr
global load_data_segments
global syscall_test

bits 64
section .text
asm_load_gdt:
	lgdt [rdi]
	ret
asm_load_idt:
	lidt [rdi]
	ret
asm_load_tss:
	ltr di
	ret
rdmsr:
	mov ecx, edi	; move msr index int ecx
	rdmsr
	shl rdx, 32
	or rax, rdx	; don't need to clear high bits, rdmsr does this
	ret
wrmsr:
	mov ecx, edi
	mov rax, rsi
	mov rdx, rsi
	shr rdx, 32
	wrmsr
	ret
load_data_segments:
	mov ax, di
	mov ss, ax
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	ret
syscall_test:
	push rbx
	push r12
	push r13
	push r14
	push r15

	mov rdx, 1
	mov rbx, 2
	mov r8, 3
	mov r9, 4
	mov rdi, 5
	mov rax, 6
	mov r12, 7
	mov r13, 8
	mov r14, 9
	mov r15, 10

	mov rsi, 0x000000ff00000000

	syscall

	mov rdx, 1
	mov rbx, 2
	mov r8, 3
	mov r9, 4
	mov rdi, 5
	mov rax, 6
	mov r12, 7
	mov r13, 8
	mov r14, 9
	mov r15, 10

	mov rsi, 0x000000ff00000000

	syscall

	pop r15
	pop r14
	pop r13
	pop r12
	pop rbx

	ret
