#ifndef __UTIL_KMEM_H__
#define __UTIL_KMEM_H__
// needs work


#include <types.hpp>


namespace mem
{
	void kmem_init (void);

	// allocates a 4 KiB pages
	void *kalloc (usize n);

	void *krealloc (void *page, usize orig, usize n);

	// doesn't quite work when ther is no free spot at end
	void kfree (void *page, usize n);
}


#endif
