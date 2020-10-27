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
			delete allocation;
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
		delete allocation;
		return NULL;
	}

	return out;
}

void *mem::addr_space::realloc (void *mem, usize n)
{
	for (usize i = 0; i < virt_allocs.get_len (); i ++)
	{
		struct virt_allocation *allocation = (struct virt_allocation *) virt_allocs.get (i);
		if (allocation->virt_addr == (usize) mem && allocation->type == alloc_type::valloc)
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
		if (allocation->virt_addr == (usize) mem && allocation->type == alloc_type::valloc)
		{
			// unmap virtual memory
			for (usize j = 0; j < allocation->allocations.get_len (); j ++)
			{
				struct phys_allocation *temp = allocation->allocations.get (j);
				unmap_internal (temp->addr, temp->len);
			}

			virt_allocs.remove (i);
			delete allocation;
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
	usize virt_addr = find_address (virt_allocs, *zone);

	if (!virt_addr)
	{
		// don't need to remove from list
		mem::kfree (zone, sizeof (struct phys_map));
		return NULL;
	}

	bool flag = map_internal (phys_addr, virt_addr, zone->len);

	if (!flag)
	{
		virt_allocs.remove_p (zone);
		mem::kfree (zone, sizeof (struct phys_map));
		return NULL;
	}

	return (void *) virt_addr;
}

bool mem::addr_space::map_at (usize phys_addr, usize virt_addr, usize n)
{
	// shouldn't be needed
	//if (get_free_space (virt_addr) < n)
	//	return false;

	phys_map *zone = new phys_map;

	if (!zone)
		return false;

	zone->phys_addr = phys_addr;
	zone->len = n;
	zone->virt_addr = virt_addr;

	if (!insert_virt_zone (*zone))
	{
		delete zone;
		return false;
	}

	// i think map internal takes in bytes
	bool flag = map_internal (phys_addr, virt_addr, n);

	if (!flag)
	{
		virt_allocs.remove_p (zone);
		delete zone;
		return false;
	}

	return true;
}

void *mem::addr_space::unmap (usize virt_addr)
{
	for (usize i = 0; i < virt_allocs.get_len (); i ++)
	{
		struct phys_map *map = (struct phys_map *) virt_allocs.get (i);
		if (map->virt_addr == virt_addr && map->type == alloc_type::map)
		{
			void *out = (void *) map->phys_addr;
			unmap_internal (map->virt_addr, map->len);
			virt_allocs.remove (i);
			mem::kfree (map, sizeof (struct phys_map));
			return out;
		}
	}

	return NULL;
}

void *mem::addr_space::reserve (usize n)
{
	struct phys_reserve *zone = new struct phys_reserve;

	if (!zone)
		return NULL;

	zone->len = n;
	usize virt_addr = find_address (virt_allocs, *zone);

	if (!virt_addr)
	{
		// don't need to remove from list
		delete zone;
		return NULL;
	}

	return (void *) virt_addr;
}

bool mem::addr_space::reserve_at (usize virt_addr, usize n)
{
	// shouldn't be needed
	//if (get_free_space (virt_addr) < n)
	//	return false;

	phys_reserve *zone = new phys_reserve;

	if (!zone)
		return false;

	zone->virt_addr = virt_addr;
	zone->len = n;

	if (!insert_virt_zone (*zone))
	{
		delete zone;
		return false;
	}

	return true;
}

void mem::addr_space::unreserve (usize virt_addr)
{
	for (usize i = 0; i < virt_allocs.get_len (); i ++)
	{
		struct phys_reserve *map = (struct phys_reserve *) virt_allocs.get (i);
		if (map->virt_addr == virt_addr && map->type == alloc_type::reserve)
		{
			virt_allocs.remove (i);
			delete map;
			return;
		}
	}
}

void *mem::addr_space::map_alloc_data (struct virt_allocation *allocation)
{
	usize virt_addr = find_address (virt_allocs, *allocation);
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

void *mem::addr_space::translate (usize virt_addr)
{
	usize *pml4_t = (usize *) canonical_addr ((pml4_table & PAGE_ADDR_POS) + kernel_vma);
	usize pml4_i = get_bits (virt_addr, 39, 47);
	if (!(pml4_t[pml4_i] & PAGE_PRESENT))
		return NULL;

	usize *pdp_t = (usize *) canonical_addr ((pml4_t[pml4_i] & PAGE_ENTRIES) + kernel_vma);
	usize pdp_i = get_bits (virt_addr, 30, 38);
	if (!(pdp_t[pdp_i] & PAGE_PRESENT))
		return NULL;

	usize *pd_t = (usize *) canonical_addr ((pdp_t[pdp_i] & PAGE_ENTRIES) + kernel_vma);
	usize pd_i = get_bits (virt_addr, 21, 29);
	if (!(pd_t[pd_i] & PAGE_PRESENT))
		return NULL;

	usize *page_t = (usize *) canonical_addr ((pd_t[pd_i] & PAGE_ENTRIES) + kernel_vma);
	usize page_i = get_bits (virt_addr, 12, 20);
	if (!(page_t[page_i] & PAGE_PRESENT))
		return NULL;

	return (void *) canonical_addr ((page_t[page_i] & PAGE_ENTRIES) + kernel_vma);
}

bool mem::addr_space::map_internal (usize phys_addr, usize virt_addr, usize n)
{
	n = align_up (n, PAGE_SIZE) / PAGE_SIZE;
	virt_addr &= PAGE_ADDR_POS;
	bool status = map_internal_recurse (phys_addr, virt_addr, n, n, PAGE_LEVELS - 1, (usize *) pml4_table);
	if (!status)
	{
		unmap_internal (virt_addr, n);
		return false;
	}
	return true;
}

bool mem::addr_space::map_internal_recurse (usize phys_addr, usize &virt_addr, usize start_page_n, usize &page_n, usize page_level, usize *page_table)
{
	// wrong
	#ifdef x64
	u16 i = get_bits (virt_addr, 39, 47);
	virt_addr <<= 9;
	#endif

	while (page_n)
	{
		// length was too long, or end of current page table
		if (i >= PAGE_ENTRIES)
			return page_level < PAGE_LEVELS - 1;

		if (page_level)
		{
			usize page_table_p;
			if (!(page_table[i] & PAGE_PRESENT))
			{
				usize temp = (usize) mem::allocz (PAGE_SIZE);
				if (temp == 0)
					return false;

				page_table_p = page_set (page_table, i, temp);
				page_counter_add (page_table[0], 1);
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
			if (!(page_table[i] & PAGE_PRESENT))
				page_counter_add (page_table[0], 1);

			page_set (page_table, i, phys_addr + PAGE_SIZE * (start_page_n - page_n));
			page_n --;
		}
		i ++;
	}

	return true;
}

void mem::addr_space::unmap_internal (usize virt_addr, usize n)
{
	n = align_up (n, PAGE_SIZE) / PAGE_SIZE;
	virt_addr &= PAGE_ADDR_POS;
	unmap_internal_recurse (virt_addr, n, PAGE_LEVELS - 1, (usize *) canonical_addr (pml4_table & PAGE_ADDR_POS));
}

bool mem::addr_space::unmap_internal_recurse (usize &virt_addr, usize &page_n, usize page_level, usize *page_table)
{
	#ifdef x64
	u16 i = get_bits (virt_addr, 39, 47);
	virt_addr <<= 9;
	#endif

	while (page_n)
	{
		// length was too long, or end of current page table
		if (i >= PAGE_ENTRIES)
			return false;

		if (page_level)
		{
			if (!(page_table[i] & PAGE_PRESENT))
			{
				// adjust accounting info to be correct
				usize page_n_sub = 1 << (PAGE_TABLE_BITS * page_level);
				// avoid integer undeflow
				if (page_n_sub >= page_n)
					page_n = 0;
				else
					page_n -= page_n_sub;

				virt_addr <<= PAGE_TABLE_BITS * page_level;
			}
			else if (unmap_internal_recurse (virt_addr, page_n, page_level - 1, (usize *) page_table[i]))
			{
				mem::free ((void *) (canonical_addr (page_table[i] & PAGE_ADDR_POS)));
				page_counter_add (page_table[0], -1);
			}
		}
		else
		{
			if (page_table[i] & PAGE_PRESENT)
				page_counter_add (page_table[0], -1);

			page_set (page_table, i, 0);
			page_n --;
		}

		// free page this function call was examaning
		if (page_counter_get (page_table[i]) == 0)
		{
			usize page_n_sub = PAGE_ENTRIES - i - 1;
			// avoid integer underflow
			if (page_n_sub >= page_n)
				page_n = 0;
			else
				page_n -= page_n_sub;

			return true;
		}

		i ++;
	}

	return false;
}

bool mem::addr_space::insert_virt_zone (virt_zone &allocation)
{
	virt_zone *last = NULL;
	virt_zone *current = NULL;

	for (usize i = 0; i < virt_allocs.get_len (); i ++)
	{
		last = current;
		current = (virt_zone *) virt_allocs.get (i);

		if (allocation.virt_addr >= current->virt_addr && allocation.virt_addr < current->virt_addr + current->len)
			return false;

		if ((last == NULL || allocation.virt_addr >= last->virt_addr + last->len) && allocation.virt_addr + allocation.len < current->virt_addr)
		{
			// shouldn't fail
			virt_allocs.insert (i, &allocation);
			return true;
		}
	}
	if (allocation.virt_addr >= current->virt_addr + current->len)
	{
		virt_allocs.append (&allocation);
		return true;
	}
	return false;
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

usize mem::addr_space::get_free_space (usize virt_addr)
{
	virt_zone *last = NULL;
	virt_zone *current = NULL;

	for (usize i = 0; i < virt_allocs.get_len (); i ++)
	{
		last = current;
		current = (virt_zone *) virt_allocs.get (i);

		if (virt_addr >= current->virt_addr && virt_addr < current->virt_addr + current->len)
			return 0;

		if ((last == NULL || virt_addr >= last->virt_addr + last->len) && virt_addr < current->virt_addr)
			return current->virt_addr - virt_addr;
	}
	return virt_addr >= last->virt_addr + last->len ? MAX_SUPPORTED_MEM - virt_addr : 0;
}

// returns address to page it was set to
usize mem::addr_space::page_set (usize *page_table, u16 i, usize n)
{
	if (n == 0)
	{
		if (i == 0)
		{
			// have to do something special because of page counter in index 0
			usize c_value = page_table[i] & ~PAGE_DATA_POS;
			n = n & PAGE_DATA_POS;
			page_table[i] = (n | c_value) & ~PAGE_PRESENT;
		}
		else
		{
			page_table[i] = n & ~PAGE_PRESENT;
		}
		return 0;
	}
	else
	{
		if (i == 0)
		{
			// have to do something special because of page counter in index 0
			usize c_value = page_table[i] & ~PAGE_DATA_POS;
			n = n & PAGE_DATA_POS;
			page_table[i] = n | c_value | PAGE_PRESENT;
		}
		else
		{
			page_table[i] = n | PAGE_PRESENT;
		}
	}
	return canonical_addr (n & PAGE_ADDR_POS);
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
