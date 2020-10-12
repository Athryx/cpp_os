#pragma once


#include <types.hpp>
#include <util/array_list.hpp>
#include <util/linked_list.hpp>


namespace mem
{
	struct phys_allocation
	{
		usize addr;
		usize size;
	};

	struct virt_allocation : public util::list_node
	{
		~virt_allocation ();

		usize addr;
		usize size;
		util::array_list<struct phys_allocation> allocations;
	};

	class addr_space
	{
		public:
			addr_space ();

			// allocates virtual memory zone and any underlying physical memory
			// allocates at least n bytes
			// returns null on failure
			void *alloc (usize n);
			// reallocates virtual address mem to be at least n bytes long
			// if n is zero, it will free mem
			// returns pointer to new virtual memory zone on success
			// returns null on failure, and does not free any mamory pointed to by mem
			// allocates and frees underlying physical memory as needed
			void *realloc (void *mem, usize n);
			// frees virtual address mem and any underlying pysical memory
			void free (void *mem);

			// allocates virtual memory pointing address addr with size atleast n bytes but no more than ceil (n / PAGE_SIZE) pages
			// addr is a linear kernel space address
			void *map (usize addr, usize n);
			// unmaps memory prevously mapped by map
			void *unmap (void *mem);

		private:
			void *map_alloc_data (struct virt_allocation *allocation);

			usize pml4_table;
			util::linked_list phys_allocs;
	};
}
