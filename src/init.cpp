#include <init.hpp>
#include <types.hpp>
#include <gdt.hpp>
#include <int.hpp>
#include <util/io.hpp>
#include <driver/kbd.hpp>
#include <driver/tty.hpp>
#include <util/time.hpp>
#include <arch/x64/common.hpp>
#include <syscall.hpp>
#include <mem/mem.hpp>
#include <mem/kmem.hpp>
#include <sched/process.hpp>
#include <sched/thread.hpp>


static void double_fault (struct int_data* data, error_code_t error_code);
static void kbd_p (struct int_data* data, error_code_t error_code);
u8 init (void *mb2_table);


static sched::registers *double_fault (struct int_data* data, error_code_t error_code, sched::registers *regs)
{
	kprintf ("double fault\n");
	return NULL;
}

static sched::registers *page_fault (struct int_data* data, error_code_t error_code, sched::registers *regs)
{
	panic ("page fault");
	return NULL;
}

static sched::registers *kbd_p (struct int_data* data, error_code_t error_code, sched::registers *regs)
{
	char s[32];
	memset (s, 32, 0);
	if (kbd_read (s, 32))
		kprintf ("%s", s);
	return NULL;
}

u8 init (void *mb2_table)
{
	vga_clear ();

	gdt_init ();

	syscall_init ();

	// remap pics
	pic_remap (PICM_OFFSET, PICM_OFFSET);

	idt_init ();

	time_init ();

	reg_int_handler (EXC_DOUBLE_FAULT, double_fault, int_handler_type::first);
	reg_int_handler (EXC_PAGE_FAULT, page_fault, int_handler_type::normal);

	// mem::init will move mb2_table
	mb2_table = mem::init (mb2_table);

	mem::kmem_init ();

	sched::proc_init ();
	sched::init ();

	// temporary
	kbd_init ();
	reg_int_handler (IRQ_KEYBOARD, kbd_p, int_handler_type::normal);

	return 1;
}
