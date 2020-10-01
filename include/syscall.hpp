#pragma once


#include <types.hpp>


typedef struct __attribute__((packed))
{
	u64 rip;
	u64 rsp;
	u64 rflags;
} syscall_vals_t;


void syscall_init (void);

void *sys_realloc (syscall_vals_t *vals, u32 options, usize addr, usize orig, usize snew);
