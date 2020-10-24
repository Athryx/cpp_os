#include <driver/kbd.hpp>
#include <types.hpp>
#include <int.hpp>
#include <arch/x64/common.hpp>
#include <util/io.hpp>
#include <util/math.hpp>
#include <sched/thread.hpp>
// this is supposed to be a bad driver, it is temporry since drivers will be in userland if I am still working on this by then


#define PS2_DATA 0x60
#define PS2_COMMAND 0x64
#define SCAN_BUF_SIZE 32


struct
{
	u8 shift;
	bool capslock;
	bool numlock;
} kbd_status;

// rotatig buffer of scancodes
static u8 scancode_buf[SCAN_BUF_SIZE];
static u8 scan_i = 0;
static u8 read_i = 0;


static void ps2_int_handler (struct int_data *data, error_code_t error);
u8 kbd_init (void);
u8 kbd_read (char *buf, u8 n);
static u8 update_kbd_status (u8 c);
static char scan_toc (u8 scan);


static sched::registers *ps2_int_handler (struct int_data *data, error_code_t error, sched::registers *regs)
{
	u8 code = inb (PS2_DATA);
	scancode_buf[scan_i] = code;

	if (scan_i == read_i)
		read_i ++;

	scan_i ++;
	scan_i = wrap (scan_i, 0, SCAN_BUF_SIZE - 1);
	read_i = wrap (read_i, 0, SCAN_BUF_SIZE - 1);
	return NULL;
}

u8 kbd_init (void)
{
	reg_int_handler (IRQ_KEYBOARD, ps2_int_handler, true);
	outb (PS2_DATA, 0xf4);
	return 1;
}

u8 kbd_read (char *buf, u8 n)
{
	// probably not good
	static bool f = true;
	u8 c = 0;
	for (;;)
	{
		if (c == n)
			return c;

		char temp = scan_toc (scancode_buf[read_i]);
		if (temp == '\0' && f)
		{
			update_kbd_status (scancode_buf[read_i]);
		}
		else if (f)
		{
			buf[c] = temp;
			c ++;
		}

		if (wrap (read_i + 1, 0, SCAN_BUF_SIZE - 1) == scan_i)
		{
			f = false;
			return c;
		}

		read_i ++;
		if (read_i == SCAN_BUF_SIZE)
			read_i = 0;
		f = true;
	}
	return c;
}

static u8 update_kbd_status (u8 c)
{
	switch (c)
	{
		case 0x3a:
			kbd_status.capslock = !kbd_status.capslock;
			break;
		case 0x2a:
		case 0x36:
			kbd_status.shift ++;
			kbd_status.shift = clamp (kbd_status.shift, 0, 2);
			break;
		case 0xaa:
		case 0xb6:
			kbd_status.shift --;
			kbd_status.shift = clamp (kbd_status.shift, 0, 2);
			break;
		case 0x45:
			kbd_status.numlock = !kbd_status.numlock;
			break;
		default:
			return 0;
	}
	return 1;
}

static char scan_toc (u8 scan)
{
	char out = '\0';
	switch (scan)
	{
		case 0x01:
			out = '\e';
			break;
		case 0x02:
			out = kbd_status.shift ? '!' : '1';
			break;
		case 0x03:
			out = kbd_status.shift ? '@' : '2';
			break;
		case 0x04:
			out = kbd_status.shift ? '#' : '3';
			break;
		case 0x05:
			out = kbd_status.shift ? '$' : '4';
			break;
		case 0x06:
			out = kbd_status.shift ? '%' : '5';
			break;
		case 0x07:
			out = kbd_status.shift ? '^' : '6';
			break;
		case 0x08:
			out = kbd_status.shift ? '&' : '7';
			break;
		case 0x09:
			out = kbd_status.shift ? '*' : '8';
			break;
		case 0x0a:
			out = kbd_status.shift ? '(' : '9';
			break;
		case 0x0b:
			out = kbd_status.shift ? ')' : '0';
			break;
		case 0x0c:
			out = kbd_status.shift ? '_' : '-';
			break;
		case 0x0d:
			out = kbd_status.shift ? '+' : '=';
			break;
		case 0x0e:
			out = '\b';
			break;
		case 0x0f:
			out = '\t';
			break;
		case 0x10:
			out = kbd_status.shift ? 'Q' : 'q';
			break;
		case 0x11:
			out = kbd_status.shift ? 'W' : 'w';
			break;
		case 0x12:
			out = kbd_status.shift ? 'E' : 'e';
			break;
		case 0x13:
			out = kbd_status.shift ? 'R' : 'r';
			break;
		case 0x14:
			out = kbd_status.shift ? 'T' : 't';
			break;
		case 0x15:
			out = kbd_status.shift ? 'Y' : 'y';
			break;
		case 0x16:
			out = kbd_status.shift ? 'U' : 'u';
			break;
		case 0x17:
			out = kbd_status.shift ? 'I' : 'i';
			break;
		case 0x18:
			out = kbd_status.shift ? 'O' : 'o';
			break;
		case 0x19:
			out = kbd_status.shift ? 'P' : 'p';
			break;
		case 0x1a:
			out = kbd_status.shift ? '{' : '[';
			break;
		case 0x1b:
			out = kbd_status.shift ? '}' : ']';
			break;
		case 0x1c:
			out = '\n';
			break;
		/* left control
		case 0x1d:
			out = '1';
			break;*/
		case 0x1e:
			out = kbd_status.shift ? 'A' : 'a';
			break;
		case 0x1f:
			out = kbd_status.shift ? 'S' : 's';
			break;
		case 0x20:
			out = kbd_status.shift ? 'D' : 'd';
			break;
		case 0x21:
			out = kbd_status.shift ? 'F' : 'f';
			break;
		case 0x22:
			out = kbd_status.shift ? 'G' : 'g';
			break;
		case 0x23:
			out = kbd_status.shift ? 'H' : 'h';
			break;
		case 0x24:
			out = kbd_status.shift ? 'J' : 'j';
			break;
		case 0x25:
			out = kbd_status.shift ? 'K' : 'k';
			break;
		case 0x26:
			out = kbd_status.shift ? 'L' : 'l';
			break;
		case 0x27:
			out = kbd_status.shift ? ':' : ';';
			break;
		case 0x28:
			out = kbd_status.shift ? '"' : '\'';
			break;
		case 0x29:
			out = kbd_status.shift ? '~' : '`';
			break;
		/* left shift
		case 0x2a:
			out = '1';
			break;*/
		case 0x2b:
			out = kbd_status.shift ? '|' : '\\';
			break;
		case 0x2c:
			out = kbd_status.shift ? 'Z' : 'z';
			break;
		case 0x2d:
			out = kbd_status.shift ? 'X' : 'x';
			break;
		case 0x2e:
			out = kbd_status.shift ? 'C' : 'c';
			break;
		case 0x2f:
			out = kbd_status.shift ? 'V' : 'v';
			break;
		case 0x30:
			out = kbd_status.shift ? 'B' : 'b';
			break;
		case 0x31:
			out = kbd_status.shift ? 'N' : 'n';
			break;
		case 0x32:
			out = kbd_status.shift ? 'M' : 'm';
			break;
		case 0x33:
			out = kbd_status.shift ? '<' : ',';
			break;
		case 0x34:
			out = kbd_status.shift ? '>' : '.';
			break;
		case 0x35:
			out = kbd_status.shift ? '?' : '/';
			break;
		/* right shift
		case 0x36:
			out = 'v';
			break;*/
		case 0x37:
			out = '*';
			break;
		/* left alt
		case 0x38:
			out = 'v';
			break;*/
		case 0x39:
			out = ' ';
			break;
		/* caps lock
		case 0x3a:
			out = 'v';
			break;*/
		/* f1-f10
		case 0x3b:
			out = 'v';
			break;
		case 0x3c:
			out = 'v';
			break;
		case 0x3d:
			out = 'v';
			break;
		case 0x3e:
			out = 'v';
			break;
		case 0x3f:
			out = 'v';
			break;
		case 0x40:
			out = 'v';
			break;
		case 0x41:
			out = 'v';
			break;
		case 0x42:
			out = 'v';
			break;
		case 0x43:
			out = 'v';
			break;
		case 0x44:
			out = 'v';
			break;*/
		/* num lock
		case 0x45:
			out = 'v';
			break;*/
		/* scroll lock
		case 0x46:
			out = 'v';
			break;*/
		case 0x47:
			out = kbd_status.numlock ? '7' : HOME;
			break;
		case 0x48:
			out = kbd_status.numlock ? '8' : ARROW_UP;
			break;
		case 0x49:
			out = kbd_status.numlock ? '9' : PAGE_UP;
			break;
		case 0x4a:
			out = '-';
			break;
		case 0x4b:
			out = kbd_status.numlock ? '4' : ARROW_LEFT;
			break;
		case 0x4c:
			out = '5';
			break;
		case 0x4d:
			out = kbd_status.numlock ? '6' : ARROW_RIGHT;
			break;
		case 0x4e:
			out = '+';
			break;
		case 0x4f:
			out = kbd_status.numlock ? '1' : END;
			break;
		case 0x50:
			out = kbd_status.numlock ? '2' : ARROW_DOWN;
			break;
		case 0x51:
			out = kbd_status.numlock ? '3' : PAGE_DOWN;
			break;
		case 0x52:
			out = '0';
			break;
		case 0x53:
			out = kbd_status.numlock ? '.' : DELETE;
			break;
		/* f11 - f12
		case 0x57:
			out = '1';
			break;
		case 0x58:
			out = '1';
			break;*/
	}
	if (kbd_status.capslock)
	{
		if (out >= 65 && out <= 90)
		{
			return out + 32;
		}
		else if (out >= 97 && out <= 122)
		{
			return out - 32;
		}
	}
	return out;
}
