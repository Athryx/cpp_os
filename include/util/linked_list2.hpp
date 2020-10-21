#pragma once


#include <util/linked_list.hpp>
#include <types.hpp>
#include <mem/kmem.hpp>
#include <util/io.hpp>


// this linked list is lower and depends on having kernel memry allocation, but an item in  this linked list can be in multiple linked lists
namespace util
{
	template <typename T>
	class linked_list2
	{
		private:
			struct list_node2 : public util::list_node
			{
				T *item;
			};

		public:
			linked_list2 () {}

			bool prepend (T *item)
			{
				list_node2 *node = new list_node2 ();
				if (node == NULL)
					return false;

				node->item = item;
				return list.prepend (node);
			}

			bool append (T *item)
			{
				list_node2 *node = new list_node2 ();
				if (node == NULL)
					return false;

				node->item = item;
				return list.append (node);
			}

			bool insert (usize index, T *item)
			{
				list_node2 *node = new list_node2 ();
				if (node == NULL)
					return false;

				node->item = item;
				return list.insert (index, node);
			}

			list_node *pop_start ()
			{
				list_node2 *node = (list_node2 *) list.pop_start ();
				if (node == NULL)
					return NULL;

				T *out = node->item;
				delete node;
				return out;
			}

			list_node *pop ()
			{
				list_node2 *node = (list_node2 *) list.pop ();
				if (node == NULL)
					return NULL;

				T *out = node->item;
				delete node;
				return out;
			}

			list_node *remove (usize index)
			{
				list_node2 *node = (list_node2 *) list.remove (index);
				if (node == NULL)
					return NULL;

				T *out = node->item;
				delete node;
				return out;
			}

			// i could implement this, but it would be a little slow
			// void remove_p (list_node *zone);

			T *get (usize index)
			{
				list_node2 *node = (list_node2 *) list.get (index);
				if (node == NULL)
					return NULL;

				return node->item;
			}

			inline usize get_len () { list.get_len (); }

		private:
			linked_list list;
	};
}
