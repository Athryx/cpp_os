// this header file contains functions that map directly to the kernel syscalls
// see epoch_kernel_syscalls for documentation on these functions
#pragma once


#include <types>


// usze extended arguments
#define __SYS_EARGS 1

#define __SYS_BLOCK_YIELD 1
#define __SYS_BLOCK_DESTROY 1
#define __SYS_BLOCK_SLEEP 1


// this is an assembly function to call the syscalls
//extern "C" usize syscall (usize num, usize arg1, usize arg2, usize arg3, usize arg4, usize arg5, usize arg6, usize arg7, usize arg8, usize arg9, usize arg10);
extern "C" usize syscall (usize num, ...);


namespace sys
{
	typedef void (*thread_func_t) ();


	bool thread_new (thread_func_t func);
	void thread_block (usize reason, usize nsec);

	void exit (usize code);

	usize create_semaphore (usize n);
	bool delete_semaphore (usize id);
	void semaphore_lock (usize id);
	bool semaphore_try_lock (usize id);
	void semaphore_unlock (usize id);
}
