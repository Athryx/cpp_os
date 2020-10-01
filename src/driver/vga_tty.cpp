#include <driver/tty.h>
#include <types.h>
#include <def/keys.h>
#include <util/string.h>


#define VGA_ROWS 25 // height
#define VGA_COLS 80 // width


#define geti() (row * VGA_COLS + col)
#define vga_form_entry(color, c) ((u16) (((u8) color << 8) | (u8) c))


enum
{
	READ_CHAR,
	READ_COLOR
} next_action;


static volatile u16 *vga_buf = (u16 *) 0xffffff80000b8000;
static i8 row = 0, col = 0;
// store index of last character in each column
static i8 coli[VGA_ROWS];

// current print color
static color_t vga_color = (VGAC_BLACK << 4) | VGAC_WHITE;
// empty char
static color_t vga_black = (VGAC_BLACK << 4) | VGAC_BLACK;


// helper functions
static void vga_scroll_down (u8 rows);
static void vga_inc_col (u16 n);
static void vga_inc_row (u16 n);

color_t vga_form_color (enum vga_color back_c, enum vga_color text_c);
void vga_clear (void);
void vga_putc (char in);
bool vga_write (const char *in);
bool vga_append (const char *in);


static void vga_scroll_down (u8 rows)
{
	for (u8 r = 0; r < VGA_ROWS - rows; r ++)
	{
		for (u8 c = 0; c < VGA_COLS; c ++)
		{
			usize i = r * VGA_COLS + c;
			vga_buf[i] = vga_buf[rows * VGA_COLS + i];
		}
		coli[r] = coli[r + rows];
	}
	for (u8 r = VGA_ROWS - rows; r < VGA_ROWS; r ++)
	{
		for (u8 c = 0; c < VGA_COLS; c ++)
		{
			vga_buf[r * VGA_COLS + c] = vga_black;
		}
		coli[r] = 0;
	}
}

static void vga_inc_col (u16 n)
{
	col += n;
	if (col >= VGA_COLS)
	{
		vga_inc_row (col / VGA_COLS);
		col %= VGA_COLS;
	}

}

static void vga_inc_row (u16 n)
{
	row += n;
	if (row >= VGA_ROWS)
	{
		u8 temp = row - VGA_ROWS + 1;
		vga_scroll_down (temp);
		row -= temp;
	}
}

color_t vga_form_color (enum vga_color back_c, enum vga_color text_c)
{
	return (color_t) (text_c | (back_c << 4));
}

void vga_clear (void)
{
	u16 len = VGA_ROWS * VGA_COLS;
	for (u16 i = 0; i < len; i ++)
	{
		vga_buf[i] = vga_black;
	}
	row = 0;
	col = 0;
}

void vga_putc (char in)
{
	switch (in)
	{
		case '\n':
			vga_inc_row (1);
			col = 0;
			break;
		default:
			vga_buf[geti()] = vga_form_entry(vga_color, in);
			vga_inc_col (1);
	}
}

bool vga_write (const char *s)
{
	vga_clear ();
	return vga_append (s);
}

bool vga_append (const char *s)
{
	const unsigned char *in = (const unsigned char *) s;
	usize len = strlen (s);
	for (usize i = 0; i < len; i ++)
	{
		switch (in[i])
		{
			case TTY_SET_COLOR:
				i ++;
				vga_color = in[i];
				break;
			case TTY_SET_POSITION:
				i ++;
				if (in[i] >= VGA_COLS)
					return false;
				col = in[i];
				i ++;
				if (in[i] >= VGA_ROWS)
					return false;
				row = in[i];
				break;
			default:
				vga_putc (in[i]);
				break;
		}
	}
	return true;
}
