#include <mem/vmem.hpp>
#include <types.hpp>
#include <mem/mem.hpp>
#include <mem/kmem.hpp>
#include <util/misc.hpp>


mem::addr_space::addr_space ()
{
	pml4_table = (usize) mem::allocz (PAGE_SIZE);
}

mem::addr_space::~addr_space ()
{
	// incomplete
	mem::free ((void *) pml4_table);
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

		struct phys_allocation palloc { .addr = (usize) mem, .len = alloc_size };
		allocation->allocations.push (palloc);
		temp -= alloc_size;
	}

	// will insert it into list and set location if success
	void *out = map_alloc_data (allocation);

	if (!out)
	{
		virt_allocs.remove_p (allocation);
		allocation->~virt_allocation ();
		kfree (allocation, sizeof (struct virt_allocation));
		return NULL;
	}

	return out;
}

void *mem::addr_space::realloc (void *mem, usize n)
{
	for (usize i = 0; i < virt_allocs.get_len (); i ++)
	{
		struct virt_allocation *allocation = (struct virt_allocation *) virt_allocs.get (i);
		if (allocation->virt_addr == (usize) mem)
		{
			usize new_size = allocation->len;

			if (new_size == n)
				return mem;

			if (new_size < n)
			{
			}

			if (new_size > n)
			{
			}
		}
	}

	return NULL;
}

void mem::addr_space::free (void *mem)
{
	for (usize i = 0; i < virt_allocs.get_len (); i ++)
	{
		struct virt_allocation *allocation = (struct virt_allocation *) virt_allocs.get (i);
		if (allocation->virt_addr == (usize) mem)
		{
			// unmap virtual memory
			for (usize j = 0; j < allocation->allocations.get_len (); j ++)
			{
				struct phys_allocation *temp = allocation->allocations.get (j);
				unmap_internal (temp->addr, temp->len);
			}

			virt_allocs.remove (i);
			allocation->~virt_allocation ();
			mem::kfree (allocation, sizeof (struct virt_allocation));
		}
	}
}

void *mem::addr_space::map (usize phys_addr, usize n)
{
	struct phys_map *zone = new struct phys_map;

	if (!zone)
		return NULL;

	zone->phys_addr = phys_addr;
	zone->len = n;
	usize virt_addr = find_address (phys_maps, *zone);

	if (!virt_addr)
	{
		mem::kfree (zone, sizeof (struct phys_map));
		return NULL;
	}

	bool flag = map_internal (phys_addr, virt_addr, zone->len);

	if (!flag)
	{
		phys_maps.remove_p (zone);
		mem::kfree (zone, sizeof (struct phys_map));
		return NULL;
	}

	return (void *) virt_addr;
}

void *mem::addr_space::unmap (usize virt_addr)
{
	for (usize i = 0; i < phys_maps.get_len (); i ++)
	{
		struct phys_map *map = (struct phys_map *) phys_maps.get (i);
		if (map->virt_addr == virt_addr)
		{
			void *out = (void *) map->phys_addr;
			unmap_internal (map->virt_addr, map->len);
			phys_maps.remove (i);
			mem::kfree (map, sizeof (struct phys_map));
			return out;
		}
	}

	return NULL;
}

void *mem::addr_space::map_alloc_data (struct virt_allocation *allocation)
{
	usize virt_addr = find_address (phys_maps, *allocation);
	usize virt_addr_copy = virt_addr;

	if (virt_addr == 0)
	{
		return NULL;
	}

	// i is needed in error routing
	usize i = 0;
	for (; i < allocation->allocations.get_len (); i ++)
	{
		struct phys_allocation *zone = allocation->allocations.get (i);

		// this can fail if there is not enough physical memory for page tables
		if (!map_internal (zone->addr, virt_addr_copy, zone->len))
		{
			// need to unmap all zones if fail, otherwise there could be a security vulnerablity
			// first zone won't be mapped
			if (i == 0)
				return NULL;

			usize unmap_len = 0;
			for (usize j = i - 1; j > 0; j --)
			{
				struct phys_allocation *zone2 = allocation->allocations.get (j);
				unmap_len += zone2->len;
			}
			unmap_internal (virt_addr, unmap_len);
			return NULL;
		}

		virt_addr_copy += zone->len;
	}

	allocation->virt_addr = (usize) virt_addr;

	return (void *) virt_addr;
}

bool mem::addr_space::map_internal (usize phys_addr, usize virt_addr, usize n)
{
	n = align_up (n, PAGE_SIZE) / PAGE_SIZE;
	bool status = map_internal_recurse (phys_addr, virt_addr, n, n, MAX_PAGE_LEVEL, (usize *) pml4_table);
	if (!status)
	{
		unmap_internal (virt_addr, n);
	}
	return true;
}

bool mem::addr_space::map_internal_recurse (usize phys_addr, usize virt_addr, usize start_page_n, usize &page_n, usize page_level, usize *page_table)
{
	#ifdef x64
	u16 i = get_bits (virt_addr, 39, 47);
	virt_addr <<= 9;
	#endif

	while (page_n)
	{
		if (page_level)
		{
			usize page_table_p;
			if (!(page_table[i] & PAGE_PRESENT))
			{
				usize temp = (usize) mem::allocz (PAGE_SIZE);
				if (temp == 0)
					return false;

				page_table_p = page_set (page_table, i, temp);
			}
			else
			{
				page_table_p = canonical_addr (page_table[i] & PAGE_ADDR_POS);
			}

			bool temp_b = map_internal_recurse (phys_addr, virt_addr, start_page_n, page_n, page_level - 1, (usize *) page_table_p);
			if (!temp_b)
				return false;
		}
		else
		{
			page_set (page_table, i, phys_addr + PAGE_SIZE * (start_page_n - page_n));
			page_n --;
		}
		page_counter_add (page_table[0], 1);
		i ++;
	}

	return true;
}

void mem::addr_space::unmap_internal (usize virt_addr, usize n)
{
}

void mem::addr_space::unmap_internal_recurse (usize virt_addr, usize n)
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
	allocations.~array_list ();
}
