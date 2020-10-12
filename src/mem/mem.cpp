#include <mem/mem.hpp>
#include <types.hpp>
#include <mem/pmem.hpp>
#include <mb2.hpp>
#include <util/string.hpp>
#include <util/misc.hpp>
#include <util/io.hpp>


struct __attribute__ ((packed)) mmap_entry
{
	u64 addr;
	u64 len;
	u32 type;
	u32 resvd;
};


mem::pallocator allocators[MAX_MEM_NODES];
usize allocator_len = 0;

usize text_start;
usize text_end;
usize rodata_start;
usize rodata_end;
usize data_start;
usize data_end;
usize bss_start;
usize bss_end;
usize kernel_end;
usize kernel_vma;
usize kernel_lma;


u8 init_allocator (mem::pallocator *allocer, struct mmap_entry *entry);


void *mem::init (void *mb2_table)
{
	text_start = (usize) &TEXT_START;
	kernel_end = (usize) &KERNEL_END;
	kernel_vma = (usize) &KERNEL_VMA;

	// move mb2 table to be in a safe place
	mb2_table = memcpy ((void *) align_up (kernel_end, 8), mb2_table, *((u32 *) mb2_table));
	kernel_end += *((u32 *) mb2_table);

	// read info from mb2 table
	void *mmap_header = mb2::get (mb2_table, MB2_MEM_MAP);
	if (mmap_header == NULL)
		panic ("memory map not found");

	usize len = (((u32 *) mmap_header)[1] - 16) / 24;
	struct mmap_entry *mmap = (struct mmap_entry *) (((u8 *) mmap_header) + 16);

	for (usize i = 0; i < len && allocator_len < MAX_MEM_NODES; i ++)
	{
		// if not regular memory
		if (mmap[i].type != 1 || mmap[i].len < BUDDY_ALLOC_MIN_SIZE)
			continue;

		mmap[i].addr += kernel_vma;

		allocator_len += init_allocator (allocators + allocator_len, mmap + i);

		if (allocator_len > MAX_MEM_NODES)
			panic ("incorrect build parameter for MAX_MEM_NODES");
	}

	return mb2_table;
}

u8 init_allocator (mem::pallocator *allocer, struct mmap_entry *entry)
{
	u8 out = 0;

	bool f = true;
	if (text_start >= entry->addr && text_start < entry->addr + entry->len)
	{
		if (text_start - entry->addr >= BUDDY_ALLOC_MIN_SIZE)
		{
			out ++;
			allocer->init (entry->addr, text_start - 1, PAGE_SIZE);
		}
		f = false;
	}

	if (kernel_end >= entry->addr && kernel_end < entry->addr + entry->len)
	{
		if ((entry->addr + entry->len - 1) - kernel_end >= BUDDY_ALLOC_MIN_SIZE)
		{
			out ++;
			allocer->init (kernel_end + 1, entry->addr + entry->len - 1, PAGE_SIZE);
		}
		f = false;
	}

	if (f)
	{
		allocer->init (entry->addr, entry->addr + entry->len - 1, PAGE_SIZE);
		out = 1;
	}

	return out;
}

void *mem::alloc (usize n)
{
	void *out = NULL;

	for (usize i = 0; i < allocator_len; i ++)
	{
		out = allocators[i].alloc (n);
		if (out != NULL)
			return out;
	}

	return out;
}

void *mem::allocz (usize n)
{
	void *out = NULL;

	for (usize i = 0; i < allocator_len; i ++)
	{
		out = allocators[i].allocz (n);
		if (out != NULL)
			return out;
	}

	return out;
}

void *mem::oalloc (u8 n)
{
	void *out = NULL;

	for (usize i = 0; i < allocator_len; i ++)
	{
		out = allocators[i].oalloc (n);
		if (out != NULL)
			return out;
	}

	return out;
}

void *mem::oallocz (u8 n)
{
	void *out = NULL;

	for (usize i = 0; i < allocator_len; i ++)
	{
		out = allocators[i].oallocz (n);
		if (out != NULL)
			return out;
	}

	return out;
}

void *mem::realloc (void *mem, usize n)
{
	usize m = (usize) mem;

	for (usize i = 0; i < allocator_len; i ++)
	{
		if (m >= allocators[i].get_start_addr () && m < allocators[i].get_end_addr ())
		{
			return allocators[i].realloc (mem, n);
		}
	}

	return NULL;
}

void *mem::orealloc (void *mem, u8 n)
{
	usize m = (usize) mem;

	for (usize i = 0; i < allocator_len; i ++)
	{
		if (m >= allocators[i].get_start_addr () && m < allocators[i].get_end_addr ())
		{
			return allocators[i].orealloc (mem, n);
		}
	}

	return NULL;
}

void mem::free (void *mem)
{
	usize m = (usize) mem;

	for (usize i = 0; i < allocator_len; i ++)
	{
		if (m >= allocators[i].get_start_addr () && m < allocators[i].get_end_addr ())
		{
			allocators[i].free (mem);
			return;
		}
	}
}
