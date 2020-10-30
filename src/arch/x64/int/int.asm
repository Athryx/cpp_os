%include "asm_def.asm"

extern pic_eoi

%macro make_asm_int_handler 1
extern c_int_handler_ %+ %1
global int_handler_ %+ %1
int_handler_ %+ %1 %+ :
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
	mov rsi, 0
	mov rdx, rsp
	mov rax, c_int_handler_ %+ %1
	call rax
	; save return value because ax is needed to load segment registers
	mov r14, rax

	cmp r14, 0
	je .restore

	; set registers according to output
	mov rax, [r14 + registers.rsp]
	mov [r15 + int_data.rsp], rax
	mov rax, [r14 + registers.rip]
	mov [r15 + int_data.rip], rax
	mov rax, [r14 + registers.rflags]
	mov [r15 + int_data.rflags], rax

	mov rax, 0
	mov ax, [r14 + registers.ds]
	mov qword [r15 + int_data.ss], rax
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ax, [r14 + registers.cs]
	mov qword [r15 + int_data.cs], rax

	mov rax, [r14 + registers.rax]
	mov rbx, [r14 + registers.rbx]
	mov rcx, [r14 + registers.rcx]
	mov rdx, [r14 + registers.rdx]
	mov rbp, [r14 + registers.rbp]
	mov rdi, [r14 + registers.rdi]
	mov rsi, [r14 + registers.rsi]
	mov r8, [r14 + registers.r8]
	mov r9, [r14 + registers.r9]
	mov r10, [r14 + registers.r10]
	mov r11, [r14 + registers.r11]
	mov r12, [r14 + registers.r12]
	mov r13, [r14 + registers.r13]
	mov r15, [r14 + registers.r15]
	mov r14, [r14 + registers.r14]

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
global int_handler_ %+ %1
int_handler_ %+ %1 %+ :
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
	mov rsi, [r15]
	add r15, 8

	; call c function
	mov rdi, r15
	mov rdx, rsp
	mov rax, c_int_handler_ %+ %1
	call rax
	; save return value because ax is needed to load segment registers
	mov r14, rax

	cmp r14, 0
	je .restore

	; set registers according to output
	mov rax, [r14 + registers.rsp]
	mov [r15 + int_data.rsp], rax
	mov rax, [r14 + registers.rip]
	mov [r15 + int_data.rip], rax
	mov rax, [r14 + registers.rflags]
	mov [r15 + int_data.rflags], rax

	mov rax, 0
	mov ax, [r14 + registers.ds]
	mov qword [r15 + int_data.ss], rax
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ax, [r14 + registers.cs]
	mov qword [r15 + int_data.cs], rax

	mov rax, [r14 + registers.rax]
	mov rbx, [r14 + registers.rbx]
	mov rcx, [r14 + registers.rcx]
	mov rdx, [r14 + registers.rdx]
	mov rbp, [r14 + registers.rbp]
	mov rdi, [r14 + registers.rdi]
	mov rsi, [r14 + registers.rsi]
	mov r8, [r14 + registers.r8]
	mov r9, [r14 + registers.r9]
	mov r10, [r14 + registers.r10]
	mov r11, [r14 + registers.r11]
	mov r12, [r14 + registers.r12]
	mov r13, [r14 + registers.r13]
	mov r15, [r14 + registers.r15]
	mov r14, [r14 + registers.r14]

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
global int_handler_ %+ %1
int_handler_ %+ %1 %+ :
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
	mov rsi, 0
	mov rdx, rsp
	mov rax, c_int_handler_ %+ %1
	call rax
	; save return value because pic_eoi overwrites it
	mov r14, rax

	; send pic eoi
	mov rdi, %1
	mov rcx, pic_eoi
	call rcx

	cmp r14, 0
	je .restore

	; set registers according to output
	mov rax, [r14 + registers.rsp]
	mov [r15 + int_data.rsp], rax
	mov rax, [r14 + registers.rip]
	mov [r15 + int_data.rip], rax
	mov rax, [r14 + registers.rflags]
	mov [r15 + int_data.rflags], rax

	mov rax, 0
	mov ax, [r14 + registers.ds]
	mov qword [r15 + int_data.ss], rax
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ax, [r14 + registers.cs]
	mov qword [r15 + int_data.cs], rax

	mov rax, [r14 + registers.rax]
	mov rbx, [r14 + registers.rbx]
	mov rcx, [r14 + registers.rcx]
	mov rdx, [r14 + registers.rdx]
	mov rbp, [r14 + registers.rbp]
	mov rdi, [r14 + registers.rdi]
	mov rsi, [r14 + registers.rsi]
	mov r8, [r14 + registers.r8]
	mov r9, [r14 + registers.r9]
	mov r10, [r14 + registers.r10]
	mov r11, [r14 + registers.r11]
	mov r12, [r14 + registers.r12]
	mov r13, [r14 + registers.r13]
	mov r15, [r14 + registers.r15]
	mov r14, [r14 + registers.r14]

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
	mov r14, [rsp + registers.r14]
	mov r15, [rsp + registers.r15]

	add rsp, registers_size
	iretq
%endmacro

section .text
bits 64
make_asm_irq_handler IRQ_TIMER
make_asm_int_handler INT_SCHED
