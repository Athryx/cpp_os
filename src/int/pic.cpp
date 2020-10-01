#include <int.hpp>
#include <arch/x64/common.hpp>
#include <types.hpp>


#define PICM_COMMAND 0x20
#define PICM_DATA 0x21
#define PICS_COMMAND 0xa0
#define PICS_DATA 0xa1

// code to tell pics to start sending interrupts again
#define PIC_EOI 0x20

// defines from osdev wiki
#define ICW1_ICW4	0x01		/* ICW4 (not) needed */
#define ICW1_SINGLE	0x02		/* Single (cascade) mode */
#define ICW1_INTERVAL4	0x04		/* Call address interval 4 (8) */
#define ICW1_LEVEL	0x08		/* Level triggered (edge) mode */
#define ICW1_INIT	0x10		/* Initialization - required! */

#define ICW4_8086	0x01		/* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO	0x02		/* Auto (normal) EOI */
#define ICW4_BUF_SLAVE	0x08		/* Buffered mode/slave */
#define ICW4_BUF_MASTER	0x0C		/* Buffered mode/master */
#define ICW4_SFNM	0x10		/* Special fully nested (not) */


void pic_remap (u8 m_offset, u8 s_offset);
void pic_eoi (u8 irq);


// from osdev wiki
// remap irqs to desired idt entry
// offsets must be multiple of 8
void pic_remap (u8 m_offset, u8 s_offset)
{
	// save masks
	u8 s1 = inb (PICM_DATA);
	u8 s2 = inb (PICS_DATA);

	// tell pics its time to remap
	outb (PICM_COMMAND, ICW1_INIT | ICW1_ICW4);
	io_wait ();
	outb (PICS_COMMAND, ICW1_INIT | ICW1_ICW4);
	io_wait ();

	// tell them offset
	outb (PICM_DATA, m_offset);
	io_wait ();
	outb (PICS_DATA, s_offset);
	io_wait ();

	// tell master pic it has slave pic chained at pin 2
	outb (PICM_DATA, 0b100);
	io_wait ();
	outb (PICS_DATA, 0b10);
	io_wait ();

	outb (PICM_DATA, ICW4_8086);
	io_wait ();
	outb (PICS_DATA, ICW4_8086);
	io_wait ();

	// restore masks from earlier
	outb (PICM_DATA, s1);
	outb (PICS_DATA, s2);
}

// tell pics irq is over
void pic_eoi (u8 irq)
{
	if (irq - PICM_OFFSET > 7)
		outb (PICS_COMMAND, PIC_EOI);

	outb (PICM_COMMAND, PIC_EOI);
}
