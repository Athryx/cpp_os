#pragma once


#include <types.hpp>
#include <stdarg.h>
#include <util/string.hpp>
#include <driver/tty.hpp>


#ifdef debug
#define error(error_message)	\
kprinte ("%s:%x: error in function %s: %s\n", __FILE__, __LINE__, __func__, error_message);
#else
#define error(error_message)
#endif


/* only supported formats for format strings are:
%s
%x
*/
void kprintf (const char *__restrict__ format, ...);
void kprinte (const char *__restrict__ format, ...);
[[ noreturn ]] void panic (const char *__restrict__ message);
