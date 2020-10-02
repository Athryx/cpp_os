#include <mem/pmem.hpp>
#include <types.hpp>
#include <mem/mem_def.hpp>
#include <util/misc.hpp>
#include <util/math.hpp>
#include <util/io.hpp>


pallocator::pallocator (usize start_addr, usize end_addr, usize first_order_size)
: start_addr (align_up (start_addr, first_order_size)),
end_meta (align_down (end_addr, first_order_size)),
first_order_size (max(first_order_size, PAGE_SIZE)),
fo_bits (log2 (max(first_order_size, PAGE_SIZE)))
{
	if (end_meta <= this->start_addr)
	{
		error("allocator passed invalid memory zone")
		return;
	}

	usize diff = end_meta - this->start_addr;
	this->end_addr = end_meta - align_up (8 * (diff / first_order_size), PAGE_SIZE);

	usize temp_s = this->start_addr;

	while (temp_s < this->end_addr)
	{
		usize size = min(align_of (temp_s), align_of (this->end_addr - temp_s));
		struct free_zone *zone = (struct free_zone *) temp_s;
		zone->n = size;

		u8 order_i = log2 (size) - fo_bits;
		lists[order_i].append (zone);
		if (order_i > max_order)
			max_order = order_i;
		temp_s += size;
	}
}

void *pallocator::alloc (usize n)
{
	u8 order = get_order (n);
	if (!lists[order].len)
		populate_order (order + 1);

	struct free_zone *out = lists[order].pop ();

	struct metadata *meta = get_meta ((usize) out);
	meta->alloced = 1;
	meta->order = order;

	return lists[order].pop ();
}

void pallocator::free (void *mem)
{
	usize m = (usize) mem;
	if (m < start_addr || m > end_addr)
		return;

	struct metadata *meta = get_meta (m);
	// avoid double free
	if (!meta->alloced)
		return;

	meta->alloced = 0;
	u8 order = meta->order;
	m = align_down (m, fo_bits + order);

	struct free_zone *zone = (struct free_zone *) m;
	// necessary for merge_order function
	lists[order].append (zone);

	merge_order (m, order);
}

u8 pallocator::get_order (usize n)
{
	return log2_up (n) - fo_bits;
}

bool pallocator::populate_order (u8 order)
{
	if (order > max_order)
		return false;

	if (lists[order].len || populate_order (order + 1))
	{
		// split zones
		struct free_zone *zone = lists[order].pop ();
		zone->n >>= 1;
		struct free_zone *zone2 = (struct free_zone *) (((usize) zone) ^ zone->n);
		zone2->n = zone->n;
		lists[order - 1].append (zone);
		lists[order - 1].append (zone2);
		return true;
	}

	return false;
}

void pallocator::merge_order (usize addr, u8 order)
{
	// might not be necessary
	if (order > max_order)
		return;

	usize order_s = 1 << (order + fo_bits);
	usize m = addr ^ order_s;
	// make sure buddy address is in bounds
	if (m < start_addr || m + order_s > end_addr)
		return;

	struct metadata *meta = get_meta (m);
	// make sure buddy addres is free
	if (meta->alloced)
		return;

	struct free_zone *z_first = (struct free_zone *) min(addr, m);
	struct free_zone *z_second = (struct free_zone *) max(addr, m);

	lists[order].remove_p (z_first);
	lists[order].remove_p (z_second);

	z_first->n = order_s;

	lists[order].append (z_first);

	merge_order (addr, order + 1);
}

struct metadata *pallocator::get_meta (usize addr)
{
	return (struct metadata *) ((addr - start_addr) / first_order_size);
}


order_list::order_list ()
: len(0),
start (NULL),
end (NULL),
current (NULL),
current_i (0)
{
}

bool order_list::prepend (struct free_zone *zone)
{
	len ++;

	if (len == 1)
	{
		start = zone;
		end = zone;
		return true;
	}

	start->prev = zone;
	zone->next = start;
	start = zone;
	return true;
}

bool order_list::append (struct free_zone *zone)
{
	len ++;

	if (len == 1)
	{
		start = zone;
		end = zone;
		return true;
	}

	end->next = zone;
	zone->prev = start;
	end = zone;
	return true;
}

bool order_list::insert (usize index, struct free_zone *zone)
{
	if (index > len)
		return false;

	if (index == len)
		return append (zone);

	if (index == 0)
		return prepend (zone);

	len ++;
	struct free_zone *temp = get (index);

	temp->prev->next = zone;
	zone->prev = temp->prev;
	temp->prev = zone;
	zone->next = temp;

	return true;
}

struct free_zone *order_list::pop_start ()
{
	if (len == 0)
		return NULL;

	len --;

	if (len == 0)
		return start;

	start = start->next;
	return start;
}

struct free_zone *order_list::pop ()
{
	if (len == 0)
		return NULL;

	len --;

	if (len == 0)
		return end;

	end = end->prev;
	return end;
}

struct free_zone *order_list::remove (usize index)
{
	if (index > len)
		return NULL;

	if (index == len - 1)
		return pop ();

	if (index == 0)
		return pop_start ();

	len --;
	struct free_zone *temp = get (index);

	temp->prev->next = temp->next;
	temp->next->prev = temp->prev;

	return temp;
}

void order_list::remove_p (struct free_zone *zone)
{
	if (len == 0)
		return;

	len --;

	if (len == 0)
		return;

	if (zone == start)
	{
		start = zone->next;
		return;
	}

	if (zone == end)
	{
		end = zone->prev;
		return;
	}

	zone->prev->next = zone->next;
	zone->next->prev = zone->prev;
}

struct free_zone *order_list::get (usize index)
{
	if (index > len)
		return NULL;

	isize diff_start = index;
	isize diff_end = len - index - 1;
	isize diff_middle = index - current_i;

	struct free_zone *zone;
	usize i;
	i8 dir;

	if (diff_middle < 0 && diff_start < diff_middle)
	{
		zone = start;
		i = 0;
		dir = 1;
	}
	else if (diff_end < diff_middle)
	{
		zone = end;
		i = len - 1;
		dir = -1;
	}
	else
	{
		zone = current;
		i = current_i;
		dir = sign (diff_middle);
	}

	for (; i != index; i += dir)
	{
		if (dir == 1)
			zone = zone->next;
		else
			zone = zone->prev;

		if (zone == NULL)
		{
			error("zone should not be null")
		}
	}

	current = zone;
	current_i = index;

	return zone;
}
