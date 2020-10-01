#include <arch/x64/common.h>
#include <types.h>
#include <util/math.h>


// irqs start disabled
static u32 irq_c = 1;


void sti_safe (void)
{
	irq_c = max(irq_c, 1) - 1;
	if (irq_c == 0)
		sti();
}

void cli_safe (void)
{
	irq_c ++;
	cli();
}
