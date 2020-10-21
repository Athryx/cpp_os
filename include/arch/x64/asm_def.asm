%ifndef ASM_DEF_ASM
%define ASM_DEF_ASM


struc registers
	.rax resq 1
	.rbx resq 1
	.rcx resq 1
	.rdx resq 1
	.rbp resq 1
	.rsp resq 1
	.rdi resq 1
	.rsi resq 1
	.r8 resq 1
	.r9 resq 1
	.r10 resq 1
	.r11 resq 1
	.r12 resq 1
	.r13 resq 1
	.r14 resq 1
	.r15 resq 1
	.rflags resq 1
	.rip resq 1
endstruc

struc int_data
	.rip resq 1
	.cs resq 1
	.rflags resq 1
	.rsp resq 1
	.ss resq 1
endstruc


%endif
