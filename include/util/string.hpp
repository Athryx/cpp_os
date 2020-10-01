#pragma once


#include <types.hpp>
#include <util/math.hpp>


void *memcpy (void *dest, const void *src, usize n);

void memset (void *s, usize n, u8 c);

usize strlen (const char *__restrict__ str);

int strcmp (const char *__restrict__ str1, const char *__restrict__ str2);
int strncmp (const char *__restrict__ str1, const char *__restrict__ str2, usize n);

void revstr (char *__restrict__ str);

// atoi and itoa don't work with negative numbers yet
u32 atoi (const char *__restrict__ str);
// str_out must be at least 11 bytes, otherwise problems could occur
void itoa (char *__restrict__ str_out, u32 num);
// str_out must be at least 9 bytes, otherwise problems could occur
void itoa_hex (char *__restrict__ str_out, u32 num);

// hex number in
char itoc (u32 num);
