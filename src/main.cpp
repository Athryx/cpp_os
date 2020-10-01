#include <types.h>
#include <init.h>
#include <gdt.h>
#include <global.h>
#include <util/misc.h>
#include <arch/x64/common.h>

#include <util/io.h>
#include <driver/tty.h>
#include <util/time.h>
#include <arch/x64/special.h>


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
	kprintf ("initfs_len: %x\n", &initfs_len);
	kprintf ("initfs: %s\n", initfs);
	//test_thread ();
	/*int *a = alloc (1);

	kprintf ("hi");

	free (a, 1);

	int *b = alloc (1);
	int *c = alloc (1);


	*c = 0xf;
	kprintf ("%x\n", *b);
	free (b, 1);

	int *d = alloc (1);
	int *e = alloc (1);

	free (c, 1);
	free (d, 1);
	free (e, 1);

	kprintf ("addresses: a: %x, b: %x, c: %x, d: %x, e: %x\n", a, b, c, d, e);

	kprintf ("hi\n");*/

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
}

extern "C" [[ noreturn ]] void _start (void)
{
	// bochs magic breakpoint for debugging
	asm volatile ("xchgw %bx, %bx");

	if (!init ())
	{
		panic ("init failed");
	}

	// test
	//kthread_new (thread_2);
	//addr_space_t *addr_space = addr_space_new (0b111, 0b111);
	//virt_map (addr_space, kernel_vma, kernel_vma, ADDR_SPACE_LEN_PAGE, addr_space->p_flags);
	//process_new (userspace_thread, addr_space);

	// add extra space on rsp incase an interupt occurs at a very unlucky time in sti_safe
	global::sys_rsp[CPU_I] = align_up (get_rsp () + 0x40, 16);
	tss.rsp0 = global::sys_rsp[CPU_I];

	sti_safe ();

	for (;;)
	{
		hlt();
	}
}
