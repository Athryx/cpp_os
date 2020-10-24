#include <cxx.hpp>
#include <mem/kmem.hpp>
#include <util/io.hpp>


// make gcc happy
void *__dso_handle;


extern "C" void __cxa_pure_virtual (void)
{
	panic ("Dispach to pure virtual function");
}

// atexit is called for global destructors when program exits. But since kernel will never exit, this doesn't need to do anything, it just needs to exist to stop linker errors
extern "C" void __cxa_atexit (void)
{
}

void *operator new (usize n)
{
	return mem::kalloc (n);
}

void *operator new[] (usize n)
{
	return mem::kalloc (n);
}

void operator delete (void *mem, usize n)
{
	mem::kfree (mem, n);
}

void operator delete[] (void *mem, usize n)
{
	mem::kfree (mem, n);
}
