#include <int.hpp>
#include <types.hpp>
#include <arch/x64/special.hpp>
#include <util/misc.hpp>
#include <sched/thread.hpp>


#define make_int_handler(vec)					\
__attribute__ ((interrupt))					\
void int_handler_##vec (struct int_data	*data)			\
{								\
	for (u8 i = 0; i < MAX_HANDLERS; i ++)			\
	{							\
		if (int_handlers[vec][i] != NULL)		\
		{						\
			int_handlers[vec][i] (data, 0, NULL);	\
		}						\
	}							\
}

#define make_int_handler_e(vec)						\
__attribute__ ((interrupt))						\
void int_handler_##vec (struct int_data *data, error_code_t error)	\
{									\
	for (u8 i = 0; i < MAX_HANDLERS; i ++)				\
	{								\
		if (int_handlers[vec][i] != NULL)			\
		{							\
			int_handlers[vec][i] (data, error, NULL);	\
		}							\
	}								\
}

#define make_irq_handler(vec)					\
__attribute__ ((interrupt))					\
void int_handler_##vec (struct int_data	*data)			\
{								\
	for (u8 i = 0; i < MAX_HANDLERS; i ++)			\
	{							\
		if (int_handlers[vec][i] != NULL)		\
		{						\
			int_handlers[vec][i] (data, 0, NULL);	\
		}						\
	}							\
	pic_eoi (vec);						\
}

#define make_c_int_handler(vec)							\
extern "C" void int_handler_##vec (void); /* for reg_b_handler */			\
extern "C" sched::registers *c_int_handler_##vec (struct int_data *data, error_code_t error, sched::registers *regs)	\
{										\
	sched::registers *out = NULL;						\
	for (u8 i = 0; i < MAX_HANDLERS; i ++)					\
	{									\
		if (int_handlers[vec][i] != NULL)				\
		{								\
			if (i == MAX_HANDLERS - 1)				\
				out = int_handlers[vec][0] (data, error, regs);	\
			else							\
				int_handlers[vec][i] (data, error, regs);	\
		}								\
	}									\
	return out;								\
}

// return to next instruction
#define reg_b_handler(vec)					\
idt_add_entry (vec, (void *) int_handler_##vec, 0b10001110)	\

// return to current instruction
#define reg_b_handler_trap(vec)					\
idt_add_entry (vec, (void *) int_handler_##vec, 0b10001111)	\


static struct idt_e idt[256];
int_handler_t int_handlers[256][MAX_HANDLERS];


void idt_init (void);
static void idt_add_entry (u8 vec, void *func, u8 attr);
bool reg_int_handler (u8 vec, int_handler_t handler, int_handler_type type);
void rem_int_handler (u8 vec, int_handler_t handler);


make_int_handler(0)
make_int_handler(1)
make_int_handler(2)
make_int_handler(3)
make_int_handler(4)
make_int_handler(5)
make_int_handler(6)
make_int_handler(7)
make_int_handler_e(8)
make_int_handler(9)
make_int_handler_e(10)
make_int_handler_e(11)
make_int_handler_e(12)
make_int_handler_e(13)
make_int_handler_e(14)
make_int_handler(15)
make_int_handler(16)
make_int_handler_e(17)
make_int_handler(18)
make_int_handler(19)
make_int_handler(20)
make_int_handler(21)
make_int_handler(22)
make_int_handler(23)
make_int_handler(24)
make_int_handler(25)
make_int_handler(26)
make_int_handler(27)
make_int_handler(28)
make_int_handler(29)
make_int_handler_e(30)
make_int_handler(31)
make_c_int_handler(32)
make_irq_handler(33)
make_irq_handler(34)
make_irq_handler(35)
make_irq_handler(36)
make_irq_handler(37)
make_irq_handler(38)
make_irq_handler(39)
make_irq_handler(40)
make_irq_handler(41)
make_irq_handler(42)
make_irq_handler(43)
make_irq_handler(44)
make_irq_handler(45)
make_irq_handler(46)
make_irq_handler(47)
make_c_int_handler(128) 


// have to do it like this since no memory allocation yet
void idt_init (void)
{
	struct idt_pointer idt_p = { .size = sizeof (idt) - 1, .address = idt/*(struct idt_e *) (0x3c011b0a0)*/ };
	asm_load_idt (&idt_p);

	// these are mostly wrong
	reg_b_handler(0);
	reg_b_handler(1);
	reg_b_handler(2);
	reg_b_handler(3);
	reg_b_handler(4);
	reg_b_handler(5);
	reg_b_handler(6);
	reg_b_handler(7);
	reg_b_handler(8);
	reg_b_handler(9);
	reg_b_handler(10);
	reg_b_handler(11);
	reg_b_handler(12);
	reg_b_handler(13);
	reg_b_handler(14);
	reg_b_handler(15);
	reg_b_handler(16);
	reg_b_handler_trap(17);
	reg_b_handler(18);
	reg_b_handler(19);
	reg_b_handler(20);
	reg_b_handler(21);
	reg_b_handler(22);
	reg_b_handler(23);
	reg_b_handler(24);
	reg_b_handler(25);
	reg_b_handler(26);
	reg_b_handler(27);
	reg_b_handler(28);
	reg_b_handler(29);
	reg_b_handler(30);
	reg_b_handler(31);
	reg_b_handler(32);
	reg_b_handler(33);
	reg_b_handler(34);
	reg_b_handler(35);
	reg_b_handler(36);
	reg_b_handler(37);
	reg_b_handler(38);
	reg_b_handler(39);
	reg_b_handler(40);
	reg_b_handler(41);
	reg_b_handler(42);
	reg_b_handler(43);
	reg_b_handler(44);
	reg_b_handler(45);
	reg_b_handler(46);
	reg_b_handler(47);
	reg_b_handler(128);
}

static void idt_add_entry (u8 vec, void *func, u8 attr)
{
	idt[vec].p1 = get_bits ((usize) func, 0, 15);
	idt[vec].gdt_select = 8;
	idt[vec].ist = 0;
	idt[vec].attr = attr;
	idt[vec].p2 = get_bits ((usize) func, 16, 31);
	idt[vec].p3 = get_bits ((usize) func, 32, 63);
	idt[vec].zero = 0;
}

// maybe later ill check which ones actually have an error code
bool reg_int_handler (u8 vec, int_handler_t handler, int_handler_type type)
{
	if (type == int_handler_type::first)
	{
		if (int_handlers[vec][0] != NULL)
			return false;
		int_handlers[vec][0] = handler;
		return true;
	}

	if (type == int_handler_type::reg)
	{
		if (int_handlers[vec][MAX_HANDLERS - 1] != NULL)
			return false;
		int_handlers[vec][MAX_HANDLERS - 1] = handler;
		return true;
	}

	u8 i = 1;
	for (; int_handlers[vec][i] != NULL; i ++)
	{
		if (i >= MAX_HANDLERS - 1)
			return false;
	}

	int_handlers[vec][i] = handler;
	return true;
}

void rem_int_handler (u8 vec, int_handler_t handler)
{
	for (u8 i = 0; i < MAX_HANDLERS; i ++)
	{
		if (int_handlers[vec][i] == handler)
		{
			int_handlers[vec][i] = NULL;
		}
	}
}
