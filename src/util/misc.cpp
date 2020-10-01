#include <util/misc.h>
#include <types.h>


usize align_up (usize addr, usize align);
usize align_down (usize addr, usize align);

u64 get_bits (u64 n, u8 l, u8 h);
u64 get_bits_raw (u64 n, u8 l, u8 h);


// align address, alignment must be power of 2
usize align_up (usize addr, usize align)
{
	return (addr + align - 1) & ~ (align - 1);
}

usize align_down (usize addr, usize align)
{
	return addr & ~ (align - 1);
}

u64 get_bits (u64 n, u8 l, u8 h)
{
	u64 temp = h == 63 ? 0xffffffffffffffff : (((u64) 1) << (h + 1)) - 1;
	return (temp & n) >> l;
}

u64 get_bits_raw (u64 n, u8 l, u8 h)
{
	u64 temp = h == 63 ? 0xffffffffffffffff : (((u64) 1) << (h + 1)) - 1;
	return ((temp & n) >> l) << l;
}
