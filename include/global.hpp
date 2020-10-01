#pragma once


#include <types.hpp>


#define CPU_COUNT 1
// temporary
#define CPU_I 0


namespace global
{
	extern "C" usize sys_rsp[CPU_COUNT];
}
