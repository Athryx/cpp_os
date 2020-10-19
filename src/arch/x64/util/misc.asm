;global canonical_addr

section .text
bits 64
; for 4 level paging
;canonical_addr:
;	test rdi, 1 << 47
;	jz .lower_half

;.lower_half
;	mov rax, 0
;	and rax, rdi
;	ret

;	mov rax, 0xffff000000000000
;	or rax, rdi
;	ret
