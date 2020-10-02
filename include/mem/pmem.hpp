#pragma once


#include <types.hpp>
#include <mem/mem_def.hpp>


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
class pallocator
{
	public:
		// end_addr must be at least a page bigger than start_addr
		pallocator (usize start_addr, usize end_addr, usize first_order_size);
		void *alloc (usize n);
		void free (void *mem);
	private:
		u8 get_order (usize n);
		// if called on already populated order, will populate one below
		bool populate_order (u8 order);
		void merge_order (usize addr, u8 oreder);
		// gets metadata entry associated with address
		// addr must be in addr range
		struct metadata *get_meta (usize addr);

		order_list lists[MAX_SUPPERTED_MEM_BITS];

		usize start_addr;
		usize end_addr;
		usize end_meta;
		usize first_order_size;
		u8 fo_bits;
		u8 max_order;
};
