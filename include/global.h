#ifndef __GLOBALS_H__
#define __GLOBALS_H__


#include <types.h>


#define CPU_COUNT 1
// temporary
#define CPU_I 0


namespace global
{
	extern "C" usize sys_rsp[CPU_COUNT];
}


#endif
