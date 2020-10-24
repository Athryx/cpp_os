#include <util/time.hpp>
#include <types.hpp>
#include <int.hpp>
#include <arch/x64/common.hpp>
#include <util/misc.hpp>
#include <util/io.hpp>
#include <sched/thread.hpp>


#define PIT_INTERRUPT_TERMINAL_COUNT 0
#define PIT_ONE_SHOT 1
#define PIT_RATE_GENERATOR 2
#define PIT_SQUARE_WAVE 3
#define PIT_SOFTWARE_STROBE 4
#define PIT_HARDWARE_STROBE 5

#define PIT_CHANNEL_0 0x40
#define PIT_CHANNEL_1 0x41
#define PIT_CHANNEL_2 0x42
#define PIT_COMMAND 0x43

#define NANOSEC_PER_CLOCK 838


static struct
{
	// elapsed time since boot in nanoseconds
	u64 elapsed_time;
	// value pit counter resets to
	u16 reset;
	// nanoseconds per reset
	u64 nano_reset;
} pit_data;


static void timer_irq_handler (struct int_data *data, error_code_t);
void time_init (void);
static void set_pit_reset (u16 value);
u64 time_nsec (void);
u64 time_sec (void);
u64 time_nsec_nolatch (void);
u64 time_sec_nolatch (void);


static sched::registers *timer_irq_handler (struct int_data *data, error_code_t error_code, sched::registers *regs)
{
	pit_data.elapsed_time += pit_data.nano_reset;
	return NULL;
}

void time_init (void)
{
	// maximum time between
	set_pit_reset (0xffff);
	if (!reg_int_handler (IRQ_TIMER, timer_irq_handler, true))
	{
		panic ("couldn't register pit irq handler");
	}
}

static void set_pit_reset (u16 reset)
{
	// channel 0, low - high byte, square wave mode, 16 bit binary
	outb (PIT_COMMAND, 0b00110110);
	io_wait ();
	outb (PIT_CHANNEL_0, get_bits (reset, 0, 7));
	io_wait ();
	outb (PIT_CHANNEL_0, get_bits (reset, 8, 15));
	pit_data.reset = reset;
	pit_data.nano_reset = NANOSEC_PER_CLOCK * reset;
}

u64 time_nsec (void)
{
	// lock the latch
	outb (PIT_COMMAND, 0b00000000);
	io_wait ();
	u8 low = inb (PIT_CHANNEL_0);
	io_wait ();
	u8 high = inb (PIT_CHANNEL_0);
	return pit_data.elapsed_time + (NANOSEC_PER_CLOCK * (high << 8 | low));
}

u64 time_sec (void)
{
	return (time_nsec () + 500000000) / 1000000000;
}

u64 time_nsec_nolatch (void)
{
	return pit_data.elapsed_time;
}

u64 time_sec_nolatch (void)
{
	return (time_nsec_nolatch () + 500000000) / 1000000000;
}
