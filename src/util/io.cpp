#include <util/io.hpp>
#include <types.hpp>
#include <arch/x64/common.hpp>


void kprintf (const char *__restrict__ format, ...);
[[ noreturn ]] void panic (const char *__restrict__ message);


void kprintf (const char *__restrict__ format, ...)
{
	va_list list;
	va_start(list, format);

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
					va_arg (list, u32);
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
				default:
					i ++;
			}
		}
		else
		{
			vga_putc (format[i]);
		}
	}
	va_end (list);
}

[[ noreturn ]] void panic (const char *__restrict__ message)
{
	kprintf ("PANIC: %s", message);

	cli();
	for (;;)
	{
		hlt();
	}
}
