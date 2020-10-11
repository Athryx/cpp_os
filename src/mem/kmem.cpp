#include <mem/kmem.hpp>
#include <types.hpp>
#include <mem/mem.hpp>
#include <mem/mem_def.hpp>
#include <util/string.hpp>
#include <util/misc.hpp>


#define MEM_CHUNK_SIZE sizeof (struct kmem_chunk)
// amount of pagesd to start with
#define INITIAL_PAGE_COUNT ((usize) 8)
// amount of pages to add when more are needed
#define PAGE_REALLOC_COUNT ((usize) 8)


static struct kmem_chunk
{
	struct kmem_chunk *next;
	usize size;
} mem_start;

//usize page_c = INITIAL_PAGE_COUNT;


void mem::kmem_init (void);
static void *grow_heap (usize page_count);
void *mem::kalloc (usize n);
static void *kmem_resize_move (void *addr, usize orig, usize n);
void *mem::krealloc (void *addr, usize orig, usize n);
void mem::kfree (void *addr, usize n);


void mem::kmem_init (void)
{
	grow_heap (INITIAL_PAGE_COUNT);
}

static void *grow_heap (usize page_count)
{
	struct kmem_chunk *head = (struct kmem_chunk *) mem::alloc (page_count);
	mem_start.next = head;
	mem_start.size = 0;
	head->next = NULL;
	head->size = page_count * PAGE_SIZE;
	return head;
}

void *mem::kalloc (usize n)
{
	if (n == 0)
		return NULL;

	n = align_up (n, MEM_CHUNK_SIZE);

	void *out = NULL;
	struct kmem_chunk *prev = &mem_start;
	struct kmem_chunk *i = mem_start.next;

	for (;;)
	{
		if (i->size >= n)
		{
			out = i;
			i->size -= n;
			if (i->size == 0)
			{
				// allocate more memory
				if (prev == &mem_start && i->next == NULL)
				{
					grow_heap (PAGE_REALLOC_COUNT);
				}
				else
				{
					prev->next = i->next;
				}
			}
			else
			{
				struct kmem_chunk *new_addr = (struct kmem_chunk *) ((u8 *) i + n);
				memcpy (new_addr, i, sizeof (struct kmem_chunk));
				prev->next = new_addr;
			}
			memset (out, n, 0);
			return out;
		}
		prev = i;
		i = i->next;

		// none of the free sections are big enough
		if (i == NULL)
		{
			return grow_heap (max(PAGE_REALLOC_COUNT, n));
		}
	}
}

// if free fails, will still allocate, so need to call correctly
static void *kmem_resize_move (void *addr, usize orig, usize n)
{
	// we have to allocate new zone and move memory contents
	void *out = mem::kalloc (n);
	if (out == NULL)
		return NULL;

	memcpy (out, addr, orig);
	mem::kfree (addr, orig);
	return out;
}

void *mem::krealloc (void *addr, usize orig, usize n)
{
	// not sure why thes maxes are needed but they are in the original code so i'll keep them
	orig = align_up (orig, MEM_CHUNK_SIZE);
	orig = max(orig, MEM_CHUNK_SIZE);

	n = align_up (n, MEM_CHUNK_SIZE);
	n = max(n, MEM_CHUNK_SIZE);

	// do nothing if same
	if (orig == n)
		return addr;

	// if new is less free extra space, and return addr, or null if all is freed
	if (orig > n)
	{
		void *temp = ((u8 *) addr) + n;
		usize size = orig - n;
		kfree (temp, size);

		if (n == 0)
			return NULL;
		return addr;
	}

	// if there is no space return null
	/*if ((usize) mem_start.next >= HEAP_END)
		return NULL;*/

	usize addr_ptr = align_down ((usize) addr, MEM_CHUNK_SIZE);
	struct kmem_chunk *prev = &mem_start;
	struct kmem_chunk *i = mem_start.next;

	// amount to be allocated
	usize diff = n - orig;

	for (;;)
	{
		if ((usize) i > addr_ptr)
		{
			// we can just expand memory chunk to be bigger if true
			if (i->size >= diff && (usize) i == addr_ptr + orig)
			{
				i->size -= diff;
				struct kmem_chunk *new_addr = (struct kmem_chunk *) ((u8 *) i + diff);
				memcpy (new_addr, i, sizeof (struct kmem_chunk));
				prev->next = new_addr;
			}
			else
			{
				return kmem_resize_move (addr, orig, n);
			}
		}

		prev = i;
		i = i->next;

		if (i == NULL)
		{
			return kmem_resize_move (addr, orig, n);
		}
	}
}

void mem::kfree (void *addr, usize n)
{
	if (n == 0)
		return;

	n = align_up (n, MEM_CHUNK_SIZE);

	usize addr_ptr = align_down ((usize) addr, MEM_CHUNK_SIZE);
	struct kmem_chunk *new_addr = (struct kmem_chunk *) addr_ptr;

	// if everything is full
	/*if ((usize) mem_start.next == HEAP_END)
	{
		mem_start.next = new_addr;
		new_addr->next = NULL;
		new_addr->size = n;
		return;
	}*/

	struct kmem_chunk *prev = &mem_start;
	struct kmem_chunk *i = mem_start.next;

	for (;;)
	{
		if ((usize) i > addr_ptr)
		{
			// make sure memory is allocated
			if ((usize) prev->next + prev->size > addr_ptr || addr_ptr + n > (usize) i)
				return;

			// join consecutive memory chunks
			if (addr_ptr + n == (usize) i)
			{
				new_addr->next = i->next;
				new_addr->size = i->size + n;
			}
			else
			{
				new_addr->next = i;
				new_addr->size = n;
			}

			if ((usize) prev + prev->size == addr_ptr)
			{
				prev->next = new_addr->next;
				prev->size += new_addr->size;
			}
			else
			{
				prev->next = new_addr;
			}
			return;
		}
		prev = i;
		i = i->next;

		// there is no next, this is end
		if (i == NULL)
		{
			prev->next = new_addr;
			new_addr->next = NULL;
			new_addr->size = n;
			return;
		}
	}
}
