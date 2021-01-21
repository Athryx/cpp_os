#include <util/io.hpp>
#include <types.hpp>
#include <def/keys.hpp>
#include <arch/x64/common.hpp>
#include <util/string.hpp>


#define DEBUGCON_PORT 0xe9


static void kprintf_internal (void (*putc) (char), void (*puts) (const char *), const char *__restrict__ format, va_list list);


void debugcon_putc (char c)
{
	outb (DEBUGCON_PORT, c);
}

void debugcon_puts (const char *str)
{
	for (usize i = 0; str[i]; i ++)
		debugcon_putc (str[i]);
}

void kprintf (const char *__restrict__ format, ...)
{
	va_list list;
	va_start(list, format);
	kvprintf (format, list);
	va_end(list);
}

extern "C" void c_asmprint ()
{
		static u32 n = 0;
		kprinte ("debug string %u\n", n);
		n ++;
}

void kprinte (const char *__restrict__ format, ...)
{
	va_list list;
	va_start(list, format);
	kvprinte (format, list);
	va_end(list);
}

void kvprintf (const char *__restrict__ format, va_list list)
{
	kprintf_internal (vga_putc, (void (*) (const char *)) vga_append, format, list);
}

void kvprinte (const char *__restrict__ format, va_list list)
{
	#ifdef VM
	kprintf_internal (debugcon_putc, debugcon_puts, format, list);
	#else
	kprintf_internal (vga_putc, (void (*) (const char *)) vga_append, format, list);
	#endif
}

[[ noreturn ]] void panic (const char *__restrict__ format, ...)
{
	kprinte ("%CPANIC:%C ", VGAC_RED, VGAC_WHITE);

	va_list list;
	va_start(list, format);
	kvprinte (format, list);
	va_end(list);

	cli();
	for (;;)
	{
		hlt();
	}
}

[[ noreturn ]] void assert_fail (const char *expr, const char *message, const char *file, const char *func, u32 line)
{
	kprinte ("%s:%u: in function '%s':\n", file, line, func);
	kprinte ("Assertion '%s' failed\n", expr);
	if (message != NULL)
		kprinte ("%s\n", message);

	cli();
	for (;;)
	{
		hlt();
	}
}

static void kprintf_internal (void (*putc) (char), void (*puts) (const char *), const char *__restrict__ format, va_list list)
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
					itoa (buf, va_arg (list, u32));
					puts (buf);
					break;
				case 'x':
					itoa_hex (buf, va_arg (list, u64));
					putc ('0');
					putc ('x');
					puts (buf);
					break;
				case 's':
					puts (va_arg (list, char *));
					break;
				case 'c':
					putc (va_arg (list, int));
					break;
				case 'C':
					buf[0] = TTY_SET_COLOR;
					buf[1] = (char) va_arg (list, int);
					buf[2] = '\0';
					puts (buf);
					break;
				default:
					putc ('%');
					putc (format[i]);
					i ++;
			}
		}
		else
		{
			putc (format[i]);
		}
	}
}
