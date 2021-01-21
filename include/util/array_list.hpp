#pragma once


#include <types.hpp>
#include <types.hpp>
#include <mem/kmem.hpp>
#include <util/io.hpp>


#define ARRAY_LIST_INTIAL_CAPACITY 8


namespace util
{
	template <typename T>
	// this arraylist is better for storing pointers to data
	class array_list
	{
		public:
			array_list ();
			array_list (bool free_members);
			~array_list ();

			bool push (T *in);
			T *pop ();

			bool insert (T *in, usize i);
			T *remove (usize i);

			T *operator[] (usize i);
			bool set (usize i, T *in);

			inline usize get_len () { return len; }
			inline void set_free_members (bool in) { free_members = in; }

		protected:
			usize len;
			usize capacity;
			T **list;
			bool free_members;
	};
}


// because how templates work this is needed
template <typename T>
util::array_list<T>::array_list ()
: free_members (true)
{
	list = (T **) mem::kalloc (ARRAY_LIST_INTIAL_CAPACITY * sizeof (void *));
	assert(list);

	capacity = ARRAY_LIST_INTIAL_CAPACITY;
	len = 0;
}

template <typename T>
util::array_list<T>::array_list (bool free_members)
: free_members (free_members)
{
	list = (T **) mem::kalloc (ARRAY_LIST_INTIAL_CAPACITY * sizeof (void *));
	assert(list);

	capacity = ARRAY_LIST_INTIAL_CAPACITY;
	len = 0;
}

template <typename T>
util::array_list<T>::~array_list ()
{
	if (free_members)
	{
		for (usize i = 0; i < len; i ++)
		{
			delete list[i];
		}
	}
	mem::kfree (list, capacity * sizeof (void *));
}

template <typename T>
bool util::array_list<T>::push (T *in)
{
	return insert (in, len);
}

template <typename T>
T *util::array_list<T>::pop ()
{
	return remove (len - 1);
}

template <typename T>
bool util::array_list<T>::insert (T *in, usize i)
{
	if (!list)
		return false;

	if (i > len)
		return false;

	if (i < capacity)
	{
		list[i] = in;
		len ++;
		return true;
	}

	T **list_old = list;
	// no need to check capacity bounds
	list = (T **) mem::krealloc (list, capacity * sizeof (void *), (capacity << 1) * sizeof (void *));

	if (!list)
	{
		list = list_old;
		return false;
	}

	capacity <<= 1;

	for (usize j = len; j > i; j --)
		list[j] = list[j - 1];

	list[i] = in;
	len ++;

	return true;
}

template <typename T>
T *util::array_list<T>::remove (usize i)
{
	if (i >= len)
		return NULL;

	T *out = list[i];
	len --;

	for (; i < len; i ++)
		list[i] = list[i + 1];

	if (len <= capacity >> 2)
	{
		// this shouldn't fail
		list = (T **) mem::krealloc (list, capacity * sizeof (void *), (capacity >> 1) * sizeof (void *));
		capacity >>= 1;
	}

	return out;
}

template <typename T>
T *util::array_list<T>::operator[] (usize i)
{
	if (i >= len)
		return NULL;

	return list[i];
}

template <typename T>
bool util::array_list<T>::set (usize i, T *in)
{
	if (i >= len)
		return false;

	list[i] = in;
	return true;
}
