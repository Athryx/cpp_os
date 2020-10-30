#pragma once


#include <types.hpp>


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


isize powi (const isize a, const usize b);
usize powu (const usize a, const usize b);
extern "C" u8 log2 (u64 in);
u8 log2_up (u64 in);
int wrap (const int in, const int lower, const int upper);
int clamp (const int in, const int lower, const int upper);
i32 abs (i32 in);
i8 sign (i32 in);
