#pragma once


#include <types.hpp>
#include <mem/mem_def.hpp>
#include <util/linked_list.hpp>


namespace mem
{
	struct free_zone : public util::list_node
	{
		usize n;
	};

	struct __attribute__ ((packed)) metadata
	{
		u8 alloced: 1;
		u8 order: 7;
	};

	// todo: construct canonical address when needed
	// todo: verify it is a canonical address, because if it isn't an attack would be possible
	class pallocator
	{
		public:
			// needed because a pallocator is needed in data section, but start and end address are not known at compile time
			pallocator () {}
			// end_addr must be at least a page bigger than start_addr
			pallocator (usize start_addr, usize end_addr, usize first_order_size);
			void init (usize start_addr, usize end_addr, usize first_order_size);
			// all pointers to realloc and free must be pointing within the first zone of the allocation
			// allocates at least n bytes, returns null if allocation failed
			void *alloc (usize n);
			// allocate and zero memory
			void *allocz (usize n);
			// allocate 1 order worth of memory
			void *oalloc (u8 order);
			// allocate orders of memory and zero
			void *oallocz (u8 order);
			// resizes memory pointed to by mem to at least n bytes, returns null if invalid arguments or new allocation failed. If new allocation failed, old memory will still be allocated
			// untested
			// i don't think its finished, but I might have finished
			void *realloc (void *mem, usize n);
			// reallocate to 1 order worth of memory
			void *orealloc (void *mem, u8 order);
			// frees memory pointed to by mem, if the memory is allocated
			void free (void *mem);

			inline usize get_start_addr () { return start_addr; }
			inline usize get_end_addr () { return end_addr; }
			inline usize get_first_order_size () { return first_order_size; }
			inline usize get_order_size (u8 order) { return 1 << (order + fo_bits); }

		private:
			// all private function that reqpuire address may break if passed address is out of bounds
			u8 get_order (usize n);
			// if called on already populated order, will populate one below
			bool populate_order (u8 order);
			void merge_order (usize addr, u8 order, u8 m_order);
			// gets metadata entry associated with address
			// addr must be in addr range
			struct metadata *get_meta (usize addr);
			// returns amount of consecutive free bytes starting at memory region pointed to by addr
			usize get_space (usize addr);


			util::linked_list lists[MAX_SUPPERTED_MEM_BITS];

			usize start_addr;
			usize end_addr;
			usize end_meta;
			usize first_order_size;
			u8 fo_bits;
			u8 max_order;
	};
}
