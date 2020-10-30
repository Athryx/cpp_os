#pragma once


#include <types.hpp>
#include <stdarg.h>
#include <driver/tty.hpp>
#include <def/keys.hpp>
#include <arch/x64/common.hpp>


#ifdef debug
#define assert(expr)	\
if (!(expr))		\
{			\
	kprinte ("%CERROR%C: %s at %s:%u:\nassertion \"%s\" failed\n", VGAC_RED, VGAC_WHITE, __func__, __FILE__, __LINE__, #expr);	\
	return 0;	\
}

#define massert(obj)	\
if ((obj) == NULL)	\
{			\
	kprinte ("%CERROR%C: %s at %s:%u:\nobject \"%s\" is null\n", VGAC_RED, VGAC_WHITE, __func__, __FILE__, __LINE__, #obj);	\
	delete (obj);	\
	return 0;	\
}

#define error(fmt, ...)	\
do			\
{			\
kprinte ("%CERROR%C: %s at %s:%u:\n",VGAC_RED, VGAC_WHITE, __func__, __FILE__, __LINE__);	\
kprinte (fmt, ##__VA_ARGS__);	\
}			\
while (0);
// semicolon needed for backward compatability

#define debug_c(n)		\
do				\
{				\
	static usize debug_counter = 0;	\
	if (debug_counter == (n))	\
	{				\
		kprintf ("here\n");	\
		for (usize i = 0; i < 0x2fffffff; i ++) {}	\
	}				\
	kprinte ("%s:%s: number %x\n", __FILE__, __func__, debug_counter ++);	\
} while (0);

#else
#define assert(expr)
#define massert(obj)
#define error(error_message)
#define debug_c(n)
#endif

/* only supported formats for format strings are:
%s
%x
%c
*/
void kprintf (const char *__restrict__ format, ...);
void kprinte (const char *__restrict__ format, ...);
// TODO: maybe pass va_list by reference
void kvprintf (const char *__restrict__ format, va_list list);
[[ noreturn ]] void panic (const char *__restrict__ format, ...);
