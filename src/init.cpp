#include <init.h>
#include <types.h>
#include <gdt.h>
#include <int.h>
#include <util/io.h>
#include <driver/kbd.h>
#include <driver/tty.h>
#include <util/time.h>
#include <arch/x64/common.h>
#include <syscall.h>


static void double_fault (struct int_data* data, error_code_t error_code);
static void kbd_p (struct int_data* data, error_code_t error_code);
u8 init (void);

void test_cs (u16 in);
void test_int (void);


static void double_fault (struct int_data* data, error_code_t error_code)
{
	kprintf ("double fault\n");
}

static void page_fault (struct int_data* data, error_code_t error_code)
{
	kprintf ("page fault\n");
}

static void kbd_p (struct int_data* data, error_code_t error_code)
{
	char s[32];
	memset (s, 32, 0);
	if (kbd_read (s, 32))
		kprintf ("%s", s);
}

u8 init (void)
{
	gdt_init ();

	syscall_init ();

	vga_clear ();
	// remap pics
	pic_remap (PICM_OFFSET, PICM_OFFSET);

	idt_init ();

	time_init ();

	reg_int_handler (EXC_DOUBLE_FAULT, double_fault, true);
	reg_int_handler (EXC_PAGE_FAULT, page_fault, false);

	// temporary
	kbd_init ();
	reg_int_handler (IRQ_KEYBOARD, kbd_p, false);

	return 1;
}
