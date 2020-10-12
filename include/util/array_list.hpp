#pragma once


#include <types.hpp>
#include <types.hpp>
#include <mem/kmem.hpp>
#include <util/io.hpp>


#define ARRAY_LIST_INTIAL_CAPACITY 8


namespace util
{
	template <typename T>
	class array_list
	{
		public:
			array_list ();

			bool push (T in);
			T pop ();

			bool insert (T in, usize i);
			T remove (usize i);

			T *get (usize i);

			inline usize get_len () { return len; }

		private:
			usize len;
			usize capacity;
			T *list;
	};
}


// because how templates work this is needed
template <typename T>
util::array_list<T>::array_list ()
{
	list = (T *) mem::kalloc (ARRAY_LIST_INTIAL_CAPACITY * sizeof (T));
	if (list == NULL)
	{
		error ("not enough memory for array list")
	}

	capacity = ARRAY_LIST_INTIAL_CAPACITY;
	len = 0;
}

template <typename T>
bool util::array_list<T>::push (T in)
{
	return insert (in, len);
}

template <typename T>
T util::array_list<T>::pop ()
{
	return remove (len - 1);
}

template <typename T>
bool util::array_list<T>::insert (T in, usize i)
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

	T *list_old = list;
	// no need to check capacity bounds
	list = (T *) mem::krealloc (list, capacity * sizeof (T), (capacity << 1) * sizeof (T));

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
T util::array_list<T>::remove (usize i)
{
	if (i >= len)
		return NULL;

	T out = list[i];
	len --;

	for (; i < len; i ++)
		list[i] = list[i + 1];

	if (len <= capacity >> 2)
	{
		// this shouldn't fail
		list = krealloc (list, capacity * sizeof (T), (capacity >> 1) * sizeof (T));
		capacity >>= 1;
	}

	return out;
}

template <typename T>
T *util::array_list<T>::get (usize i)
{
	if (i >= len)
		return NULL;

	return list + i;
}
