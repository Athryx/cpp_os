#pragma once


#include <types.hpp>
#include <stdarg.h>
#include <driver/tty.hpp>
#include <def/keys.hpp>
#include <arch/x64/common.hpp>


#ifdef debug
#define assert(expr)	\
(expr) ? (void) 0 : assert_fail (#expr, NULL, __FILE__, __func__, __LINE__)

#define assertm(expr, message)	\
(expr) ? (void) 0 : assert_fail (#expr, message, __FILE__, __func__, __LINE__)

#else
#define assert(expr)
#define assertm(expr, message)
#endif

/* only supported formats for format strings are:
%s
%x
%c
%u
*/
void debugcon_putc (char c);
void debugcon_puts (const char *str);

void kprintf (const char *__restrict__ format, ...);
void kprinte (const char *__restrict__ format, ...);
void kvprintf (const char *__restrict__ format, va_list list);
void kvprinte (const char *__restrict__ format, va_list list);
[[ noreturn ]] void panic (const char *__restrict__ format, ...);
[[ noreturn ]] void assert_fail (const char *expr, const char *message, const char *file, const char *func, u32 line);
