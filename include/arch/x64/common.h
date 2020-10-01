#ifndef __X64_COMMON_H__
#define __X64_COMMON_H__


#include <types.h>


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

extern "C" void int_1 (void);

// wait for a bit
extern "C" void io_wait (void);

extern "C" usize get_rsp (void);

void sti_safe (void);
void cli_safe (void);


#endif
