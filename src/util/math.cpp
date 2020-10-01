#include <util/math.hpp>
#include <types.hpp>


int powi (const int a, const int b);
int wrap (const int in, const int lower, const int upper);
int clamp (const int in, const int lower, const int upper);
int abs (int in);
i8 sign (i32 in);


int powi (const int a, const int b)
{
	int out = 1;
	for (int i = 0; i < b; i ++)
	{
		out *= a;
	}
	return out;
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
