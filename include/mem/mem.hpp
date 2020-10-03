#pragma once


#include <types.hpp>
#include <mem/pmem.hpp>
#include <mem/mem_def.hpp>


namespace mem
{
	void init ();
	void *alloc (usize n);
	void free (void *mem);
}
