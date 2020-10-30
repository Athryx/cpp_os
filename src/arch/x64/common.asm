global outb
global outw
global outd

global inb
global inw
global ind

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

%macro cr_funcs 1
global get_cr %+ %1
get_cr %+ %1 %+ :
	mov rax, cr %+ %1
	ret

global set_cr %+ %1
set_cr %+ %1 %+ :
	mov cr %+ %1, rdi
	ret
%endmacro

cr_funcs 0
cr_funcs 2
cr_funcs 3
cr_funcs 4

; from osdev wiki, makes cpu wait for a bit
io_wait:
	out 0x80, al
	ret

get_rsp:
	mov rax, rsp
	sub rax, 8	; there is ret instruction on stack
	ret
