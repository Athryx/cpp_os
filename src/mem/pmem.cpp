#include <mem/pmem.hpp>
#include <types.hpp>
#include <mem/mem.hpp>
#include <util/misc.hpp>
#include <util/string.hpp>
#include <util/io.hpp>


using namespace mem;


pallocator::pallocator (usize start_addr, usize end_addr, usize first_order_size)
: lock (1)
{
	init (start_addr, end_addr, first_order_size);
}

void pallocator::init (usize start_addr, usize end_addr, usize first_order_size)
{
	free_space = 0;

	first_order_size = max(first_order_size, PAGE_SIZE);

	usize temp_s = align_up (start_addr, first_order_size);
	this->start_addr = temp_s;

	this->end_meta = align_down (end_addr, first_order_size);

	this->first_order_size = first_order_size;
	this->fo_bits = log2 (first_order_size);

	assertm(end_meta > this->start_addr, "allocator passed invalid memory zone");

	usize diff = end_meta - this->start_addr;
	this->end_addr = align_down (end_meta - align_up (8 * (diff / first_order_size), PAGE_SIZE), first_order_size);

	// make sure metalist has all chunks free at start
	memset ((void *) this->end_addr, this->end_meta - this->end_addr, 0);

	while (temp_s < this->end_addr)
	{
		usize size = align_of (temp_s);
		// this feels like a bad way to do it
		while (size > this->end_addr - temp_s)
			size >>= 1;

		struct free_zone *zone = (struct free_zone *) temp_s;
		zone->n = size;
		free_space += size;

		u8 order_i = log2 (size) - fo_bits;
		lists[order_i].append (zone);
		if (order_i > max_order)
			max_order = order_i;
		temp_s += size;
	}
}

void *pallocator::alloc (usize n)
{
	if (n == 0)
		return NULL;
	return oalloc (get_order (n));
}

void *pallocator::allocz (usize n)
{
	void *out = alloc (n);
	if (out == NULL)
		return NULL;

	memset (out, n, 0);
	return out;
}

void *pallocator::oalloc (u8 order)
{
	if (order > max_order)
		return NULL;

	if (!lists[order].get_len ())
		populate_order (order + 1);

	struct free_zone *out = (struct free_zone *) lists[order].pop ();

	if (out == NULL)
		return NULL;

	struct metadata *meta = get_meta ((usize) out);
	meta->alloced = 1;
	meta->order = order;
	free_space -= get_order_size (order);

	return out;
}

void *pallocator::oallocz (u8 order)
{
	void *out = oalloc (order);
	if (out == NULL)
		return NULL;

	memset (out, get_order_size (order), 0);
	return out;
}

void *pallocator::realloc (void *mem, usize n)
{
	usize m = (usize) mem;
	if (m < start_addr || m >= end_addr)
		return NULL;

	struct metadata *meta = get_meta (m);
	if (!meta->alloced)
		return NULL;

	u8 order = meta->order;
	u8 n_order = get_order (n);
	if (n_order > max_order)
		return NULL;

	usize len = get_order_size (order);
	m = align_down (m, len);

	if (n_order == order)
		return (void *) m;

	if (n_order < order)
	{
		// free extra space
		struct free_zone *zone = (struct free_zone *) m;
		lists[order].remove_p (zone);
		while (order > n_order)
		{
			zone->n >>= 1;

			struct free_zone *zone2 = (struct free_zone *) (((usize) zone) ^ zone->n);
			zone2->n = zone->n;

			order --;
			lists[order].append (zone2);

			struct metadata *meta2 = get_meta ((usize) zone2);
			meta2->alloced = 0;
			meta2->order = order;
		}
		meta->order = n_order;
		return (void *) m;
	}

	// get trailing space after allocation
	usize space = get_space (m + len);
	// align of check is due to restrictions of alloctor
	if (n <= space && align_of (m) >= get_order_size (n_order))
	{
		// allocate onto end
		usize len = get_order_size (order);
		while (order < n_order)
		{
			lists[order].remove_p ((struct free_zone *) (m ^ len));
			len <<= 1;
			order ++;
		}

		struct free_zone *zone = (struct free_zone *) m;
		zone->n = len;
		meta->order = n_order;
		return (void *) m;
	}
	else
	{
		// move memory contents to new zone
		void *out = alloc (n);
		if (out == NULL)
		{
			out = mem::alloc (n);
			if (out == NULL)
				return NULL;
		}

		memcpy (out, (void *) m, len);
		free ((void *) m);
		return out;
	}
}

void *pallocator::orealloc (void *mem, u8 order)
{
	if (order > max_order)
		return NULL;
	return realloc (mem, get_order_size (order));
}

void pallocator::free (void *mem)
{
	usize m = (usize) mem;
	if (m < start_addr || m >= end_addr)
		return;

	struct metadata *meta = get_meta (m);
	// avoid double free
	if (!meta->alloced)
		return;

	meta->alloced = 0;
	u8 order = meta->order;
	// might be wrong
	m = align_down (m, get_order_size (order));

	struct free_zone *zone = (struct free_zone *) m;

	// necessary for merge_order function
	zone->n = get_order_size (order);
	lists[order].append (zone);

	merge_order (m, order, max_order);
}

bool pallocator::populate_order (u8 order)
{
	if (order > max_order)
		return false;

	if (lists[order].get_len () || populate_order (order + 1))
	{
		// split zones
		struct free_zone *zone = (struct free_zone *) lists[order].pop ();
		zone->n >>= 1;
		struct free_zone *zone2 = (struct free_zone *) (((usize) zone) ^ zone->n);
		zone2->n = zone->n;
		lists[order - 1].append (zone);
		lists[order - 1].append (zone2);
		return true;
	}

	return false;
}

void pallocator::merge_order (usize addr, u8 order, u8 m_order)
{
	// might not be necessary
	if (order > m_order)
		return;

	usize order_s = get_order_size (order);
	usize m = addr ^ order_s;
	// make sure buddy address is in bounds
	// greater than, not greater than equal
	if (m < start_addr || m + order_s > end_addr)
		return;

	struct metadata *meta = get_meta (m);
	// make sure buddy addres is free
	if (meta->alloced)
		return;

	struct free_zone *z_first = (struct free_zone *) min(addr, m);
	struct free_zone *z_second = (struct free_zone *) max(addr, m);

	// make sure buddy is same size
	if (z_first->n != z_second->n)
		return;

	lists[order].remove_p (z_first);
	lists[order].remove_p (z_second);

	z_first->n = order_s << 1;

	lists[order + 1].append (z_first);

	merge_order ((usize) z_first, order + 1, m_order);
}

struct metadata *pallocator::get_meta (usize addr)
{
	return (struct metadata *) (((addr - start_addr) / first_order_size) + end_addr);
}

usize pallocator::get_space (usize addr)
{
	if (addr < start_addr || addr >= end_addr)
		return 0;

	usize out = 0;
	struct metadata *meta = get_meta (addr);

	while (!meta->alloced)
	{
		usize len = get_order_size (meta->order);
		out += len;
		addr += len;

		if (addr >= end_addr)
			return out;

		meta = get_meta (addr);
	}

	return out;
}
