#include <mem/vmem.hpp>
#include <types.hpp>
#include <mem/mem.hpp>
#include <mem/kmem.hpp>
#include <util/misc.hpp>


mem::addr_space::addr_space ()
{
	pml4_table = (usize) mem::allocz (PAGE_SIZE);
}

void *mem::addr_space::alloc (usize n)
{
	if (n > mem::get_free_space ())
		return NULL;

	struct virt_allocation *allocation = new struct virt_allocation ();

	usize temp = n;
	usize alloc_size = mem::get_order_size (mem::get_order (n));
	while (temp)
	{
		void *mem = alloc (alloc_size);
		if (!mem)
		{
			alloc_size >>= 1;
			continue;
		}

		struct phys_allocation palloc { .addr = (usize) mem, .size = alloc_size };
		allocation->allocations.push (palloc);
		temp -= alloc_size;
	}

	void *out = map_alloc_data (allocation);

	if (!out)
	{
		allocation->~virt_allocation ();
		kfree (allocation, sizeof (struct virt_allocation));
		return NULL;
	}

	phys_allocs.append (allocation);
	return out;
}

mem::virt_allocation::~virt_allocation ()
{
	for (usize i = 0; i < allocations.get_len (); i ++)
	{
		struct phys_allocation* alloc = allocations.get (i);
		mem::free ((void *) alloc->addr);
	}
}

void *mem::addr_space::map_alloc_data (struct virt_allocation *allocation)
{
	
}
