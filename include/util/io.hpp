#pragma once


#include <types.hpp>
#include <stdarg.h>
#include <util/string.hpp>
#include <driver/tty.hpp>
#include <arch/x64/common.hpp>


#ifdef debug
#define error(error_message)	\
kprinte ("%s:%x: error in function %s: %s\n", __FILE__, __LINE__, __func__, error_message);

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
#define error(error_message)
#define debug_c(n)
#endif


/* only supported formats for format strings are:
%s
%x
*/
void kprintf (const char *__restrict__ format, ...);
void kprinte (const char *__restrict__ format, ...);
[[ noreturn ]] void panic (const char *__restrict__ message);
