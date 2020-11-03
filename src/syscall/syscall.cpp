#include <syscall.hpp>
#include <types.hpp>
#include <arch/x64/special.hpp>
#include <util/io.hpp>
#include <util/misc.hpp>
#include <mem/mem.hpp>
#include <sched/process.hpp>
#include <sched/thread.hpp>


#define SYSCALL_STACK_LEN 32678


// temp
void hi (void)
{
	kprintf ("hi :)\n");
}


void *syscalls[16] = {
(void *) hi,
NULL,
(void *) sched::sys_thread_new,
(void *) sched::sys_thread_block,
NULL,
(void *) sched::sys_create_semaphore,
(void *) sched::sys_delete_semaphore,
(void *) sched::sys_semaphore_lock,
(void *) sched::sys_semaphore_try_lock,
(void *) sched::sys_semaphore_unlock,
(void *) mem::sys_realloc
};


extern "C" void syscall_entry (void);


void call::init (void)
{
	// enable syscall instruction
	u64 efer = rdmsr (EFER_MSR) | 1;
	wrmsr (EFER_MSR, efer);

	// set syscall entry point
	wrmsr (LSTAR_MSR, (u64) syscall_entry);

	// tell cpu to disable interrupts on syscall
	wrmsr (FMASK_MSR, 0b1000000000);

	// load correct cs and ss values after syscall and sysret
	wrmsr (STAR_MSR, 0x0013000800000000);
}
