#ifndef __UTIL_MISC_H__
#define __UTIL_MISC_H__


#include <types.h>


usize align_up (usize addr, usize align);
usize align_down (usize addr, usize align);

// gets bits from low to high, l and h included
u64 get_bits (u64 n, u8 l, u8 h);
u64 get_bits_raw (u64 n, u8 l, u8 h);


#endif
