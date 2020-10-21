%include "asm_def.asm"

extern pic_eoi

%macro make_asm_int_handler 1
extern c_int_handler_ %+ %1
asm_int_handler %+ %1:
	; make register structure for c function
	; rsp, rflags, and rip are set on interrupt stack frame
	sub rsp, registers_size
	mov [rsp + registers.rax], rax
	mov [rsp + registers.rbx], rbx
	mov [rsp + registers.rcx], rcx
	mov [rsp + registers.rdx], rdx
	mov [rsp + registers.rbp], rbp
	mov [rsp + registers.rdi], rdi
	mov [rsp + registers.rsi], rsi
	mov [rsp + registers.r8], r8
	mov [rsp + registers.r9], r9
	mov [rsp + registers.r10], r10
	mov [rsp + registers.r11], r11
	mov [rsp + registers.r12], r12
	mov [rsp + registers.r13], r13
	mov [rsp + registers.r14], r14
	mov [rsp + registers.r15], r15

	; get location of interrupt stack frame data structure
	mov r15, rsp
	add r15, registers_size

	; call c function
	mov rdi, r15
	mov rsi, rsp
	mov rdx, 0
	mov rax, c_int_handler_ %+ %1
	call rax

	cmp rax, 0
	je .restore

	; set registers according to output
	mov rcx, [rax + registers.rsp]
	mov [r15 + int_data.rsp], rcx
	mov rcx, [rax + registers.rip]
	mov [r15 + int_data.rip], rcx
	mov rcx, [rax + registers.rflags]
	mov [r15 + int_data.rflags], rcx

	mov rbx, [rax + registers.rbx]
	mov rcx, [rax + registers.rcx]
	mov rdx, [rax + registers.rdx]
	mov rbp, [rax + registers.rbp]
	mov rdi, [rax + registers.rdi]
	mov rsi, [rax + registers.rsi]
	mov r8, [rax + registers.r8]
	mov r9, [rax + registers.r9]
	mov r10, [rax + registers.r10]
	mov r11, [rax + registers.r11]
	mov r12, [rax + registers.r12]
	mov r13, [rax + registers.r13]
	mov r14, [rax + registers.r14]
	mov r15, [rax + registers.r15]
	mov rax, [rax + registers.rax]

	add rsp, registers_size
	iretq

	; do not change any registers
	.restore
	mov rax, [rsp + registers.rax]
	mov rcx, [rsp + registers.rcx]
	mov rdx, [rsp + registers.rdx]
	mov rdi, [rsp + registers.rdi]
	mov rsi, [rsp + registers.rsi]
	mov r8, [rsp + registers.r8]
	mov r9, [rsp + registers.r9]
	mov r10, [rsp + registers.r10]
	mov r11, [rsp + registers.r11]
	mov r15, [rsp + registers.r15]

	add rsp, registers_size
	iretq
%endmacro

%macro make_asm_int_handler_e 1
extern c_int_handler_ %+ %1
asm_int_handler %+ %1:
	; make register structure for c function
	; rsp, rflags, and rip are set on interrupt stack frame
	sub rsp, registers_size
	mov [rsp + registers.rax], rax
	mov [rsp + registers.rbx], rbx
	mov [rsp + registers.rcx], rcx
	mov [rsp + registers.rdx], rdx
	mov [rsp + registers.rbp], rbp
	mov [rsp + registers.rdi], rdi
	mov [rsp + registers.rsi], rsi
	mov [rsp + registers.r8], r8
	mov [rsp + registers.r9], r9
	mov [rsp + registers.r10], r10
	mov [rsp + registers.r11], r11
	mov [rsp + registers.r12], r12
	mov [rsp + registers.r13], r13
	mov [rsp + registers.r14], r14
	mov [rsp + registers.r15], r15

	; get location of interrupt stack frame data structure
	mov r15, rsp
	add r15, registers_size

	; get error code
	mov rdx, [r15]
	add r15, 8

	; call c function
	mov rdi, r15
	mov rsi, rsp
	mov rax, c_int_handler_ %+ %1
	call rax

	cmp rax, 0
	je .restore

	; set registers according to output
	mov rcx, [rax + registers.rsp]
	mov [r15 + int_data.rsp], rcx
	mov rcx, [rax + registers.rip]
	mov [r15 + int_data.rip], rcx
	mov rcx, [rax + registers.rflags]
	mov [r15 + int_data.rflags], rcx

	mov rbx, [rax + registers.rbx]
	mov rcx, [rax + registers.rcx]
	mov rdx, [rax + registers.rdx]
	mov rbp, [rax + registers.rbp]
	mov rdi, [rax + registers.rdi]
	mov rsi, [rax + registers.rsi]
	mov r8, [rax + registers.r8]
	mov r9, [rax + registers.r9]
	mov r10, [rax + registers.r10]
	mov r11, [rax + registers.r11]
	mov r12, [rax + registers.r12]
	mov r13, [rax + registers.r13]
	mov r14, [rax + registers.r14]
	mov r15, [rax + registers.r15]
	mov rax, [rax + registers.rax]

	add rsp, registers_size
	; not sure if needed
	add rsp, 8
	iretq

	; do not change any registers
	.restore
	mov rax, [rsp + registers.rax]
	mov rcx, [rsp + registers.rcx]
	mov rdx, [rsp + registers.rdx]
	mov rdi, [rsp + registers.rdi]
	mov rsi, [rsp + registers.rsi]
	mov r8, [rsp + registers.r8]
	mov r9, [rsp + registers.r9]
	mov r10, [rsp + registers.r10]
	mov r11, [rsp + registers.r11]
	mov r15, [rsp + registers.r15]

	add rsp, registers_size
	add rsp, 8
	iretq
%endmacro

%macro make_asm_irq_handler 1
extern c_int_handler_ %+ %1
asm_int_handler %+ %1:
	; make register structure for c function
	; rsp, rflags, and rip are set on interrupt stack frame
	sub rsp, registers_size
	mov [rsp + registers.rax], rax
	mov [rsp + registers.rbx], rbx
	mov [rsp + registers.rcx], rcx
	mov [rsp + registers.rdx], rdx
	mov [rsp + registers.rbp], rbp
	mov [rsp + registers.rdi], rdi
	mov [rsp + registers.rsi], rsi
	mov [rsp + registers.r8], r8
	mov [rsp + registers.r9], r9
	mov [rsp + registers.r10], r10
	mov [rsp + registers.r11], r11
	mov [rsp + registers.r12], r12
	mov [rsp + registers.r13], r13
	mov [rsp + registers.r14], r14
	mov [rsp + registers.r15], r15

	; get location of interrupt stack frame data structure
	mov r15, rsp
	add r15, registers_size

	; call c function
	mov rdi, r15
	mov rsi, rsp
	mov rdx, 0
	mov rax, c_int_handler_ %+ %1
	call rax

	; send pic eoi
	mov rdi, %1
	mov rcx, pic_eoi
	call rcx

	cmp rax, 0
	je .restore

	; set registers according to output
	mov rcx, [rax + registers.rsp]
	mov [r15 + int_data.rsp], rcx
	mov rcx, [rax + registers.rip]
	mov [r15 + int_data.rip], rcx
	mov rcx, [rax + registers.rflags]
	mov [r15 + int_data.rflags], rcx

	mov rbx, [rax + registers.rbx]
	mov rcx, [rax + registers.rcx]
	mov rdx, [rax + registers.rdx]
	mov rbp, [rax + registers.rbp]
	mov rdi, [rax + registers.rdi]
	mov rsi, [rax + registers.rsi]
	mov r8, [rax + registers.r8]
	mov r9, [rax + registers.r9]
	mov r10, [rax + registers.r10]
	mov r11, [rax + registers.r11]
	mov r12, [rax + registers.r12]
	mov r13, [rax + registers.r13]
	mov r14, [rax + registers.r14]
	mov r15, [rax + registers.r15]
	mov rax, [rax + registers.rax]

	add rsp, registers_size
	iretq

	; do not change any registers
	.restore
	mov rax, [rsp + registers.rax]
	mov rcx, [rsp + registers.rcx]
	mov rdx, [rsp + registers.rdx]
	mov rdi, [rsp + registers.rdi]
	mov rsi, [rsp + registers.rsi]
	mov r8, [rsp + registers.r8]
	mov r9, [rsp + registers.r9]
	mov r10, [rsp + registers.r10]
	mov r11, [rsp + registers.r11]
	mov r15, [rsp + registers.r15]

	add rsp, registers_size
	iretq
%endmacro

section .text
bits 64
