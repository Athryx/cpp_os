#include <mem/mem.hpp>
#include <types.hpp>


pmem::pallocator allocator;

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


void mem::init ()
{
	kernel_end = (usize) &KERNEL_END;
	kernel_vma = (usize) &KERNEL_VMA;

	allocator.init (kernel_end, kernel_end + HEAP_SIZE, FIRST_ORDER_SIZE);
}

void *mem::alloc (usize n)
{
	return allocator.alloc (n);
}

void mem::free (void *mem)
{
	allocator.free (mem);
}
