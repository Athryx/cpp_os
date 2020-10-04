#pragma once


#include <types.hpp>
#include <mem/pmem.hpp>
#include <mem/mem_def.hpp>


namespace mem
{
	// returns new location of mb2 table
	void *init (void *mb2_table);
	void *alloc (usize n);
	void *realloc (void *mem, usize n);
	void free (void *mem);
}
