#include <gdt.hpp>
#include <types.hpp>
#include <arch/x64/special.hpp>
#include <util/misc.hpp>


struct gdt gdt;
struct tss tss;


static void gdt_add_entry (u8 index, u32 base, u32 limit, u8 access, u8 flags);
void gdt_init (void);


static void gdt_add_entry (u8 index, u32 base, u32 limit, u8 access, u8 flags)
{
	gdt.entries[index].base_1 = get_bits (base, 0, 15);
	gdt.entries[index].base_2 = get_bits (base, 16, 23);
	gdt.entries[index].base_3 = get_bits (base, 24, 31);
	gdt.entries[index].limit_1 = get_bits (limit, 0, 15);
	gdt.entries[index].limit_2 = get_bits (limit, 16, 19);
	gdt.entries[index].access = access;
	gdt.entries[index].flags = 0x0f & flags;
}

void gdt_init (void)
{
	gdt_add_entry (0, 0, 0, 0, 0);
	// kernel land code segment
	gdt_add_entry (1, 0, 0xffffffff, 0x9a, 0x0a);
	// kenal land data segment
	gdt_add_entry (2, 0, 0xffffffff, 0x92, 0x0a);
	// user land data segment
	gdt_add_entry (3, 0, 0xffffffff, 0xf2, 0x0a);
	// user land code segment
	gdt_add_entry (4, 0, 0xffffffff, 0xfa, 0x0a);

	// tss
	usize tss_addr = (usize) &tss;
	gdt.tss.base_1 = get_bits (tss_addr, 0, 15);
	gdt.tss.base_2 = get_bits (tss_addr, 16, 23);
	gdt.tss.base_3 = get_bits (tss_addr, 24, 31);
	gdt.tss.base_4 = get_bits (tss_addr, 32, 63);
	gdt.tss.limit_1 = 104; // length of tss
	gdt.tss.limit_2 = 0;
	gdt.tss.acces = 0x89; // probably wrong
	gdt.tss.flags = 0;

	// needed, according to osdev wiki
	tss.iomap = sizeof (struct tss);

	struct gdt_pointer gdt_p = { .size = sizeof (gdt) - 1, .address = &gdt };
	asm_load_gdt (&gdt_p);

	asm_load_tss (0x28); // maybe wrong

	load_data_segments (16);
}
