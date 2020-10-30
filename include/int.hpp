#pragma once


#include <types.hpp>
#include <sched/thread.hpp>


#define MAX_HANDLERS 16

#define PICM_OFFSET 32
#define PICS_OFFSET 40

#define EXC_DIVIDE_BY_ZERO 0
#define EXC_DEBUG 1
#define EXC_NON_MASK_INTERRUPT 2
#define EXC_BREAKPOINT 3
#define EXC_OVERFLOW 4
#define EXC_BOUND_RANGE_EXCEED 5
#define EXC_INVALID_OPCODE 6
#define EXC_DEVICE_UNAVAILABLE 7
#define EXC_DOUBLE_FAULT 8
#define EXC_NONE_9 9
#define EXC_INVALID_TSS 10
#define EXC_SEGMENT_NOT_PRESENT 11
#define EXC_STACK_SEGMENT_FULL 12
#define EXC_GENERAL_PROTECTION_FAULT 13

#define EXC_PAGE_FAULT 14
#define PAGE_FAULT_PROTECTION 1
#define PAGE_FAULT_WRITE (1 << 1)
#define PAGE_FAULT_USER (1 << 2)
#define PAGE_FAULT_RESERVED (1 << 3)
#define PAGE_FAULT_EXECUTE (1 << 4)

#define EXC_NONE_15 15
#define EXC_X87_FLOATING_POINT 16
#define EXC_ALIGNMENT_CHECK 17
#define EXC_MACHINE_CHECK 18
#define EXC_SIMD_FLOATING_POINT 19
#define EXC_VIRTUALIZATION 20
#define EXC_NONE_21 21
#define EXC_NONE_22 22
#define EXC_NONE_23 23
#define EXC_NONE_24 24
#define EXC_NONE_25 25
#define EXC_NONE_26 26
#define EXC_NONE_27 27
#define EXC_NONE_28 28
#define EXC_NONE_29 29
#define EXC_SECURITY 30
#define EXC_NONE_31 31

#define IRQ_TIMER (PICM_OFFSET)
#define IRQ_KEYBOARD (PICM_OFFSET + 1)
#define IRQ_SERIAL_PORT_2 (PICM_OFFSET + 3)
#define IRQ_SERIAL_PORT_1 (PICM_OFFSET + 4)
#define IRQ_PARALLEL_PORT_2_3 (PICM_OFFSET + 5)
#define IRQ_FLOPPY_DISK (PICM_OFFSET + 6)
#define IRQ_PARALLEL_PORT_1 (PICM_OFFSET + 7)

#define IRQ_CLOCK (PICS_OFFSET)
#define IRQ_ACPI (PICS_OFFSET + 1)
#define IRQ_NONE_1 (PICS_OFFSET + 2)
#define IRQ_NONE_2 (PICS_OFFSET + 3)
#define IRQ_MOUSE (PICS_OFFSET + 4)
#define IRQ_CO_PROCESSOR (PICS_OFFSET + 5)
#define IRQ_PRIMARY_ATA (PICS_OFFSET + 6)
#define IRQ_SECONDARY_ATA (PICS_OFFSET + 7)

#define INT_SCHED 128


struct idt_e
{
	u16 p1;
	u16 gdt_select;
	u8 ist;
	u8 attr;
	u16 p2;
	u32 p3;
	u32 zero;
} __attribute__ ((packed));

struct idt_pointer
{
	u16 size;
	struct idt_e *address;
} __attribute__ ((packed));

struct int_data
{
	u64 rip;
	u64 cs;
	u64 rflags;
	u64 rsp;
	u64 ss;
} __attribute__ ((packed));

#ifdef x64
typedef u64 error_code_t;
#endif

typedef sched::registers *(*int_handler_t) (struct int_data *, error_code_t, sched::registers *);

enum int_handler_type
{
	normal,
	first,
	reg
};


// load the idt
void idt_init (void);

bool reg_int_handler (u8 vec, int_handler_t handler, int_handler_type type);
void rem_int_handler (u8 vec, int_handler_t handler);

// remap pic controllers
// offsets must be multiples of 8
void pic_remap (u8 m_offset, u8 s_offset);

// tell pics to start sending irqs again
extern "C" void pic_eoi (u8 irq);
