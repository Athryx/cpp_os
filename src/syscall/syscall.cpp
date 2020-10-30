#include <syscall.hpp>
#include <types.hpp>
#include <arch/x64/special.hpp>

#include <util/io.hpp>


//typedef usize (*syscall_func_t) (usize, usize, usize, usize, usize, usize, u32);


void *syscalls[16] = { (void *) sys_hi };


void syscall_init (void);

extern "C" void syscall_entry (void);


void syscall_init (void)
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
}

void sys_hi (void)
{
	kprintf ("hi userspace!\n");
}
