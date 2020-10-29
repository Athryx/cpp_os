#pragma once


#include <types.hpp>


namespace util
{
	class list_node
	{
		public:
			list_node *prev;
			list_node *next;
	};

	class linked_list
	{
		public:
			linked_list ();

			bool prepend (list_node *zone);
			bool append (list_node *zone);
			bool insert (usize index, list_node *zone);

			list_node *pop_start ();
			list_node *pop ();
			list_node *remove (usize index);

			void remove_p (list_node *zone);

			list_node *operator[] (usize index);

			inline usize get_len () { return len; }

		private:
			usize len;
			list_node *start;
			list_node *end;
			list_node *current;
			usize current_i;
	};
}
