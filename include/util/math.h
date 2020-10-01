#ifndef __UTIL_MATH_H__
#define __UTIL_MATH_H__


#include <types.h>


#define min(a, b) ({			\
	__typeof__ (a) _a = (a);	\
	__typeof__ (b) _b = (b);	\
	_a < _b ? _a : _b;		\
})

#define max(a, b) ({			\
	__typeof__ (a) _a = (a);	\
	__typeof__ (b) _b = (b);	\
	_a > _b ? _a : _b;		\
})


int powi (const int a, const int b);
int wrap (const int in, const int lower, const int upper);
int clamp (const int in, const int lower, const int upper);
i32 abs (int i32);
i8 sign (i32 in);


#endif
