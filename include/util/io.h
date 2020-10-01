#ifndef __UTIL_IO_H__
#define __UTIL_IO_H__


#include <types.h>
#include <stdarg.h>
#include <util/string.h>
#include <driver/tty.h>


/* only supported formats for format strings are:
%s
%x
*/
void kprintf (const char *__restrict__ formaat, ...);
[[ noreturn ]] void panic (const char *__restrict__ message);


#endif
