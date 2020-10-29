global sync_page

section .text
bits 64
sync_page:
	invlpg [rdi]
	ret
