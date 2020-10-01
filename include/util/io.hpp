#pragma once


#include <types.hpp>
#include <stdarg.h>
#include <util/string.hpp>
#include <driver/tty.hpp>


/* only supported formats for format strings are:
%s
%x
*/
void kprintf (const char *__restrict__ formaat, ...);
[[ noreturn ]] void panic (const char *__restrict__ message);
