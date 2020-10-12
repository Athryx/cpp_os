#include <mem/vmem.hpp>
#include <types.hpp>
#include <mem/mem.hpp>


mem::addr_space::addr_space ()
{
	pml4_table = (usize) mem::allocz (PAGE_SIZE);
}


