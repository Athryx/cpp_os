#pragma once


#include <types.hpp>
#include <util/linked_list.hpp>


namespace mem
{
	template <class T>
	class slab : public util::list_node
	{
		public:
			slab (usize slab_start, usize slab_end, usize object_align);

			inline u8 get_status () { return status; }

			slab *prev;
			slab *next;

		private:
			usize slab_start;
			usize slab_end;
			usize object_align;
			u8 status;
	};

	template <class T>
	class cache_base
	{
		public:
			cache_base ();
			cache_base (usize object_align, u8 slab_order);

		protected:
			virtual slab<T> *slab_new ();

			util::linked_list slab_list;

		private:
			usize object_align;
			u8 slab_order;
	};
}
