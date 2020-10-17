#pragma once


#include <types.hpp>
#include <util/array_list.hpp>
#include <util/linked_list.hpp>
#include <util/misc.hpp>
#include <mem/mem_def.hpp>


namespace mem
{
	struct phys_allocation
	{
		usize addr;
		usize len;
	};

	struct virt_zone : public util::list_node
	{
		usize virt_addr;
		usize len;
	};

	struct virt_allocation : public virt_zone
	{
		~virt_allocation ();

		util::array_list<struct phys_allocation> allocations;
	};

	struct phys_map : public virt_zone
	{
		usize phys_addr;
	};

	class addr_space
	{
		public:
			addr_space ();
			~addr_space ();

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
			// can be multiple virt address mapped to same phys address
			// addr is a linear kernel space address
			void *map (usize phys_addr, usize n);
			// unmaps memory prevously mapped by map
			void *unmap (usize virt_addr);

		private:
			void *map_alloc_data (struct virt_allocation *allocation);

			// doesn't keep track of the mapped regions
			// returns true if success
			bool map_internal (usize phys_addr, usize virt_addr, usize n);
			// helper function
			bool map_internal_recurse (usize phys_addr, usize virt_addr, usize start_page_n, usize &page_n, usize page_level, usize *page_table);

			void unmap_internal (usize virt_addr, usize n);
			// helper function
			void unmap_internal_recurse (usize virt_addr, usize n);

			// adds n to page counter c
			// page counter is stored in ignored bits of first entry in every page table data structure
			#ifdef x64
			inline usize page_counter_add (usize &c, i16 n)
			{
				usize temp = ((get_bits (c, 52, 58) << 3) | get_bits (c, 9, 11)) + n;
				// data excluding counter
				usize temp2 = c & PAGE_DATA_POS;
				c = (get_bits (temp, 3, 9) << 52) | (get_bits (temp, 0, 2) << 9) | temp2;
				return c;
			}

			// returns address to page it was set to
			inline usize page_set (usize *page_table, u16 i, usize n)
			{
				if (n == 0)
				{
					if (i == 0)
					{
						// have to do something special because of page counter in index 0
						usize c_value = page_table[i] & ~PAGE_DATA_POS;
						n = n & PAGE_DATA_POS;
						page_table[i] = (n | c_value) & ~PAGE_PRESENT;
					}
					else
					{
						page_table[i] = n & ~PAGE_PRESENT;
					}
					return 0;
				}
				else
				{
					if (i == 0)
					{
						// have to do something special because of page counter in index 0
						usize c_value = page_table[i] & ~PAGE_DATA_POS;
						n = n & PAGE_DATA_POS;
						page_table[i] = n | c_value | PAGE_PRESENT;
					}
					else
					{
						page_table[i] = n | PAGE_PRESENT;
					}
				}
				return canonical_addr (n & PAGE_ADDR_POS);
			}
			#endif

			usize find_address (util::linked_list &list, struct virt_zone &allocation);

			usize pml4_table;
			util::linked_list virt_allocs;
			util::linked_list phys_maps;
	};
}
