#pragma once


#include <types.hpp>


#define sti() asm volatile ("sti")
#define cli() asm volatile ("cli")
#define hlt() asm volatile ("hlt")


// output to port
extern "C" void outb (u16 port, u8 data);
extern "C" void outw (u16 port, u16 data);
extern "C" void outd (u16 port, u32 data);

// get input from port
extern "C" u8 inb (u16 port);
extern "C" u16 inw (u16 port);
extern "C" u32 ind (u16 port);

// wait for a bit
extern "C" void io_wait (void);

extern "C" usize get_rsp (void);

// TODO: move these to special
// cr8 has some things as well, but it doesn't sound very useful
// cpu control flags
extern "C" usize get_cr0 (void);
// page fault linear address
extern "C" usize get_cr2 (void);
// pml4 table pointer
extern "C" usize get_cr3 (void);
// more cpu control flags
extern "C" usize get_cr4 (void);

extern "C" void set_cr0 (usize val);
extern "C" void set_cr2 (usize val);
extern "C" void set_cr3 (usize val);
extern "C" void set_cr4 (usize val);

void sti_safe (void);
void cli_safe (void);
