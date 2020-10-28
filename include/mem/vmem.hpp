#pragma once


#include <types.hpp>
#include <util/array_list.hpp>
#include <util/linked_list.hpp>
#include <util/misc.hpp>
#include <mem/mem_def.hpp>


#define V_PRESENT 1
#define V_WRITE ((usize) 1 << 1)
#define V_USER ((usize) 1 << 2)
#define V_PWT ((usize) 1 << 3)
#define V_PCD ((usize) 1 << 4)
#define V_GLOBAL ((usize) 1 << 7)
#define V_XD ((usize) 1 << 63)


namespace mem
{
	struct phys_allocation
	{
		usize addr;
		usize len;
	};

	enum alloc_type
	{
		valloc,
		map,
		reserve,
		kernel
	};

	struct virt_zone : public util::list_node
	{
		usize virt_addr;
		usize len;

		alloc_type type;

		util::array_list<phys_allocation> allocations;
		usize flags;
		usize phys_addr;
	};

	struct virt_allocation : public virt_zone
	{
		virt_allocation () { type = alloc_type::valloc; }
		~virt_allocation ();
	};

	struct phys_map : public virt_zone
	{
		phys_map () { type = alloc_type::map; }
	};

	struct phys_reserve : public virt_zone
	{
		phys_reserve () { type = alloc_type::reserve; }
	};

	struct kernel_zone : public virt_zone
	{
		kernel_zone () {
			type = alloc_type::kernel;
			virt_addr = kernel_vma;
			phys_addr = kernel_vma;
			len = MAX_MEM - kernel_vma;
		}
	};

	// TODO: adde structured allocation function
	class addr_space
	{
		public:
			// FIXME: handle case when there is not enough memory for pml4_table
			addr_space ();
			// TODO: finish
			~addr_space ();

			// TODO: support setting read, write, and execute permissions on pages
			// allocates virtual memory zone and any underlying physical memory
			// allocates at least n bytes
			// uses same flags that alloc used on memory
			// returns null on failure
			void *alloc (usize n, usize flags);
			// reallocates virtual address mem to be at least n bytes long
			// if n is zero, it will free mem
			// returns pointer to new virtual memory zone on success
			// returns null on failure, and does not free any mamory pointed to by mem
			// allocates and frees underlying physical memory as needed
			// TODO: finish
			void *realloc (void *mem, usize n);
			// frees virtual address mem and any underlying pysical memory
			void free (void *mem);

			// allocates virtual memory pointing address addr with size atleast n bytes but no more than ceil (n / PAGE_SIZE) pages
			// can be multiple virt address mapped to same phys address
			// addr is a linear kernel space address
			void *map (usize phys_addr, usize n, usize flags);
			// map_at does the same as map, but try to map at specific virtual address, and they return false if it is already taken
			bool map_at (usize phys_addr, usize virt_addr, usize n, usize flags);
			// unmaps memory prevously mapped by map
			void *unmap (usize virt_addr);

			// marks virt_addr zone reserved, so alloc and map will not pick this address
			void *reserve (usize n);
			bool reserve_at (usize virt_addr, usize n);
			// marks the zone as no longer reserved
			void unreserve (usize virt_addr);

			// translates a virtual address to a kernel addres which points to the start of the page
			void *translate (usize virt_addr);

			inline usize get_cr3 () { return pml4_table - kernel_vma; }

		private:
			void *map_alloc_data (struct virt_allocation *allocation);

			// doesn't keep track of the mapped regions
			// returns true if success
			bool map_internal (usize phys_addr, usize virt_addr, usize n, usize flags);
			// helper function
			bool map_internal_recurse (usize phys_addr, usize &virt_addr, usize start_page_n, usize &page_n, usize page_level, usize *page_table, usize flags);

			void unmap_internal (usize virt_addr, usize n);
			// helper function
			// returns true when page table it was examaning needss to be freed
			bool unmap_internal_recurse (usize &virt_addr, usize &page_n, usize page_level, usize *page_table);

			bool insert_virt_zone (virt_zone &allocation);
			usize find_address (util::linked_list &list, struct virt_zone &allocation);
			usize get_free_space (usize virt_addr);

			#ifdef x64
			usize page_set (usize *page_table, u16 i, usize n, usize flags);

			// adds n to page counter c
			// page counter is stored in ignored bits of first entry in every page table data structure
			inline usize page_counter_add (usize &c, i16 n)
			{
				usize temp = ((get_bits (c, 52, 58) << 3) | get_bits (c, 9, 11)) + n;
				// data excluding counter
				usize temp2 = c & PAGE_DATA_POS;
				c = (get_bits (temp, 3, 9) << 52) | (get_bits (temp, 0, 2) << 9) | temp2;
				return c;
			}

			inline usize page_counter_get (usize c)
			{
				return (get_bits (c, 52, 58) << 3) | get_bits (c, 9, 11);
			}
			#endif


			kernel_zone kernel_z;
			usize pml4_table;
			util::linked_list virt_allocs;
	};
}
