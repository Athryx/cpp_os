%include "asm_def.asm"

global int_sched
global load_cr3

section .text
bits 64
int_sched:
	int INT_SCHED
	ret
