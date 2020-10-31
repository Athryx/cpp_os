#include <syscall.hpp>
#include <types.hpp>
#include <arch/x64/special.hpp>
#include <util/io.hpp>
#include <util/misc.hpp>
#include <mem/mem.hpp>
#include <sched/thread.hpp>


#define SYSCALL_STACK_LEN 32678


void *syscalls[16] = {NULL,
NULL,
NULL,
(void *) sched::sys_thread_new,
(void *) sched::sys_thread_block
};
// start address of syscall_stack
usize syscall_stack;
// current pointer to safe syscall rsp value
// this value should be 16 byte aligned
usize syscall_sp;


extern "C" void syscall_entry (void);


void call::init (void)
{
	// enable syscall instruction
	u64 efer = rdmsr (EFER_MSR) | 1;
	wrmsr (EFER_MSR, efer);

	// set syscall entry point
	wrmsr (LSTAR_MSR, (u64) syscall_entry);

	// tell cpu to disabke interrupts on syscall
	wrmsr (FMASK_MSR, 0b1000000000);

	// load correct cs and ss values after syscall and sysret
	wrmsr (STAR_MSR, 0x0013000800000000);

	syscall_stack = (usize) mem::alloc (SYSCALL_STACK_LEN);
	if (syscall_stack == 0)
	{
		panic ("Could not allocate syscall stack");
	}

	syscall_sp = syscall_stack + SYSCALL_STACK_LEN;
}

void call::update_stack (usize rsp)
{
	if (rsp >= syscall_stack && rsp < syscall_sp)
		syscall_sp = align_down (rsp, 16);
}
