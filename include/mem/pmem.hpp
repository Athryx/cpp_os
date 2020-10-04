#pragma once


#include <types.hpp>
#include <mem/mem_def.hpp>


namespace pmem
{
	struct free_zone
	{
		struct free_zone *prev;
		struct free_zone *next;
		usize n;
	};

	// need special linked list to keep track of free zones in each order
	class order_list
	{
		public:
			order_list ();

			bool prepend (struct free_zone *zone);
			bool append (struct free_zone *zone);
			bool insert (usize index, struct free_zone *zone);

			struct free_zone *pop_start ();
			struct free_zone *pop ();
			struct free_zone *remove (usize index);

			void remove_p (struct free_zone *zone);

			struct free_zone *get (usize index);

			usize len;
		private:
			struct free_zone *start;
			struct free_zone *end;
			struct free_zone *current;
			usize current_i;
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
			// resizes memory pointed to by mem to at least n bytes, returns null if invalid arguments or new allocation failed. If new allocation failed, old memory will still be allocated
			// untested
			void *realloc (void *mem, usize n);
			// frees memory pointed to by mem, if the memory is allocated
			void free (void *mem);

			usize start_addr;
			usize end_addr;
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

			inline usize size (u8 order) { return 1 << (order + fo_bits); }

			order_list lists[MAX_SUPPERTED_MEM_BITS];

			usize end_meta;
			usize first_order_size;
			u8 fo_bits;
			u8 max_order;
	};
}
