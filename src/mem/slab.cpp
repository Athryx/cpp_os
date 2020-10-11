#include <mem/slab.hpp>
#include <types.hpp>
#include <cxx.hpp>
#include <mem/mem.hpp>
#include <mem/mem_def.hpp>
#include <util/math.hpp>


using namespace mem;


template <class T>
slab<T>::slab (usize slab_start, usize slab_end, usize object_align)
: slab_start (slab_start),
slab_end (slab_end),
object_align (object_align)
{
}


template <class T>
cache_base<T>::cache_base ()
: object_align (1 << log2_up (sizeof (T))),
slab_order (0)
{
}

template <class T>
cache_base<T>::cache_base (usize object_align, u8 slab_order)
: object_align (object_align),
slab_order (slab_order)
{
}

template <class T>
slab<T> *cache_base<T>::slab_new ()
{
	usize mem = (usize) oalloc (slab_order);
	return new ((void *) mem) slab<T> (mem + sizeof (slab<T>), mem + FIRST_ORDER_SIZE);
}
