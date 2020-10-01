#ifndef __DRIVER_TTY_H__
#define __DRIVER_TTY_H__


#include <types.h>


typedef u8 color_t;

enum vga_color
{
	VGAC_BLACK = 0,
	VGAC_BLUE = 1,
	VGAC_GREEN = 2,
	VGAC_CYAN = 3,
	VGAC_RED = 4,
	VGAC_MAGENTA = 5,
	VGAC_BROWN = 6,
	VGAC_L_GREY = 7,
	VGAC_GREY = 8,
	VGAC_L_BLUE = 9,
	VGAC_L_GREEN = 10,
	VGAC_L_CYAN = 11,
	VGAC_L_RED = 12,
	VGAC_L_MAGENTA = 13,
	VGAC_L_BROWN = 14,
	VGAC_WHITE = 15,
};


color_t vga_form_color (enum vga_color back_c, enum vga_color text_c);
void vga_clear (void);
void vga_putc (char in);
bool vga_write (const char *in);
bool vga_append (const char *in);


#endif
