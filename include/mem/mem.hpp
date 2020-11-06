#pragma once


#include <types.hpp>
#include <mem/pmem.hpp>
#include <mem/mem_def.hpp>
#include <syscall.hpp>


extern mem::pallocator allocators[MAX_MEM_NODES];
namespace mem
{
	// returns new location of mb2 table
	void *init (void *mb2_table);

	// sizes are in bytes
	usize sys_realloc (syscall_vals_t *vals, u32 options, usize mem, usize n);

	void *alloc (usize n);
	void *allocz (usize n);
	void *oalloc (u8 n);
	void *oallocz (u8 n);
	void *realloc (void *mem, usize n);
	void *orealloc (void *mem, u8 n);
	void free (void *mem);
	usize get_free_space ();
	u8 get_order (usize n);
	usize get_order_size (u8 order);
}
