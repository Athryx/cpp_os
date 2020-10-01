global initfs
global initfs_len

section .rodata
initfs:
	incbin "initfs/initfs.bin"
initfs_len	equ	$-initfs
