#include <util/io.hpp>
#include <types.hpp>
#include <def/keys.hpp>
#include <arch/x64/common.hpp>
#include <util/string.hpp>


void kprintf (const char *__restrict__ format, ...)
{
	va_list list;
	va_start(list, format);
	kvprintf (format, list);
	va_end (list);
}

// in the future, print to serial console
void kprinte (const char *__restrict__ format, ...)
{
	va_list list;
	va_start(list, format);
	kvprintf (format, list);
	va_end (list);
}

void kvprintf (const char *__restrict__ format, va_list list)
{
	usize len = strlen (format);
	for (usize i = 0; i < len; i ++)
	{
		if (format[i] == '%')
		{
			i ++;
			char buf[32];
			switch (format[i])
			{
				case 'u':
					itoa (buf, va_arg (list, usize));
					vga_append (buf);
					break;
				case 'x':
					itoa_hex (buf, va_arg (list, u32));
					vga_append ("0x");
					vga_append (buf);
					break;
				case 's':
					vga_append (va_arg (list, char *));
					break;
				case 'c':
					vga_putc (va_arg (list, int));
					break;
				case 'C':
					buf[0] = TTY_SET_COLOR;
					buf[1] = (char) va_arg (list, int);
					buf[2] = '\0';
					vga_append (buf);
					break;
				default:
					i ++;
			}
		}
		else
		{
			vga_putc (format[i]);
		}
	}
}

[[ noreturn ]] void panic (const char *__restrict__ format, ...)
{
	kprintf ("%CPANIC:%C ", VGAC_RED, VGAC_WHITE);

	va_list list;
	va_start(list, format);
	kvprintf (format, list);
	va_end (list);

	cli();
	for (;;)
	{
		hlt();
	}
}
