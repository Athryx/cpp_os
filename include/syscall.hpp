#pragma once


#include <types.hpp>


typedef struct __attribute__((packed))
{
	u64 rip;
	u64 rsp;
	u64 rflags;
} syscall_vals_t;


namespace call
{
	// sycall functions have the signature usize sys_func (syscall_vals_t *vals, u32 options, usize args1 - usize args10)
	void init (void);

	// pass in rsp, if it is in syscall stack unused zone, update syscall_stack_pointer
	// TODO: make stack regrowable
	//void update_stack (usize rsp);
}
