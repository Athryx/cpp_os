global log2

section .text
bits 64
log2:
	o64 bsr rax, rdi
	jz .undef
	ret
.undef:
	mov rax, 0xffffffffffffffff
	ret
