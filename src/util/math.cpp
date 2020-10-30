#include <util/math.hpp>
#include <types.hpp>
#include <util/misc.hpp>


isize powi (const isize a, const usize b);
usize powu (const usize a, const usize b);
u8 log2_up (u64 in);
int wrap (const int in, const int lower, const int upper);
int clamp (const int in, const int lower, const int upper);
i32 abs (i32 in);
i8 sign (i32 in);


isize powi (const isize a, const usize b)
{
	isize out = 1;
	for (usize i = 0; i < b; i ++)
	{
		out *= a;
	}
	return out;
}

usize powu (const usize a, const usize b)
{
	isize out = 1;
	for (usize i = 0; i < b; i ++)
	{
		out *= a;
	}
	return out;
}

// probably can do faaster
u8 log2_up (u64 in)
{
	return log2 (align_up (in, (1 << log2 (in))));
}

int wrap (const int in, const int lower, const int upper)
{
	if (in < lower || in > upper)
		return (in % (upper - lower + 1)) + lower;
	else
		return in;
}

int clamp (const int in, const int lower, const int upper)
{
	return in < lower ? lower : (in > upper ? upper : in);
}

i32 abs (i32 in)
{
	u32 temp = in >> 31;
	in ^= temp;
	return in + temp;
}

i8 sign (i32 in)
{
	if (in == 0)
		return 0;
	else if (in > 0)
		return 1;
	else
		return -1;
}
