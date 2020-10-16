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
	// there might
	if (n > mem::get_free_space ())
		return NULL;

	struct virt_allocation *allocation = new struct virt_allocation ();
	allocation->len = n;

	usize temp = n;
	usize alloc_size = mem::get_order_size (mem::get_order (n));
	while (temp)
	{
		if (alloc_size < PAGE_SIZE)
		{
			allocation->~virt_allocation ();
			kfree (allocation, sizeof (struct virt_allocation));
			return NULL;
		}

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

	// will insert it into list and set location if success
	void *out = map_alloc_data (allocation);

	if (!out)
	{
		allocation->~virt_allocation ();
		kfree (allocation, sizeof (struct virt_allocation));
		return NULL;
	}

	return out;
}

void *mem::addr_space::realloc (void *mem, usize n)
{
}

void mem::addr_space::free (void *mem)
{
}

void *mem::addr_space::map (usize addr, usize n)
{
	struct phys_map *zone = new struct phys_map;

	if (!zone)
		return NULL;

	zone->phys_addr = addr;
	zone->len = n;
	usize virt_addr = find_address (phys_maps, *zone);

	if (!virt_addr)
	{
		mem::kfree (zone, sizeof (struct phys_map));
		return NULL;
	}

	void *out = map_internal (virt_addr, zone->len);

	if (!out)
	{
		phys_maps.remove_p (zone);
		mem::kfree (zone, sizeof (struct phys_map));
		return NULL;
	}

	return out;
}

void *mem::addr_space::unmap (void *mem)
{
}

void *mem::addr_space::map_alloc_data (struct virt_allocation *allocation)
{
	usize virt_addr = find_address (phys_maps, *allocation);

	if (virt_addr == 0)
	{
		return NULL;
	}

	// this can fail if there is not enough physical memory for page tables
	void *out = map_internal (virt_addr, allocation->len);

	if (out == NULL)
	{
		virt_allocs.remove_p (allocation);
		return NULL;
	}

	allocation->virt_addr = (usize) out;

	return out;
}

void *mem::addr_space::map_internal (usize addr, usize n)
{
}

void *mem::addr_space::unmap_internal (void *mem)
{
}

usize mem::addr_space::find_address (util::linked_list &list, struct virt_zone &allocation)
{
	usize addr = 0;
	usize i = list.get_len ();
	struct virt_zone *current = (struct virt_zone *) list.get (i - 1);
	struct virt_zone *last = NULL;

	if (i == 0)
	{
		// don't null pointer, it is returned on failure
		addr = PAGE_SIZE;
		list.append (&allocation);
	}
	else if (i == 1 && (kernel_vma - ((usize) current + current->len)) >= allocation.len)
	{
		if (kernel_vma - ((usize) current + current->len) >= allocation.len)
		{
			addr = (usize) current + current->len;
			list.append (&allocation);
		}
		else if ((usize) current - PAGE_SIZE >= allocation.len)
		{
			addr = (usize) current + current->len;
			list.prepend (&allocation);
		}
	}
	else if (i > 1)
	{
		// one for i being length, one because we all ready got the first one
		i -= 2;
		for (; i >= 0; i --)
		{
			last = current;
			current = (struct virt_zone *) list.get (i);
			usize current_end = (usize) current + current->len;
			if (((usize) last - current_end)  >= allocation.len)
			{
				// insert in between
				list.insert (i + 1, &allocation);
				addr = current_end;
				break;
			}
		}
	}

	if (addr == 0)
		return 0;

	allocation.virt_addr = addr;
	return addr;
}

mem::virt_allocation::~virt_allocation ()
{
	for (usize i = 0; i < allocations.get_len (); i ++)
	{
		struct phys_allocation* alloc = allocations.get (i);
		mem::free ((void *) alloc->addr);
	}
}
