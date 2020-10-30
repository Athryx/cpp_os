#include <types.hpp>
#include <init.hpp>
#include <gdt.hpp>
#include <global.hpp>
#include <util/misc.hpp>
#include <arch/x64/common.hpp>

#include <util/io.hpp>
#include <driver/tty.hpp>
#include <util/time.hpp>
#include <arch/x64/special.hpp>
#include <mem/mem.hpp>
#include <sched/process.hpp>
#include <sched/thread.hpp>
#include <sched/sched_def.hpp>


using namespace mem;


void thread_2 (void)
{
	kprintf ("hello thread 2");
}

void test_thread (void)
{
	kprintf ("hello");
	//kthread_new (thread_2);
	kprintf ("hello thread 1");
}

void userspace_thread (void)
{
	kprintf ("hello userspace\n");
	syscall_test (0);
}

extern u64 initfs_len;
extern char initfs[];

void main_thread (void)
{
	cli_safe ();
	kprintf ("initfs_len: %x\n", &initfs_len);
	//test_thread ();
	int *a = (int *) alloc (1);

	kprintf ("hi");

	free (a);

	int *b = (int *) alloc (1);
	int *c = (int *) alloc (1);


	*c = 0xf;
	kprintf ("%x\n", *b);
	free (b);

	int *d = (int *) alloc (1);
	int *e = (int *) alloc (1);

	free (c);
	free (d);
	free (e);

	kprintf ("addresses: a: %x, b: %x, c: %x, d: %x, e: %x\n", a, b, c, d, e);
	// currently first part works

	void *a1 = oalloc (2);
	void *a2 = oalloc (2);
	void *a3 = oalloc (5);
	void *a4 = oalloc (0);
	void *a5 = oalloc (1);
	void *a6 = oalloc (3);

	free (a2);
	free (a2);

	void *a1_old = a1;

	a1 = orealloc (a1, 4);

	free (a3);

	void *a7 = alloc (25);
	a5 = orealloc (a5, 2);

	void *a8 = oalloc (1);

	void *a9 = oalloc (255);

	free (a1_old);
	free (a1);
	free (a4);
	free (a5);
	free (a6);
	free (a7);
	free (a8);
	free (a9);


	/*int *a = (int *) kalloc (1);

	kprintf ("hi");

	kfree (a, 1);

	int *b = (int *) kalloc (1);
	int *c = (int *) kalloc (1);


	*c = 0xf;
	kprintf ("%x\n", *b);
	kfree (b, 1);

	int *d = (int *) kalloc (1);
	int *e = (int *) kalloc (1);

	kfree (c, 1);
	kfree (d, 1);
	kfree (e, 1);

	kprintf ("addresses: a: %x, b: %x, c: %x, d: %x, e: %x\n", a, b, c, d, e);

	kprintf ("hi\n");

	// will fail
	process_new (userspace_thread, NULL);*/
	kprintf ("didn't crash\n");
	cli_safe ();
	asm volatile ("int $32");
}

extern "C" [[ noreturn ]] void _start (void *mb2_table)
{
	// bochs magic breakpoint for debugging
	asm volatile ("xchgw %bx, %bx");

	if (!::init (mb2_table))
	{
		panic ("init failed");
	}


	auto *proc_elf = sched::process::load_elf ((void *) &initfs, (usize) &initfs_len, SUID);
	//sched::thread thread (sched::proc_c (), main_thread);
	//main_thread ();
	// test
	//kthread_new (thread_2);
	//addr_space_t *addr_space = addr_space_new (0b111, 0b111);
	//virt_map (addr_space, kernel_vma, kernel_vma, ADDR_SPACE_LEN_PAGE, addr_space->p_flags);
	//process_new (userspace_thread, addr_space);

	kprintf ("epoch v0.0.1\n");

	// add extra space on rsp incase an interupt occurs at a very unlucky time in sti_safe
	global::sys_rsp[CPU_I] = align_up (get_rsp () + 0x40, 16);
	tss.rsp0 = global::sys_rsp[CPU_I];

	sti_safe ();

	for (;;)
	{
		hlt();
	}
}
