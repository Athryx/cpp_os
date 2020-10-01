#pragma once


#include <types.hpp>
#include <def/keys.hpp>


#define KBD_BUF_SIZE 32


u8 kbd_init (void);

u8 kbd_read (char *buf, u8 n);
