#pragma once


#include <gdt.hpp>
#include <int.hpp>
#include <types.hpp>


#define EFER_MSR 0xc0000080
#define STAR_MSR 0xc0000081
#define LSTAR_MSR 0xc0000082
#define RSTAR_MSR 0xc0000083
#define FMASK_MSR 0xc0000084

#define FSBASE_MSR 0xC0000100
#define GSBASE_MSR 0xC0000101
#define GSBASEK_MSR 0xC0000102

// allows execute disable in pages
#define EFER_NXE (1 << 11)


#define swapgs() asm volatile ("swapgs")


extern "C" void asm_load_gdt (struct gdt_pointer *pointer);

extern "C" void asm_load_idt (struct idt_pointer *pointer);

extern "C" void asm_load_tss (u16 tss_vector);

extern "C" u64 rdmsr (u32 msr);
extern "C" void wrmsr (u32 msr, u64 data);

extern "C" void load_data_segments (u16 n);

extern "C" void syscall_test (u32 syscall);
