#ifndef __GDT_H__
#define __GDT_H__


#include <types.h>


#define GDT_SIZE 5


struct gdt_e
{
	u16 limit_1;
	u16 base_1;
	u8 base_2;
	u8 access;
	/* access bits:
	0: accesed bit, set by cpu
	1: read write bits
	2: direction control bit, best to not touch
	3: 1 if code selectos, 0 if data selector
	4: 1 for code, data segments, 0 for others
	5, 6: privalage
	7: present bit
	*/
	u8 limit_2: 4;
	u8 flags: 4;
	/* flags bits:
	0: unused
	1: long mode bit
	2: 0 in long mode and 16 bit mode, 1 in 32 bit mode
	3: granularity, 0 for bytes, 1 for pages
	*/
	u8 base_3;
} __attribute__ ((packed));

struct tss_e
{
	u16 limit_1; // length of tss
	u16 base_1;
	u8 base_2;
	u8 acces;
	u8 limit_2: 4;
	u8 flags: 4;
	u8 base_3;
	u32 base_4;
	u32 zero;
} __attribute__ ((packed));

struct tss
{
	u32 zero_1;
	u64 rsp0;
	u64 rsp1;
	u64 rsp2;
	u64 zero_2;
	u64 ist1;
	u64 ist2;
	u64 ist3;
	u64 ist4;
	u64 ist5;
	u64 ist6;
	u64 ist7;
	u64 zero_3;
	u16 zero_4;
	u16 iomap;
} __attribute__ ((packed));

struct gdt
{
	struct gdt_e entries[GDT_SIZE];
	struct tss_e tss;
} __attribute__ ((packed));

struct gdt_pointer
{
	u16 size;
	void *address;
} __attribute__ ((packed));

extern struct gdt gdt;
extern struct tss tss;


void gdt_init (void);


#endif
