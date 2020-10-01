global outb
global outw
global outd

global inb
global inw
global ind

global int_1

global io_wait

global get_rsp

bits 64
section .text
outb:
	mov dx, di
	mov al, sil
	out dx, al
	ret

outw:
	mov dx, di
	mov ax, si
	out dx, ax
	ret

outd:
	mov dx, di
	mov eax, esi
	out dx, eax
	ret

inb:
	mov dx, di
	in al, dx
	ret

inw:
	mov dx, di
	in ax, dx
	ret

ind:
	mov dx, di
	in eax, dx
	ret

int_1:
	int 1
	ret

; from osdev wiki, makes cpu wait for a bit
io_wait:
	out 0x80, al
	ret

get_rsp:
	mov rax, rsp
	sub rax, 8	; there is ret instruction on stack
	ret
